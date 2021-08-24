// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#if defined(_MSC_VER)

#define _SCL_SECURE_NO_WARNINGS

#endif

// NOTE: in order to use the MPFR print functions,
// we must make sure that stdio.h is included before mpfr.h:
// https://www.mpfr.org/mpfr-current/mpfr.html#Formatted-Output-Functions
#include <cstdio>

#include <mp++/config.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstddef>
#include <ios>
#include <iostream>
#include <limits>
#include <locale>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(MPPP_HAVE_STRING_VIEW)
#include <string_view>
#endif

#if defined(MPPP_WITH_BOOST_S11N)

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/binary_object.hpp>

#endif

#include <mp++/detail/gmp.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/integer.hpp>
#include <mp++/real.hpp>

#if defined(MPPP_WITH_QUADMATH)
#include <mp++/real128.hpp>
#endif

namespace mppp
{

namespace detail
{

namespace
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

// Double check that real is a standard layout class.
static_assert(std::is_standard_layout<real>::value, "real is not a standard layout class.");

#if MPPP_CPLUSPLUS >= 201703L

// If we have C++17, we can use structured bindings to test the layout of mpfr_struct_t
// and its members' types.
constexpr bool test_mpfr_struct_t()
{
    auto [prec, sign, exp, ptr] = mpfr_struct_t{};
    static_assert(std::is_same<decltype(ptr), ::mp_limb_t *>::value);
    ignore(prec, sign, exp, ptr);

    return true;
}

static_assert(test_mpfr_struct_t());

#endif

} // namespace

// Wrapper for calling mpfr_lgamma().
void real_lgamma_wrapper(::mpfr_t rop, const ::mpfr_t op, ::mpfr_rnd_t)
{
    // NOTE: we ignore the sign for consistency with lgamma.
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    int signp;
    ::mpfr_lgamma(rop, &signp, op, MPFR_RNDN);
}

// Wrapper for calling mpfr_li2().
void real_li2_wrapper(::mpfr_t rop, const ::mpfr_t op, ::mpfr_rnd_t rnd)
{
    // NOTE: mpfr_li2() returns the *real* part of the result,
    // which, for op >= 1, is a complex number. For consistency
    // with other functions, if the result is complex just set
    // rop to nan.
    // NOTE: check for nan before checking for >= 1, otherwise
    // the comparison function will set the erange flag.
    if (!mpfr_nan_p(op) && mpfr_cmp_ui(op, 1u) >= 0) {
        ::mpfr_set_nan(rop);
    } else {
        ::mpfr_li2(rop, op, rnd);
    }
}

// Wrappers for calling integer and remainder-related functions
// with NaN checking.
void real_ceil_wrapper(::mpfr_t rop, const ::mpfr_t op)
{
    if (mppp_unlikely(mpfr_nan_p(op) != 0)) {
        throw std::domain_error("Cannot compute the ceiling of a NaN value");
    }

    mpfr_ceil(rop, op);
}

void real_floor_wrapper(::mpfr_t rop, const ::mpfr_t op)
{
    if (mppp_unlikely(mpfr_nan_p(op) != 0)) {
        throw std::domain_error("Cannot compute the floor of a NaN value");
    }

    mpfr_floor(rop, op);
}

void real_round_wrapper(::mpfr_t rop, const ::mpfr_t op)
{
    if (mppp_unlikely(mpfr_nan_p(op) != 0)) {
        throw std::domain_error("Cannot round a NaN value");
    }

    mpfr_round(rop, op);
}

#if defined(MPPP_MPFR_HAVE_MPFR_ROUNDEVEN)

void real_roundeven_wrapper(::mpfr_t rop, const ::mpfr_t op)
{
    if (mppp_unlikely(mpfr_nan_p(op) != 0)) {
        throw std::domain_error("Cannot round a NaN value");
    }

    ::mpfr_roundeven(rop, op);
}

#endif

void real_trunc_wrapper(::mpfr_t rop, const ::mpfr_t op)
{
    if (mppp_unlikely(mpfr_nan_p(op) != 0)) {
        throw std::domain_error("Cannot truncate a NaN value");
    }

    mpfr_trunc(rop, op);
}

void real_frac_wrapper(::mpfr_t rop, const ::mpfr_t op)
{
    if (mppp_unlikely(mpfr_nan_p(op) != 0)) {
        throw std::domain_error("Cannot compute the fractional part of a NaN value");
    }

    ::mpfr_frac(rop, op, MPFR_RNDN);
}

void mpfr_to_stream(const ::mpfr_t r, std::ostream &os, int base)
{
    // All chars potentially used by MPFR for representing the digits up to base 62, sorted.
    constexpr char all_chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    // Check the base.
    if (mppp_unlikely(base < 2 || base > 62)) {
        throw std::invalid_argument("Cannot convert a real to a string in base " + to_string(base)
                                    + ": the base must be in the [2,62] range");
    }
    // Special values first.
    if (mpfr_nan_p(r)) {
        // NOTE: up to base 16 we can use nan, inf, etc., but with larger
        // bases we have to use the syntax with @.
        os << (base <= 16 ? "nan" : "@nan@");
        return;
    }
    if (mpfr_inf_p(r)) {
        if (mpfr_sgn(r) < 0) {
            os << '-';
        }
        os << (base <= 16 ? "inf" : "@inf@");
        return;
    }

    // Get the string fractional representation via the MPFR function,
    // and wrap it into a smart pointer.
    ::mpfr_exp_t exp(0);
    std::unique_ptr<char, void (*)(char *)> str(::mpfr_get_str(nullptr, &exp, base, 0, r, MPFR_RNDN), ::mpfr_free_str);
    // LCOV_EXCL_START
    if (mppp_unlikely(!str)) {
        throw std::runtime_error("Error in the conversion of a real to string: the call to mpfr_get_str() failed");
    }
    // LCOV_EXCL_STOP

    // Print the string, inserting a decimal point after the first digit.
    bool dot_added = false;
    // NOLINTNEXTLINE(llvm-qualified-auto, readability-qualified-auto)
    for (auto cptr = str.get(); *cptr != '\0'; ++cptr) {
        os << (*cptr);
        if (!dot_added) {
            if (base <= 10) {
                // For bases up to 10, we can use the followig guarantee
                // from the standard:
                // http://stackoverflow.com/questions/13827180/char-ascii-relation
                // """
                // The mapping of integer values for characters does have one guarantee given
                // by the Standard: the values of the decimal digits are contiguous.
                // (i.e., '1' - '0' == 1, ... '9' - '0' == 9)
                // """
                // http://eel.is/c++draft/lex.charset#3
                if (*cptr >= '0' && *cptr <= '9') {
                    os << '.';
                    dot_added = true;
                }
            } else {
                // For bases larger than 10, we do a binary search among all the allowed characters.
                // NOTE: we need to search into the whole all_chars array (instead of just up to all_chars
                // + base) because apparently mpfr_get_str() seems to ignore lower/upper case when the base
                // is small enough (e.g., it uses 'a' instead of 'A' when printing in base 11).
                // NOTE: the range needs to be sizeof() - 1 because sizeof() also includes the terminator.
                if (std::binary_search(all_chars, all_chars + (sizeof(all_chars) - 1u), *cptr)) {
                    os << '.';
                    dot_added = true;
                }
            }
        }
    }
    assert(dot_added);

    // Adjust the exponent. Do it in multiprec in order to avoid potential overflow.
    integer<1> z_exp{exp};
    --z_exp;
    const auto exp_sgn = z_exp.sgn();
    if (exp_sgn != 0 && !mpfr_zero_p(r)) {
        // Add the exponent at the end of the string, if both the value and the exponent
        // are nonzero.
        // NOTE: for bases greater than 10 we need '@' for the exponent, rather than 'e' or 'E'.
        // https://www.mpfr.org/mpfr-current/mpfr.html#Assignment-Functions
        os << (base <= 10 ? 'e' : '@');
        if (exp_sgn == 1) {
            // Add extra '+' if the exponent is positive, for consistency with
            // real128's string format (and possibly other formats too?).
            os << '+';
        }
        os << z_exp;
    }
}

} // namespace detail

// Default constructor.
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
real::real()
{
    ::mpfr_init2(&m_mpfr, real_prec_min());
    ::mpfr_set_zero(&m_mpfr, 1);
}

// Init a real with precision p, setting its value to nan. No precision
// checking is performed.
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
real::real(const ptag &, ::mpfr_prec_t p, bool ignore_prec)
{
    assert(ignore_prec);
    assert(detail::real_prec_check(p));
    detail::ignore(ignore_prec);
    ::mpfr_init2(&m_mpfr, p);
}

// Copy constructor.
real::real(const real &other) : real(&other.m_mpfr) {}

// Copy constructor with custom precision.
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
real::real(const real &other, ::mpfr_prec_t p)
{
    // Init with custom precision, and then set.
    ::mpfr_init2(&m_mpfr, check_init_prec(p));
    mpfr_set(&m_mpfr, &other.m_mpfr, MPFR_RNDN);
}

// Move constructor with custom precision.
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
real::real(real &&other, ::mpfr_prec_t p)
{
    // Check the precision first of all.
    check_init_prec(p);

    // Shallow copy other.
    m_mpfr = other.m_mpfr;
    // Mark the other as moved-from.
    other.m_mpfr._mpfr_d = nullptr;

    // Apply the precision.
    prec_round_impl<false>(p);
}

// Construction from FPs.
template <typename Func, typename T>
void real::dispatch_fp_construction(const Func &func, const T &x)
{
    func(&m_mpfr, x, MPFR_RNDN);
}

void real::dispatch_construction(const float &x)
{
    dispatch_fp_construction(::mpfr_set_flt, x);
}

