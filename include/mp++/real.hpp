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

// Utility function to convert an arf to an mpfr with checks on the exponent range (arf has multiprecision
// exponents, mpfr has fixed precision exponents).
inline void arf_to_mpfr(::mpfr_t m, const ::arf_t a)
{
    // Get the min/max exponents allowed in MPFR.
    const ::mpfr_exp_t e_min = ::mpfr_get_emin(), e_max = ::mpfr_get_emax();
    // We need to make sure the exponent of a is within range.
    if (mppp_unlikely(e_min < std::numeric_limits<slong>::min() || e_max > std::numeric_limits<slong>::max()
                      || ::fmpz_cmp_si(&a->exp, static_cast<slong>(e_min)) < 0
                      || ::fmpz_cmp_si(&a->exp, static_cast<slong>(e_max)) > 0)) {
        throw std::invalid_argument("In the conversion of an arf_t to an mpfr_t, the exponent of the arf_t object "
                                    "overflows the exponent range of MPFR (the minimum allowed MPFR exponent is "
                                    + std::to_string(e_min) + ", the maximum is " + std::to_string(e_max) + ")");
    }
    // Extract an mpfr from the arf.
    ::arf_get_mpfr(m, a, MPFR_RNDN);
}
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
 * it specifies the target precision of operations involving :cpp:class:`~mppp::real` objects.
 * See the documentation of the individual methods and functions for a detailed explanation of how the precision
 * attribute is used in different situations.
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
        if (prec) {
            // prec is given explicitly. Attempt to set it (this will check it as well).
            set_prec(prec);
            // Init and set the mpz with the desired rounding. All noexcept.
            ::arf_init(&m_arf);
            ::arf_set_round_mpz(&m_arf, n.get_mpz_view(), get_prec(), ARF_RND_NEAR);
        } else {
            // Compute the number of bits used by n.
            const auto ls = n.size();
            // Check that ls * GMP_NUMB_BITS <= max_prec.
            if (mppp_unlikely(ls
                              > static_cast<std::make_unsigned<slong>::type>(max_prec()) / unsigned(GMP_NUMB_BITS))) {
                throw std::invalid_argument(
                    "The deduced precision for a real constructed from an integer is too large");
            }
            // Set the precision directly. We already know it's a non-negative value not greater
            // than the max allowed precision. We just need to make sure it's not smaller than the
            // min allowed precision.
            m_prec = c_max(static_cast<slong>(static_cast<slong>(ls) * int(GMP_NUMB_BITS)), min_prec());
            // Init and set the mpz. All noexcept.
            ::arf_init(&m_arf);
            ::arf_set_mpz(&m_arf, n.get_mpz_view());
        }
    }
    template <std::size_t SSize>
    void dispatch_generic_ctor(const rational<SSize> &q, slong prec)
    {
        // Setup a temporary mpfr real. We will set the actual precision later as needed.
        MPPP_MAYBE_TLS mpfr_raii mpfr(static_cast<::mpfr_prec_t>(min_prec()));

        if (prec) {
            // prec is given explicitly. Attempt to set it (this will check it as well).
            set_prec(prec);
        } else {
            // Compute the number of bits used by q.
            const auto n_size = q.get_num().size();
            const auto d_size = q.get_den().size();
            const auto tot_size = n_size + d_size;
            // Size checks.
            if (mppp_unlikely(
                    // Overflow in tot_size.
                    (n_size > std::numeric_limits<decltype(q.get_num().size())>::max() - d_size)
                    // Check that tot_size * GMP_NUMB_BITS <= max_prec.
                    || (tot_size
                        > static_cast<std::make_unsigned<slong>::type>(max_prec()) / unsigned(GMP_NUMB_BITS)))) {
                throw std::invalid_argument(
                    "The deduced precision for a real constructed from a rational is too large");
            }
            // Set the precision directly. We already know it's a non-negative value not greater
            // than the max allowed precision. We just need to make sure it's not smaller than the
            // min allowed precision.
            m_prec = c_max(static_cast<slong>(static_cast<slong>(tot_size) * int(GMP_NUMB_BITS)), min_prec());
        }

        // Set the precision of the tmp mpfr.
        ::mpfr_set_prec(&mpfr.m_mpfr, static_cast<::mpfr_prec_t>(get_prec()));
        // mpq to mpfr.
        ::mpfr_set_q(&mpfr.m_mpfr, q.get_mpq_view(), MPFR_RNDN);
        // mpfr to arf. All noexcept from now on.
        ::arf_init(&m_arf);
        ::arf_set_mpfr(&m_arf, &mpfr.m_mpfr);
    }
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_unsigned<T>>::value, int> = 0>
    void dispatch_generic_ctor(T n, slong prec)
    {
        if (n <= std::numeric_limits<ulong>::max()) {
            if (prec) {
                // Check and set the desired precision.
                set_prec(prec);
                // Init and then set with rounding. All noexcept.
                ::arf_init(&m_arf);
                ::arf_set_round_ui(&m_arf, static_cast<ulong>(n), get_prec(), ARF_RND_NEAR);
            } else {
                static_assert(std::numeric_limits<T>::digits <= std::numeric_limits<slong>::max(), "Overflow error.");
                // Check and set the detected precision.
                set_prec(c_max(min_prec(), static_cast<slong>(std::numeric_limits<T>::digits)));
                // Init and set exactly.
                ::arf_init_set_ui(&m_arf, static_cast<ulong>(n));
            }
        } else {
            dispatch_generic_ctor(integer<1>{n}, prec);
        }
    }
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_signed<T>>::value, int> = 0>
    void dispatch_generic_ctor(T n, slong prec)
    {
        if (n <= std::numeric_limits<slong>::max() && n >= std::numeric_limits<slong>::min()) {
            if (prec) {
                // Check and set the desired precision.
                set_prec(prec);
                // Init and then set with rounding. All noexcept.
                ::arf_init(&m_arf);
                ::arf_set_round_si(&m_arf, static_cast<slong>(n), get_prec(), ARF_RND_NEAR);
            } else {
                static_assert(std::numeric_limits<T>::digits <= std::numeric_limits<slong>::max(), "Overflow error.");
                // Check and set the detected precision.
                set_prec(c_max(min_prec(), static_cast<slong>(std::numeric_limits<T>::digits)));
                // Init and set exactly.
                ::arf_init_set_si(&m_arf, static_cast<slong>(n));
            }
        } else {
            dispatch_generic_ctor(integer<1>{n}, prec);
        }
    }
    template <typename T, enable_if_t<disjunction<std::is_same<T, float>, std::is_same<T, double>>::value, int> = 0>
    void dispatch_generic_ctor(T x, slong prec)
    {
        static_assert(std::numeric_limits<T>::radix == 2, "The float/double type's radix is not 2.");
        static_assert(std::numeric_limits<T>::digits <= std::numeric_limits<slong>::max(), "Overflow error.");
        // Check and set the precision, either autodetected or specified.
        set_prec(prec ? prec : c_max(min_prec(), static_cast<slong>(std::numeric_limits<T>::digits)));
        // Init and set exactly. All noexcept from now on.
        ::arf_init(&m_arf);
        ::arf_set_d(&m_arf, static_cast<double>(x));
        // If the configured precision is less than the precision of the significand of T,
        // we need to round.
        if (get_prec() < std::numeric_limits<T>::digits) {
            round();
        }
    }
    void dispatch_generic_ctor(long double x, slong prec)
    {
        static_assert(std::numeric_limits<long double>::radix == 2, "The long double type's radix is not 2.");
        static_assert(std::numeric_limits<long double>::digits <= std::numeric_limits<slong>::max(), "Overflow error.");
        // Check and set the precision, either autodetected or specified.
        set_prec(prec ? prec : c_max(min_prec(), static_cast<slong>(std::numeric_limits<long double>::digits)));

        // Setup the temp mpfr.
        MPPP_MAYBE_TLS mpfr_raii mpfr(static_cast<::mpfr_prec_t>(min_prec()));
        // Set the configured precision.
        ::mpfr_set_prec(&mpfr.m_mpfr, static_cast<::mpfr_prec_t>(get_prec()));
        // Set the value.
        ::mpfr_set_ld(&mpfr.m_mpfr, x, MPFR_RNDN);
        // Init and set exactly. All noexcept from now on.
        ::arf_init(&m_arf);
        ::arf_set_mpfr(&m_arf, &mpfr.m_mpfr);
    }

