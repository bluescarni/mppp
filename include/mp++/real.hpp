// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_REAL_HPP
#define MPPP_REAL_HPP

#include <mp++/config.hpp>

#if defined(MPPP_WITH_MPFR)

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include <mp++/concepts.hpp>
#include <mp++/detail/fwd_decl.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#if defined(MPPP_WITH_QUADMATH)
#include <mp++/real128.hpp>
#endif

namespace mppp
{

inline namespace detail
{

// Some misc tests to check that the mpfr struct conforms to our expectations.
struct expected_mpfr_struct_t {
    ::mpfr_prec_t _mpfr_prec;
    ::mpfr_sign_t _mpfr_sign;
    ::mpfr_exp_t _mpfr_exp;
    ::mp_limb_t *_mpfr_d;
};

static_assert(sizeof(expected_mpfr_struct_t) == sizeof(mpfr_struct_t) && offsetof(mpfr_struct_t, _mpfr_prec) == 0u
                  && offsetof(mpfr_struct_t, _mpfr_sign) == offsetof(expected_mpfr_struct_t, _mpfr_sign)
                  && offsetof(mpfr_struct_t, _mpfr_exp) == offsetof(expected_mpfr_struct_t, _mpfr_exp)
                  && offsetof(mpfr_struct_t, _mpfr_d) == offsetof(expected_mpfr_struct_t, _mpfr_d)
                  && std::is_same<::mp_limb_t *, decltype(std::declval<mpfr_struct_t>()._mpfr_d)>::value,
              "Invalid mpfr_t struct layout and/or MPFR types.");

#if MPPP_CPLUSPLUS >= 201703L

// If we have C++17, we can use structured bindings to test the layout of mpfr_struct_t
// and its members' types.
constexpr void test_mpfr_struct_t()
{
    auto[prec, sign, exp, ptr] = mpfr_struct_t{};
    static_assert(std::is_same<decltype(ptr), ::mp_limb_t *>::value);
    (void)prec;
    (void)sign;
    (void)exp;
    (void)ptr;
}

#endif

// Clamp the MPFR precision between the min and max allowed values. This is used in the generic constructor.
constexpr ::mpfr_prec_t clamp_mpfr_prec(::mpfr_prec_t p)
{
    return mpfr_prec_check(p) ? p : (p < mpfr_prec_min() ? mpfr_prec_min() : mpfr_prec_max());
}

// Utility function to determine the number of base-2 digits of the significand
// of a floating-point type which is not in base-2.
template <typename T>
inline ::mpfr_prec_t dig2mpfr_prec()
{
    static_assert(std::is_floating_point<T>::value, "Invalid type.");
    // NOTE: just do a raw cast for the time being, it's not like we have ways of testing
    // this in any case. In the future we could consider switching to a compile-time implementation
    // of the integral log2, and do everything as compile-time integral computations.
    return static_cast<::mpfr_prec_t>(
        std::ceil(std::numeric_limits<T>::digits * std::log2(std::numeric_limits<T>::radix)));
}

// Helper function to print an mpfr to stream in base 10.
inline void mpfr_to_stream(const ::mpfr_t r, std::ostream &os)
{
    // Special values first.
    if (mpfr_nan_p(r)) {
        os << "nan";
        return;
    }
    if (mpfr_inf_p(r)) {
        os << (mpfr_sgn(r) < 0 ? "-inf" : "inf");
        return;
    }

    // Get the string fractional representation via the MPFR function,
    // and wrap it into a smart pointer.
    ::mpfr_exp_t exp(0);
    smart_mpfr_str str(::mpfr_get_str(nullptr, &exp, 10, 0, r, MPFR_RNDN), ::mpfr_free_str);
    if (mppp_unlikely(!str)) {
        throw std::runtime_error("Error in the conversion of a real to string: the call to mpfr_get_str() failed");
    }

    // Print the string, inserting a decimal point after the first digit.
    bool dot_added = false;
    for (auto cptr = str.get(); *cptr != '\0'; ++cptr) {
        os << (*cptr);
        // NOTE: check this answer:
        // http://stackoverflow.com/questions/13827180/char-ascii-relation
        // """
        // The mapping of integer values for characters does have one guarantee given
        // by the Standard: the values of the decimal digits are contiguous.
        // (i.e., '1' - '0' == 1, ... '9' - '0' == 9)
        // """
        if (!dot_added && *cptr >= '0' && *cptr <= '9') {
            os << '.';
            dot_added = true;
        }
    }
    assert(dot_added);

    // Adjust the exponent. Do it in multiprec in order to avoid potential overflow.
    integer<1> z_exp{exp};
    --z_exp;
    const auto exp_sgn = z_exp.sgn();
    if (exp_sgn && !mpfr_zero_p(r)) {
        // Add the exponent at the end of the string, if both the value and the exponent
        // are nonzero.
        os << 'e';
        if (exp_sgn == 1) {
            // Add extra '+' if the exponent is positive, for consistency with
            // real128's string format (and possibly other formats too?).
            os << '+';
        }
        os << z_exp;
    }
}

template <typename = void>
struct real_constants {
    // A bare real with static memory allocation, represented as
    // an mpfr_struct_t paired to storage for the limbs.
    template <::mpfr_prec_t Prec>
    using static_real
        = std::pair<mpfr_struct_t,
                    typename std::aligned_storage<mpfr_custom_get_size(Prec), alignof(::mp_limb_t)>::type>;
    // Shortcut for the size of the real_2_112 constant. We just need
    // 1 bit of precision for this, but make sure we don't go below
    // the minimum allowed precision.
    static const ::mpfr_prec_t size_2_112 = c_max(::mpfr_prec_t(1), mpfr_prec_min());
    // Create a static real with value 2**112. This represents the "hidden bit"
    // of the significand of a quadruple-precision FP.
    static static_real<size_2_112> get_2_112()
    {
        // NOTE: pair's def ctor value-inits the members: everything in retval is zeroed out.
        static_real<size_2_112> retval;
        // Init the limbs first, as indicated by the mpfr docs.
        static_assert(size_2_112 <= mpfr_prec_max(), "Invalid precision.");
        mpfr_custom_init(&retval.second, size_2_112);
        // Do the custom init with a zero value, exponent 0 (unused), precision matching the previous call,
        // and the limbs storage pointer.
        mpfr_custom_init_set(&retval.first, MPFR_ZERO_KIND, 0, size_2_112, &retval.second);
        // Set the actual value.
        ::mpfr_set_ui_2exp(&retval.first, 1ul, static_cast<::mpfr_exp_t>(112), MPFR_RNDN);
        return retval;
    }
    // Actually instantiate the constant.
    static const static_real<size_2_112> real_2_112;
};

template <typename T>
const typename real_constants<T>::template static_real<real_constants<T>::size_2_112> real_constants<T>::real_2_112
    = real_constants<T>::get_2_112();
}

template <typename T>
using is_real_interoperable = disjunction<is_cpp_interoperable<T>, is_integer<T>, is_rational<T>
#if defined(MPPP_WITH_QUADMATH)
                                          ,
                                          std::is_same<T, real128>
#endif
                                          >;

template <typename T>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool RealInteroperable = is_real_interoperable<T>::value;
#else
using real_interoperable_enabler = enable_if_t<is_real_interoperable<T>::value, int>;
#endif

class real
{
    // Utility function to check the precision upon init.
    static ::mpfr_prec_t check_init_prec(::mpfr_prec_t p)
    {
        if (mppp_unlikely(!mpfr_prec_check(p))) {
            throw std::invalid_argument("Cannot init a real with a precision of " + std::to_string(p)
                                        + ": the maximum allowed precision is " + std::to_string(mpfr_prec_max())
                                        + ", the minimum allowed precision is " + std::to_string(mpfr_prec_min()));
        }
        return p;
    }

public:
    /// Default constructor.
    /**
     * The value will be initialised to positive zero with the minimum allowed precision.
     */
    real()
    {
        // Init with minimum precision.
        ::mpfr_init2(&m_mpfr, mpfr_prec_min());
        ::mpfr_set_zero(&m_mpfr, 1);
    }
    real(const real &other)
    {
        // Init with the same precision as other, and then set.
        ::mpfr_init2(&m_mpfr, other.get_prec());
        ::mpfr_set(&m_mpfr, &other.m_mpfr, MPFR_RNDN);
    }
    explicit real(const real &other, ::mpfr_prec_t p)
    {
        // Init with custom precision, and then set.
        ::mpfr_init2(&m_mpfr, check_init_prec(p));
        ::mpfr_set(&m_mpfr, &other.m_mpfr, MPFR_RNDN);
    }
    real(real &&other) noexcept
    {
        m_mpfr._mpfr_prec = other.m_mpfr._mpfr_prec;
        m_mpfr._mpfr_sign = other.m_mpfr._mpfr_sign;
        m_mpfr._mpfr_exp = other.m_mpfr._mpfr_exp;
        m_mpfr._mpfr_d = other.m_mpfr._mpfr_d;
        // Mark the other as moved-from.
        other.m_mpfr._mpfr_d = nullptr;
    }

private:
    // Construction from FPs.
    // Alias for the MPFR assignment functions from FP types.
    template <typename T>
    using fp_a_ptr = int (*)(::mpfr_t, T, ::mpfr_rnd_t);
    template <typename T>
    void dispatch_fp_construction(fp_a_ptr<T> ptr, const T &x, ::mpfr_prec_t p)
    {
        static_assert(std::numeric_limits<T>::digits <= std::numeric_limits<::mpfr_prec_t>::max(), "Overflow error.");
        ::mpfr_init2(&m_mpfr, p ? check_init_prec(p)
                                : clamp_mpfr_prec(std::numeric_limits<T>::radix == 2
                                                      ? static_cast<::mpfr_prec_t>(std::numeric_limits<T>::digits)
                                                      : dig2mpfr_prec<T>()));
        ptr(&m_mpfr, x, MPFR_RNDN);
    }
    void dispatch_construction(const float &x, ::mpfr_prec_t p)
    {
        dispatch_fp_construction(::mpfr_set_flt, x, p);
    }
    void dispatch_construction(const double &x, ::mpfr_prec_t p)
    {
        dispatch_fp_construction(::mpfr_set_d, x, p);
    }
    void dispatch_construction(const long double &x, ::mpfr_prec_t p)
    {
        dispatch_fp_construction(::mpfr_set_ld, x, p);
    }
    // Construction from integral types.
    template <typename T>
    void dispatch_integral_init(::mpfr_prec_t p)
    {
        static_assert(std::numeric_limits<T>::digits <= std::numeric_limits<::mpfr_prec_t>::max(), "Overflow error.");
        ::mpfr_init2(&m_mpfr, p ? check_init_prec(p)
                                : clamp_mpfr_prec(static_cast<::mpfr_prec_t>(std::numeric_limits<T>::digits)));
    }
    // Special casing for bool, otherwise MSVC warns if we fold this into the
    // constructor from unsigned.
    void dispatch_construction(const bool &b, ::mpfr_prec_t p)
    {
        dispatch_integral_init<bool>(p);
        ::mpfr_set_ui(&m_mpfr, static_cast<unsigned long>(b), MPFR_RNDN);
    }
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_unsigned<T>>::value, int> = 0>
    void dispatch_construction(const T &n, ::mpfr_prec_t p)
    {
        dispatch_integral_init<T>(p);
        if (n <= std::numeric_limits<unsigned long>::max()) {
            ::mpfr_set_ui(&m_mpfr, static_cast<unsigned long>(n), MPFR_RNDN);
        } else {
            ::mpfr_set_z(&m_mpfr, integer<1>(n).get_mpz_view(), MPFR_RNDN);
        }
    }
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_signed<T>>::value, int> = 0>
    void dispatch_construction(const T &n, ::mpfr_prec_t p)
    {
        dispatch_integral_init<T>(p);
        if (n <= std::numeric_limits<long>::max() && n >= std::numeric_limits<long>::min()) {
            ::mpfr_set_si(&m_mpfr, static_cast<long>(n), MPFR_RNDN);
        } else {
            ::mpfr_set_z(&m_mpfr, integer<1>(n).get_mpz_view(), MPFR_RNDN);
        }
    }
    template <std::size_t SSize>
    void dispatch_construction(const integer<SSize> &n, ::mpfr_prec_t p)
    {
        ::mpfr_prec_t prec;
        if (p) {
            prec = check_init_prec(p);
        } else {
            // Infer the precision from the bit size of n.
            const auto ls = n.size();
            // Check that ls * GMP_NUMB_BITS <= max_prec.
            if (mppp_unlikely(ls > static_cast<std::make_unsigned<::mpfr_prec_t>::type>(mpfr_prec_max())
                                       / unsigned(GMP_NUMB_BITS))) {
                throw std::invalid_argument(
                    "The deduced precision for a real constructed from an integer is too large");
            }
            // Compute the precision. We already know it's a non-negative value not greater
            // than the max allowed precision. We just need to make sure it's not smaller than the
            // min allowed precision.
            prec = c_max(static_cast<::mpfr_prec_t>(static_cast<::mpfr_prec_t>(ls) * int(GMP_NUMB_BITS)),
                         mpfr_prec_min());
        }
        ::mpfr_init2(&m_mpfr, prec);
        ::mpfr_set_z(&m_mpfr, n.get_mpz_view(), MPFR_RNDN);
    }
    template <std::size_t SSize>
    void dispatch_construction(const rational<SSize> &q, ::mpfr_prec_t p)
    {
        ::mpfr_prec_t prec;
        if (p) {
            prec = check_init_prec(p);
        } else {
            // Infer the precision from the bit size of num/den.
            const auto n_size = q.get_num().size();
            const auto d_size = q.get_den().size();
            // Overflow checks.
            if (mppp_unlikely(
                    // Overflow in total size.
                    (n_size > std::numeric_limits<decltype(q.get_num().size())>::max() - d_size)
                    // Check that tot_size * GMP_NUMB_BITS <= max_prec.
                    || ((n_size + d_size) > static_cast<std::make_unsigned<::mpfr_prec_t>::type>(mpfr_prec_max())
                                                / unsigned(GMP_NUMB_BITS)))) {
                throw std::invalid_argument(
                    "The deduced precision for a real constructed from a rational is too large");
            }
            // Compute the precision. We already know it's a non-negative value not greater
            // than the max allowed precision. We just need to make sure it's not smaller than the
            // min allowed precision.
            prec = c_max(static_cast<::mpfr_prec_t>(static_cast<::mpfr_prec_t>(n_size + d_size) * int(GMP_NUMB_BITS)),
                         mpfr_prec_min());
        }
        ::mpfr_init2(&m_mpfr, prec);
        ::mpfr_set_q(&m_mpfr, q.get_mpq_view(), MPFR_RNDN);
    }
#if defined(MPPP_WITH_QUADMATH)
    void dispatch_construction(const real128 &x, ::mpfr_prec_t p)
    {
        // Get the IEEE repr. of x.
        const auto t = x.get_ieee();
        // A utility function to write the significand of x
        // as a big integer inside m_mpfr.
        auto write_significand = [this, &t]() {
            // The 4 32-bits part of the significand, from most to least
            // significant digits.
            const auto p1 = std::get<2>(t) >> 32;
            const auto p2 = std::get<2>(t) % (1ull << 32);
            const auto p3 = std::get<3>(t) >> 32;
            const auto p4 = std::get<3>(t) % (1ull << 32);
            // Build the significand, from most to least significant.
            // NOTE: unsigned long is guaranteed to be at least 32 bit.
            ::mpfr_set_ui(&this->m_mpfr, static_cast<unsigned long>(p1), MPFR_RNDN);
            ::mpfr_mul_2ui(&this->m_mpfr, &this->m_mpfr, 32ul, MPFR_RNDN);
            ::mpfr_add_ui(&this->m_mpfr, &this->m_mpfr, static_cast<unsigned long>(p2), MPFR_RNDN);
            ::mpfr_mul_2ui(&this->m_mpfr, &this->m_mpfr, 32ul, MPFR_RNDN);
            ::mpfr_add_ui(&this->m_mpfr, &this->m_mpfr, static_cast<unsigned long>(p3), MPFR_RNDN);
            ::mpfr_mul_2ui(&this->m_mpfr, &this->m_mpfr, 32ul, MPFR_RNDN);
            ::mpfr_add_ui(&this->m_mpfr, &this->m_mpfr, static_cast<unsigned long>(p4), MPFR_RNDN);
        };
        // Check if the significand is zero.
        const bool sig_zero = !std::get<2>(t) && !std::get<3>(t);
        // Init the value.
        // The significand precision in bits is 113 for real128. Let's double-check it.
        static_assert(real128_sig_digits() == 113u, "Invalid number of digits.");
        ::mpfr_init2(&m_mpfr, p ? check_init_prec(p) : clamp_mpfr_prec(113));
        if (std::get<1>(t) == 0u) {
            // Zero or subnormal numbers.
            if (sig_zero) {
                // Zero.
                ::mpfr_set_zero(&m_mpfr, 1);
            } else {
                // Subnormal.
                write_significand();
                ::mpfr_div_2ui(&m_mpfr, &m_mpfr, 16382ul + 112ul, MPFR_RNDN);
            }
        } else if (std::get<1>(t) == 32767u) {
            // NaN or inf.
            if (sig_zero) {
                // inf.
                ::mpfr_set_inf(&m_mpfr, 1);
            } else {
                // NaN.
                ::mpfr_set_nan(&m_mpfr);
            }
        } else {
            // Write the significand into this.
            write_significand();
            // Add the hidden bit on top.
            ::mpfr_add(&m_mpfr, &m_mpfr, &real_constants<>::real_2_112.first, MPFR_RNDN);
            // Multiply by 2 raised to the adjusted exponent.
            ::mpfr_mul_2si(&m_mpfr, &m_mpfr, static_cast<long>(std::get<1>(t)) - (16383l + 112), MPFR_RNDN);
        }
        if (std::get<0>(t)) {
            // Negate if the sign bit is set.
            ::mpfr_neg(&m_mpfr, &m_mpfr, MPFR_RNDN);
        }
    }
#endif

public:
#if defined(MPPP_HAVE_CONCEPTS)
    explicit real(const RealInteroperable &x,
#else
    template <typename T, real_interoperable_enabler<T> = 0>
    explicit real(const T &x,
#endif
                  ::mpfr_prec_t p = 0)
    {
        dispatch_construction(x, p);
    }
    ~real()
    {
        if (m_mpfr._mpfr_d) {
            // The object is not moved-from, destroy it.
            ::mpfr_clear(&m_mpfr);
        }
    }
    real &operator=(const real &other)
    {
        if (mppp_likely(this != &other)) {
            if (m_mpfr._mpfr_d) {
                // this has not been moved-from.
                // Copy the precision. This will also reset the internal value.
                // No need for prec checking as we assume other has a valid prec.
                set_prec_impl<false>(other.get_prec());
            } else {
                // this has been moved-from: init before setting.
                ::mpfr_init2(&m_mpfr, other.get_prec());
            }
            // Perform the actual copy from other.
            ::mpfr_set(&m_mpfr, &other.m_mpfr, MPFR_RNDN);
        }
        return *this;
    }
    real &operator=(real &&other) noexcept
    {
        return swap(other);
    }
    real &swap(real &other)
    {
        ::mpfr_swap(&m_mpfr, &other.m_mpfr);
        return *this;
    }
    const mpfr_struct_t *get_mpfr_t() const
    {
        return &m_mpfr;
    }
    mpfr_struct_t *_get_mpfr_t()
    {
        return &m_mpfr;
    }
    ::mpfr_prec_t get_prec() const
    {
        return mpfr_get_prec(&m_mpfr);
    }
    bool nan_p() const
    {
        return mpfr_nan_p(&m_mpfr) != 0;
    }
    bool inf_p() const
    {
        return mpfr_inf_p(&m_mpfr) != 0;
    }
    bool number_p() const
    {
        return mpfr_number_p(&m_mpfr) != 0;
    }
    bool zero_p() const
    {
        return mpfr_zero_p(&m_mpfr) != 0;
    }
    bool regular_p() const
    {
        return mpfr_regular_p(&m_mpfr) != 0;
    }
    int sgn() const
    {
        return mpfr_sgn(&m_mpfr);
    }
    bool signbit() const
    {
        return mpfr_signbit(&m_mpfr) != 0;
    }

private:
    // Utility function to check precision in set_prec().
    static ::mpfr_prec_t check_set_prec(::mpfr_prec_t p)
    {
        if (mppp_unlikely(!mpfr_prec_check(p))) {
            throw std::invalid_argument("Cannot set the precision of a real to the value " + std::to_string(p)
                                        + ": the maximum allowed precision is " + std::to_string(mpfr_prec_max())
                                        + ", the minimum allowed precision is " + std::to_string(mpfr_prec_min()));
        }
        return p;
    }
    // mpfr_set_prec() wrapper, with or without prec checking.
    template <bool Check>
    void set_prec_impl(::mpfr_prec_t p)
    {
        ::mpfr_set_prec(&m_mpfr, Check ? check_set_prec(p) : p);
    }

public:
    real &set_prec(::mpfr_prec_t p)
    {
        set_prec_impl<true>(p);
        return *this;
    }
    real &prec_round(::mpfr_prec_t p)
    {
        ::mpfr_prec_round(&m_mpfr, check_set_prec(p), MPFR_RNDN);
        return *this;
    }

private:
    // Generic conversion.
    // integer.
    template <typename T, enable_if_t<is_integer<T>::value, int> = 0>
    T dispatch_conversion() const
    {
        if (mppp_unlikely(!number_p())) {
            throw std::domain_error("Cannot convert a non-finite real to integer");
        }
        MPPP_MAYBE_TLS mpz_raii mpz;
        // Truncate the value when converting to integer.
        ::mpfr_get_z(&mpz.m_mpz, &m_mpfr, MPFR_RNDZ);
        return T{&mpz.m_mpz};
    }
    // rational.
    template <typename T, enable_if_t<is_rational<T>::value, int> = 0>
    T dispatch_conversion() const
    {
        if (mppp_unlikely(!number_p())) {
            throw std::domain_error("Cannot convert a non-finite real to rational");
        }
        // Go through an mpf conversion, as mpfr_get_q() does not exist.
        MPPP_MAYBE_TLS mpf_raii mpf(static_cast<::mp_bitcnt_t>(32));
        MPPP_MAYBE_TLS mpq_raii mpq;
        // Set the temp mpf to the same precision as this.
        ::mpf_set_prec(&mpf.m_mpf, safe_cast<::mp_bitcnt_t>(get_prec()));
        // Copy this into the mpf.
        ::mpfr_get_f(&mpf.m_mpf, &m_mpfr, MPFR_RNDN);
        // Read the mpq from the mpf.
        ::mpq_set_f(&mpq.m_mpq, &mpf.m_mpf);
        // Construct the return, no canonicalisation will be performed here.
        return T{&mpq.m_mpq};
    }
    // C++ floating-point.
    template <typename T, enable_if_t<std::is_floating_point<T>::value, int> = 0>
    T dispatch_conversion() const
    {
        if (std::is_same<T, float>::value) {
            return static_cast<T>(::mpfr_get_flt(&m_mpfr, MPFR_RNDN));
        }
        if (std::is_same<T, double>::value) {
            return static_cast<T>(::mpfr_get_d(&m_mpfr, MPFR_RNDN));
        }
        assert((std::is_same<T, long double>::value));
        return static_cast<T>(::mpfr_get_ld(&m_mpfr, MPFR_RNDN));
    }
    // Small helper to raise an exception when converting to C++ integrals.
    template <typename T>
    [[noreturn]] void raise_overflow_error() const
    {
        throw std::overflow_error("Conversion of the real " + to_string() + " to the type " + typeid(T).name()
                                  + " results in overflow");
    }
    // Unsigned integrals, excluding bool.
    template <typename T,
              enable_if_t<conjunction<negation<std::is_same<bool, T>>, std::is_integral<T>, std::is_unsigned<T>>::value,
                          int> = 0>
    T dispatch_conversion() const
    {
        if (mppp_unlikely(!number_p())) {
            throw std::domain_error("Cannot convert a non-finite real to a C++ unsigned integral type");
        }
        // Clear the range error flag before attempting the conversion.
        ::mpfr_clear_erangeflag();
        // NOTE: this will handle correctly the case in which this is negative but greater than -1.
        const unsigned long candidate = ::mpfr_get_ui(&m_mpfr, MPFR_RNDZ);
        if (::mpfr_erangeflag_p()) {
            // If the range error flag is set, it means the conversion failed because this is outside
            // the range of unsigned long. Let's clear the error flag first.
            ::mpfr_clear_erangeflag();
            // If the value is positive, and the target type has a range greater than unsigned long,
            // we will attempt the conversion again via integer.
            if (std::numeric_limits<T>::max() > std::numeric_limits<unsigned long>::max() && sgn() > 0) {
                try {
                    return static_cast<T>(static_cast<integer<1>>(*this));
                } catch (const std::overflow_error &) {
                }
            }
            // We end up here because either:
            // - this is negative and not greater than -1,
            // - the range of the target type is not greater than unsigned long's,
            // - the range of the target type is greater than unsigned long's but the conversion
            //   via integer failed anyway.
            raise_overflow_error<T>();
        }
        if (candidate <= std::numeric_limits<T>::max()) {
            return static_cast<T>(candidate);
        }
        raise_overflow_error<T>();
    }
    // bool.
    template <typename T, enable_if_t<std::is_same<bool, T>::value, int> = 0>
    T dispatch_conversion() const
    {
        if (mppp_unlikely(nan_p())) {
            throw std::domain_error("Cannot convert NaN to bool");
        }
        return !zero_p();
    }
    // Signed integrals.
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_signed<T>>::value, int> = 0>
    T dispatch_conversion() const
    {
        if (mppp_unlikely(!number_p())) {
            throw std::domain_error("Cannot convert a non-finite real to a C++ signed integral type");
        }
        ::mpfr_clear_erangeflag();
        const long candidate = ::mpfr_get_si(&m_mpfr, MPFR_RNDZ);
        if (::mpfr_erangeflag_p()) {
            // If the range error flag is set, it means the conversion failed because this is outside
            // the range of long. Let's clear the error flag first.
            ::mpfr_clear_erangeflag();
            // If the target type has a range greater than long,
            // we will attempt the conversion again via integer.
            if (std::numeric_limits<T>::min() < std::numeric_limits<long>::min()
                && std::numeric_limits<T>::max() > std::numeric_limits<long>::max()) {
                try {
                    return static_cast<T>(static_cast<integer<1>>(*this));
                } catch (const std::overflow_error &) {
                }
            }
            // We end up here because either:
            // - the range of the target type is not greater than long's,
            // - the range of the target type is greater than long's but the conversion
            //   via integer failed anyway.
            raise_overflow_error<T>();
        }
        if (candidate >= std::numeric_limits<T>::min() && candidate <= std::numeric_limits<T>::max()) {
            return static_cast<T>(candidate);
        }
        raise_overflow_error<T>();
    }
#if defined(MPPP_WITH_QUADMATH)
    template <typename T, enable_if_t<std::is_same<real128, T>::value, int> = 0>
    T dispatch_conversion() const
    {
        // Handle the special cases first.
        if (nan_p()) {
            return real128_nan();
        }
        // NOTE: the number 2**18 = 262144 is chosen so that it's amply outside the exponent
        // range of real128 (a 15 bit value with some offset) but well within the
        // range of long (around +-2**31 guaranteed by the standard).
        if (inf_p() || m_mpfr._mpfr_exp > (1l << 18)) {
            return sgn() > 0 ? real128_inf() : -real128_inf();
        }
        if (zero_p() || m_mpfr._mpfr_exp < -(1l << 18)) {
            // Preserve signedness of zero.
            return signbit() ? real128{} : -real128{};
        }
        // NOTE: this is similar to the code in real128.hpp for the constructor from integer,
        // with some modification due to the different padding in MPFR vs GMP (see below).
        const auto prec = get_prec();
        // Number of limbs in this.
        auto nlimbs = prec / ::mp_bits_per_limb + static_cast<bool>(prec % ::mp_bits_per_limb);
        assert(nlimbs != 0);
        // NOTE: in MPFR the most significant (nonzero) bit of the significand
        // is always at the high end of the most significand limb. In other words,
        // MPFR pads the multiprecision significand on the right, the opposite
        // of GMP integers (which have padding in the top limb).
        // NOTE: contrary to real128, the MPFR format does not have a hidden bit on top.
        //
        // Init retval with the highest limb.
        real128 retval{m_mpfr._mpfr_d[--nlimbs] & GMP_NUMB_MASK};
        // Init the number of read bits.
        unsigned read_bits = static_cast<unsigned>(::mp_bits_per_limb);
        while (nlimbs && read_bits < real128_sig_digits()) {
            // Number of bits to be read from the current limb. Either mp_bits_per_limb or less.
            const unsigned rbits = c_min(static_cast<unsigned>(::mp_bits_per_limb), real128_sig_digits() - read_bits);
            // Shift up by rbits.
            // NOTE: cast to int is safe, as rbits is no larger than mp_bits_per_limb which is
            // representable by int.
            retval = scalbn(retval, static_cast<int>(rbits));
            // Add the next limb, removing lower bits if they are not to be read.
            retval += (m_mpfr._mpfr_d[--nlimbs] & GMP_NUMB_MASK) >> (static_cast<unsigned>(::mp_bits_per_limb) - rbits);
            // Update the number of read bits.
            // NOTE: due to the definition of rbits, read_bits can never reach past real128_sig_digits().
            // Hence, this addition can never overflow (as sig_digits is unsigned itself).
            read_bits += rbits;
        }
        // NOTE: from earlier we know the exponent is well within the range of long, and read_bits
        // cannot be larger than mp_bits_per_limb or 113.
        retval = scalbln(retval, static_cast<long>(m_mpfr._mpfr_exp) - static_cast<long>(read_bits));
        return sgn() > 0 ? retval : -retval;
    }
#endif

public:
#if defined(MPPP_HAVE_CONCEPTS)
    template <RealInteroperable T>
#else
    template <typename T, real_interoperable_enabler<T> = 0>
#endif
    explicit operator T() const
    {
        return dispatch_conversion<T>();
    }
    std::string to_string() const
    {
        std::ostringstream oss;
        mpfr_to_stream(&m_mpfr, oss);
        return oss.str();
    }

private:
    mpfr_struct_t m_mpfr;
};

inline void add(real &rop, const real &op1, const real &op2)
{
    ::mpfr_add(rop._get_mpfr_t(), op1.get_mpfr_t(), op2.get_mpfr_t(), MPFR_RNDN);
}

inline std::ostream &operator<<(std::ostream &os, const real &r)
{
    mpfr_to_stream(r.get_mpfr_t(), os);
    return os;
}
}

#else

#error The real.hpp header was included but mp++ was not configured with the MPPP_WITH_MPFR option.

#endif

#endif