void real::dispatch_construction(const double &x)
{
    dispatch_fp_construction(::mpfr_set_d, x);
}

void real::dispatch_construction(const long double &x)
{
    dispatch_fp_construction(::mpfr_set_ld, x);
}

// Special casing for bool, otherwise MSVC warns if we fold this into the
// constructor from unsigned.
void real::dispatch_construction(const bool &b)
{
    mpfr_set_ui(&m_mpfr, static_cast<unsigned long>(b), MPFR_RNDN);
}

void real::dispatch_mpz_construction(const ::mpz_t n)
{
    ::mpfr_set_z(&m_mpfr, n, MPFR_RNDN);
}

void real::dispatch_mpq_construction(const ::mpq_t q)
{
    ::mpfr_set_q(&m_mpfr, q, MPFR_RNDN);
}

#if defined(MPPP_WITH_QUADMATH)

void real::dispatch_construction(const real128 &x)
{
    assign_real128(x);
}

#endif

// Various helpers and constructors from string-like entities.
void real::construct_from_c_string(const char *s, int base, ::mpfr_prec_t p)
{
    if (mppp_unlikely(base && (base < 2 || base > 62))) {
        throw std::invalid_argument("Cannot construct a real from a string in base " + detail::to_string(base)
                                    + ": the base must either be zero or in the [2,62] range");
    }
    ::mpfr_init2(&m_mpfr, check_init_prec(p));
    const auto ret = ::mpfr_set_str(&m_mpfr, s, base, MPFR_RNDN);
    if (mppp_unlikely(ret == -1)) {
        ::mpfr_clear(&m_mpfr);
        throw std::invalid_argument(std::string{"The string '"} + s + "' does not represent a valid real in base "
                                    + detail::to_string(base));
    }
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
real::real(const ptag &, const char *s, int base, ::mpfr_prec_t p)
{
    construct_from_c_string(s, base, p);
}

real::real(const ptag &, const std::string &s, int base, ::mpfr_prec_t p) : real(s.c_str(), base, p) {}

#if defined(MPPP_HAVE_STRING_VIEW)
real::real(const ptag &, const std::string_view &s, int base, ::mpfr_prec_t p)
    : real(s.data(), s.data() + s.size(), base, p)
{
}
#endif

// Constructor from range of characters, base and precision.
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
real::real(const char *begin, const char *end, int base, ::mpfr_prec_t p)
{
    MPPP_MAYBE_TLS std::vector<char> buffer;
    buffer.assign(begin, end);
    buffer.emplace_back('\0');
    construct_from_c_string(buffer.data(), base, p);
}

// Constructor from range of characters and precision.
real::real(const char *begin, const char *end, ::mpfr_prec_t p) : real(begin, end, 10, p) {}

// Constructor from a special value, sign and precision.
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
real::real(real_kind k, int sign, ::mpfr_prec_t p)
{
    ::mpfr_init2(&m_mpfr, check_init_prec(p));
    // NOTE: handle all cases explicitly, in order to avoid
    // compiler warnings.
    switch (k) {
        case real_kind::nan:
            break;
        case real_kind::inf:
            set_inf(sign);
            break;
        case real_kind::zero:
            set_zero(sign);
            break;
        default:
            // Clean up before throwing.
            ::mpfr_clear(&m_mpfr);
            using kind_cast_t = std::underlying_type<::mpfr_kind_t>::type;
            throw std::invalid_argument(
                "The 'real_kind' value passed to the constructor of a real ("
                + std::to_string(static_cast<kind_cast_t>(k)) + ") is not one of the three allowed values ('nan'="
                + std::to_string(static_cast<kind_cast_t>(real_kind::nan))
                + ", 'inf'=" + std::to_string(static_cast<kind_cast_t>(real_kind::inf))
                + " and 'zero'=" + std::to_string(static_cast<kind_cast_t>(real_kind::zero)) + ")");
    }
}

// Constructor from a special value and precision.
real::real(real_kind k, ::mpfr_prec_t p) : real(k, 0, p) {}

// Constructors from n*2**e.
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
real::real(unsigned long n, ::mpfr_exp_t e, ::mpfr_prec_t p)
{
    ::mpfr_init2(&m_mpfr, check_init_prec(p));
    set_ui_2exp(*this, n, e);
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
real::real(long n, ::mpfr_exp_t e, ::mpfr_prec_t p)
{
    ::mpfr_init2(&m_mpfr, check_init_prec(p));
    set_si_2exp(*this, n, e);
}

// Copy constructor from mpfr_t.
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
real::real(const ::mpfr_t x)
{
    // Init with the same precision as other, and then set.
    ::mpfr_init2(&m_mpfr, mpfr_get_prec(x));
    mpfr_set(&m_mpfr, x, MPFR_RNDN);
}

// Copy assignment operator.
real &real::operator=(const real &other)
{
    if (mppp_likely(this != &other)) {
        if (is_valid()) {
            // this has not been moved-from.
            // Copy the precision. This will also reset the internal value.
            // No need for prec checking as we assume other has a valid prec.
            set_prec_impl<false>(other.get_prec());
        } else {
            // this has been moved-from: init before setting.
            ::mpfr_init2(&m_mpfr, other.get_prec());
        }
        // Perform the actual copy from other.
        mpfr_set(&m_mpfr, &other.m_mpfr, MPFR_RNDN);
    }
    return *this;
}

// Implementation of the assignment from string.
void real::string_assignment_impl(const char *s, int base)
{
    if (mppp_unlikely(base && (base < 2 || base > 62))) {
        throw std::invalid_argument("Cannot assign a real from a string in base " + detail::to_string(base)
                                    + ": the base must either be zero or in the [2,62] range");
    }
    const auto ret = ::mpfr_set_str(&m_mpfr, s, base, MPFR_RNDN);
    if (mppp_unlikely(ret == -1)) {
        ::mpfr_set_nan(&m_mpfr);
        throw std::invalid_argument(std::string{"The string '"} + s
                                    + "' cannot be interpreted as a floating-point value in base "
                                    + detail::to_string(base));
    }
}

#if defined(MPPP_WITH_QUADMATH)

// Implementation of real128's assignment
// from real.
real128 &real128::operator=(const real &x)
{
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
    return *this = static_cast<real128>(x);
}

#endif

// Copy assignment from mpfr_t.
real &real::operator=(const ::mpfr_t x)
{
    // Set the precision, assuming the prec of x is valid.
    set_prec_impl<false>(mpfr_get_prec(x));
    // Set the value.
    mpfr_set(&m_mpfr, x, MPFR_RNDN);
    return *this;
}

#if !defined(_MSC_VER) || defined(__clang__)

// Move assignment from mpfr_t.
real &real::operator=(::mpfr_t &&x)
{
    // Clear this.
    ::mpfr_clear(&m_mpfr);
    // Shallow copy x.
    m_mpfr = *x;
    return *this;
}

#endif

// Set to another real.
real &real::set(const real &other)
{
    return set(&other.m_mpfr);
}

// Implementation of string setters.
real &real::set_impl(const char *s, int base)
{
    string_assignment_impl(s, base);
    return *this;
}

real &real::set_impl(const std::string &s, int base)
{
    return set(s.c_str(), base);
}

#if defined(MPPP_HAVE_STRING_VIEW)
real &real::set_impl(const std::string_view &s, int base)
{
    return set(s.data(), s.data() + s.size(), base);
}
#endif

// Set to character range.
real &real::set(const char *begin, const char *end, int base)
{
    MPPP_MAYBE_TLS std::vector<char> buffer;
    buffer.assign(begin, end);
    buffer.emplace_back('\0');
    return set(buffer.data(), base);
}

// Set to an mpfr_t.
real &real::set(const ::mpfr_t x)
{
    mpfr_set(&m_mpfr, x, MPFR_RNDN);
    return *this;
}

// Set to NaN.
real &real::set_nan()
{
    ::mpfr_set_nan(&m_mpfr);
    return *this;
}

// Set to infinity.
real &real::set_inf(int sign)
{
    ::mpfr_set_inf(&m_mpfr, sign);
    return *this;
}

// Set to zero.
real &real::set_zero(int sign)
{
    ::mpfr_set_zero(&m_mpfr, sign);
    return *this;
}

// Detect one.
bool real::is_one() const
{
    // NOTE: preempt calling the comparison function, if this is NaN
    // (in such case, the range flag will be touched and we do not
    // want to bother with that).
    return !nan_p() && (mpfr_cmp_ui(&m_mpfr, 1u) == 0);
}

#if defined(MPPP_WITH_QUADMATH)

namespace detail
{

// Some private machinery for the interaction between real and real128.
namespace
{

// Helper to create a real with the value 2**112. This represents the "hidden bit"
// of the significand of a quadruple-precision FP.
real real_2_112()
{
    // NOTE: we need only 1 bit of precision,
    // but make sure that we don't choose a value
    // too small.
    real retval{1, clamp_mpfr_prec(1)};

    // NOTE: do the computation using the MPFR API,
    // in order to avoid mp++'s precision management.
    ::mpfr_mul_2ui(retval._get_mpfr_t(), retval.get_mpfr_t(), 112u, MPFR_RNDN);

    return retval;
}

} // namespace

} // namespace detail

void real::assign_real128(const real128 &x)
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
        mpfr_set_ui(&this->m_mpfr, static_cast<unsigned long>(p1), MPFR_RNDN);
        ::mpfr_mul_2ui(&this->m_mpfr, &this->m_mpfr, 32ul, MPFR_RNDN);
        ::mpfr_add_ui(&this->m_mpfr, &this->m_mpfr, static_cast<unsigned long>(p2), MPFR_RNDN);
        ::mpfr_mul_2ui(&this->m_mpfr, &this->m_mpfr, 32ul, MPFR_RNDN);
        ::mpfr_add_ui(&this->m_mpfr, &this->m_mpfr, static_cast<unsigned long>(p3), MPFR_RNDN);
        ::mpfr_mul_2ui(&this->m_mpfr, &this->m_mpfr, 32ul, MPFR_RNDN);
        ::mpfr_add_ui(&this->m_mpfr, &this->m_mpfr, static_cast<unsigned long>(p4), MPFR_RNDN);
    };
    // Check if the significand is zero.
    const bool sig_zero = std::get<2>(t) == 0u && std::get<3>(t) == 0u;
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
        const MPPP_MAYBE_TLS real r_2_112 = detail::real_2_112();
        ::mpfr_add(&m_mpfr, &m_mpfr, r_2_112.get_mpfr_t(), MPFR_RNDN);
        // Multiply by 2 raised to the adjusted exponent.
        ::mpfr_mul_2si(&m_mpfr, &m_mpfr, static_cast<long>(std::get<1>(t)) - (16383l + 112), MPFR_RNDN);
    }
    if (std::get<0>(t) != 0u) {
        // Negate if the sign bit is set.
        ::mpfr_neg(&m_mpfr, &m_mpfr, MPFR_RNDN);
    }
}