public:
/// Generic constructor.
/**
 * \rststar
 * This constructor will initialise the value of ``this`` with ``x``. The precision of ``this``
 * is either automatically deduced (if ``prec`` is zero), or explicitly specified by the user
 * (if ``prec`` is nonzero).
 *
 * If ``prec`` is zero, then the precision of ``this`` will be set according to the following heuristic:
 *
 * * if the type of ``x`` is a C++ integral type ``I``, then the precision will be set to the bit
 *   width of ``I``;
 * * if the type of ``x`` is a C++ floating-point type ``F``, then the precision will be set to the bit
 *   width of the significand of ``F``;
 * * if ``x`` is an instance of :cpp:class:`~mppp::integer`, then the precision will be set to the number
 *   of bits used by the representation of ``x`` (rounded up to next multiple of the limb size);
 * * if ``x`` is an instance of :cpp:class:`~mppp::rational`, then the precision will be set to the sum of the number
 *   of bits used by the representations of the numerator and denominator of ``x`` (both rounded up to next multiple of
 *   the limb size).
 *
 * If ``x`` is *not* a :cpp:class:`~mppp::rational`, then ``this`` will be set to the exact value of ``x``, and no
 * rounding takes place during construction. If ``x`` is a :cpp:class:`~mppp::rational`, then the value of ``this``
 * will be rounded to the closest value to ``x`` representable with the automatically-deduced precision.
 *
 * If ``prec`` is nonzero, then the precision of ``this`` is set to ``prec``, and the value of ``this`` will be
 * rounded the closest value to ``x`` representable with the specified precision.
 * \endrststar
 *
 * @param x the construction argument.
 * @param prec the desired precision.
 *
 * @throws std::invalid_argument if the deduced precision when constructing from an mppp::integer or an mppp::rational
 * is larger than an implementation-defined value.
 * @throws unspecified any exception thrown by set_prec(),
 */
