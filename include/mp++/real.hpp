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

#if defined(MPPP_WITH_ARB)

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <mp++/concepts.hpp>
#include <mp++/detail/arb.hpp>
#include <mp++/detail/fwd_decl.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>

namespace mppp
{

inline namespace detail
{
// constexpr max/min implementations.
template <typename T>
constexpr T c_max(T a, T b)
{
    return a > b ? a : b;
}

template <typename T>
constexpr T c_min(T a, T b)
{
    return a < b ? a : b;
}

using mpfr_min_prec_t = std::make_unsigned<uncvref_t<decltype(MPFR_PREC_MIN)>>::type;

// Minimum precision allowed for real values. It's the max value between the minimum precs of mpfr and Arb,
// and guaranteed to be representable by slong (otherwise, a static assert will fire).
constexpr mpfr_min_prec_t real_min_prec_impl()
{
    // Min prec for Arb is 2.
    return c_max(static_cast<mpfr_min_prec_t>(MPFR_PREC_MIN), static_cast<mpfr_min_prec_t>(2));
}

static_assert(real_min_prec_impl() <= static_cast<std::make_unsigned<slong>::type>(std::numeric_limits<slong>::max()),
              "The minimum precision for real cannot be represented by slong.");

// Maximum precision allowed for real values. For MPFR there's a macro, for Arb the documentation suggests
// < 2**24 for 32-bit and < 2**36 for 64-bit.
// http://arblib.org/issues.html#integer-overflow
constexpr unsigned long long arb_max_prec()
{
    // We use slightly smaller max prec values for Arb.
    // NOTE: the docs of ulong state that it has exactly either 64 or 32 bit width.
    return std::numeric_limits<ulong>::digits == 64 ? (1ull << 32) : (1ull << 20);
}

constexpr unsigned long long real_max_prec_impl()
{
    return c_min(arb_max_prec(), static_cast<unsigned long long>(MPFR_PREC_MAX));
}

static_assert(real_max_prec_impl() <= static_cast<std::make_unsigned<slong>::type>(std::numeric_limits<slong>::max()),
              "The maximum precision for real cannot be represented by slong.");

constexpr slong real_min_prec()
{
    return static_cast<slong>(real_min_prec_impl());
}

constexpr slong real_max_prec()
{
    return static_cast<slong>(real_max_prec_impl());
}

static_assert(real_min_prec() <= real_max_prec(),
              "The minimum precision for real is larger than the maximum precision.");
}

template <typename T>
using is_real_interoperable = disjunction<is_cpp_interoperable<T>, is_integer<T>, is_rational<T>>;

template <typename T>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool RealInteroperable = is_real_interoperable<T>::value;
#else
using real_interoperable_enabler = enable_if_t<is_real_interoperable<T>::value, int>;
#endif

/// Multiprecision floating-point class.
/**
 * \rststar
 * This class represents arbitrary-precision floating-point values, that is, floating-point values whose
 * significand size and exponent range are limited only by the available memory.
 * The implementation is based on the `arf_t <http://arblib.org/arf.html#c.arf_t>`__ type from the
 * `Arb library <http://arblib.org>`__.
 *
 * The :cpp:class:`~mppp::real` class stores internally two data members:
 *
 * * an `arf_struct <http://arblib.org/arf.html#c.arf_t>`__, representing the floating-point value as a
 *   significand-exponent pair,
 * * an integral value representing the *precision* (in bits) associated to the :cpp:class:`~mppp::real` object.
 *
 * Contrary to other multiprecision floating-point libraries (e.g., MPFR), the precision of a
 * :cpp:class:`~mppp::real` is, in general, unrelated to the number of bits used by the significand. Rather,
 * it specifies the target precision of mathematical operations involving :cpp:class:`~mppp::real` objects.
 *
 * For instance, regardless of the selected precision, the significand of a :cpp:class:`~mppp::real` representing
 * the integral value :math:`2` always uses exactly 2 bits of storage (as :math:`2` can be represented exactly
 * using only 2 bits). The computation of :math:`\sqrt{2}`, on the other hand, will yield a :cpp:class:`~mppp::real`
 * object whose significand uses a number of bits equal (roughly) to the target precision: :math:`\sqrt{2}`
 * is an irrational number, thus :cpp:class:`~mppp::real` will
 * use all the bits specified by the desired precision in order to provide an approximation to the exact value as
 * accurate as possible.
 * \endrststar
 */
class real
{
public:
    /// Default constructor.
    /**
     * The default constructor will intialise the value to zero and the precision
     * to the minimum possible value (as indicated by min_prec()).
     */
    real()
    {
        ::arf_init(&m_arf);
        m_prec = min_prec();
    }
    /// Copy constructor.
    /**
     * The copy constructor will perform a deep copy of \p other (i.e., both the value
     * and the precision of \p other will be copied).
     *
     * @param other the value that will be copied into \p this.
     */
    real(const real &other)
    {
        ::arf_init(&m_arf);
        ::arf_set(&m_arf, &other.m_arf);
        m_prec = other.m_prec;
    }
    /// Move constructor.
    /**
     * Both the value and the precision of \p other will be moved into \p this.
     * After the move, \p other will be left in an unspecified but valid state.
     *
     * @param other the value that will be moved into \p this.
     */
    real(real &&other) noexcept
    {
        ::arf_init(&m_arf);
        ::arf_swap(&m_arf, &other.m_arf);
        m_prec = other.m_prec;
    }

private:
    template <std::size_t SSize>
    void dispatch_generic_ctor(const integer<SSize> &n, slong prec)
    {
        ::arf_init(&m_arf);
        ::arf_set_mpz(&m_arf, n.get_mpz_view());
        if (prec) {
            set_prec(prec);
        } else {
            // The precision will be set to the number of limbs multiplied by the effective bit width
            // of each limb.
            const auto ls = n.size();
            // Check we do not overflow the slong range.
            if (mppp_unlikely(ls > static_cast<std::make_unsigned<slong>::type>(std::numeric_limits<slong>::max())
                                       / unsigned(GMP_NUMB_BITS))) {
                throw std::overflow_error("Overflow in the computation of the precision for a real constructed from an "
                                          "integer - the limb size is "
                                          + std::to_string(ls));
            }
            set_prec(c_max(min_prec(), static_cast<slong>(ls) * static_cast<slong>(GMP_NUMB_BITS)));
        }
    }
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_unsigned<T>>::value, int> = 0>
    void dispatch_generic_ctor(T n, slong prec)
    {
        if (n <= std::numeric_limits<ulong>::max()) {
            ::arf_init_set_ui(&m_arf, static_cast<ulong>(n));
            static_assert(std::numeric_limits<T>::digits <= std::numeric_limits<slong>::max(), "Overflow error.");
            set_prec(prec ? prec : c_max(min_prec(), static_cast<slong>(std::numeric_limits<T>::digits)));
        } else {
            dispatch_generic_ctor(integer<1>{n}, prec);
        }
    }
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_signed<T>>::value, int> = 0>
    void dispatch_generic_ctor(T n, slong prec)
    {
        if (n <= std::numeric_limits<slong>::max() && n >= std::numeric_limits<slong>::min()) {
            ::arf_init_set_si(&m_arf, static_cast<slong>(n));
            static_assert(std::numeric_limits<T>::digits <= std::numeric_limits<slong>::max(), "Overflow error.");
            set_prec(prec ? prec : c_max(min_prec(), static_cast<slong>(std::numeric_limits<T>::digits)));
        } else {
            dispatch_generic_ctor(integer<1>{n}, prec);
        }
    }
    template <typename T, enable_if_t<disjunction<std::is_same<T, float>, std::is_same<T, double>>::value, int> = 0>
    void dispatch_generic_ctor(T x, slong prec)
    {
        ::arf_init(&m_arf);
        ::arf_set_d(&m_arf, static_cast<double>(x));
        static_assert(std::numeric_limits<T>::radix == 2, "The float/double type's radix is not 2.");
        static_assert(std::numeric_limits<T>::digits <= std::numeric_limits<slong>::max(), "Overflow error.");
        set_prec(prec ? prec : c_max(min_prec(), static_cast<slong>(std::numeric_limits<T>::digits)));
    }
    void dispatch_generic_ctor(long double x, slong prec)
    {
        // NOTE: static checks for overflows are done in mpfr.hpp.
        constexpr int d2 = std::numeric_limits<long double>::digits10 * 4;
        MPPP_MAYBE_TLS mpfr_raii mpfr(static_cast<::mpfr_prec_t>(d2));
        ::mpfr_set_ld(&mpfr.m_mpfr, x, MPFR_RNDN);
        ::arf_init(&m_arf);
        ::arf_set_mpfr(&m_arf, &mpfr.m_mpfr);
        static_assert(std::numeric_limits<long double>::radix == 2, "The long double type's radix is not 2.");
        static_assert(std::numeric_limits<long double>::digits <= std::numeric_limits<slong>::max(), "Overflow error.");
        set_prec(prec ? prec : c_max(min_prec(), static_cast<slong>(std::numeric_limits<long double>::digits)));
    }

public:
/// Generic constructor.
#if defined(MPPP_HAVE_CONCEPTS)
    explicit real(const RealInteroperable &x
#else
    template <typename T, real_interoperable_enabler<T> = 0>
    explicit real(const T &x
#endif
                  ,
                  slong prec = 0, bool round = false)
    {
        dispatch_generic_ctor(x, prec);
        if (prec && round) {
            ::arf_set_round(&m_arf, &m_arf, m_prec, ARF_RND_NEAR);
        }
    }
    /// Destructor.
    ~real()
    {
        ::arf_clear(&m_arf);
    }
    /// Get const reference to the internal structure.
    const ::arf_struct *get_arf_t() const
    {
        return &m_arf;
    }
    /// Get mutable reference to the internal structure.
    ::arf_struct *get_arf_t()
    {
        return &m_arf;
    }
    /// Get the precision.
    slong get_prec() const
    {
        return m_prec;
    }
    /// Set the precision.
    void set_prec(slong prec)
    {
        if (mppp_unlikely(prec > max_prec() || prec < min_prec())) {
            throw std::invalid_argument("An invalid precision of " + std::to_string(prec)
                                        + " was specified for a real object (the minimum allowed precision is "
                                        + std::to_string(min_prec()) + ", while the maximum allowed precision is "
                                        + std::to_string(max_prec()) + ")");
        }
        m_prec = prec;
    }
    /// Size in bits.
    slong nbits() const
    {
        return ::arf_bits(&m_arf);
    }
    /// Minimum precision.
    static constexpr slong min_prec()
    {
        return real_min_prec();
    }
    /// Maximum precision.
    static constexpr slong max_prec()
    {
        return real_max_prec();
    }

private:
    ::arf_struct m_arf;
    slong m_prec;
};

inline std::ostream &operator<<(std::ostream &os, const real &r)
{
    // Handle special values first.
    if (::arf_is_nan(r.get_arf_t())) {
        os << "nan";
        return os;
    }

    if (::arf_is_pos_inf(r.get_arf_t())) {
        os << "inf";
        return os;
    }

    if (::arf_is_neg_inf(r.get_arf_t())) {
        os << "-inf";
        return os;
    }

    // Setup a suitable mpfr real. We will set the actual precision in the following line.
    MPPP_MAYBE_TLS mpfr_raii mpfr(static_cast<::mpfr_prec_t>(real::min_prec()));
    // NOTE: the precision of r is always guaranteed to be a valid precision for both MPFR and arb.
    ::mpfr_set_prec(&mpfr.m_mpfr, static_cast<::mpfr_prec_t>(r.get_prec()));
    // Extract an mpfr from the arf.
    ::arf_get_mpfr(&mpfr.m_mpfr, r.get_arf_t(), MPFR_RNDN);

    // Get the string fractional representation via the MPFR function,
    // and wrap it into a smart pointer.
    ::mpfr_exp_t exp(0);
    smart_mpfr_str str(::mpfr_get_str(nullptr, &exp, 10, 0, &mpfr.m_mpfr, MPFR_RNDN), ::mpfr_free_str);
    if (mppp_unlikely(!str)) {
        throw std::runtime_error("Error in the stream operator of real: the call to mpfr_get_str() failed");
    }

    // Print the string, inserting a decimal point after the first digit.
    bool dot_added = false;
    for (auto cptr = str.get(); *cptr != '\0'; ++cptr) {
        os << (*cptr);
        // NOTE: check this answer:
        // http://stackoverflow.com/questions/13827180/char-ascii-relation
        // """
        // The mapping of integer values for characters does have one guarantee given
        // by the Standard: the values of the decimal digits are continguous.
        // (i.e., '1' - '0' == 1, ... '9' - '0' == 9)
        // """
        if (!dot_added && *cptr >= '0' && *cptr <= '9') {
            os << '.';
            dot_added = true;
        }
    }
    assert(dot_added);

    // Adjust the exponent.
    if (mppp_unlikely(exp == std::numeric_limits<::mpfr_exp_t>::min())) {
        throw std::overflow_error("Overflow in the conversion of a real to string");
    }
    --exp;
    if (exp && !mpfr_zero_p(&mpfr.m_mpfr)) {
        // Add the exponent at the end of the string, if both the value and the exponent
        // are nonzero.
        os << "e" << exp;
    }
    return os;
}
}

#else

#error The real.hpp header was included but mp++ was not configured with the MPPP_WITH_ARB option.

#endif

#endif