real128 real::convert_to_real128() const
{
    // Handle the special cases first.
    if (nan_p()) {
        return real128_nan();
    }
    // NOTE: the number 2**18 = 262144 is chosen so that it's amply outside the exponent
    // range of real128 (a 15 bit value with some offset) but well within the
    // range of long (around +-2**31 guaranteed by the standard).
    //
    // NOTE: the reason why we do these checks with large positive and negative exponents
    // is that they ensure we can safely convert _mpfr_exp to long later.
    if (inf_p() || m_mpfr._mpfr_exp > (1l << 18)) {
        return sgn() > 0 ? real128_inf() : -real128_inf();
    }
    if (zero_p() || m_mpfr._mpfr_exp < -(1l << 18)) {
        // Preserve the signedness of zero.
        return signbit() ? -real128{} : real128{};
    }
    // NOTE: this is similar to the code in real128.hpp for the constructor from integer,
    // with some modification due to the different padding in MPFR vs GMP (see below).
    const auto prec = get_prec();
    // Number of limbs in this.
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    auto nlimbs = prec / ::mp_bits_per_limb + static_cast<bool>(prec % ::mp_bits_per_limb);
    assert(nlimbs != 0);
    // NOTE: in MPFR the most significant (nonzero) bit of the significand
    // is always at the high end of the most significand limb. In other words,
    // MPFR pads the multiprecision significand on the right, the opposite
    // of GMP integers (which have padding on the left, i.e., in the top limb).
    //
    // NOTE: contrary to real128, the MPFR format does not have a hidden bit on top.
    //
    // Init retval with the highest limb.
    //
    // NOTE: MPFR does not support nail builds in GMP, so we don't have to worry about that.
    real128 retval{m_mpfr._mpfr_d[--nlimbs]};
    // Init the number of read bits.
    // NOTE: we have read a full limb in the line above, so mp_bits_per_limb bits. If mp_bits_per_limb > 113,
    // then the constructor of real128 truncated the input limb value to 113 bits of precision, so effectively
    // we have read 113 bits only in such a case.
    unsigned read_bits = detail::c_min(static_cast<unsigned>(::mp_bits_per_limb), real128_sig_digits());
    while (nlimbs != 0 && read_bits < real128_sig_digits()) {
        // Number of bits to be read from the current limb. Either mp_bits_per_limb or less.
        const unsigned rbits
            = detail::c_min(static_cast<unsigned>(::mp_bits_per_limb), real128_sig_digits() - read_bits);
        // Shift up by rbits.
        // NOTE: cast to int is safe, as rbits is no larger than mp_bits_per_limb which is
        // representable by int.
        retval = scalbn(retval, static_cast<int>(rbits));
        // Add the next limb, removing lower bits if they are not to be read.
        retval += m_mpfr._mpfr_d[--nlimbs] >> (static_cast<unsigned>(::mp_bits_per_limb) - rbits);
        // Update the number of read bits.
        // NOTE: due to the definition of rbits, read_bits can never reach past real128_sig_digits().
        // Hence, this addition can never overflow (as sig_digits is unsigned itself).
        read_bits += rbits;
    }
    // NOTE: from earlier we know the exponent is well within the range of long, and read_bits
    // cannot be larger than 113.
    retval = scalbln(retval, static_cast<long>(m_mpfr._mpfr_exp) - static_cast<long>(read_bits));
    return sgn() > 0 ? retval : -retval;
}

bool real::dispatch_get(real128 &x) const
{
    x = static_cast<real128>(*this);
    return true;
}

#endif

// Wrapper to apply the input unary MPFR function to this with
// MPFR_RNDN rounding mode. Returns a reference to this.
template <typename T>
real &real::self_mpfr_unary(T &&f)
{
    std::forward<T>(f)(&m_mpfr, &m_mpfr, MPFR_RNDN);
    return *this;
}

// In-place Gamma function.
real &real::gamma()
{
    return self_mpfr_unary(::mpfr_gamma);
}

// In-place logarithm of the Gamma function.
real &real::lngamma()
{
    return self_mpfr_unary(::mpfr_lngamma);
}

// In-place logarithm of the absolute value of the Gamma function.
real &real::lgamma()
{
    detail::real_lgamma_wrapper(&m_mpfr, &m_mpfr, MPFR_RNDN);
    return *this;
}

// In-place Digamma function.
real &real::digamma()
{
    return self_mpfr_unary(::mpfr_digamma);
}

// In-place Bessel function of the first kind of order 0.
real &real::j0()
{
    return self_mpfr_unary(::mpfr_j0);
}

// In-place Bessel function of the first kind of order 1.
real &real::j1()
{
    return self_mpfr_unary(::mpfr_j1);
}

// In-place Bessel function of the second kind of order 0.
real &real::y0()
{
    return self_mpfr_unary(::mpfr_y0);
}

// In-place Bessel function of the second kind of order 1.
real &real::y1()
{
    return self_mpfr_unary(::mpfr_y1);
}

// In-place exponential integral.
real &real::eint()
{
    return self_mpfr_unary(::mpfr_eint);
}

// In-place dilogarithm.
real &real::li2()
{
    return self_mpfr_unary(detail::real_li2_wrapper);
}

// In-place Riemann Zeta function.
real &real::zeta()
{
    return self_mpfr_unary(::mpfr_zeta);
}

// In-place error function.
real &real::erf()
{
    return self_mpfr_unary(::mpfr_erf);
}

// In-place complementary error function.
real &real::erfc()
{
    return self_mpfr_unary(::mpfr_erfc);
}

// In-place Airy function.
real &real::ai()
{
    return self_mpfr_unary(::mpfr_ai);
}

// Negate in-place.
real &real::neg()
{
    return self_mpfr_unary(::mpfr_neg);
}

// In-place absolute value.
real &real::abs()
{
    return self_mpfr_unary(::mpfr_abs);
}

// Destructively set the precision.
real &real::set_prec(::mpfr_prec_t p)
{
    set_prec_impl<true>(p);
    return *this;
}

// Set the precision maintaining the current value.
real &real::prec_round(::mpfr_prec_t p)
{
    prec_round_impl<true>(p);
    return *this;
}

// Convert to string.
std::string real::to_string(int base) const
{
    std::ostringstream oss;
    oss.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    detail::mpfr_to_stream(&m_mpfr, oss, base);
    return oss.str();
}

// In-place square root.
real &real::sqrt()
{
    return self_mpfr_unary(::mpfr_sqrt);
}

// In-place reciprocal square root.
real &real::rec_sqrt()
{
    return self_mpfr_unary(::mpfr_rec_sqrt);
}

// In-place cubic root.
real &real::cbrt()
{
    return self_mpfr_unary(::mpfr_cbrt);
}

// In-place squaring.
real &real::sqr()
{
    return self_mpfr_unary(::mpfr_sqr);
}

// In-place sine.
real &real::sin()
{
    return self_mpfr_unary(::mpfr_sin);
}

// In-place cosine.
real &real::cos()
{
    return self_mpfr_unary(::mpfr_cos);
}

// In-place tangent.
real &real::tan()
{
    return self_mpfr_unary(::mpfr_tan);
}

// In-place secant.
real &real::sec()
{
    return self_mpfr_unary(::mpfr_sec);
}

// In-place cosecant.
real &real::csc()
{
    return self_mpfr_unary(::mpfr_csc);
}

// In-place cotangent.
real &real::cot()
{
    return self_mpfr_unary(::mpfr_cot);
}

// In-place arccosine.
real &real::acos()
{
    return self_mpfr_unary(::mpfr_acos);
}

// In-place arcsine.
real &real::asin()
{
    return self_mpfr_unary(::mpfr_asin);
}

// In-place arctangent.
real &real::atan()
{
    return self_mpfr_unary(::mpfr_atan);
}

// In-place hyperbolic cosine.
real &real::cosh()
{
    return self_mpfr_unary(::mpfr_cosh);
}

// In-place hyperbolic sine.
real &real::sinh()
{
    return self_mpfr_unary(::mpfr_sinh);
}

// In-place hyperbolic tangent.
real &real::tanh()
{
    return self_mpfr_unary(::mpfr_tanh);
}

// In-place hyperbolic secant.
real &real::sech()
{
    return self_mpfr_unary(::mpfr_sech);
}