#if defined(MPPP_HAVE_CONCEPTS)
    explicit real(const RealInteroperable &x
#else
    template <typename T, real_interoperable_enabler<T> = 0>
    explicit real(const T &x
#endif
                  ,
                  slong prec = 0)
    {
        dispatch_generic_ctor(x, prec);
    }
    explicit real(const char *str, slong prec)
    {
        set_prec(prec);

        // Setup the temp mpfr.
        MPPP_MAYBE_TLS mpfr_raii mpfr(static_cast<::mpfr_prec_t>(min_prec()));
        // Set the configured precision.
        ::mpfr_set_prec(&mpfr.m_mpfr, static_cast<::mpfr_prec_t>(get_prec()));
        // Convert the string.
        ::mpfr_set_str(&mpfr.m_mpfr, str, 10, MPFR_RNDN);

        // Init and set exactly. All noexcept from now on.
        ::arf_init(&m_arf);
        ::arf_set_mpfr(&m_arf, &mpfr.m_mpfr);
    }
    real &operator=(const real &other)
    {
        if (mppp_likely(this != &other)) {
            ::arf_set(&m_arf, &other.m_arf);
            m_prec = other.m_prec;
        }
        return *this;
    }
    real &operator=(real &&other) noexcept
    {
        if (mppp_likely(this != &other)) {
            ::arf_swap(&m_arf, &other.m_arf);
            m_prec = other.m_prec;
        }
        return *this;
    }
    /// Destructor.
    ~real()
    {
        assert(m_prec <= max_prec() && m_prec >= min_prec());
        ::arf_clear(&m_arf);
    }
    /// Get a const pointer to the internal Arb structure.
    /**
     * The returned value can be used as a ``const arf_t`` parameter in the Arb API.
     *
     * @return a const pointer to the internal Arb structure holding the value of \p this.
     */
    const ::arf_struct *get_arf_t() const
    {
        return &m_arf;
    }
    /// Get a pointer to the internal Arb structure.
    /**
     * \rststar
     * The returned value can be used as an ``arf_t`` parameter in the Arb API.
     *
     * .. warning::
     *    ``arf_clear()`` should never be called on the returned pointer, as ``this``
     *    will also call ``arf_clear()`` on destruction, thus leading to memory errors.
     *    If ``arf_clear()`` is called on the returned pointer, care must be taken to re-initialise
     *    the internal Arb struct before the destruction of ``this``.
     * \endrststar
     *
     * @return a mutable pointer to the internal Arb structure holding the value of \p this.
     */
    ::arf_struct *_get_arf_t()
    {
        return &m_arf;
    }
    /// Get the precision.
    /**
     * @return the precision associated to \p this.
     */
    slong get_prec() const
    {
        return m_prec;
    }
    /// Set the precision.
    /**
     * This method will set the precision of \p this to \p prec.
     *
     * @param prec the desired precision.
     *
     * @return a reference to \p this.
     *
     * @throws std::invalid_argument if \p prec is not in the range established by min_prec() and max_prec().
     */
    real &set_prec(slong prec)
    {
        if (mppp_unlikely(prec > max_prec() || prec < min_prec())) {
            throw std::invalid_argument("An invalid precision of " + std::to_string(prec)
                                        + " was specified for a real object (the minimum allowed precision is "
                                        + std::to_string(min_prec()) + ", while the maximum allowed precision is "
                                        + std::to_string(max_prec()) + ")");
        }
        m_prec = prec;
        return *this;
    }
    void _set_prec(slong prec)
    {
        m_prec = prec;
    }
    /// Round.
    /**
     * This method will round \p this to its associated precision in the direction of the nearest representable
     * number.
     *
     * @return a reference to \p this.
     */
    real &round()
    {
        ::arf_set_round(_get_arf_t(), get_arf_t(), get_prec(), ARF_RND_NEAR);
        return *this;
    }
    /// Size of the significand in bits.
    /**
     * @return the number of bits needed to represent the absolute value of the significand of \p this.
     */
    slong bits() const
    {
        return ::arf_bits(get_arf_t());
    }
    /// Minimum precision.
    /**
     * \rststar
     * This function will return the minimum allowed precision for a :cpp:class:`~mppp::real`. The value
     * is guaranteed to be strictly greater than 1.
     * \endrststar
     *
     * @return the minimum allowed precision for an mppp::real.
     */
    static constexpr slong min_prec()
    {
        return real_min_prec();
    }
    /// Maximum precision.
    /**
     * \rststar
     * This function will return an implementation-defined positive value representing the maximum allowed
     * precision for a :cpp:class:`~mppp::real`. The value depends on the target architecture (it will be
     * larger in 64-bit environments).
     * \endrststar
     *
     * @return the maximum allowed precision for an mppp::real.
     */
    static constexpr slong max_prec()
    {
        return real_max_prec();
    }

private:
    ::arf_struct m_arf;
    slong m_prec;
};