// In-place hyperbolic cosecant.
real &real::csch()
{
    return self_mpfr_unary(::mpfr_csch);
}

// In-place hyperbolic cotangent.
real &real::coth()
{
    return self_mpfr_unary(::mpfr_coth);
}

// In-place inverse hyperbolic cosine.
real &real::acosh()
{
    return self_mpfr_unary(::mpfr_acosh);
}

// In-place inverse hyperbolic sine.
real &real::asinh()
{
    return self_mpfr_unary(::mpfr_asinh);
}

// In-place inverse hyperbolic tangent.
real &real::atanh()
{
    return self_mpfr_unary(::mpfr_atanh);
}

// In-place exponential.
real &real::exp()
{
    return self_mpfr_unary(::mpfr_exp);
}

// In-place base-2 exponential.
real &real::exp2()
{
    return self_mpfr_unary(::mpfr_exp2);
}

// In-place base-10 exponential.
real &real::exp10()
{
    return self_mpfr_unary(::mpfr_exp10);
}

// In-place exponential minus 1.
real &real::expm1()
{
    return self_mpfr_unary(::mpfr_expm1);
}

// In-place logarithm.
real &real::log()
{
    return self_mpfr_unary(::mpfr_log);
}

// In-place base-2 logarithm.
real &real::log2()
{
    return self_mpfr_unary(::mpfr_log2);
}

// In-place base-10 logarithm.
real &real::log10()
{
    return self_mpfr_unary(::mpfr_log10);
}

// In-place augmented logarithm.
real &real::log1p()
{
    return self_mpfr_unary(::mpfr_log1p);
}

// Check if the value is an integer.
bool real::integer_p() const
{
    return ::mpfr_integer_p(&m_mpfr) != 0;
}

// In-place integer and remainder-related functions.
real &real::ceil()
{
    return self_mpfr_unary_nornd(detail::real_ceil_wrapper);
}

real &real::floor()
{
    return self_mpfr_unary_nornd(detail::real_floor_wrapper);
}

real &real::round()
{
    return self_mpfr_unary_nornd(detail::real_round_wrapper);
}

#if defined(MPPP_MPFR_HAVE_MPFR_ROUNDEVEN)

real &real::roundeven()
{
    return self_mpfr_unary_nornd(detail::real_roundeven_wrapper);
}

#endif

real &real::trunc()
{
    return self_mpfr_unary_nornd(detail::real_trunc_wrapper);
}

real &real::frac()
{
    return self_mpfr_unary_nornd(detail::real_frac_wrapper);
}

// Set to n*2**e.
real &set_ui_2exp(real &r, unsigned long n, ::mpfr_exp_t e)
{
    ::mpfr_set_ui_2exp(r._get_mpfr_t(), n, e, MPFR_RNDN);
    return r;
}

real &set_si_2exp(real &r, long n, ::mpfr_exp_t e)
{
    ::mpfr_set_si_2exp(r._get_mpfr_t(), n, e, MPFR_RNDN);
    return r;
}

// Implementation bits for in-place addition.
namespace detail
{

void dispatch_real_in_place_add_integer_impl(real &a, const ::mpz_t n, ::mpfr_prec_t dprec)
{
    auto wrapper = [&n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_add_z(r, o, n, MPFR_RNDN); };

    // NOTE: in these mpfr_nary_op_impl() invocations, we are passing a min_prec
    // which is by definition valid because it is produced by an invocation of
    // real_deduce_precision() (which does clamping).
    mpfr_nary_op_impl<false>(dprec, wrapper, a, a);
}

void dispatch_real_in_place_add(real &a, bool n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_add_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };

    mpfr_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
}

void dispatch_real_in_place_add_rational_impl(real &a, const ::mpq_t q, ::mpfr_prec_t dprec)
{
    auto wrapper = [&q](::mpfr_t r, const ::mpfr_t o) { ::mpfr_add_q(r, o, q, MPFR_RNDN); };

    mpfr_nary_op_impl<false>(dprec, wrapper, a, a);
}

namespace
{

template <typename T>
inline void dispatch_real_in_place_add_fd_impl(real &a, const T &x)
{
    // NOTE: the MPFR docs state that mpfr_add_d() assumes that
    // the radix of double is a power of 2. If we ever run into platforms
    // for which this is not true, we can add a compile-time dispatch
    // that uses the long double implementation instead.
    constexpr auto dradix = static_cast<unsigned>(std::numeric_limits<double>::radix);
    static_assert(!(dradix & (dradix - 1)), "mpfr_add_d() requires the radix of the 'double' type to be a power of 2.");

    auto wrapper = [x](::mpfr_t r, const ::mpfr_t o) { ::mpfr_add_d(r, o, static_cast<double>(x), MPFR_RNDN); };

    mpfr_nary_op_impl<false>(real_deduce_precision(x), wrapper, a, a);
}

} // namespace

void dispatch_real_in_place_add(real &a, const float &x)
{
    dispatch_real_in_place_add_fd_impl(a, x);
}

void dispatch_real_in_place_add(real &a, const double &x)
{
    dispatch_real_in_place_add_fd_impl(a, x);
}

namespace
{

template <typename T>
inline void dispatch_real_in_place_add_generic_impl(real &a, const T &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp.set_prec(c_max(a.get_prec(), real_deduce_precision(x)));
    tmp.set(x);
    dispatch_real_in_place_add(a, tmp);
}

} // namespace

void dispatch_real_in_place_add(real &a, const long double &x)
{
    dispatch_real_in_place_add_generic_impl(a, x);
}

#if defined(MPPP_WITH_QUADMATH)

void dispatch_real_in_place_add(real &a, const real128 &x)
{
    dispatch_real_in_place_add_generic_impl(a, x);
}

#endif

} // namespace detail

real &operator++(real &x)
{
    if (mppp_unlikely(x.get_prec() < detail::real_deduce_precision(1))) {
        x.prec_round(detail::real_deduce_precision(1));
    }
    ::mpfr_add_ui(x._get_mpfr_t(), x.get_mpfr_t(), 1ul, MPFR_RNDN);
    return x;
}

real operator++(real &x, int)
{
    auto retval(x);
    ++x;
    return retval;
}

// Implementation bits for in-place subtraction.
namespace detail
{

void dispatch_real_in_place_sub_integer_impl(real &a, const ::mpz_t n, ::mpfr_prec_t dprec)
{
    auto wrapper = [&n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_sub_z(r, o, n, MPFR_RNDN); };

    // NOTE: in these mpfr_nary_op_impl() invocations, we are passing a min_prec
    // which is by definition valid because it is produced by an invocation of
    // real_deduce_precision() (which does clamping).
    mpfr_nary_op_impl<false>(dprec, wrapper, a, a);
}

void dispatch_real_in_place_sub(real &a, bool n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_sub_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };

    mpfr_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
}

void dispatch_real_in_place_sub_rational_impl(real &a, const ::mpq_t q, ::mpfr_prec_t dprec)
{
    auto wrapper = [&q](::mpfr_t r, const ::mpfr_t o) { ::mpfr_sub_q(r, o, q, MPFR_RNDN); };

    mpfr_nary_op_impl<false>(dprec, wrapper, a, a);
}

namespace
{

template <typename T>
inline void dispatch_real_in_place_sub_fd_impl(real &a, const T &x)
{
    // NOTE: the MPFR docs state that mpfr_sub_d() assumes that
    // the radix of double is a power of 2. If we ever run into platforms
    // for which this is not true, we can add a compile-time dispatch
    // that uses the long double implementation instead.
    constexpr auto dradix = static_cast<unsigned>(std::numeric_limits<double>::radix);
    static_assert(!(dradix & (dradix - 1)), "mpfr_sub_d() requires the radix of the 'double' type to be a power of 2.");

    auto wrapper = [x](::mpfr_t r, const ::mpfr_t o) { ::mpfr_sub_d(r, o, static_cast<double>(x), MPFR_RNDN); };

    mpfr_nary_op_impl<false>(real_deduce_precision(x), wrapper, a, a);
}

} // namespace

void dispatch_real_in_place_sub(real &a, const float &x)
{
    dispatch_real_in_place_sub_fd_impl(a, x);
}

void dispatch_real_in_place_sub(real &a, const double &x)
{
    dispatch_real_in_place_sub_fd_impl(a, x);
}

namespace
{

template <typename T>
inline void dispatch_real_in_place_sub_generic_impl(real &a, const T &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp.set_prec(c_max(a.get_prec(), real_deduce_precision(x)));
    tmp.set(x);
    dispatch_real_in_place_sub(a, tmp);
}

} // namespace

void dispatch_real_in_place_sub(real &a, const long double &x)
{
    dispatch_real_in_place_sub_generic_impl(a, x);
}

#if defined(MPPP_WITH_QUADMATH)

void dispatch_real_in_place_sub(real &a, const real128 &x)
{
    dispatch_real_in_place_sub_generic_impl(a, x);
}

#endif

} // namespace detail

real &operator--(real &x)
{
    if (mppp_unlikely(x.get_prec() < detail::real_deduce_precision(1))) {
        x.prec_round(detail::real_deduce_precision(1));
    }
    ::mpfr_sub_ui(x._get_mpfr_t(), x.get_mpfr_t(), 1ul, MPFR_RNDN);
    return x;
}

real operator--(real &x, int)
{
    auto retval(x);
    --x;
    return retval;
}

// Implementation bits for in-place multiplication.
namespace detail
{

void dispatch_real_in_place_mul_integer_impl(real &a, const ::mpz_t n, ::mpfr_prec_t dprec)
{
    auto wrapper = [&n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_mul_z(r, o, n, MPFR_RNDN); };

    // NOTE: in these mpfr_nary_op_impl() invocations, we are passing a min_prec
    // which is by definition valid because it is produced by an invocation of
    // real_deduce_precision() (which does clamping).
    mpfr_nary_op_impl<false>(dprec, wrapper, a, a);
}

void dispatch_real_in_place_mul(real &a, bool n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { mpfr_mul_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };

    mpfr_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
}

void dispatch_real_in_place_mul_rational_impl(real &a, const ::mpq_t q, ::mpfr_prec_t dprec)
{
    auto wrapper = [&q](::mpfr_t r, const ::mpfr_t o) { ::mpfr_mul_q(r, o, q, MPFR_RNDN); };

    mpfr_nary_op_impl<false>(dprec, wrapper, a, a);
}

namespace
{

template <typename T>
inline void dispatch_real_in_place_mul_fd_impl(real &a, const T &x)
{
    // NOTE: the MPFR docs state that mpfr_mul_d() assumes that
    // the radix of double is a power of 2. If we ever run into platforms
    // for which this is not true, we can add a compile-time dispatch
    // that uses the long double implementation instead.
    constexpr auto dradix = static_cast<unsigned>(std::numeric_limits<double>::radix);
    static_assert(!(dradix & (dradix - 1)), "mpfr_mul_d() requires the radix of the 'double' type to be a power of 2.");

    auto wrapper = [x](::mpfr_t r, const ::mpfr_t o) { ::mpfr_mul_d(r, o, static_cast<double>(x), MPFR_RNDN); };

    mpfr_nary_op_impl<false>(real_deduce_precision(x), wrapper, a, a);
}

} // namespace

void dispatch_real_in_place_mul(real &a, const float &x)
{
    dispatch_real_in_place_mul_fd_impl(a, x);
}

void dispatch_real_in_place_mul(real &a, const double &x)
{
    dispatch_real_in_place_mul_fd_impl(a, x);
}

namespace
{

template <typename T>
inline void dispatch_real_in_place_mul_generic_impl(real &a, const T &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp.set_prec(c_max(a.get_prec(), real_deduce_precision(x)));
    tmp.set(x);
    dispatch_real_in_place_mul(a, tmp);
}

} // namespace

void dispatch_real_in_place_mul(real &a, const long double &x)
{
    dispatch_real_in_place_mul_generic_impl(a, x);
}

#if defined(MPPP_WITH_QUADMATH)

void dispatch_real_in_place_mul(real &a, const real128 &x)
{
    dispatch_real_in_place_mul_generic_impl(a, x);
}

#endif

} // namespace detail

// Implementation bits for in-place division.
namespace detail
{

void dispatch_real_in_place_div_integer_impl(real &a, const ::mpz_t n, ::mpfr_prec_t dprec)
{
    auto wrapper = [&n](::mpfr_t r, const ::mpfr_t o) { ::mpfr_div_z(r, o, n, MPFR_RNDN); };

    // NOTE: in these mpfr_nary_op_impl() invocations, we are passing a min_prec
    // which is by definition valid because it is produced by an invocation of
    // real_deduce_precision() (which does clamping).
    mpfr_nary_op_impl<false>(dprec, wrapper, a, a);
}

void dispatch_real_in_place_div(real &a, bool n)
{
    auto wrapper = [n](::mpfr_t r, const ::mpfr_t o) { mpfr_div_ui(r, o, static_cast<unsigned long>(n), MPFR_RNDN); };

    mpfr_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
}

void dispatch_real_in_place_div_rational_impl(real &a, const ::mpq_t q, ::mpfr_prec_t dprec)
{
    auto wrapper = [&q](::mpfr_t r, const ::mpfr_t o) { ::mpfr_div_q(r, o, q, MPFR_RNDN); };

    mpfr_nary_op_impl<false>(dprec, wrapper, a, a);
}

namespace
{

template <typename T>
inline void dispatch_real_in_place_div_fd_impl(real &a, const T &x)
{
    // NOTE: the MPFR docs state that mpfr_div_d() assumes that
    // the radix of double is a power of 2. If we ever run into platforms
    // for which this is not true, we can add a compile-time dispatch
    // that uses the long double implementation instead.
    constexpr auto dradix = static_cast<unsigned>(std::numeric_limits<double>::radix);
    static_assert(!(dradix & (dradix - 1)), "mpfr_div_d() requires the radix of the 'double' type to be a power of 2.");

    auto wrapper = [x](::mpfr_t r, const ::mpfr_t o) { ::mpfr_div_d(r, o, static_cast<double>(x), MPFR_RNDN); };

    mpfr_nary_op_impl<false>(real_deduce_precision(x), wrapper, a, a);
}

} // namespace

void dispatch_real_in_place_div(real &a, const float &x)
{
    dispatch_real_in_place_div_fd_impl(a, x);
}

void dispatch_real_in_place_div(real &a, const double &x)
{
    dispatch_real_in_place_div_fd_impl(a, x);
}

namespace
{

template <typename T>
inline void dispatch_real_in_place_div_generic_impl(real &a, const T &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp.set_prec(c_max(a.get_prec(), real_deduce_precision(x)));
    tmp.set(x);
    dispatch_real_in_place_div(a, tmp);
}

} // namespace

void dispatch_real_in_place_div(real &a, const long double &x)
{
    dispatch_real_in_place_div_generic_impl(a, x);
}

#if defined(MPPP_WITH_QUADMATH)

void dispatch_real_in_place_div(real &a, const real128 &x)
{
    dispatch_real_in_place_div_generic_impl(a, x);
}

#endif

} // namespace detail

// Implementation details for equality.
namespace detail
{

bool dispatch_real_equality(const real &x, const real &y)
{
    return ::mpfr_equal_p(x.get_mpfr_t(), y.get_mpfr_t()) != 0;
}

bool dispatch_real_equality_integer_impl(const real &r, const ::mpz_t n)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_z(r.get_mpfr_t(), n) == 0;
    }
}

bool dispatch_real_equality(const real &r, bool b)
{
    if (r.nan_p()) {
        return false;
    } else {
        return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(b)) == 0;
    }
}

bool dispatch_real_equality_rational_impl(const real &r, const ::mpq_t q)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_q(r.get_mpfr_t(), q) == 0;
    }
}

bool dispatch_real_equality(const real &r, const float &x)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), static_cast<double>(x)) == 0;
    }
}

bool dispatch_real_equality(const real &r, const double &x)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), x) == 0;
    }
}

bool dispatch_real_equality(const real &r, const long double &x)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_ld(r.get_mpfr_t(), x) == 0;
    }
}

#if defined(MPPP_WITH_QUADMATH)

bool dispatch_real_equality(const real &r1, const real128 &r2)
{
    // NOTE: straight assignment here is fine: tmp
    // will represent r2 exactly.
    MPPP_MAYBE_TLS real tmp;
    tmp = r2;

    return dispatch_real_equality(r1, tmp);
}

#endif

} // namespace detail

// Implementation details for gt.
namespace detail
{

bool dispatch_real_gt(const real &x, const real &y)
{
    return ::mpfr_greater_p(x.get_mpfr_t(), y.get_mpfr_t()) != 0;
}

bool dispatch_real_gt_integer_impl(const real &r, const ::mpz_t n)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_z(r.get_mpfr_t(), n) > 0;
    }
}

bool dispatch_real_gt_integer_impl(const ::mpz_t n, const real &r)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_z(r.get_mpfr_t(), n) < 0;
    }
}

bool dispatch_real_gt(const real &r, bool b)
{
    if (r.nan_p()) {
        return false;
    } else {
        return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(b)) > 0;
    }
}

bool dispatch_real_gt(bool b, const real &r)
{
    if (r.nan_p()) {
        return false;
    } else {
        return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(b)) < 0;
    }
}

bool dispatch_real_gt_rational_impl(const real &r, const ::mpq_t q)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_q(r.get_mpfr_t(), q) > 0;
    }
}

bool dispatch_real_gt_rational_impl(const ::mpq_t q, const real &r)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_q(r.get_mpfr_t(), q) < 0;
    }
}

bool dispatch_real_gt(const real &r, const float &x)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), static_cast<double>(x)) > 0;
    }
}

bool dispatch_real_gt(const real &r, const double &x)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), x) > 0;
    }
}

bool dispatch_real_gt(const real &r, const long double &x)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_ld(r.get_mpfr_t(), x) > 0;
    }
}

bool dispatch_real_gt(const float &x, const real &r)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), static_cast<double>(x)) < 0;
    }
}

bool dispatch_real_gt(const double &x, const real &r)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), x) < 0;
    }
}

bool dispatch_real_gt(const long double &x, const real &r)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_ld(r.get_mpfr_t(), x) < 0;
    }
}

#if defined(MPPP_WITH_QUADMATH)

bool dispatch_real_gt(const real &r1, const real128 &r2)
{
    // NOTE: straight assignment here is fine: tmp
    // will represent r2 exactly.
    MPPP_MAYBE_TLS real tmp;
    tmp = r2;

    return dispatch_real_gt(r1, tmp);
}

bool dispatch_real_gt(const real128 &r1, const real &r2)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = r1;

    return dispatch_real_gt(tmp, r2);
}

#endif

} // namespace detail

// Implementation details for gte.
namespace detail
{

bool dispatch_real_gte(const real &x, const real &y)
{
    return ::mpfr_greaterequal_p(x.get_mpfr_t(), y.get_mpfr_t()) != 0;
}

bool dispatch_real_gte_integer_impl(const real &r, const ::mpz_t n)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_z(r.get_mpfr_t(), n) >= 0;
    }
}

bool dispatch_real_gte_integer_impl(const ::mpz_t n, const real &r)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_z(r.get_mpfr_t(), n) <= 0;
    }
}