/** @defgroup real_io real_io
 *  @{
 */

/// Output stream operator.
/**
 * \rststar
 * This operator will print to the stream ``os`` the :cpp:class:`~mppp::real` ``r`` in base-10 scientific
 * notation. In order to provide a visual clue for the precision associated to ``r``, ``r`` will be copied
 * into temporary storage and rounded to its associated precision before being printed. As a consequence,
 * two :cpp:class:`~mppp::real` objects that compare equal (that is, they represent identical values) will
 * be printed differently if their associated precisions do not match.
 *
 * This operator uses internally the MPFR API. Note that in MPFR the exponent of floating-point values has a fixed
 * range, while in Arb the exponent is a multiprecision integer. As a consequence, if the exponent of ``r``
 * overflows an implementation-defined range, an error will be raised.
 *
 * The special values are printed as ``"nan"``, ``"inf"`` and ``"-inf"``.
 * \endrststar
 *
 * @param os the target stream.
 * @param r the input real.
 *
 * @return a reference to \p os.
 *
 * @throws std::invalid_argument if the exponent of \p r is outside an implementation-defined range.
 * @throws std::runtime_error in case of other unspecified errors (e.g., nonzero return values from the MPFR API).
 * @throws unspecified any exception raised by the use of the public API of \p std::ostream.
 */
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
    arf_to_mpfr(&mpfr.m_mpfr, r.get_arf_t());

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

    // Adjust the exponent. Do it in multiprec in order to avoid potential overflow.
    integer<1> z_exp{exp};
    --z_exp;
    if (z_exp && !mpfr_zero_p(&mpfr.m_mpfr)) {
        // Add the exponent at the end of the string, if both the value and the exponent
        // are nonzero.
        os << "e" << z_exp;
    }
    return os;
}

/** @} */

inline void add(real &rop, const real &op1, const real &op2)
{
    const auto r_prec = c_max(op1.get_prec(), op2.get_prec());
    ::arf_add(rop._get_arf_t(), op1.get_arf_t(), op2.get_arf_t(), r_prec, ARF_RND_NEAR);
    rop._set_prec(r_prec);
}

inline void mul(real &rop, const real &op1, const real &op2)
{
    const auto r_prec = c_max(op1.get_prec(), op2.get_prec());
    arf_mul(rop._get_arf_t(), op1.get_arf_t(), op2.get_arf_t(), r_prec, ARF_RND_NEAR);
    rop._set_prec(r_prec);
}
}

#else

#error The real.hpp header was included but mp++ was not configured with the MPPP_WITH_ARB option.

#endif

#endif