bool dispatch_real_gte(const real &r, bool b)
{
    if (r.nan_p()) {
        return false;
    } else {
        return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(b)) >= 0;
    }
}

bool dispatch_real_gte(bool b, const real &r)
{
    if (r.nan_p()) {
        return false;
    } else {
        return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(b)) <= 0;
    }
}

bool dispatch_real_gte_rational_impl(const real &r, const ::mpq_t q)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_q(r.get_mpfr_t(), q) >= 0;
    }
}

bool dispatch_real_gte_rational_impl(const ::mpq_t q, const real &r)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_q(r.get_mpfr_t(), q) <= 0;
    }
}

bool dispatch_real_gte(const real &r, const float &x)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), static_cast<double>(x)) >= 0;
    }
}

bool dispatch_real_gte(const real &r, const double &x)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), x) >= 0;
    }
}

bool dispatch_real_gte(const real &r, const long double &x)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_ld(r.get_mpfr_t(), x) >= 0;
    }
}

bool dispatch_real_gte(const float &x, const real &r)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), static_cast<double>(x)) <= 0;
    }
}

bool dispatch_real_gte(const double &x, const real &r)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), x) <= 0;
    }
}

bool dispatch_real_gte(const long double &x, const real &r)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_ld(r.get_mpfr_t(), x) <= 0;
    }
}

#if defined(MPPP_WITH_QUADMATH)

bool dispatch_real_gte(const real &r1, const real128 &r2)
{
    // NOTE: straight assignment here is fine: tmp
    // will represent r2 exactly.
    MPPP_MAYBE_TLS real tmp;
    tmp = r2;

    return dispatch_real_gte(r1, tmp);
}

bool dispatch_real_gte(const real128 &r1, const real &r2)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = r1;

    return dispatch_real_gte(tmp, r2);
}

#endif

} // namespace detail

// Implementation details for lt.
namespace detail
{

bool dispatch_real_lt(const real &x, const real &y)
{
    return ::mpfr_less_p(x.get_mpfr_t(), y.get_mpfr_t()) != 0;
}

bool dispatch_real_lt_integer_impl(const real &r, const ::mpz_t n)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_z(r.get_mpfr_t(), n) < 0;
    }
}

bool dispatch_real_lt_integer_impl(const ::mpz_t n, const real &r)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_z(r.get_mpfr_t(), n) > 0;
    }
}

bool dispatch_real_lt(const real &r, bool b)
{
    if (r.nan_p()) {
        return false;
    } else {
        return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(b)) < 0;
    }
}

bool dispatch_real_lt(bool b, const real &r)
{
    if (r.nan_p()) {
        return false;
    } else {
        return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(b)) > 0;
    }
}

bool dispatch_real_lt_rational_impl(const real &r, const ::mpq_t q)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_q(r.get_mpfr_t(), q) < 0;
    }
}

bool dispatch_real_lt_rational_impl(const ::mpq_t q, const real &r)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_q(r.get_mpfr_t(), q) > 0;
    }
}

bool dispatch_real_lt(const real &r, const float &x)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), static_cast<double>(x)) < 0;
    }
}

bool dispatch_real_lt(const real &r, const double &x)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), x) < 0;
    }
}

bool dispatch_real_lt(const real &r, const long double &x)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_ld(r.get_mpfr_t(), x) < 0;
    }
}

bool dispatch_real_lt(const float &x, const real &r)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), static_cast<double>(x)) > 0;
    }
}

bool dispatch_real_lt(const double &x, const real &r)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), x) > 0;
    }
}

bool dispatch_real_lt(const long double &x, const real &r)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_ld(r.get_mpfr_t(), x) > 0;
    }
}

#if defined(MPPP_WITH_QUADMATH)

bool dispatch_real_lt(const real &r1, const real128 &r2)
{
    // NOTE: straight assignment here is fine: tmp
    // will represent r2 exactly.
    MPPP_MAYBE_TLS real tmp;
    tmp = r2;

    return dispatch_real_lt(r1, tmp);
}

bool dispatch_real_lt(const real128 &r1, const real &r2)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = r1;

    return dispatch_real_lt(tmp, r2);
}

#endif

} // namespace detail

// Implementation details for lte.
namespace detail
{

bool dispatch_real_lte(const real &x, const real &y)
{
    return ::mpfr_lessequal_p(x.get_mpfr_t(), y.get_mpfr_t()) != 0;
}

bool dispatch_real_lte_integer_impl(const real &r, const ::mpz_t n)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_z(r.get_mpfr_t(), n) <= 0;
    }
}

bool dispatch_real_lte_integer_impl(const ::mpz_t n, const real &r)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_z(r.get_mpfr_t(), n) >= 0;
    }
}

bool dispatch_real_lte(const real &r, bool b)
{
    if (r.nan_p()) {
        return false;
    } else {
        return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(b)) <= 0;
    }
}

bool dispatch_real_lte(bool b, const real &r)
{
    if (r.nan_p()) {
        return false;
    } else {
        return mpfr_cmp_ui(r.get_mpfr_t(), static_cast<unsigned long>(b)) >= 0;
    }
}

bool dispatch_real_lte_rational_impl(const real &r, const ::mpq_t q)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_q(r.get_mpfr_t(), q) <= 0;
    }
}

bool dispatch_real_lte_rational_impl(const ::mpq_t q, const real &r)
{
    if (r.nan_p()) {
        return false;
    } else {
        return ::mpfr_cmp_q(r.get_mpfr_t(), q) >= 0;
    }
}

bool dispatch_real_lte(const real &r, const float &x)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), static_cast<double>(x)) <= 0;
    }
}

bool dispatch_real_lte(const real &r, const double &x)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), x) <= 0;
    }
}

bool dispatch_real_lte(const real &r, const long double &x)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_ld(r.get_mpfr_t(), x) <= 0;
    }
}

bool dispatch_real_lte(const float &x, const real &r)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), static_cast<double>(x)) >= 0;
    }
}

bool dispatch_real_lte(const double &x, const real &r)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_d(r.get_mpfr_t(), x) >= 0;
    }
}

bool dispatch_real_lte(const long double &x, const real &r)
{
    if (r.nan_p() || std::isnan(x)) {
        return false;
    } else {
        return ::mpfr_cmp_ld(r.get_mpfr_t(), x) >= 0;
    }
}

#if defined(MPPP_WITH_QUADMATH)

bool dispatch_real_lte(const real &r1, const real128 &r2)
{
    // NOTE: straight assignment here is fine: tmp
    // will represent r2 exactly.
    MPPP_MAYBE_TLS real tmp;
    tmp = r2;

    return dispatch_real_lte(r1, tmp);
}

bool dispatch_real_lte(const real128 &r1, const real &r2)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = r1;

    return dispatch_real_lte(tmp, r2);
}

#endif

} // namespace detail

// Three-way comparison.
int cmp(const real &a, const real &b)
{
    ::mpfr_clear_erangeflag();
    auto retval = mpfr_cmp(a.get_mpfr_t(), b.get_mpfr_t());
    if (mppp_unlikely(::mpfr_erangeflag_p())) {
        ::mpfr_clear_erangeflag();
        throw std::domain_error("Cannot compare two reals if at least one of them is NaN");
    }
    return retval;
}

// Three-way comparison of absolute values.
int cmpabs(const real &a, const real &b)
{
    ::mpfr_clear_erangeflag();
    auto retval = ::mpfr_cmpabs(a.get_mpfr_t(), b.get_mpfr_t());
    if (mppp_unlikely(::mpfr_erangeflag_p())) {
        ::mpfr_clear_erangeflag();
        throw std::domain_error("Cannot compare the absolute values of two reals if at least one of them is NaN");
    }
    return retval;
}

// Comparison with n*2**e.
int cmp_ui_2exp(const real &x, unsigned long n, ::mpfr_exp_t e)
{
    ::mpfr_clear_erangeflag();
    auto retval = ::mpfr_cmp_ui_2exp(x.get_mpfr_t(), n, e);
    if (mppp_unlikely(::mpfr_erangeflag_p())) {
        ::mpfr_clear_erangeflag();
        throw std::domain_error("Cannot compare a real NaN to an integral multiple of a power of 2");
    }
    return retval;
}

int cmp_si_2exp(const real &x, long n, ::mpfr_exp_t e)
{
    ::mpfr_clear_erangeflag();
    auto retval = ::mpfr_cmp_si_2exp(x.get_mpfr_t(), n, e);
    if (mppp_unlikely(::mpfr_erangeflag_p())) {
        ::mpfr_clear_erangeflag();
        throw std::domain_error("Cannot compare a real NaN to an integral multiple of a power of 2");
    }
    return retval;
}

// Equality predicate with special NaN handling.
bool real_equal_to(const real &a, const real &b)
{
    const bool a_nan = a.nan_p(), b_nan = b.nan_p();
    return (!a_nan && !b_nan) ? (::mpfr_equal_p(a.get_mpfr_t(), b.get_mpfr_t()) != 0) : (a_nan && b_nan);
}

// Less-than predicate with special NaN/moved handling.
bool real_lt(const real &a, const real &b)
{
    if (!a.is_valid()) {
        // a is moved-from, consider it the largest possible value.
        return false;
    }
    if (!b.is_valid()) {
        // a is not moved-from, b is. a is smaller.
        return true;
    }
    const bool a_nan = a.nan_p();
    return (!a_nan && !b.nan_p()) ? (::mpfr_less_p(a.get_mpfr_t(), b.get_mpfr_t()) != 0) : !a_nan;
}

// Greater-than predicate with special NaN/moved handling.
bool real_gt(const real &a, const real &b)
{
    if (!b.is_valid()) {
        // b is moved-from, nothing can be bigger.
        return false;
    }
    if (!a.is_valid()) {
        // b is not moved-from, a is. a is bigger.
        return true;
    }
    const bool b_nan = b.nan_p();
    return (!a.nan_p() && !b_nan) ? (::mpfr_greater_p(a.get_mpfr_t(), b.get_mpfr_t()) != 0) : !b_nan;
}

// Output stream operator.
std::ostream &operator<<(std::ostream &os, const real &r)
{
    // Get the stream width.
    const auto width = os.width();

    // Fetch the stream's flags.
    const auto flags = os.flags();

    // Check if we are using scientific, fixed or both.
    const auto scientific = (flags & std::ios_base::scientific) != 0;
    const auto fixed = (flags & std::ios_base::fixed) != 0;
    const auto hexfloat = scientific && fixed;

    // Force decimal point character?
    const auto showpoint = (flags & std::ios_base::showpoint) != 0;

    // Force the plus sign?
    const bool with_plus = (flags & std::ios_base::showpos) != 0;

    // Uppercase?
    const bool uppercase = (flags & std::ios_base::uppercase) != 0;

    // Fetch the precision too.
    auto precision = os.precision();
    // NOTE: if the precision is negative, reset it
    // to the default value of 6.
    if (precision < 0) {
        precision = 6;
    }

    // Put together the format string.
    std::ostringstream oss;
    oss.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    oss.imbue(std::locale::classic());

    oss << '%';

    if (showpoint) {
        oss << '#';
    }

    if (with_plus) {
        oss << '+';
    }

    if (hexfloat) {
        // NOTE: in hexfloat mode, we want to ignore the precision
        // setting in the stream object and print the exact representation:
        // https://en.cppreference.com/w/cpp/locale/num_put/put
        oss << 'R';
    } else {
        oss << '.' << precision << 'R';
    }

    if (hexfloat) {
        oss << (uppercase ? 'A' : 'a');
    } else if (scientific) {
        oss << (uppercase ? 'E' : 'e');
    } else if (fixed) {
        // NOTE: in fixed format, the uppercase
        // setting is ignored for floating-point types:
        // https://en.cppreference.com/w/cpp/locale/num_put/put
        oss << 'f';
    } else {
        oss << (uppercase ? 'G' : 'g');
    }

    const auto fmt_str = oss.str();

    // Print out via the MPFR printf() function.
    char *out_str = nullptr;
    const auto ret = ::mpfr_asprintf(&out_str, fmt_str.c_str(), r.get_mpfr_t());
    if (ret == -1) {
        // LCOV_EXCL_START
        // NOTE: the MPFR docs state that if printf() returns -1, the errno variable and erange
        // flag are set. Let's clear them out before throwing.
        errno = 0;
        ::mpfr_clear_erangeflag();

        throw std::invalid_argument("The mpfr_asprintf() function returned the error code -1");
        // LCOV_EXCL_STOP
    }

    // printf() was successful, let's set up the RAII machinery to automatically
    // clear out_str before anything else.
    // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
    struct out_str_cleaner {
        ~out_str_cleaner()
        {
            ::mpfr_free_str(ptr);
        }
        char *ptr;
    };

    out_str_cleaner osc{out_str};

    // NOTE: I am not sure if it is possible to get a string of size 0
    // from a printf() function. Let's throw for the time being in such a case,
    // as zero-length output complicates indexing below.
    if (ret == 0) {
        // LCOV_EXCL_START
        throw std::invalid_argument("The mpfr_asprintf() function returned an empty string");
        // LCOV_EXCL_STOP
    }

    // We are going to do the filling
    // only if the stream width is larger
    // than the total size of the string repr.
    // NOTE: width > ret already implies width >= 0.
    if (width > ret) {
        // Create a std::string from the string outout
        // of printf().
        std::string tmp(out_str, out_str + ret);

        // Determine the fill type.
        const auto fill = detail::stream_flags_to_fill(flags);

        // Compute how much fill we need.
        const auto fill_size = detail::safe_cast<decltype(tmp.size())>(width - ret);

        // Get the fill character.
        const auto fill_char = os.fill();

        switch (fill) {
            case 1:
                // Left fill: fill characters at the end.
                tmp.insert(tmp.end(), fill_size, fill_char);
                break;
            case 2:
                // Right fill: fill characters at the beginning.
                tmp.insert(tmp.begin(), fill_size, fill_char);
                break;
            default: {
                assert(fill == 3);

                // Internal fill: the fill characters are always after the sign (if present).
                const auto delta = static_cast<int>(tmp[0] == '+' || tmp[0] == '-');
                tmp.insert(tmp.begin() + delta, fill_size, fill_char);
                break;
            }
        }

        os.write(tmp.data(), detail::safe_cast<std::streamsize>(tmp.size()));
    } else {
        os.write(out_str, detail::safe_cast<std::streamsize>(ret));
    }

    // Reset the stream width to zero, like the operator<<() does for builtin types.
    // https://en.cppreference.com/w/cpp/io/manip/setw
    // Do it here so we ensure we don't alter the state of the stream until the very end.
    os.width(0);

    return os;
}

namespace detail
{

// NOTE: don't put in unnamed namespace as
// this needs do be just in detail:: for friendship
// with real.
template <typename F>
inline real real_constant(const F &f, ::mpfr_prec_t p)
{
    if (mppp_unlikely(!real_prec_check(p))) {
        throw std::invalid_argument("Cannot init a real constant with a precision of " + detail::to_string(p)
                                    + ": the value must be between " + detail::to_string(real_prec_min()) + " and "
                                    + detail::to_string(real_prec_max()));
    }

    real retval{real::ptag{}, p, true};
    f(retval._get_mpfr_t(), MPFR_RNDN);
    return retval;
}

} // namespace detail

// Pi constant.
real real_pi(::mpfr_prec_t p)
{
    return detail::real_constant(::mpfr_const_pi, p);
}

real &real_pi(real &rop)
{
    ::mpfr_const_pi(rop._get_mpfr_t(), MPFR_RNDN);
    return rop;
}

real real_log2(::mpfr_prec_t p)
{
    return detail::real_constant(::mpfr_const_log2, p);
}

real &real_log2(real &rop)
{
    ::mpfr_const_log2(rop._get_mpfr_t(), MPFR_RNDN);
    return rop;
}

real real_euler(::mpfr_prec_t p)
{
    return detail::real_constant(::mpfr_const_euler, p);
}

real &real_euler(real &rop)
{
    ::mpfr_const_euler(rop._get_mpfr_t(), MPFR_RNDN);
    return rop;
}

real real_catalan(::mpfr_prec_t p)
{
    return detail::real_constant(::mpfr_const_catalan, p);
}

real &real_catalan(real &rop)
{
    ::mpfr_const_catalan(rop._get_mpfr_t(), MPFR_RNDN);
    return rop;
}

namespace detail
{

namespace
{

// std::size_t addition with overflow checking.
std::size_t rbs_checked_add(std::size_t a, std::size_t b)
{
    // LCOV_EXCL_START
    if (mppp_unlikely(a > std::numeric_limits<std::size_t>::max() - b)) {
        throw std::overflow_error("Overflow detected in the computation of the binary size of a real");
    }
    // LCOV_EXCL_STOP

    return a + b;
}

// Turn an MPFR precision into the number of limbs
// necessary to represent the significand of
// a number with that precision.
std::size_t rbs_prec_to_nlimbs(::mpfr_prec_t p)
{
    // NOTE: currently both mpfr_prec_t and GMP_NUMB_BITS
    // are signed.
    using uprec_t = std::make_unsigned<::mpfr_prec_t>::type;
    return safe_cast<std::size_t>(static_cast<uprec_t>(p / GMP_NUMB_BITS + static_cast<int>((p % GMP_NUMB_BITS) != 0)));
}

// Turn an MPFR precision into the number of bytes
// necessary to represent the significand of
// a number with that precision.
std::size_t rbs_prec_to_size(::mpfr_prec_t p)
{
    const auto nlimbs = rbs_prec_to_nlimbs(p);

    // LCOV_EXCL_START
    if (mppp_unlikely(nlimbs > std::numeric_limits<std::uint32_t>::max() / sizeof(::mp_limb_t))) {
        throw std::overflow_error("Overflow detected in the computation of the binary size of a real");
    }
    // LCOV_EXCL_STOP

    return nlimbs * sizeof(::mp_limb_t);
}

// The base binary size of a real: accounts for the size of precision,
// sign bit and exponent.
std::size_t rbs_base_size()
{
    // Precision.
    auto retval = sizeof(::mpfr_prec_t);
    // Sign bit.
    retval = rbs_checked_add(retval, sizeof(::mpfr_sign_t));
    // Exponent.
    return rbs_checked_add(retval, sizeof(::mpfr_exp_t));
}

// The minimum binary size of a real (a real must have at least 1 limb
// of data).
std::size_t rbs_min_size()
{
    return rbs_checked_add(rbs_base_size(), sizeof(::mp_limb_t));
}

// Load the serializaed precision value from a char buffer.
::mpfr_prec_t rbs_read_prec(const char *src)
{
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    ::mpfr_prec_t p;
    std::copy(src, src + sizeof(p), reinterpret_cast<char *>(&p));
    return p;
}

} // namespace

} // namespace detail

#if defined(MPPP_MPFR_HAVE_MPFR_GET_STR_NDIGITS)

// Get the number of significant digits required for a round-tripping representation.
std::size_t real::get_str_ndigits(int base) const
{
    if (mppp_unlikely(base < 2 || base > 62)) {
        throw std::invalid_argument(
            "Invalid base value for get_str_ndigits(): the base must be in the [2,62] range, but it is "
            + std::to_string(base) + " instead");
    }

    return ::mpfr_get_str_ndigits(base, get_prec());
}

std::size_t get_str_ndigits(const real &r, int base)
{
    return r.get_str_ndigits(base);
}

#endif

// Size of the serialised binary representation: base size + limbs data.
std::size_t real::binary_size() const
{
    return detail::rbs_checked_add(detail::rbs_base_size(), detail::rbs_prec_to_size(get_prec()));
}

std::size_t binary_size(const real &x)
{
    return x.binary_size();
}

// Save to a char buffer, given a binary size computed
// via binary_size().
void real::binary_save_impl(char *dest, std::size_t bs) const
{
    // Save the precision.
    std::copy(reinterpret_cast<const char *>(&m_mpfr._mpfr_prec),
              reinterpret_cast<const char *>(&m_mpfr._mpfr_prec) + sizeof(::mpfr_prec_t), dest);
    dest += sizeof(::mpfr_prec_t);

    // Save the sign bit.
    std::copy(reinterpret_cast<const char *>(&m_mpfr._mpfr_sign),
              reinterpret_cast<const char *>(&m_mpfr._mpfr_sign) + sizeof(::mpfr_sign_t), dest);
    dest += sizeof(::mpfr_sign_t);

    // Save the exponent.
    std::copy(reinterpret_cast<const char *>(&m_mpfr._mpfr_exp),
              reinterpret_cast<const char *>(&m_mpfr._mpfr_exp) + sizeof(::mpfr_exp_t), dest);
    dest += sizeof(::mpfr_exp_t);

    // Save the significand.
    std::copy(reinterpret_cast<const char *>(m_mpfr._mpfr_d),
              reinterpret_cast<const char *>(m_mpfr._mpfr_d) + (bs - detail::rbs_base_size()), dest);
}

// Save to a char buffer.
std::size_t real::binary_save(char *dest) const
{
    const auto bs = binary_size();
    binary_save_impl(dest, bs);
    return bs;
}

// Save to a std::vector buffer.
std::size_t real::binary_save(std::vector<char> &dest) const
{
    const auto bs = binary_size();
    if (dest.size() < bs) {
        dest.resize(detail::safe_cast<decltype(dest.size())>(bs));
    }
    binary_save_impl(dest.data(), bs);
    return bs;
}

// Save to a stream.
std::size_t real::binary_save(std::ostream &dest) const
{
    const auto bs = binary_size();
    // NOTE: there does not seem to be a reliable way of detecting how many bytes
    // are actually written via write(). See the question here and especially the comments:
    // https://stackoverflow.com/questions/14238572/how-many-bytes-actually-written-by-ostreamwrite
    // Seems almost like tellp() would work, but if an error occurs in the stream, then
    // it returns unconditionally -1, so it is not very useful for our purposes.
    // Thus, we will just return 0 on failure, and the full binary size otherwise.
    //
    // Write the raw data to stream.
    dest.write(reinterpret_cast<const char *>(&m_mpfr._mpfr_prec),
               detail::safe_cast<std::streamsize>(sizeof(::mpfr_prec_t)));
    if (!dest.good()) {
        // !dest.good() means that the last write operation failed. Bail out now.
        return 0;
    }

    dest.write(reinterpret_cast<const char *>(&m_mpfr._mpfr_sign),
               detail::safe_cast<std::streamsize>(sizeof(::mpfr_sign_t)));
    if (!dest.good()) {
        // LCOV_EXCL_START
        return 0;
        // LCOV_EXCL_STOP
    }

    dest.write(reinterpret_cast<const char *>(&m_mpfr._mpfr_exp),
               detail::safe_cast<std::streamsize>(sizeof(::mpfr_exp_t)));
    if (!dest.good()) {
        // LCOV_EXCL_START
        return 0;
        // LCOV_EXCL_STOP
    }

    dest.write(reinterpret_cast<const char *>(m_mpfr._mpfr_d),
               detail::safe_cast<std::streamsize>(bs - detail::rbs_base_size()));
    return dest.good() ? bs : 0u;
}

// Load from a char buffer of unknown size.
std::size_t real::binary_load_impl(const char *src)
{
    // Fetch the precision first.
    const auto p = detail::rbs_read_prec(src);
    src += sizeof(p);

    // Fetch the sign bit.
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    ::mpfr_sign_t sb;
    std::copy(src, src + sizeof(sb), reinterpret_cast<char *>(&sb));
    src += sizeof(sb);

    // Fetch the exponent.
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    ::mpfr_exp_t e;
    std::copy(src, src + sizeof(e), reinterpret_cast<char *>(&e));
    src += sizeof(e);

    // Compute the size in bytes of the significand from the precision.
    const auto sbs = detail::rbs_prec_to_size(p);

    // Compute the return value.
    auto retval = detail::rbs_checked_add(detail::rbs_base_size(), sbs);

    // Set the precision of this.
    set_prec(p);
    // NOTE: from now on, everything is noexcept.
    // Set the sign bit.
    m_mpfr._mpfr_sign = sb;
    // Set the exponent.
    m_mpfr._mpfr_exp = e;
    // Copy over the limbs.
    std::copy(src, src + sbs, reinterpret_cast<char *>(m_mpfr._mpfr_d));

    return retval;
}

// Load from a char buffer of size 'size' belonging to a container of type 'name'.
std::size_t real::binary_load_impl(const char *src, std::size_t size, const char *name)
{
    if (mppp_unlikely(size < detail::rbs_min_size())) {
        throw std::invalid_argument(std::string("Invalid size detected in the deserialisation of a real via a ") + name
                                    + ": the " + name + " size must be at least "
                                    + std::to_string(detail::rbs_min_size()) + " bytes, but it is only "
                                    + std::to_string(size) + " bytes");
    }

    // Read the precision.
    const auto p = detail::rbs_read_prec(src);

    // Determine the binary size from the loaded precision.
    const auto expected_size = detail::rbs_checked_add(detail::rbs_base_size(), detail::rbs_prec_to_size(p));

    // Check that the buffer contains at least the expected amount of data.
    if (mppp_unlikely(size < expected_size)) {
        throw std::invalid_argument(std::string("Invalid size detected in the deserialisation of a real via a ") + name
                                    + ": the " + name + " size must be at least " + std::to_string(expected_size)
                                    + " bytes, but it is only " + std::to_string(size) + " bytes");
    }

    return binary_load_impl(src);
}

// Load from a char buffer of unknown size.
std::size_t real::binary_load(const char *src)
{
    return binary_load_impl(src);
}

// Load from a std::vector.
std::size_t real::binary_load(const std::vector<char> &v)
{
    return binary_load_impl(v.data(), detail::safe_cast<std::size_t>(v.size()), "std::vector");
}

// Load from an input stream.
std::size_t real::binary_load(std::istream &src)
{
    // Let's try to read the precision first.
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    ::mpfr_prec_t p;
    src.read(reinterpret_cast<char *>(&p), detail::safe_cast<std::streamsize>(sizeof(p)));
    if (!src.good()) {
        // Something went wrong with reading, return 0.
        return 0;
    }

    // The sign bit.
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    ::mpfr_sign_t sb;
    src.read(reinterpret_cast<char *>(&sb), detail::safe_cast<std::streamsize>(sizeof(sb)));
    if (!src.good()) {
        return 0;
    }

    // The exponent.
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    ::mpfr_exp_t e;
    src.read(reinterpret_cast<char *>(&e), detail::safe_cast<std::streamsize>(sizeof(e)));
    if (!src.good()) {
        return 0;
    }

    // Compute the size in bytes of the significand from the precision.
    const auto sbs = detail::rbs_prec_to_size(p);

    // Read the limb data into a local buffer.
    MPPP_MAYBE_TLS std::vector<char> buffer;
    buffer.resize(detail::safe_cast<decltype(buffer.size())>(sbs));
    src.read(buffer.data(), detail::safe_cast<std::streamsize>(sbs));
    if (!src.good()) {
        // Something went wrong with reading, return 0.
        return 0;
    }

    // Compute the return value.
    auto retval = detail::rbs_checked_add(detail::rbs_base_size(), sbs);

    // Set the precision of this.
    set_prec(p);
    // NOTE: from now on, everything is noexcept.
    // Set the sign bit.
    m_mpfr._mpfr_sign = sb;
    // Set the exponent.
    m_mpfr._mpfr_exp = e;
    // Copy over the limbs.
    std::copy(buffer.begin(), buffer.end(), reinterpret_cast<char *>(m_mpfr._mpfr_d));

    return retval;
}

#if defined(MPPP_WITH_BOOST_S11N)

// Fast serialization implementations for Boost's binary archives.
void real::save(boost::archive::binary_oarchive &ar, unsigned) const
{
    MPPP_MAYBE_TLS std::vector<char> buffer;
    binary_save(buffer);

    // Record the size and the raw data.
    ar << buffer.size();
    ar << boost::serialization::make_binary_object(buffer.data(), detail::safe_cast<std::size_t>(buffer.size()));
}

void real::load(boost::archive::binary_iarchive &ar, unsigned)
{
    MPPP_MAYBE_TLS std::vector<char> buffer;

    // Recover the size.
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    decltype(buffer.size()) s;
    ar >> s;
    buffer.resize(s);

    ar >> boost::serialization::make_binary_object(buffer.data(), detail::safe_cast<std::size_t>(buffer.size()));

    binary_load(buffer);
}

#endif

} // namespace mppp

#if defined(_MSC_VER)

#undef _SCL_SECURE_NO_WARNINGS

#endif
