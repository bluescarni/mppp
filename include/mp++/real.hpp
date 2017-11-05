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

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#if MPPP_CPLUSPLUS >= 201703L
#include <string_view>
#endif
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

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

// Clamp the precision between the min and max allowed values. This is used in the generic constructor/assignment
// operator.
constexpr ::mpfr_prec_t clamp_mpfr_prec(::mpfr_prec_t p)
{
    return real_prec_check(p) ? p : (p < real_prec_min() ? real_prec_min() : real_prec_max());
}

// Helper function to print an mpfr to stream in base 10.
inline void mpfr_to_stream(const ::mpfr_t r, std::ostream &os, int base)
{
    // All chars potentially used by MPFR for representing the digits up to base 62, sorted.
    constexpr char all_chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    // Check the base.
    if (mppp_unlikely(base < 2 || base > 62)) {
        throw std::invalid_argument("Cannot convert a real to a string in base " + std::to_string(base)
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
    smart_mpfr_str str(::mpfr_get_str(nullptr, &exp, base, 0, r, MPFR_RNDN), ::mpfr_free_str);
    // LCOV_EXCL_START
    if (mppp_unlikely(!str)) {
        throw std::runtime_error("Error in the conversion of a real to string: the call to mpfr_get_str() failed");
    }
    // LCOV_EXCL_STOP

    // Print the string, inserting a decimal point after the first digit.
    bool dot_added = false;
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
                if (*cptr >= '0' && *cptr <= '9') {
                    os << '.';
                    dot_added = true;
                }
            } else {
                // For bases larger than 10, we do a binary search among all the allowed characters.
                // NOTE: we need to search into the whole all_chars array (instead of just up to all_chars
                // + base) because apparently mpfr_get_str() seems to ignore lower/upper case when the base
                // is small enough (e.g., it uses 'a' instead of 'A' when printing in base 11).
                if (std::binary_search(all_chars, all_chars + sizeof(all_chars), *cptr)) {
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
    if (exp_sgn && !mpfr_zero_p(r)) {
        // Add the exponent at the end of the string, if both the value and the exponent
        // are nonzero.
        // NOTE: for bases greater than 10 we need '@' for the exponent, rather than 'e' or 'E'.
        // http://www.mpfr.org/mpfr-current/mpfr.html#Assignment-Functions
        os << (base <= 10 ? 'e' : '@');
        if (exp_sgn == 1) {
            // Add extra '+' if the exponent is positive, for consistency with
            // real128's string format (and possibly other formats too?).
            os << '+';
        }
        os << z_exp;
    }
}

#if !defined(MPPP_DOXYGEN_INVOKED)

// Helpers to deduce the precision when constructing/assigning a real via another type.
template <typename T, enable_if_t<std::is_integral<T>::value, int> = 0>
inline ::mpfr_prec_t real_deduce_precision(const T &)
{
    static_assert(std::numeric_limits<T>::digits < std::numeric_limits<::mpfr_prec_t>::max(), "Overflow error.");
    // NOTE: for signed integers, include the sign bit as well.
    return static_cast<::mpfr_prec_t>(std::numeric_limits<T>::digits) + std::is_signed<T>::value;
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

template <typename T, enable_if_t<std::is_floating_point<T>::value, int> = 0>
inline ::mpfr_prec_t real_deduce_precision(const T &)
{
    static_assert(std::numeric_limits<T>::digits <= std::numeric_limits<::mpfr_prec_t>::max(), "Overflow error.");
    return std::numeric_limits<T>::radix == 2 ? static_cast<::mpfr_prec_t>(std::numeric_limits<T>::digits)
                                              : dig2mpfr_prec<T>();
}

template <std::size_t SSize>
inline ::mpfr_prec_t real_deduce_precision(const integer<SSize> &n)
{
    // Infer the precision from the bit size of n.
    const auto ls = n.size();
    // Check that ls * GMP_NUMB_BITS is representable by mpfr_prec_t.
    // LCOV_EXCL_START
    if (mppp_unlikely(ls > static_cast<make_unsigned_t<::mpfr_prec_t>>(std::numeric_limits<::mpfr_prec_t>::max())
                               / unsigned(GMP_NUMB_BITS))) {
        throw std::overflow_error("The deduced precision for a real from an integer is too large");
    }
    // LCOV_EXCL_STOP
    return static_cast<::mpfr_prec_t>(static_cast<::mpfr_prec_t>(ls) * GMP_NUMB_BITS);
}

template <std::size_t SSize>
inline ::mpfr_prec_t real_deduce_precision(const rational<SSize> &q)
{
    // Infer the precision from the bit size of num/den.
    const auto n_size = q.get_num().size();
    const auto d_size = q.get_den().size();
    // Overflow checks.
    // LCOV_EXCL_START
    if (mppp_unlikely(
            // Overflow in total size.
            (n_size > std::numeric_limits<decltype(q.get_num().size())>::max() - d_size)
            // Check that tot_size * GMP_NUMB_BITS is representable by mpfr_prec_t.
            || ((n_size + d_size)
                > static_cast<make_unsigned_t<::mpfr_prec_t>>(std::numeric_limits<::mpfr_prec_t>::max())
                      / unsigned(GMP_NUMB_BITS)))) {
        throw std::overflow_error("The deduced precision for a real from a rational is too large");
    }
    // LCOV_EXCL_STOP
    return static_cast<::mpfr_prec_t>(static_cast<::mpfr_prec_t>(n_size + d_size) * GMP_NUMB_BITS);
}

#if defined(MPPP_WITH_QUADMATH)
inline ::mpfr_prec_t real_deduce_precision(const real128 &)
{
    // The significand precision in bits is 113 for real128. Let's double-check it.
    static_assert(real128_sig_digits() == 113u, "Invalid number of digits.");
    return 113;
}
#endif

#endif

template <typename = void>
struct real_constants {
    // A bare real with static memory allocation, represented as
    // an mpfr_struct_t paired to storage for the limbs.
    template <::mpfr_prec_t Prec>
    using static_real = std::pair<mpfr_struct_t,
                                  // Use std::array as storage for the limbs.
                                  std::array<::mp_limb_t, mpfr_custom_get_size(Prec) / sizeof(::mp_limb_t)
                                                              + mpfr_custom_get_size(Prec) % sizeof(::mp_limb_t)>>;
    // Shortcut for the size of the real_2_112 constant. We just need
    // 1 bit of precision for this, but make sure we don't go outside
    // the allowed precision range.
    static const ::mpfr_prec_t size_2_112 = clamp_mpfr_prec(1);
    // Create a static real with value 2**112. This represents the "hidden bit"
    // of the significand of a quadruple-precision FP.
    // NOTE: this could be instantiated as a global static instead of being re-computed every time.
    // However, since this is not constexpr, there's a high risk of static order initialization screwups
    // (e.g., if initing a static real from a real128), so for the time being let's keep things basic.
    // We can determine in the future if we can make this constexpr somehow and have a 2**112 instance
    // inited during constant initialization.
    static static_real<size_2_112> get_2_112()
    {
        // NOTE: pair's def ctor value-inits the members: everything in retval is zeroed out.
        static_real<size_2_112> retval;
        // Init the limbs first, as indicated by the mpfr docs.
        mpfr_custom_init(retval.second.data(), size_2_112);
        // Do the custom init with a zero value, exponent 0 (unused), precision matching the previous call,
        // and the limbs storage pointer.
        mpfr_custom_init_set(&retval.first, MPFR_ZERO_KIND, 0, size_2_112, retval.second.data());
        // Set the actual value.
        ::mpfr_set_ui_2exp(&retval.first, 1ul, static_cast<::mpfr_exp_t>(112), MPFR_RNDN);
        return retval;
    }
    // Default precision value.
    static std::atomic<::mpfr_prec_t> default_prec;
};

// NOTE: the use of ATOMIC_VAR_INIT ensures that the initialisation of default_prec
// is constant initialisation:
//
// http://en.cppreference.com/w/cpp/atomic/ATOMIC_VAR_INIT
//
// This essentially means that this initialisation happens before other types of
// static initialisation:
//
// http://en.cppreference.com/w/cpp/language/initialization
//
// This ensures that static reals, which are subject to dynamic initialization, are initialised
// when this variable has already been constructed, and thus access to it will be safe.
template <typename T>
std::atomic<::mpfr_prec_t> real_constants<T>::default_prec = ATOMIC_VAR_INIT(::mpfr_prec_t(0));

// Fwd declare for friendship.
template <typename F, typename Arg0, typename... Args>
real &mpfr_nary_op(::mpfr_prec_t, const F &, real &, Arg0 &&, Args &&...);

template <typename F, typename Arg0, typename... Args>
real mpfr_nary_op_return(::mpfr_prec_t, const F &, Arg0 &&, Args &&...);

template <typename F>
real real_constant(const F &, ::mpfr_prec_t);
}

// Fwd declare swap.
void swap(real &, real &) noexcept;

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

#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
concept bool CvrReal = std::is_same<uncvref_t<T>, real>::value;
#else
template <typename... Args>
using cvr_real_enabler = enable_if_t<conjunction<std::is_same<uncvref_t<Args>, real>...>::value, int>;
#endif

/// Get the default precision for \link mppp::real real\endlink objects.
/**
 * \ingroup real_prec
 * \rststar
 * This function returns the value of the precision used during the construction of
 * :cpp:class:`~mppp::real` objects when an explicit precision value
 * is **not** specified . On program startup, the value returned by this function
 * is zero, meaning that the precision of a :cpp:class:`~mppp::real` object will be chosen
 * automatically according to heuristics depending on the specific situation, if possible.
 *
 * The default precision is stored in a global variable, and its value can be changed via
 * :cpp:func:`~mppp::real_set_default_prec()`. It is safe to read and modify concurrently
 * from multiple threads the default precision.
 * \endrststar
 *
 * @return the value of the default precision for \link mppp::real real\endlink objects.
 */
inline mpfr_prec_t real_get_default_prec()
{
    return real_constants<>::default_prec.load(std::memory_order_relaxed);
}

/// Set the default precision for \link mppp::real real\endlink objects.
/**
 * \ingroup real_prec
 * \rststar
 * See :cpp:func:`~mppp::real_get_default_prec()` for an explanation of how the default precision value
 * is used.
 * \endrststar
 *
 * @param p the desired value for the default precision for \link mppp::real real\endlink objects.
 *
 * @throws std::invalid_argument if \p p is nonzero and not in the range established by
 * \link mppp::real_prec_min() real_prec_min()\endlink and \link mppp::real_prec_max() real_prec_max()\endlink.
 */
inline void real_set_default_prec(::mpfr_prec_t p)
{
    if (mppp_unlikely(p && !real_prec_check(p))) {
        throw std::invalid_argument("Cannot set the default precision to " + std::to_string(p)
                                    + ": the value must be either zero or between " + std::to_string(real_prec_min())
                                    + " and " + std::to_string(real_prec_max()));
    }
    real_constants<>::default_prec.store(p, std::memory_order_relaxed);
}

/// Reset the default precision for \link mppp::real real\endlink objects.
/**
 * \ingroup real_prec
 * \rststar
 * This function will reset the default precision value to zero (i.e., the same value assigned
 * on program startup). See :cpp:func:`~mppp::real_get_default_prec()` for an explanation of how the default precision
 * value is used.
 * \endrststar
 */
inline void real_reset_default_prec()
{
    real_constants<>::default_prec.store(0);
}

inline namespace detail
{

// Get the default precision, if set, otherwise the clamped deduced precision for x.
template <typename T>
inline ::mpfr_prec_t real_dd_prec(const T &x)
{
    // NOTE: this is guaranteed to be a valid precision value.
    const auto dp = real_get_default_prec();
    // Return default precision if nonzero, otherwise return the clamped deduced precision.
    return dp ? dp : clamp_mpfr_prec(real_deduce_precision(x));
}
}

// NOTE: this is apparently necessary to work around a doxy bug.
/** @defgroup real_dummy real_dummy
 *  @{
 */

/// Special initialisation tags for \link mppp::real real\endlink.
/**
 * \rststar
 * This scoped enum is used to initialise a :cpp:class:`~mppp::real` with
 * one of the three special values NaN, infinity or zero.
 * \endrststar
 */
enum class real_kind { nan = MPFR_NAN_KIND, inf = MPFR_INF_KIND, zero = MPFR_ZERO_KIND };

/** @} */

// For the future:
// - construction from/conversion to interoperables can probably be improved performance wise, especially
//   if we exploit the mpfr_t internals.
// - probably we should have a build in the CI against the latest MPFR, built with sanitizers on.

/// Multiprecision floating-point class.
/**
 * \rststar
 * This class represents arbitrary-precision real values encoded in a binary floating-point format.
 * It acts as a wrapper around the MPFR ``mpfr_t`` type, which in turn consists of a multiprecision significand
 * (whose size can be set at runtime) paired to a fixed-size exponent. In other words, :cpp:class:`~mppp::real`
 * values can have an arbitrary number of binary digits of precision (limited only by the available memory),
 * but the exponent range is limited.
 *
 * :cpp:class:`~mppp::real` aims to behave like a C++ floating-point type whose precision is a runtime property
 * of the class instances rather than a compile-time property of the type. Because of this, the way precision
 * is handled in :cpp:class:`~mppp::real` differs from the way it is managed in MPFR. The most important difference
 * is that in operations involving :cpp:class:`~mppp::real` the precision of the result is determined
 * by the precision of the operands, whereas in MPFR the precision of the operation is determined by the precision
 * of the return value (which is always passed as the first function parameter in the MPFR API). In other words,
 * :cpp:class:`~mppp::real` mirrors the behaviour of C++ code such as
 *
 * .. code-block:: c++
 *
 *    auto x = double(5) + float(6);
 *
 * where the type of ``x`` is ``double`` because ``double`` has a precision always higher than (or at least equal to)
 * the precision of ``float``. In the equivalent code with :cpp:class:`~mppp::real`
 *
 * .. code-block:: c++
 *
 *    auto x = real{5,200} + real{6,150};
 *
 * the first operand has a value of 5 and precision of 200 bits, while the second operand has a value of 6 and precision
 * 150 bits. Thus, the precision of the result (and the precision at which the addition is computed) will be
 * :math:`\mathrm{max}\left(200,150\right)=200` bits.
 *
 * The precision of a :cpp:class:`~mppp::real` can be set at construction, or it can be changed later via functions and
 * methods such as :cpp:func:`mppp::real::set_prec()`, :cpp:func:`mppp::real::prec_round()`, etc. By default,
 * the precision of a :cpp:class:`~mppp::real` is automatically deduced upon construction following a set of heuristics
 * aimed at ensuring that the constructed :cpp:class:`~mppp::real` represents exactly the value used for initialisation.
 * For instance, by default, the construction of a :cpp:class:`~mppp::real` from a 32 bit integer will yield a
 * :cpp:class:`~mppp::real` with a precision of 32 bits. This behaviour can be altered either by specifying explicitly
 * the desired precision value, or by setting a global default precision via :cpp:func:`~mppp::real_set_default_prec()`.
 *
 * TODO: mention it's always MPFR_RNDN.
 * \endrststar
 */
class real
{
#if !defined(MPPP_DOXYGEN_INVOKED)
    // Make friends, for accessing the non-checking prec setting funcs.
    template <typename F, typename Arg0, typename... Args>
    friend real &detail::mpfr_nary_op(::mpfr_prec_t, const F &, real &, Arg0 &&, Args &&...);
    template <typename F, typename Arg0, typename... Args>
    friend real detail::mpfr_nary_op_return(::mpfr_prec_t, const F &, Arg0 &&, Args &&...);
    template <typename F>
    friend real detail::real_constant(const F &, ::mpfr_prec_t);
#endif
    // Utility function to check the precision upon init.
    static ::mpfr_prec_t check_init_prec(::mpfr_prec_t p)
    {
        if (mppp_unlikely(!real_prec_check(p))) {
            throw std::invalid_argument("Cannot init a real with a precision of " + std::to_string(p)
                                        + ": the maximum allowed precision is " + std::to_string(real_prec_max())
                                        + ", the minimum allowed precision is " + std::to_string(real_prec_min()));
        }
        return p;
    }

public:
    /// Default constructor.
    /**
     * \rststar
     * The value will be initialised to positive zero. The precision of ``this`` will be
     * either the default precision, if set, or the value returned by :cpp:func:`~mppp::real_prec_min()`
     * otherwise.
     * \endrststar
     */
    real()
    {
        // Init with minimum or default precision.
        const auto dp = real_get_default_prec();
        ::mpfr_init2(&m_mpfr, dp ? dp : real_prec_min());
        ::mpfr_set_zero(&m_mpfr, 1);
    }

private:
    // A tag to call private ctors.
    struct ptag {
    };
    // Init a real with precision p, setting its value to n. No precision
    // checking is performed.
    explicit real(const ptag &, ::mpfr_prec_t p, bool ignore_prec)
    {
        assert(ignore_prec);
        assert(real_prec_check(p));
        (void)ignore_prec;
        ::mpfr_init2(&m_mpfr, p);
    }

public:
    /// Copy constructor.
    /**
     * The copy constructor performs an exact deep copy of the input object.
     *
     * @param other the \link mppp::real real\endlink that will be copied.
     */
    real(const real &other) : real(&other.m_mpfr) {}
    /// Copy constructor with custom precision.
    /**
     * This constructor will set \p this to a copy of \p other with precision \p p. If \p p
     * is smaller than the precision of \p other, a rounding operation will be performed,
     * otherwise the value will be copied exactly.
     *
     * @param other the \link mppp::real real\endlink that will be copied.
     * @param p the desired precision.
     *
     * @throws std::invalid_argument if \p p is outside the range established by
     * \link mppp::real_prec_min() real_prec_min()\endlink and \link mppp::real_prec_max() real_prec_max()\endlink.
     */
    explicit real(const real &other, ::mpfr_prec_t p)
    {
        // Init with custom precision, and then set.
        ::mpfr_init2(&m_mpfr, check_init_prec(p));
        ::mpfr_set(&m_mpfr, &other.m_mpfr, MPFR_RNDN);
    }
    /// Move constructor.
    /**
     * \rststar
     * .. warning::
     *    Unless otherwise noted, the only valid operations on the moved-from ``other`` object are
     *    destruction and copy/move assignment. After re-assignment, ``other`` can be used normally again.
     * \endrststar
     *
     * @param other the \link mppp::real real\endlink that will be moved.
     */
    real(real &&other) noexcept
    {
        // Shallow copy other.
        m_mpfr = other.m_mpfr;
        // Mark the other as moved-from.
        other.m_mpfr._mpfr_d = nullptr;
    }
    /// Constructor from a special value, sign and precision.
    /**
     * \rststar
     * This constructor will initialise ``this`` with one of the special values
     * specified by the :cpp:type:`~mppp::real_kind` enum. The precision of ``this``
     * will be ``p``. If ``p`` is zero, the precision will be set to the default
     * precision (as indicated by :cpp:func:`~mppp::real_get_default_prec()`).
     *
     * If ``k`` is not NaN, the sign bit will be set to positive if ``sign``
     * is nonnegative, negative otherwise.
     * \endrststar
     *
     * @param k the desired special value.
     * @param sign the desired sign for \p this.
     * @param p the desired precision for \p this.
     *
     * @throws std::invalid_argument if \p p is not within the bounds established by
     * \link mppp::real_prec_min() real_prec_min()\endlink and \link mppp::real_prec_max() real_prec_max()\endlink,
     * or if \p p is zero but no default precision has been set.
     */
    explicit real(real_kind k, int sign, ::mpfr_prec_t p)
    {
        ::mpfr_prec_t prec;
        if (p) {
            prec = check_init_prec(p);
        } else {
            const auto dp = real_get_default_prec();
            if (mppp_unlikely(!dp)) {
                throw std::invalid_argument("Cannot init a real with an automatically-deduced precision if "
                                            "the global default precision has not been set");
            }
            prec = dp;
        }
        ::mpfr_init2(&m_mpfr, prec);
        switch (k) {
            case real_kind::nan:
                break;
            case real_kind::inf:
                set_inf(sign);
                break;
            case real_kind::zero:
                set_zero(sign);
                break;
        }
    }
    /// Constructor from a special value and precision.
    /**
     * This constructor is equivalent to the constructor from a special value, sign and precision
     * with a hard-coded sign of 0.
     *
     * @param k the desired special value.
     * @param p the desired precision for \p this.
     *
     * @throws unspecified any exception thrown by the constructor from a special value, sign and precision.
     */
    explicit real(real_kind k, ::mpfr_prec_t p) : real(k, 0, p) {}
    /// Constructor from a special value.
    /**
     * This constructor is equivalent to the constructor from a special value, sign and precision
     * with a hard-coded sign of 0 and hard-coded precision of 0.
     *
     * @param k the desired special value.
     *
     * @throws unspecified any exception thrown by the constructor from a special value, sign and precision.
     */
    explicit real(real_kind k) : real(k, 0, 0) {}

private:
    // A helper to determine the precision to use in the generic constructors. The precision
    // can be manually provided, taken from the default global value, or deduced according
    // to the properties of the type/value.
    template <typename T>
    static ::mpfr_prec_t compute_init_precision(::mpfr_prec_t provided, const T &x)
    {
        if (provided) {
            // Provided precision trumps everything. Check it and return it.
            return check_init_prec(provided);
        }
        // Return the default or deduced precision.
        return real_dd_prec(x);
    }
    // Construction from FPs.
    // Alias for the MPFR assignment functions from FP types.
    template <typename T>
    using fp_a_ptr = int (*)(::mpfr_t, T, ::mpfr_rnd_t);
    template <typename T>
    void dispatch_fp_construction(fp_a_ptr<T> ptr, const T &x, ::mpfr_prec_t p)
    {
        ::mpfr_init2(&m_mpfr, compute_init_precision(p, x));
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
    void dispatch_integral_init(::mpfr_prec_t p, const T &n)
    {
        ::mpfr_init2(&m_mpfr, compute_init_precision(p, n));
    }
    // Special casing for bool, otherwise MSVC warns if we fold this into the
    // constructor from unsigned.
    void dispatch_construction(const bool &b, ::mpfr_prec_t p)
    {
        dispatch_integral_init(p, b);
        ::mpfr_set_ui(&m_mpfr, static_cast<unsigned long>(b), MPFR_RNDN);
    }
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_unsigned<T>>::value, int> = 0>
    void dispatch_construction(const T &n, ::mpfr_prec_t p)
    {
        dispatch_integral_init(p, n);
        if (n <= std::numeric_limits<unsigned long>::max()) {
            ::mpfr_set_ui(&m_mpfr, static_cast<unsigned long>(n), MPFR_RNDN);
        } else {
            // NOTE: here and elsewhere let's use a 2-limb integer, in the hope
            // of avoiding dynamic memory allocation.
            ::mpfr_set_z(&m_mpfr, integer<2>(n).get_mpz_view(), MPFR_RNDN);
        }
    }
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_signed<T>>::value, int> = 0>
    void dispatch_construction(const T &n, ::mpfr_prec_t p)
    {
        dispatch_integral_init(p, n);
        if (n <= std::numeric_limits<long>::max() && n >= std::numeric_limits<long>::min()) {
            ::mpfr_set_si(&m_mpfr, static_cast<long>(n), MPFR_RNDN);
        } else {
            ::mpfr_set_z(&m_mpfr, integer<2>(n).get_mpz_view(), MPFR_RNDN);
        }
    }
    template <std::size_t SSize>
    void dispatch_construction(const integer<SSize> &n, ::mpfr_prec_t p)
    {
        ::mpfr_init2(&m_mpfr, compute_init_precision(p, n));
        ::mpfr_set_z(&m_mpfr, n.get_mpz_view(), MPFR_RNDN);
    }
    template <std::size_t SSize>
    void dispatch_construction(const rational<SSize> &q, ::mpfr_prec_t p)
    {
        const auto v = get_mpq_view(q);
        ::mpfr_init2(&m_mpfr, compute_init_precision(p, q));
        ::mpfr_set_q(&m_mpfr, &v, MPFR_RNDN);
    }
#if defined(MPPP_WITH_QUADMATH)
    void dispatch_construction(const real128 &x, ::mpfr_prec_t p)
    {
        // Init the value.
        ::mpfr_init2(&m_mpfr, compute_init_precision(p, x));
        assign_real128(x);
    }
    // NOTE: split this off from the dispatch_construction() overload, so we can re-use it in the
    // generic assignment.
    void assign_real128(const real128 &x)
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
            const auto r_2_112 = real_constants<>::get_2_112();
            ::mpfr_add(&m_mpfr, &m_mpfr, &r_2_112.first, MPFR_RNDN);
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
    /// Generic constructor.
    /**
     * \rststar
     * The generic constructor will set ``this`` to the value of ``x`` with a precision of ``p``.
     *
     * If ``p`` is nonzero, then ``this`` will be initialised exactly to a precision of ``p``, and
     * a rounding operation might occurr.
     *
     * If ``p`` is zero, the constructor will first fetch the default precision ``dp`` via
     * :cpp:func:`~mppp::real_get_default_prec()`. If ``dp`` is nonzero, then ``dp`` will be used
     * as precision for ``this`` and a rounding operation might occurr.
     *
     * Otherwise, if ``dp`` is zero, the precision of ``this`` will be set according to the following
     * heuristics:
     *
     * * if ``x`` is a C++ integral type ``I``, then the precision is set to the bit width of ``I``;
     * * if ``x`` is a C++ floating-point type ``F``, then the precision is set to the number of binary digits
     *   in the significand of ``F``;
     * * if ``x`` is :cpp:class:`~mppp::integer`, then the precision is set to the number of bits in use by
     *   ``x`` (rounded up to the next multiple of the limb type's bit width);
     * * if ``x`` is :cpp:class:`~mppp::rational`, then the precision is set to the sum of the number of bits
     *   used by numerator and denominator (as established by the previous heuristic for :cpp:class:`~mppp::integer`);
     * * if ``x`` is :cpp:class:`~mppp::real128`, then the precision is set to 113.
     *
     * These heuristics aim at ensuring that, whatever the type of ``x``, its value is preserved exactly in the
     * constructed :cpp:class:`~mppp::real`.
     *
     * Construction from ``bool`` will initialise ``this`` to 1 for ``true``, and ``0`` for ``false``.
     * \endrststar
     *
     * @param x the construction argument.
     * @param p the desired precision.
     *
     * @throws std::overflow_error if an overflow occurs in the computation of the automatically-deduced precision.
     * @throws std::invalid_argument if \p p is nonzero and outside the range established by
     * \link mppp::real_prec_min() real_prec_min()\endlink and \link mppp::real_prec_max() real_prec_max()\endlink.
     */
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

private:
    void construct_from_c_string(const char *s, int base, ::mpfr_prec_t p)
    {
        if (mppp_unlikely(base && (base < 2 || base > 62))) {
            throw std::invalid_argument("Cannot construct a real from a string in base " + std::to_string(base)
                                        + ": the base must either be zero or in the [2,62] range");
        }
        const ::mpfr_prec_t prec = p ? check_init_prec(p) : real_get_default_prec();
        if (mppp_unlikely(!prec)) {
            throw std::invalid_argument("Cannot construct a real from a string if the precision is not explicitly "
                                        "specified and no default precision has been set");
        }
        ::mpfr_init2(&m_mpfr, prec);
        const auto ret = ::mpfr_set_str(&m_mpfr, s, base, MPFR_RNDN);
        if (mppp_unlikely(ret == -1)) {
            ::mpfr_clear(&m_mpfr);
            throw std::invalid_argument(std::string{"The string '"} + s + "' does not represent a valid real in base "
                                        + std::to_string(base));
        }
    }
    explicit real(const ptag &, const char *s, int base, ::mpfr_prec_t p)
    {
        construct_from_c_string(s, base, p);
    }
    explicit real(const ptag &, const std::string &s, int base, ::mpfr_prec_t p) : real(s.c_str(), base, p) {}
#if MPPP_CPLUSPLUS >= 201703L
    explicit real(const ptag &, const std::string_view &s, int base, ::mpfr_prec_t p)
        : real(s.data(), s.data() + s.size(), base, p)
    {
    }
#endif

public:
    /// Constructor from string, base and precision.
    /**
     * \rststar
     * This constructor will set ``this`` to the value represented by the :cpp:concept:`~mppp::StringType` ``s``, which
     * is interpreted as a floating-point number in base ``base``. ``base`` must be either zero (in which case the base
     * will be automatically deduced) or a number in the [2,62] range. The valid string formats are detailed in the
     * documentation of the MPFR function ``mpfr_set_str()``. Note that leading whitespaces are ignored, but trailing
     * whitespaces will raise an error.
     *
     * The precision of ``this`` will be ``p`` if ``p`` is nonzero, the default precision otherwise. If ``p`` is zero
     * and no default precision has been set, an error will be raised.
     *
     * .. seealso::
     *    http://www.mpfr.org/mpfr-current/mpfr.html#Assignment-Functions
     * \endrststar
     *
     * @param s the input string.
     * @param base the base used in the string representation.
     * @param p the desired precision.
     *
     * @throws std::invalid_argument in the following cases:
     * - \p base is not zero and not in the [2,62] range,
     * - \p p is either outside the valid bounds for a precision value, or it is zero and no
     *   default precision value has been set,
     * - \p s cannot be interpreted as a floating-point number.
     * @throws unspecified any exception thrown by memory errors in standard containers.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    explicit real(const StringType &s,
#else
    template <typename T, string_type_enabler<T> = 0>
    explicit real(const T &s,
#endif
                  int base, ::mpfr_prec_t p)
        : real(ptag{}, s, base, p)
    {
    }
        /// Constructor from string and precision.
        /**
         * This constructor is equivalent to the constructor from string with a ``base`` value hard-coded to 10.
         *
         * @param s the input string.
         * @param p the desired precision.
         *
         * @throws unspecified any exception thrown by the constructor from string, base and precision.
         */
#if defined(MPPP_HAVE_CONCEPTS)
    explicit real(const StringType &s,
#else
    template <typename T, string_type_enabler<T> = 0>
    explicit real(const T &s,
#endif
                  ::mpfr_prec_t p)
        : real(s, 10, p)
    {
    }
        /// Constructor from string.
        /**
         * This constructor is equivalent to the constructor from string with a ``base`` value hard-coded to 10
         * and a precision value hard-coded to zero (that is, the precision will be the default precision, if set).
         *
         * @param s the input string.
         *
         * @throws unspecified any exception thrown by the constructor from string, base and precision.
         */
#if defined(MPPP_HAVE_CONCEPTS)
    explicit real(const StringType &s)
#else
    template <typename T, string_type_enabler<T> = 0>
    explicit real(const T &s)
#endif
        : real(s, 10, 0)
    {
    }
    /// Constructor from range of characters, base and precision.
    /**
     * This constructor will initialise \p this from the content of the input half-open range,
     * which is interpreted as the string representation of a floating-point value in base \p base.
     *
     * Internally, the constructor will copy the content of the range to a local buffer, add a
     * string terminator, and invoke the constructor from string, base and precision.
     *
     * @param begin the start of the input range.
     * @param end the end of the input range.
     * @param base the base used in the string representation.
     * @param p the desired precision.
     *
     * @throws unspecified any exception thrown by the constructor from string, or by memory
     * allocation errors in standard containers.
     */
    explicit real(const char *begin, const char *end, int base, ::mpfr_prec_t p)
    {
        MPPP_MAYBE_TLS std::vector<char> buffer;
        buffer.assign(begin, end);
        buffer.emplace_back('\0');
        construct_from_c_string(buffer.data(), base, p);
    }
    /// Constructor from range of characters and precision.
    /**
     * This constructor is equivalent to the constructor from range of characters with a ``base`` value hard-coded
     * to 10.
     *
     * @param begin the start of the input range.
     * @param end the end of the input range.
     * @param p the desired precision.
     *
     * @throws unspecified any exception thrown by the constructor from range of characters, base and precision.
     */
    explicit real(const char *begin, const char *end, ::mpfr_prec_t p) : real(begin, end, 10, p) {}
    /// Constructor from range of characters.
    /**
     * This constructor is equivalent to the constructor from range of characters with a ``base`` value hard-coded
     * to 10 and a precision value hard-coded to zero (that is, the precision will be the default precision, if set).
     *
     * @param begin the start of the input range.
     * @param end the end of the input range.
     *
     * @throws unspecified any exception thrown by the constructor from range of characters, base and precision.
     */
    explicit real(const char *begin, const char *end) : real(begin, end, 10, 0) {}
    /// Copy constructor from ``mpfr_t``.
    /**
     * This constructor will initialise ``this`` with an exact deep copy of ``x``.
     *
     * \rststar
     * .. warning::
     *    It is the user's responsibility to ensure that ``x`` has been correctly initialised
     *    with a precision within the bounds established by :cpp:func:`~mppp::real_prec_min()`
     *    and :cpp:func:`~mppp::real_prec_max()`.
     * \endrststar
     *
     * @param x the ``mpfr_t`` that will be deep-copied.
     */
    explicit real(const ::mpfr_t x)
    {
        // Init with the same precision as other, and then set.
        ::mpfr_init2(&m_mpfr, mpfr_get_prec(x));
        ::mpfr_set(&m_mpfr, x, MPFR_RNDN);
    }
    /// Move constructor from ``mpfr_t``.
    /**
     * This constructor will initialise ``this`` with a shallow copy of ``x``.
     *
     * \rststar
     * .. warning::
     *    It is the user's responsibility to ensure that ``x`` has been correctly initialised
     *    with a precision within the bounds established by :cpp:func:`~mppp::real_prec_min()`
     *    and :cpp:func:`~mppp::real_prec_max()`.
     *
     *    Additionally, the user must ensure that, after construction, ``mpfr_clear()`` is never
     *    called on ``x``: the resources previously owned by ``x`` are now owned by ``this``, which
     *    will take care of releasing them when the destructor is called.
     * \endrststar
     *
     * @param x the ``mpfr_t`` that will be moved.
     */
    explicit real(::mpfr_t &&x) : m_mpfr(*x) {}
    /// Destructor.
    /**
     * The destructor will free any resource held by the internal ``mpfr_t`` instance.
     */
    ~real()
    {
        if (m_mpfr._mpfr_d) {
            // The object is not moved-from, destroy it.
            assert(real_prec_check(get_prec()));
            ::mpfr_clear(&m_mpfr);
        }
    }
    /// Copy assignment operator.
    /**
     * The operator will deep-copy \p other into \p this.
     *
     * @param other the \link mppp::real real\endlink that will be copied into \p this.
     *
     * @return a reference to \p this.
     */
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
    /// Move assignment operator.
    /**
     * @param other the \link mppp::real real\endlink that will be moved into \p this.
     *
     * @return a reference to \p this.
     */
    real &operator=(real &&other) noexcept
    {
        // NOTE: for generic code, std::swap() is not a particularly good way of implementing
        // the move assignment:
        //
        // https://stackoverflow.com/questions/6687388/why-do-some-people-use-swap-for-move-assignments
        //
        // Here however it is fine, as we know there are no side effects we need to maintain.
        //
        // NOTE: we use a raw std::swap() here (instead of mpfr_swap()) because we don't know in principle
        // if mpfr_swap() relies on the operands not to be in a moved-from state (although it's unlikely).
        std::swap(m_mpfr, other.m_mpfr);
        return *this;
    }

private:
    // Assignment from FPs.
    template <bool SetPrec, typename T>
    void dispatch_fp_assignment(fp_a_ptr<T> ptr, const T &x)
    {
        if (SetPrec) {
            set_prec_impl<false>(real_dd_prec(x));
        }
        ptr(&m_mpfr, x, MPFR_RNDN);
    }
    template <bool SetPrec>
    void dispatch_assignment(const float &x)
    {
        dispatch_fp_assignment<SetPrec>(::mpfr_set_flt, x);
    }
    template <bool SetPrec>
    void dispatch_assignment(const double &x)
    {
        dispatch_fp_assignment<SetPrec>(::mpfr_set_d, x);
    }
    template <bool SetPrec>
    void dispatch_assignment(const long double &x)
    {
        dispatch_fp_assignment<SetPrec>(::mpfr_set_ld, x);
    }
    // Assignment from integral types.
    template <bool SetPrec, typename T>
    void dispatch_integral_ass_prec(const T &n)
    {
        if (SetPrec) {
            set_prec_impl<false>(real_dd_prec(n));
        }
    }
    // Special casing for bool.
    template <bool SetPrec>
    void dispatch_assignment(const bool &b)
    {
        dispatch_integral_ass_prec<SetPrec>(b);
        ::mpfr_set_ui(&m_mpfr, static_cast<unsigned long>(b), MPFR_RNDN);
    }
    template <bool SetPrec, typename T,
              enable_if_t<conjunction<std::is_integral<T>, std::is_unsigned<T>>::value, int> = 0>
    void dispatch_assignment(const T &n)
    {
        dispatch_integral_ass_prec<SetPrec>(n);
        if (n <= std::numeric_limits<unsigned long>::max()) {
            ::mpfr_set_ui(&m_mpfr, static_cast<unsigned long>(n), MPFR_RNDN);
        } else {
            ::mpfr_set_z(&m_mpfr, integer<2>(n).get_mpz_view(), MPFR_RNDN);
        }
    }
    template <bool SetPrec, typename T,
              enable_if_t<conjunction<std::is_integral<T>, std::is_signed<T>>::value, int> = 0>
    void dispatch_assignment(const T &n)
    {
        dispatch_integral_ass_prec<SetPrec>(n);
        if (n <= std::numeric_limits<long>::max() && n >= std::numeric_limits<long>::min()) {
            ::mpfr_set_si(&m_mpfr, static_cast<long>(n), MPFR_RNDN);
        } else {
            ::mpfr_set_z(&m_mpfr, integer<2>(n).get_mpz_view(), MPFR_RNDN);
        }
    }
    template <bool SetPrec, std::size_t SSize>
    void dispatch_assignment(const integer<SSize> &n)
    {
        if (SetPrec) {
            set_prec_impl<false>(real_dd_prec(n));
        }
        ::mpfr_set_z(&m_mpfr, n.get_mpz_view(), MPFR_RNDN);
    }
    template <bool SetPrec, std::size_t SSize>
    void dispatch_assignment(const rational<SSize> &q)
    {
        if (SetPrec) {
            set_prec_impl<false>(real_dd_prec(q));
        }
        const auto v = get_mpq_view(q);
        ::mpfr_set_q(&m_mpfr, &v, MPFR_RNDN);
    }
#if defined(MPPP_WITH_QUADMATH)
    template <bool SetPrec>
    void dispatch_assignment(const real128 &x)
    {
        if (SetPrec) {
            set_prec_impl<false>(real_dd_prec(x));
        }
        assign_real128(x);
    }
#endif

public:
    /// Generic assignment operator.
    /**
     * \rststar
     * The generic assignment operator will set ``this`` to the value of ``x``.
     *
     * The operator will first fetch the default precision ``dp`` via
     * :cpp:func:`~mppp::real_get_default_prec()`. If ``dp`` is nonzero, then ``dp`` will be used
     * as a new precision for ``this`` and a rounding operation might occurr during the assignment.
     *
     * Otherwise, if ``dp`` is zero, the precision of ``this`` will be set according to the same
     * heuristics described in the generic constructor.
     * \endrststar
     *
     * @param x the assignment argument.
     *
     * @return a reference to \p this.
     *
     * @throws std::overflow_error if an overflow occurs in the computation of the automatically-deduced precision.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    real &operator=(const RealInteroperable &x)
#else
    template <typename T, real_interoperable_enabler<T> = 0>
    real &operator=(const T &x)
#endif
    {
        dispatch_assignment<true>(x);
        return *this;
    }

private:
    // Implementation of the assignment from string.
    void string_assignment_impl(const char *s, int base)
    {
        if (mppp_unlikely(base && (base < 2 || base > 62))) {
            throw std::invalid_argument("Cannot assign a real from a string in base " + std::to_string(base)
                                        + ": the base must either be zero or in the [2,62] range");
        }
        const auto ret = ::mpfr_set_str(&m_mpfr, s, base, MPFR_RNDN);
        if (mppp_unlikely(ret == -1)) {
            ::mpfr_set_nan(&m_mpfr);
            throw std::invalid_argument(std::string{"The string '"} + s
                                        + "' cannot be interpreted as a floating-point value in base "
                                        + std::to_string(base));
        }
    }
    // Dispatching for string assignment.
    real &string_assignment(const char *s)
    {
        const auto dp = real_get_default_prec();
        if (mppp_unlikely(!dp)) {
            throw std::invalid_argument("Cannot assign a string to a real if a default precision is not set");
        }
        set_prec_impl<false>(dp);
        string_assignment_impl(s, 10);
        return *this;
    }
    real &string_assignment(const std::string &s)
    {
        return string_assignment(s.c_str());
    }
#if MPPP_CPLUSPLUS >= 201703L
    real &string_assignment(const std::string_view &s)
    {
        MPPP_MAYBE_TLS std::vector<char> buffer;
        buffer.assign(s.begin(), s.end());
        buffer.emplace_back('\0');
        return string_assignment(buffer.data());
    }
#endif

public:
    /// Assignment from string.
    /**
     * \rststar
     * This operator will set ``this`` to the value represented by the :cpp:concept:`~mppp::StringType` ``s``, which is
     * interpreted as a floating-point number in base 10. The precision of ``this`` will be set to the value returned by
     * :cpp:func:`~mppp::real_get_default_prec()`. If no default precision has been set, an error will be raised. If
     * ``s`` is not a valid representation of a floating-point number in base 10, ``this`` will be set to NaN and an
     * error will be raised.
     * \endrststar
     *
     * @param s the string that will be assigned to \p this.
     *
     * @return a reference to \p this.
     *
     * @throws std::invalid_argument if a default precision has not been set, or if \p s cannot be parsed
     * as a floating-point value in base 10.
     * @throws unspecified any exception thrown by memory allocation errors in standard containers.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    real &operator=(const StringType &s)
#else
    template <typename T, string_type_enabler<T> = 0>
    real &operator=(const T &s)
#endif
    {
        return string_assignment(s);
    }
    /// Copy assignment from ``mpfr_t``.
    /**
     * This operator will set ``this`` to a deep copy of ``x``.
     *
     * \rststar
     * .. warning::
     *    It is the user's responsibility to ensure that ``x`` has been correctly initialised
     *    with a precision within the bounds established by :cpp:func:`~mppp::real_prec_min()`
     *    and :cpp:func:`~mppp::real_prec_max()`.
     * \endrststar
     *
     * @param x the ``mpfr_t`` that will be copied.
     *
     * @return a reference to \p this.
     */
    real &operator=(const ::mpfr_t x)
    {
        // Set the precision, assuming the prec of x is valid.
        set_prec_impl<false>(mpfr_get_prec(x));
        // Set the value.
        ::mpfr_set(&m_mpfr, x, MPFR_RNDN);
        return *this;
    }
    /// Move assignment from ``mpfr_t``.
    /**
     * This operator will set ``this`` to a shallow copy of ``x``.
     *
     * \rststar
     * .. warning::
     *    It is the user's responsibility to ensure that ``x`` has been correctly initialised
     *    with a precision within the bounds established by :cpp:func:`~mppp::real_prec_min()`
     *    and :cpp:func:`~mppp::real_prec_max()`.
     *
     *    Additionally, the user must ensure that, after the assignment, ``mpfr_clear()`` is never
     *    called on ``x``: the resources previously owned by ``x`` are now owned by ``this``, which
     *    will take care of releasing them when the destructor is called.
     * \endrststar
     *
     * @param x the ``mpfr_t`` that will be moved.
     *
     * @return a reference to \p this.
     */
    real &operator=(::mpfr_t &&x)
    {
        // Clear this.
        ::mpfr_clear(&m_mpfr);
        // Shallow copy x.
        m_mpfr = *x;
        return *this;
    }
    /// Set to another \link mppp::real real\endlink value.
    /**
     * \rststar
     * This method will set ``this`` to the value of ``other``. Contrary to the copy assignment operator,
     * the precision of the assignment is dictated by the precision of ``this``, rather than
     * the precision of ``other``. Consequently, the precision of ``this`` will not be altered by the
     * assignment, and a rounding might occur, depending on the values
     * and the precisions of the operands.
     *
     * This method is a thin wrapper around the ``mpfr_set()`` assignment function from the MPFR API.
     *
     * .. seealso ::
     *    http://www.mpfr.org/mpfr-current/mpfr.html#Assignment-Functions
     * \endrststar
     *
     * @param other the value to which \p this will be set.
     *
     * @return a reference to \p this.
     */
    real &set(const real &other)
    {
        return set(&other.m_mpfr);
    }
        /// Generic setter.
        /**
         * \rststar
         * This method will set ``this`` to the value of ``x``. Contrary to the generic assignment operator,
         * the precision of the assignment is dictated by the precision of ``this``, rather than
         * being deduced from the type and value of ``x``. Consequently, the precision of ``this`` will not be altered
         * by the assignment, and a rounding might occur, depending on the operands.
         *
         * This method is a thin wrapper around various ``mpfr_set_*()``
         * assignment functions from the MPFR API.
         *
         * .. seealso ::
         *    http://www.mpfr.org/mpfr-current/mpfr.html#Assignment-Functions
         * \endrststar
         *
         * @param x the value to which \p this will be set.
         *
         * @return a reference to \p this.
         */
#if defined(MPPP_HAVE_CONCEPTS)
    real &set(const RealInteroperable &x)
#else
    template <typename T, real_interoperable_enabler<T> = 0>
    real &set(const T &x)
#endif
    {
        dispatch_assignment<false>(x);
        return *this;
    }

private:
    // Implementation of string setters.
    real &set_impl(const char *s, int base)
    {
        string_assignment_impl(s, base);
        return *this;
    }
    real &set_impl(const std::string &s, int base)
    {
        return set(s.c_str(), base);
    }
#if MPPP_CPLUSPLUS >= 201703L
    real &set_impl(const std::string_view &s, int base)
    {
        return set(s.data(), s.data() + s.size(), base);
    }
#endif

public:
    /// Setter to string.
    /**
     * \rststar
     * This method will set ``this`` to the value represented by the :cpp:concept:`~mppp::StringType` ``s``, which will
     * be interpreted as a floating-point number in base ``base``. ``base`` must be either 0 (in which case the base is
     * automatically deduced), or a value in the [2,62] range. Contrary to the assignment operator from string,
     * the global default precision is ignored and the precision of the assignment is dictated by the precision of
     * ``this``. Consequently, the precision of ``this`` will not be altered by the assignment, and a rounding might
     * occur, depending on the operands.
     *
     * If ``s`` is not a valid representation of a floating-point number in base ``base``, ``this``
     * will be set to NaN and an error will be raised.
     *
     * This method is a thin wrapper around the ``mpfr_set_str()`` assignment function from the MPFR API.
     *
     * .. seealso ::
     *    http://www.mpfr.org/mpfr-current/mpfr.html#Assignment-Functions
     * \endrststar
     *
     * @param s the string to which \p this will be set.
     * @param base the base used in the string representation.
     *
     * @return a reference to \p this.
     *
     * @throws std::invalid_argument if \p s cannot be parsed as a floating-point value in base 10, or if the value
     * of \p base is invalid.
     * @throws unspecified any exception thrown by memory
     * allocation errors in standard containers.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    real &set(const StringType &s,
#else
    template <typename T, string_type_enabler<T> = 0>
    real &set(const T &s,
#endif
              int base = 10)
    {
        return set_impl(s, base);
    }
    /// Set to character range.
    /**
     * This setter will set \p this to the content of the input half-open range,
     * which is interpreted as the string representation of a floating-point value in base \p base.
     *
     * Internally, the setter will copy the content of the range to a local buffer, add a
     * string terminator, and invoke the setter to string.
     *
     * @param begin the start of the input range.
     * @param end the end of the input range.
     * @param base the base used in the string representation.
     *
     * @return a reference to \p this.
     *
     * @throws unspecified any exception thrown by the setter to string, or by memory
     * allocation errors in standard containers.
     */
    real &set(const char *begin, const char *end, int base = 10)
    {
        MPPP_MAYBE_TLS std::vector<char> buffer;
        buffer.assign(begin, end);
        buffer.emplace_back('\0');
        return set(buffer.data(), base);
    }
    /// Set to an ``mpfr_t``.
    /**
     * \rststar
     * This method will set ``this`` to the value of ``x``. Contrary to the corresponding assignment operator,
     * the precision of the assignment is dictated by the precision of ``this``, rather than
     * the precision of ``x``. Consequently, the precision of ``this`` will not be altered by the
     * assignment, and a rounding might occur, depending on the values
     * and the precisions of the operands.
     *
     * This method is a thin wrapper around the ``mpfr_set()`` assignment function from the MPFR API.
     *
     * .. warning::
     *    It is the user's responsibility to ensure that ``x`` has been correctly initialised.
     *
     * .. seealso ::
     *    http://www.mpfr.org/mpfr-current/mpfr.html#Assignment-Functions
     * \endrststar
     *
     * @param x the ``mpfr_t`` to which \p this will be set.
     *
     * @return a reference to \p this.
     */
    real &set(const ::mpfr_t x)
    {
        ::mpfr_set(&m_mpfr, x, MPFR_RNDN);
        return *this;
    }
    /// Set to NaN.
    /**
     * This setter will set \p this to NaN with an unspecified sign bit. The precision of \p this
     * will not be altered.
     *
     * @return a reference to \p this.
     */
    real &set_nan()
    {
        ::mpfr_set_nan(&m_mpfr);
        return *this;
    }
    /// Set to infinity.
    /**
     * This setter will set \p this to infinity. The sign bit will be positive if \p sign
     * is nonnegative, negative otherwise. The precision of \p this will not be altered.
     *
     * @param sign the sign of the infinity to which \p this will be set.
     *
     * @return a reference to \p this.
     */
    real &set_inf(int sign = 0)
    {
        ::mpfr_set_inf(&m_mpfr, sign);
        return *this;
    }
    /// Set to zero.
    /**
     * This setter will set \p this to zero. The sign bit will be positive if \p sign
     * is nonnegative, negative otherwise. The precision of \p this will not be altered.
     *
     * @param sign the sign of the zero to which \p this will be set.
     *
     * @return a reference to \p this.
     */
    real &set_zero(int sign = 0)
    {
        ::mpfr_set_zero(&m_mpfr, sign);
        return *this;
    }
    /// Const reference to the internal <tt>mpfr_t</tt>.
    /**
     * This method will return a const pointer to the internal MPFR structure used
     * to represent the value of \p this. The returned object can be used as a
     * <tt>const mpfr_t</tt> function parameter in the MPFR API.
     *
     * @return a const reference to the internal MPFR structure.
     */
    const mpfr_struct_t *get_mpfr_t() const
    {
        return &m_mpfr;
    }
    /// Mutable reference to the internal <tt>mpfr_t</tt>.
    /**
     * This method will return a mutable pointer to the internal MPFR structure used
     * to represent the value of \p this. The returned object can be used as an
     * <tt>mpfr_t</tt> function parameter in the MPFR API.
     *
     * \rststar
     * .. warning::
     *    When using this mutable getter, it is the user's responsibility to ensure
     *    that the internal MPFR structure is kept in a state which respects the invariants
     *    of the :cpp:class:`~mppp::real` class. Specifically, the precision value
     *    must be in the bounds established by :cpp:func:`~mppp::real_prec_min()` and
     *    :cpp:func:`~mppp::real_prec_max()`, and upon destruction a :cpp:class:`~mppp::real`
     *    object must contain a valid ``mpfr_t`` object.
     * \endrststar
     *
     * @return a mutable reference to the internal MPFR structure.
     */
    mpfr_struct_t *_get_mpfr_t()
    {
        return &m_mpfr;
    }
    /// Detect NaN.
    /**
     * @return \p true if \p this is NaN, \p false otherwise.
     */
    bool nan_p() const
    {
        return mpfr_nan_p(&m_mpfr) != 0;
    }
    /// Detect infinity.
    /**
     * @return \p true if \p this is an infinity, \p false otherwise.
     */
    bool inf_p() const
    {
        return mpfr_inf_p(&m_mpfr) != 0;
    }
    /// Detect finite number.
    /**
     * @return \p true if \p this is a finite number (i.e., not NaN or infinity), \p false otherwise.
     */
    bool number_p() const
    {
        return mpfr_number_p(&m_mpfr) != 0;
    }
    /// Detect zero.
    /**
     * @return \p true if \p this is zero, \p false otherwise.
     */
    bool zero_p() const
    {
        return mpfr_zero_p(&m_mpfr) != 0;
    }
    /// Detect regular number.
    /**
     * @return \p true if \p this is a regular number (i.e., not NaN, infinity or zero), \p false otherwise.
     */
    bool regular_p() const
    {
        return mpfr_regular_p(&m_mpfr) != 0;
    }
    /// Detect sign.
    /**
     * @return a positive value if \p this is positive, zero if \p this is zero, a negative value if \p this
     * is negative.
     *
     * @throws std::domain_error if \p this is NaN.
     */
    int sgn() const
    {
        if (mppp_unlikely(nan_p())) {
            // NOTE: mpfr_sgn() in this case would set an error flag, and we generally
            // handle error flags as exceptions.
            throw std::domain_error("Cannot determine the sign of a real NaN");
        }
        return mpfr_sgn(&m_mpfr);
    }
    /// Get the sign bit.
    /**
     * @return the sign bit of \p this.
     */
    bool signbit() const
    {
        return mpfr_signbit(&m_mpfr) != 0;
    }
    /// Get the precision of \p this.
    /**
     * @return the current precision (i.e., the number of binary digits in the significand) of \p this.
     */
    ::mpfr_prec_t get_prec() const
    {
        return mpfr_get_prec(&m_mpfr);
    }
    /// Negate in-place.
    /**
     * This method will set ``this`` to ``-this``.
     *
     * @return a reference to ``this``.
     */
    real &neg()
    {
        ::mpfr_neg(&m_mpfr, &m_mpfr, MPFR_RNDN);
        return *this;
    }
    /// In-place absolute value.
    /**
     * This method will set ``this`` to its absolute value.
     *
     * @return a reference to ``this``.
     */
    real &abs()
    {
        ::mpfr_abs(&m_mpfr, &m_mpfr, MPFR_RNDN);
        return *this;
    }

private:
    // Utility function to check precision in set_prec().
    static ::mpfr_prec_t check_set_prec(::mpfr_prec_t p)
    {
        if (mppp_unlikely(!real_prec_check(p))) {
            throw std::invalid_argument("Cannot set the precision of a real to the value " + std::to_string(p)
                                        + ": the maximum allowed precision is " + std::to_string(real_prec_max())
                                        + ", the minimum allowed precision is " + std::to_string(real_prec_min()));
        }
        return p;
    }
    // mpfr_set_prec() wrapper, with or without prec checking.
    template <bool Check>
    void set_prec_impl(::mpfr_prec_t p)
    {
        ::mpfr_set_prec(&m_mpfr, Check ? check_set_prec(p) : p);
    }
    // mpfr_prec_round() wrapper, with or without prec checking.
    template <bool Check>
    void prec_round_impl(::mpfr_prec_t p)
    {
        ::mpfr_prec_round(&m_mpfr, Check ? check_set_prec(p) : p, MPFR_RNDN);
    }

public:
    /// Destructively set the precision.
    /**
     * \rststar
     * This method will set the precision of ``this`` to exactly ``p`` bits. The value
     * of ``this`` will be set to NaN.
     * \endrststar
     *
     * @param p the desired precision.
     *
     * @return a reference to \p this.
     *
     * @throws std::invalid_argument if the value of \p p is not in the range established by
     * \link mppp::real_prec_min() real_prec_min()\endlink and \link mppp::real_prec_max() real_prec_max()\endlink.
     */
    real &set_prec(::mpfr_prec_t p)
    {
        set_prec_impl<true>(p);
        return *this;
    }
    /// Set the precision maintaining the current value.
    /**
     * \rststar
     * This method will set the precision of ``this`` to exactly ``p`` bits. If ``p``
     * is smaller than the current precision of ``this``, a rounding operation will be performed,
     * otherwise the value will be preserved exactly.
     * \endrststar
     *
     * @param p the desired precision.
     *
     * @return a reference to \p this.
     *
     * @throws std::invalid_argument if the value of \p p is not in the range established by
     * \link mppp::real_prec_min() real_prec_min()\endlink and \link mppp::real_prec_max() real_prec_max()\endlink.
     */
    real &prec_round(::mpfr_prec_t p)
    {
        prec_round_impl<true>(p);
        return *this;
    }

private:
    // Generic conversion.
    // integer.
    template <typename T, enable_if_t<is_integer<T>::value, int> = 0>
    T dispatch_conversion() const
    {
        if (mppp_unlikely(!number_p())) {
            throw std::domain_error("Cannot convert a non-finite real to an integer");
        }
        MPPP_MAYBE_TLS mpz_raii mpz;
        // Truncate the value when converting to integer.
        ::mpfr_get_z(&mpz.m_mpz, &m_mpfr, MPFR_RNDZ);
        return T{&mpz.m_mpz};
    }
    // rational.
    template <std::size_t SSize>
    bool rational_conversion(rational<SSize> &rop) const
    {
        // Clear the range error flag before attempting the conversion.
        ::mpfr_clear_erangeflag();
        // NOTE: this call can fail if the exponent of this is very close to the upper/lower limits of the exponent
        // type. If the call fails (signalled by a range flag being set), we will return error.
        MPPP_MAYBE_TLS mpz_raii mpz;
        const ::mpfr_exp_t exp2 = ::mpfr_get_z_2exp(&mpz.m_mpz, &m_mpfr);
        // NOTE: not sure at the moment how to trigger this, let's leave it for now.
        // LCOV_EXCL_START
        if (mppp_unlikely(::mpfr_erangeflag_p())) {
            // Let's first reset the error flag.
            ::mpfr_clear_erangeflag();
            return false;
        }
        // LCOV_EXCL_STOP
        // The conversion to n * 2**exp succeeded. We will build a rational
        // from n and exp.
        rop._get_num() = &mpz.m_mpz;
        rop._get_den().set_one();
        if (exp2 >= ::mpfr_exp_t(0)) {
            // The output value will be an integer.
            rop._get_num() <<= static_cast<make_unsigned_t<::mpfr_exp_t>>(exp2);
        } else {
            // The output value will be a rational. Canonicalisation will be needed.
            rop._get_den() <<= nint_abs(exp2);
            canonicalise(rop);
        }
        return true;
    }
    template <typename T, enable_if_t<is_rational<T>::value, int> = 0>
    T dispatch_conversion() const
    {
        if (mppp_unlikely(!number_p())) {
            throw std::domain_error("Cannot convert a non-finite real to a rational");
        }
        T rop;
        // LCOV_EXCL_START
        if (mppp_unlikely(!rational_conversion(rop))) {
            throw std::overflow_error("The exponent of a real is too large for conversion to rational");
        }
        // LCOV_EXCL_STOP
        return rop;
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
    template <typename T>
    bool uint_conversion(T &rop) const
    {
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
                return mppp::get(rop, static_cast<integer<2>>(*this));
            }
            return false;
        }
        if (candidate <= std::numeric_limits<T>::max()) {
            rop = static_cast<T>(candidate);
            return true;
        }
        return false;
    }
    template <typename T,
              enable_if_t<conjunction<negation<std::is_same<bool, T>>, std::is_integral<T>, std::is_unsigned<T>>::value,
                          int> = 0>
    T dispatch_conversion() const
    {
        if (mppp_unlikely(!number_p())) {
            throw std::domain_error("Cannot convert a non-finite real to a C++ unsigned integral type");
        }
        T rop;
        if (mppp_unlikely(!uint_conversion(rop))) {
            raise_overflow_error<T>();
        }
        return rop;
    }
    // bool.
    template <typename T, enable_if_t<std::is_same<bool, T>::value, int> = 0>
    T dispatch_conversion() const
    {
        // NOTE: in C/C++ the conversion of NaN to bool gives true:
        // https://stackoverflow.com/questions/9158567/nan-to-bool-conversion-true-or-false
        return !zero_p();
    }
    // Signed integrals.
    template <typename T>
    bool sint_conversion(T &rop) const
    {
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
                return mppp::get(rop, static_cast<integer<2>>(*this));
            }
            return false;
        }
        if (candidate >= std::numeric_limits<T>::min() && candidate <= std::numeric_limits<T>::max()) {
            rop = static_cast<T>(candidate);
            return true;
        }
        return false;
    }
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_signed<T>>::value, int> = 0>
    T dispatch_conversion() const
    {
        if (mppp_unlikely(!number_p())) {
            throw std::domain_error("Cannot convert a non-finite real to a C++ signed integral type");
        }
        T rop;
        if (mppp_unlikely(!sint_conversion(rop))) {
            raise_overflow_error<T>();
        }
        return rop;
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
        auto nlimbs = prec / ::mp_bits_per_limb + static_cast<bool>(prec % ::mp_bits_per_limb);
        assert(nlimbs != 0);
        // NOTE: in MPFR the most significant (nonzero) bit of the significand
        // is always at the high end of the most significand limb. In other words,
        // MPFR pads the multiprecision significand on the right, the opposite
        // of GMP integers (which have padding on the left, i.e., in the top limb).
        //
        // NOTE: contrary to real128, the MPFR format does not have a hidden bit on top.
        //
        // NOTE: it's not clear to me here if we should MASK or not. Let's keep it for consistency
        // with integer etc., but at one point we should probably have some nail-enabled build
        // to check this.
        //
        // Init retval with the highest limb.
        real128 retval{m_mpfr._mpfr_d[--nlimbs] & GMP_NUMB_MASK};
        // Init the number of read bits.
        // NOTE: we have read a full limb in the line above, so mp_bits_per_limb bits. If mp_bits_per_limb > 113,
        // then the constructor of real128 truncated the input limb value to 113 bits of precision, so effectively
        // we have read 113 bits only in such a case.
        unsigned read_bits = c_min(static_cast<unsigned>(::mp_bits_per_limb), real128_sig_digits());
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
        // cannot be larger than 113.
        retval = scalbln(retval, static_cast<long>(m_mpfr._mpfr_exp) - static_cast<long>(read_bits));
        return sgn() > 0 ? retval : -retval;
    }
#endif

public:
    /// Generic conversion operator.
    /**
     * \rststar
     * This operator will convert ``this`` to a :cpp:concept:`~mppp::RealInteroperable` type. The conversion
     * proceeds as follows:
     *
     * - if ``T`` is ``bool``, then the conversion returns ``false`` if ``this`` is zero, ``true`` otherwise
     *   (including if ``this`` is NaN);
     * - if ``T`` is a C++ integral type other than ``bool``, the conversion will yield the truncated counterpart
     *   of ``this`` (i.e., the conversion rounds to zero). The conversion may fail due to overflow or domain errors
     *   (i.e., when trying to convert non-finite values);
     * - if ``T`` if a C++ floating-point type, the conversion calls directly the low-level MPFR functions (e.g.,
     *   ``mpfr_get_d()``), and might yield infinities for finite input values;
     * - if ``T`` is :cpp:class:`~mppp::integer`, the conversion rounds to zero and might fail due to domain errors,
     *   but it will never overflow;
     * - if ``T`` is :cpp:class:`~mppp::rational`, the conversion may fail if ``this`` is not finite or if the
     *   conversion produces an overflow in the manipulation of the exponent of ``this`` (that is, if
     *   the absolute value of ``this`` is very large or very small). If the conversion succeeds, it will be exact;
     * - if ``T`` is :cpp:class:`~mppp::real128`, the conversion might yield infinities for finite input values.
     *
     * \endrststar
     *
     * @return \p this converted to \p T.
     *
     * @throws std::domain_error if \p this is not finite and the target type cannot represent non-finite numbers.
     * @throws std::overflow_error if the conversion results in overflow.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    template <RealInteroperable T>
#else
    template <typename T, real_interoperable_enabler<T> = 0>
#endif
    explicit operator T() const
    {
        return dispatch_conversion<T>();
    }

private:
    template <std::size_t SSize>
    bool dispatch_get(integer<SSize> &rop) const
    {
        if (!number_p()) {
            return false;
        }
        MPPP_MAYBE_TLS mpz_raii mpz;
        // Truncate the value when converting to integer.
        ::mpfr_get_z(&mpz.m_mpz, &m_mpfr, MPFR_RNDZ);
        rop = &mpz.m_mpz;
        return true;
    }
    template <std::size_t SSize>
    bool dispatch_get(rational<SSize> &rop) const
    {
        if (!number_p()) {
            return false;
        }
        return rational_conversion(rop);
    }
    bool dispatch_get(bool &b) const
    {
        b = !zero_p();
        return true;
    }
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_unsigned<T>>::value, int> = 0>
    bool dispatch_get(T &n) const
    {
        if (!number_p()) {
            return false;
        }
        return uint_conversion(n);
    }
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_signed<T>>::value, int> = 0>
    bool dispatch_get(T &n) const
    {
        if (!number_p()) {
            return false;
        }
        return sint_conversion(n);
    }
    template <typename T, enable_if_t<std::is_floating_point<T>::value, int> = 0>
    bool dispatch_get(T &x) const
    {
        x = static_cast<T>(*this);
        return true;
    }
#if defined(MPPP_WITH_QUADMATH)
    bool dispatch_get(real128 &x) const
    {
        x = static_cast<real128>(*this);
        return true;
    }
#endif

public:
    /// Generic conversion method.
    /**
     * \rststar
     * This method, similarly to the conversion operator, will convert ``this`` to a
     * :cpp:concept:`~mppp::RealInteroperable` type, storing the result of the conversion into ``rop``. Differently
     * from the conversion operator, this method does not raise any exception: if the conversion is successful, the
     * method will return ``true``, otherwise the method will return ``false``. If the conversion fails,
     * ``rop`` will not be altered.
     * \endrststar
     *
     * @param rop the variable which will store the result of the conversion.
     *
     * @return ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail in the ways
     * specified in the documentation of the conversion operator.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    template <RealInteroperable T>
#else
    template <typename T, real_interoperable_enabler<T> = 0>
#endif
    bool get(T &rop) const
    {
        return dispatch_get(rop);
    }

    /// Convert to string.
    /**
     * \rststar
     * This method will convert ``this`` to a string representation in base ``base``. The returned string is guaranteed
     * to produce exactly the original value when used in one of the constructors from string of
     * :cpp:class:`~mppp::real` (provided that the original precision and base are used in the construction).
     * \endrststar
     *
     * @param base the base to be used for the string representation.
     *
     * @return \p this converted to a string.
     *
     * @throws std::invalid_argument if \p base is not in the [2,62] range.
     * @throws std::runtime_error if the call to the ``mpfr_get_str()`` function of the MPFR API fails.
     */
    std::string to_string(int base = 10) const
    {
        std::ostringstream oss;
        mpfr_to_stream(&m_mpfr, oss, base);
        return oss.str();
    }

private:
    mpfr_struct_t m_mpfr;
};

template <typename T, typename U>
using are_real_op_types
    = disjunction<conjunction<std::is_same<real, uncvref_t<T>>, std::is_same<real, uncvref_t<U>>>,
                  conjunction<std::is_same<real, uncvref_t<T>>, is_real_interoperable<uncvref_t<U>>>,
                  conjunction<std::is_same<real, uncvref_t<U>>, is_real_interoperable<uncvref_t<T>>>>;

template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool RealOpTypes = (CvrReal<T> && CvrReal<U>) || (CvrReal<T> && RealInteroperable<uncvref_t<U>>)
                           || (CvrReal<U> && RealInteroperable<uncvref_t<T>>);
#else
using real_op_types_enabler = enable_if_t<are_real_op_types<T, U>::value, int>;
#endif

template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool RealCompoundOpTypes = RealOpTypes<T, U> && !std::is_const<unref_t<T>>::value;
#else
using real_compound_op_types_enabler
    = enable_if_t<conjunction<are_real_op_types<T, U>, negation<std::is_const<unref_t<T>>>>::value, int>;
#endif

/// Destructively set the precision of a \link mppp::real real\endlink.
/**
 * \ingroup real_prec
 * \rststar
 * This function will set the precision of ``r`` to exactly ``p`` bits. The value
 * of ``r`` will be set to NaN.
 * \endrststar
 *
 * @param r the \link mppp::real real\endlink whose precision will be set.
 * @param p the desired precision.
 *
 * @throws unspecified any exception thrown by mppp::real::set_prec().
 */
inline void set_prec(real &r, ::mpfr_prec_t p)
{
    r.set_prec(p);
}

/// Set the precision of a \link mppp::real real\endlink maintaining the current value.
/**
 * \ingroup real_prec
 * \rststar
 * This function will set the precision of ``r`` to exactly ``p`` bits. If ``p``
 * is smaller than the current precision of ``r``, a rounding operation will be performed,
 * otherwise the value will be preserved exactly.
 * \endrststar
 *
 * @param r the \link mppp::real real\endlink whose precision will be set.
 * @param p the desired precision.
 *
 * @throws unspecified any exception thrown by mppp::real::prec_round().
 */
inline void prec_round(real &r, ::mpfr_prec_t p)
{
    r.prec_round(p);
}

/// Get the precision of a \link mppp::real real\endlink.
/**
 * \ingroup real_prec
 *
 * @param r the \link mppp::real real\endlink whose precision will be returned.
 *
 * @return the precision of \p r.
 */
inline mpfr_prec_t get_prec(const real &r)
{
    return r.get_prec();
}

inline namespace detail
{

template <typename... Args>
using real_set_t = decltype(std::declval<real &>().set(std::declval<const Args &>()...));
}

/** @defgroup real_assignment real_assignment
 *  @{
 */

#if !defined(MPPP_DOXYGEN_INVOKED)

template <typename... Args>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool RealSetArgs = is_detected<real_set_t, Args...>::value;
#else
using real_set_args_enabler = enable_if_t<is_detected<real_set_t, Args...>::value, int>;
#endif

#endif

/// Generic setter for \link mppp::real real\endlink.
/**
 * \rststar
 * This function will use the arguments ``args`` to set the value of the :cpp:class:`~mppp::real` ``r``,
 * using one of the available :cpp:func:`mppp::real::set()` overloads. That is,
 * the body of this function is equivalent to
 *
 * .. code-block:: c++
 *
 *    return r.set(args...);
 *
 * The input arguments must satisfy the :cpp:concept:`~mppp::RealSetArgs` concept.
 * \endrststar
 *
 * @param r the \link mppp::real real\endlink that will be set.
 * @param args the arguments that will be passed to mppp::real::set().
 *
 * @return a reference to \p r.
 *
 * @throws unspecified any exception thrown by the invoked mppp::real::set() overload.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <RealSetArgs... Args>
#else
template <typename... Args, real_set_args_enabler<Args...> = 0>
#endif
inline real &set(real &r, const Args &... args)
{
    return r.set(args...);
}

/// Set \link mppp::real real\endlink to NaN.
/**
 * This function will set \p r to NaN with an unspecified sign bit. The precision of \p r
 * will not be altered.
 *
 * @param r the \link mppp::real real\endlink argument.
 *
 * @return a reference to \p r.
 */
inline real &set_nan(real &r)
{
    return r.set_nan();
}

/// Set \link mppp::real real\endlink to infinity.
/**
 * This function will set \p r to infinity. The sign bit will be positive if \p sign
 * is nonnegative, negative otherwise. The precision of \p r will not be altered.
 *
 * @param r the \link mppp::real real\endlink argument.
 * @param sign the sign of the infinity to which \p r will be set.
 *
 * @return a reference to \p r.
 */
inline real &set_inf(real &r, int sign = 0)
{
    return r.set_inf(sign);
}

/// Set \link mppp::real real\endlink to zero.
/**
 * This function will set \p r to zero. The sign bit will be positive if \p sign
 * is nonnegative, negative otherwise. The precision of \p r will not be altered.
 *
 * @param r the \link mppp::real real\endlink argument.
 * @param sign the sign of the zero to which \p r will be set.
 *
 * @return a reference to \p r.
 */
inline real &set_zero(real &r, int sign = 0)
{
    return r.set_zero(sign);
}

/// Swap \link mppp::real real\endlink objects.
/**
 * This function will efficiently swap the content of \p a and \p b.
 *
 * @param a the first operand.
 * @param b the second operand.
 */
inline void swap(real &a, real &b) noexcept
{
    ::mpfr_swap(a._get_mpfr_t(), b._get_mpfr_t());
}

    /** @} */

    /** @defgroup real_conversion real_conversion
     *  @{
     */

    /// Generic conversion function for \link mppp::real real\endlink.
    /**
     * \rststar
     * This function will convert the input :cpp:class:`~mppp::real` ``x`` to a
     * :cpp:concept:`~mppp::RealInteroperable` type, storing the result of the conversion into ``rop``.
     * If the conversion is successful, the function
     * will return ``true``, otherwise the function will return ``false``. If the conversion fails, ``rop`` will
     * not be altered.
     * \endrststar
     *
     * @param rop the variable which will store the result of the conversion.
     * @param x the input \link mppp::real real\endlink.
     *
     * @return ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail in the ways
     * specified in the documentation of the conversion operator for \link mppp::real real\endlink.
     */
#if defined(MPPP_HAVE_CONCEPTS)
inline bool get(RealInteroperable &rop, const real &x)
#else
template <typename T, real_interoperable_enabler<T> = 0>
inline bool get(T &rop, const real &x)
#endif
{
    return x.get(rop);
}

/** @} */

inline namespace detail
{

#if !defined(MPPP_DOXYGEN_INVOKED)

// A small helper to init the pairs in the functions below. We need this because
// we cannot take the address of a const real as a real *.
template <typename Arg, enable_if_t<!is_ncrvr<Arg &&>::value, int> = 0>
inline std::pair<real *, ::mpfr_prec_t> mpfr_nary_op_init_pair(::mpfr_prec_t min_prec, Arg &&arg)
{
    // arg is not a non-const rvalue ref, we cannot steal from it. Init with nullptr.
    return std::make_pair(static_cast<real *>(nullptr), c_max(arg.get_prec(), min_prec));
}

template <typename Arg, enable_if_t<is_ncrvr<Arg &&>::value, int> = 0>
inline std::pair<real *, ::mpfr_prec_t> mpfr_nary_op_init_pair(::mpfr_prec_t min_prec, Arg &&arg)
{
    // arg is a non-const rvalue ref, and a candidate for stealing resources.
    return std::make_pair(&arg, c_max(arg.get_prec(), min_prec));
}

// A recursive function to determine, in an MPFR function call,
// the largest argument we can steal resources from, and the max precision among
// all the arguments.
inline void mpfr_nary_op_check_steal(std::pair<real *, ::mpfr_prec_t> &) {}

// NOTE: we need 2 overloads for this, as we cannot extract a non-const pointer from
// arg0 if arg0 is a const ref.
template <typename Arg0, typename... Args, enable_if_t<!is_ncrvr<Arg0 &&>::value, int> = 0>
inline void mpfr_nary_op_check_steal(std::pair<real *, ::mpfr_prec_t> &p, Arg0 &&arg0, Args &&... args)
{
    // arg0 is not a non-const rvalue ref, we won't be able to steal from it regardless. Just
    // update the max prec.
    p.second = c_max(arg0.get_prec(), p.second);
    mpfr_nary_op_check_steal(p, std::forward<Args>(args)...);
}

template <typename Arg0, typename... Args, enable_if_t<is_ncrvr<Arg0 &&>::value, int> = 0>
inline void mpfr_nary_op_check_steal(std::pair<real *, ::mpfr_prec_t> &p, Arg0 &&arg0, Args &&... args)
{
    const auto prec0 = arg0.get_prec();
    if (!p.first || prec0 > p.first->get_prec()) {
        // The current argument arg0 is a non-const rvalue reference, and either it's
        // the first argument we encounter we can steal from, or it has a precision
        // larger than the current candidate for stealing resources from. This means that
        // arg0 is the new candidate.
        p.first = &arg0;
    }
    // Update the max precision among the arguments, if necessary.
    p.second = c_max(prec0, p.second);
    mpfr_nary_op_check_steal(p, std::forward<Args>(args)...);
}

// Apply the MPFR n-ary function F with return value rop and real arguments (arg0, args...).
// The precision of rop will be set to the maximum of the precision among the arguments,
// but not less than min_prec.
// Resources may be stolen from one of the arguments, if possible.
template <typename F, typename Arg0, typename... Args>
inline real &mpfr_nary_op(::mpfr_prec_t min_prec, const F &f, real &rop, Arg0 &&arg0, Args &&... args)
{
    // This pair contains:
    // - a pointer to the largest-precision real from which we can steal resources (may be nullptr),
    // - the largest precision among all arguments.
    // It's inited with arg0's precision (but no less than min_prec), and a pointer to arg0, if arg0 is a nonconst
    // rvalue ref (a nullptr otherwise).
    auto p = mpfr_nary_op_init_pair(min_prec, std::forward<Arg0>(arg0));
    // Examine the remaining arguments.
    mpfr_nary_op_check_steal(p, std::forward<Args>(args)...);
    // Cache this for convenience.
    const auto r_prec = rop.get_prec();
    if (p.second == r_prec) {
        // The largest precision among the operands and the precision of the return value
        // match. No need to steal, just execute the function.
        f(rop._get_mpfr_t(), arg0.get_mpfr_t(), args.get_mpfr_t()..., MPFR_RNDN);
    } else {
        if (r_prec > p.second) {
            // The precision of the return value is larger than the largest precision
            // among the operands. We can reset its precision destructively
            // because we know it does not overlap with any operand.
            rop.set_prec_impl<false>(p.second);
            f(rop._get_mpfr_t(), arg0.get_mpfr_t(), args.get_mpfr_t()..., MPFR_RNDN);
        } else if (!p.first || p.first->get_prec() != p.second) {
            // This covers 2 cases:
            // - the precision of the return value is smaller than the largest precision
            //   among the operands and we cannot steal from any argument,
            // - the precision of the return value is smaller than the largest precision
            //   among the operands, we can steal from an argument but the target argument
            //   does not have enough precision.
            // In these cases, we will just set the precision of rop and call the function.
            // NOTE: we need to set the precision without destroying the rop, as it might
            // overlap with one of the arguments. Since this will be an increase in precision,
            // it should not entail a rounding operation.
            // NOTE: we assume all the precs in the operands are valid, so we will not need
            // to check them.
            rop.prec_round_impl<false>(p.second);
            f(rop._get_mpfr_t(), arg0.get_mpfr_t(), args.get_mpfr_t()..., MPFR_RNDN);
        } else {
            // The precision of the return value is smaller than the largest precision among the operands,
            // and we have a candidate for stealing with enough precision: we will use it as return
            // value and then swap out the result to rop.
            f(p.first->_get_mpfr_t(), arg0.get_mpfr_t(), args.get_mpfr_t()..., MPFR_RNDN);
            swap(*p.first, rop);
        }
    }
    return rop;
}

// Invoke an MPFR function with arguments (arg0, args...), and store the result
// in a value to be created by this function. If possible, this function will try
// to re-use the storage provided by the input arguments, if one or more of these
// arguments are rvalue references and their precision is large enough. As usual,
// the precision of the return value will be the max precision among the operands,
// but not less than min_prec.
template <typename F, typename Arg0, typename... Args>
inline real mpfr_nary_op_return(::mpfr_prec_t min_prec, const F &f, Arg0 &&arg0, Args &&... args)
{
    auto p = mpfr_nary_op_init_pair(min_prec, std::forward<Arg0>(arg0));
    mpfr_nary_op_check_steal(p, std::forward<Args>(args)...);
    if (p.first && p.first->get_prec() == p.second) {
        // There's at least one arg we can steal from, and its precision is large enough
        // to contain the result. Use it.
        f(p.first->_get_mpfr_t(), arg0.get_mpfr_t(), args.get_mpfr_t()..., MPFR_RNDN);
        return std::move(*p.first);
    }
    // Either we cannot steal from any arg, or the candidate(s) do not have enough precision.
    // Init a new value and use it instead.
    real retval{real::ptag{}, p.second, true};
    f(retval._get_mpfr_t(), arg0.get_mpfr_t(), args.get_mpfr_t()..., MPFR_RNDN);
    return retval;
}

#endif
}

/** @defgroup real_arithmetic real_arithmetic
 *  @{
 */

/// Ternary \link mppp::real real\endlink addition.
/**
 * This function will compute \f$a+b\f$, storing the result in \p rop.
 * The precision of the result will be set to the largest precision among the operands.
 *
 * @param rop the return value.
 * @param a the first operand.
 * @param b the second operand.
 *
 * @return a reference to \p rop.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <CvrReal T, CvrReal U>
#else
template <typename T, typename U, cvr_real_enabler<T, U> = 0>
#endif
inline real &add(real &rop, T &&a, U &&b)
{
    return mpfr_nary_op(0, ::mpfr_add, rop, std::forward<T>(a), std::forward<U>(b));
}

    /// Ternary \link mppp::real real\endlink subtraction.
    /**
     * This function will compute \f$a-b\f$, storing the result in \p rop.
     * The precision of the result will be set to the largest precision among the operands.
     *
     * @param rop the return value.
     * @param a the first operand.
     * @param b the second operand.
     *
     * @return a reference to \p rop.
     */
#if defined(MPPP_HAVE_CONCEPTS)
template <CvrReal T, CvrReal U>
#else
template <typename T, typename U, cvr_real_enabler<T, U> = 0>
#endif
inline real &sub(real &rop, T &&a, U &&b)
{
    return mpfr_nary_op(0, ::mpfr_sub, rop, std::forward<T>(a), std::forward<U>(b));
}

    /// Ternary \link mppp::real real\endlink multiplication.
    /**
     * This function will compute \f$a \times b\f$, storing the result in \p rop.
     * The precision of the result will be set to the largest precision among the operands.
     *
     * @param rop the return value.
     * @param a the first operand.
     * @param b the second operand.
     *
     * @return a reference to \p rop.
     */
#if defined(MPPP_HAVE_CONCEPTS)
template <CvrReal T, CvrReal U>
#else
template <typename T, typename U, cvr_real_enabler<T, U> = 0>
#endif
inline real &mul(real &rop, T &&a, U &&b)
{
    return mpfr_nary_op(0, ::mpfr_mul, rop, std::forward<T>(a), std::forward<U>(b));
}

    /// Ternary \link mppp::real real\endlink division.
    /**
     * This function will compute \f$a / b\f$, storing the result in \p rop.
     * The precision of the result will be set to the largest precision among the operands.
     *
     * @param rop the return value.
     * @param a the first operand.
     * @param b the second operand.
     *
     * @return a reference to \p rop.
     */
#if defined(MPPP_HAVE_CONCEPTS)
template <CvrReal T, CvrReal U>
#else
template <typename T, typename U, cvr_real_enabler<T, U> = 0>
#endif
inline real &div(real &rop, T &&a, U &&b)
{
    return mpfr_nary_op(0, ::mpfr_div, rop, std::forward<T>(a), std::forward<U>(b));
}

    /// Quaternary \link mppp::real real\endlink fused multiply–add.
    /**
     * This function will compute \f$a \times b + c\f$, storing the result in \p rop.
     * The precision of the result will be set to the largest precision among the operands.
     *
     * @param rop the return value.
     * @param a the first operand.
     * @param b the second operand.
     * @param c the third operand.
     *
     * @return a reference to \p rop.
     */
#if defined(MPPP_HAVE_CONCEPTS)
template <CvrReal T, CvrReal U, CvrReal V>
#else
template <typename T, typename U, typename V, cvr_real_enabler<T, U, V> = 0>
#endif
inline real &fma(real &rop, T &&a, U &&b, V &&c)
{
    return mpfr_nary_op(0, ::mpfr_fma, rop, std::forward<T>(a), std::forward<U>(b), std::forward<V>(c));
}

/// Ternary \link mppp::real real\endlink fused multiply–add.
/**
 * \rststar
 * This function will compute and return :math:`a \times b + c`.
 * The precision of the result will be set to the largest precision among the operands.
 * \endrststar
 *
 * @param a the first operand.
 * @param b the second operand.
 * @param c the third operand.
 *
 * @return \f$ a \times b + c \f$.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <CvrReal T, CvrReal U, CvrReal V>
#else
template <typename T, typename U, typename V, cvr_real_enabler<T, U, V> = 0>
#endif
inline real fma(T &&a, U &&b, V &&c)
{
    return mpfr_nary_op_return(0, ::mpfr_fma, std::forward<T>(a), std::forward<U>(b), std::forward<V>(c));
}

    /// Quaternary \link mppp::real real\endlink fused multiply–sub.
    /**
     * This function will compute \f$a \times b - c\f$, storing the result in \p rop.
     * The precision of the result will be set to the largest precision among the operands.
     *
     * @param rop the return value.
     * @param a the first operand.
     * @param b the second operand.
     * @param c the third operand.
     *
     * @return a reference to \p rop.
     */
#if defined(MPPP_HAVE_CONCEPTS)
template <CvrReal T, CvrReal U, CvrReal V>
#else
template <typename T, typename U, typename V, cvr_real_enabler<T, U, V> = 0>
#endif
inline real &fms(real &rop, T &&a, U &&b, V &&c)
{
    return mpfr_nary_op(0, ::mpfr_fms, rop, std::forward<T>(a), std::forward<U>(b), std::forward<V>(c));
}

/// Ternary \link mppp::real real\endlink fused multiply–sub.
/**
 * \rststar
 * This function will compute and return :math:`a \times b - c`.
 * The precision of the result will be set to the largest precision among the operands.
 * \endrststar
 *
 * @param a the first operand.
 * @param b the second operand.
 * @param c the third operand.
 *
 * @return \f$ a \times b - c \f$.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <CvrReal T, CvrReal U, CvrReal V>
#else
template <typename T, typename U, typename V, cvr_real_enabler<T, U, V> = 0>
#endif
inline real fms(T &&a, U &&b, V &&c)
{
    return mpfr_nary_op_return(0, ::mpfr_fms, std::forward<T>(a), std::forward<U>(b), std::forward<V>(c));
}

/// Unary negation for \link mppp::real real\endlink.
/**
 * @param x the \link mppp::real real\endlink that will be negated.
 *
 * @return the negative of \p x.
 */
#if defined(MPPP_HAVE_CONCEPTS)
inline real neg(CvrReal &&x)
#else
template <typename T, cvr_real_enabler<T> = 0>
inline real neg(T &&x)
#endif
{
    return mpfr_nary_op_return(0, ::mpfr_neg, std::forward<decltype(x)>(x));
}

/// Binary negation for \link mppp::real real\endlink.
/**
 * This function will set \p rop to the negation of \p x.
 *
 * @param rop the \link mppp::real real\endlink that will hold the result.
 * @param x the \link mppp::real real\endlink that will be negated.
 *
 * @return a reference to \p rop.
 */
#if defined(MPPP_HAVE_CONCEPTS)
inline real &neg(real &rop, CvrReal &&x)
#else
template <typename T, cvr_real_enabler<T> = 0>
inline real neg(real &rop, T &&x)
#endif
{
    return mpfr_nary_op(0, ::mpfr_neg, rop, std::forward<decltype(x)>(x));
}

/// Unary absolute value for \link mppp::real real\endlink.
/**
 * @param x the \link mppp::real real\endlink whose absolute value will be computed.
 *
 * @return the absolute value of \p x.
 */
#if defined(MPPP_HAVE_CONCEPTS)
inline real abs(CvrReal &&x)
#else
template <typename T, cvr_real_enabler<T> = 0>
inline real abs(T &&x)
#endif
{
    return mpfr_nary_op_return(0, ::mpfr_abs, std::forward<decltype(x)>(x));
}

/// Binary absolute value for \link mppp::real real\endlink.
/**
 * This function will set \p rop to the absolute value of \p x.
 *
 * @param rop the \link mppp::real real\endlink that will hold the result.
 * @param x the \link mppp::real real\endlink whose absolute value will be computed.
 *
 * @return a reference to \p rop.
 */
#if defined(MPPP_HAVE_CONCEPTS)
inline real &abs(real &rop, CvrReal &&x)
#else
template <typename T, cvr_real_enabler<T> = 0>
inline real abs(real &rop, T &&x)
#endif
{
    return mpfr_nary_op(0, ::mpfr_abs, rop, std::forward<decltype(x)>(x));
}

/** @} */

/** @defgroup real_comparison real_comparison
 *  @{
 */

/// Detect if a \link mppp::real real\endlink is NaN.
/**
 * @param r the \link mppp::real real\endlink that will be examined.
 *
 * @return \p true if \p r is NaN, \p false otherwise.
 */
inline bool nan_p(const real &r)
{
    return r.nan_p();
}

/// Detect if a \link mppp::real real\endlink is infinity.
/**
 * @param r the \link mppp::real real\endlink that will be examined.
 *
 * @return \p true if \p r is an infinity, \p false otherwise.
 */
inline bool inf_p(const real &r)
{
    return r.inf_p();
}

/// Detect if \link mppp::real real\endlink is a finite number.
/**
 * @param r the \link mppp::real real\endlink that will be examined.
 *
 * @return \p true if \p r is a finite number (i.e., not NaN or infinity), \p false otherwise.
 */
inline bool number_p(const real &r)
{
    return r.number_p();
}

/// Detect if a \link mppp::real real\endlink is zero.
/**
 * @param r the \link mppp::real real\endlink that will be examined.
 *
 * @return \p true if \p r is zero, \p false otherwise.
 */
inline bool zero_p(const real &r)
{
    return r.zero_p();
}

/// Detect if a \link mppp::real real\endlink is a regular number.
/**
 * @param r the \link mppp::real real\endlink that will be examined.
 *
 * @return \p true if \p r is a regular number (i.e., not NaN, infinity or zero), \p false otherwise.
 */
inline bool regular_p(const real &r)
{
    return r.regular_p();
}

/// Detect the sign of a \link mppp::real real\endlink.
/**
 * @param r the \link mppp::real real\endlink that will be examined.
 *
 * @return a positive value if \p r is positive, zero if \p r is zero, a negative value if \p thris
 * is negative.
 *
 * @throws unspecified any exception thrown by mppp::real::sgn().
 */
inline int sgn(const real &r)
{
    return r.sgn();
}

/// Get the sign bit of a \link mppp::real real\endlink.
/**
 * @param r the \link mppp::real real\endlink that will be examined.
 *
 * @return the sign bit of \p r.
 */
inline bool signbit(const real &r)
{
    return r.signbit();
}

/// \link mppp::real Real\endlink comparison.
/**
 * \rststar
 * This function will compare ``a`` and ``b``, returning:
 *
 * - zero if ``a`` equals ``b``,
 * - a negative value if ``a`` is less than ``b``,
 * - a positive value if ``a`` is greater than ``b``.
 *
 * If at least one NaN value is involved in the comparison, an error will be raised.
 *
 * This function is useful to distinguish the three possible cases. The comparison operators
 * are recommended instead if it is needed to distinguish only two cases.
 * \endrststar
 *
 * @param a the first operand.
 * @param b the second operand.
 *
 * @return an integral value expressing how ``a`` compares to ``b``.
 *
 * @throws std::domain_error if at least one of the operands is NaN.
 */
inline int cmp(const real &a, const real &b)
{
    ::mpfr_clear_erangeflag();
    auto retval = ::mpfr_cmp(a.get_mpfr_t(), b.get_mpfr_t());
    if (mppp_unlikely(::mpfr_erangeflag_p())) {
        ::mpfr_clear_erangeflag();
        throw std::domain_error("Cannot compare two reals if at least one of them is NaN");
    }
    return retval;
}

    /** @} */

    /** @defgroup real_roots real_roots
     *  @{
     */

    /// Binary \link mppp::real real\endlink square root.
    /**
     * @param rop the return value.
     * @param op the operand.
     *
     * @return a reference to \p rop.
     */
#if defined(MPPP_HAVE_CONCEPTS)
inline real &sqrt(real &rop, CvrReal &&op)
#else
template <typename T, cvr_real_enabler<T> = 0>
inline real &sqrt(real &rop, T &&op)
#endif
{
    return mpfr_nary_op(0, ::mpfr_sqrt, rop, std::forward<decltype(op)>(op));
}

/// Unary \link mppp::real real\endlink square root.
/**
 * @param r the operand.
 *
 * @return the square root of \p r.
 */
#if defined(MPPP_HAVE_CONCEPTS)
inline real sqrt(CvrReal &&r)
#else
template <typename T, cvr_real_enabler<T> = 0>
inline real sqrt(T &&r)
#endif
{
    return mpfr_nary_op_return(0, ::mpfr_sqrt, std::forward<decltype(r)>(r));
}

    /** @} */

    /** @defgroup real_exponentiation real_exponentiation
     *  @{
     */

/// Ternary \link mppp::real real\endlink exponentiation.
/**
 * This function will set \p rop to \p op1 raised to the power of \p op2.
 * The precision of \p rop will be set to the largest precision among the operands.
 *
 * @param rop the return value.
 * @param op1 the base.
 * @param op2 the exponent.
 *
 * @return a reference to \p rop.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <CvrReal T, CvrReal U>
#else
template <typename T, typename U, cvr_real_enabler<T, U> = 0>
#endif
inline real &pow(real &rop, T &&op1, U &&op2)
{
    return mpfr_nary_op(0, ::mpfr_pow, rop, std::forward<T>(op1), std::forward<U>(op2));
}

inline namespace detail
{

template <typename T, typename U,
          enable_if_t<conjunction<std::is_same<real, uncvref_t<T>>, std::is_same<real, uncvref_t<U>>>::value, int> = 0>
inline real dispatch_pow(T &&op1, U &&op2)
{
    return mpfr_nary_op_return(0, ::mpfr_pow, std::forward<T>(op1), std::forward<U>(op2));
}

template <typename T, typename U,
          enable_if_t<conjunction<std::is_same<real, uncvref_t<T>>, is_real_interoperable<U>>::value, int> = 0>
inline real dispatch_pow(T &&a, const U &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    return dispatch_pow(std::forward<T>(a), tmp);
}

template <typename T, typename U,
          enable_if_t<conjunction<is_real_interoperable<T>, std::is_same<real, uncvref_t<U>>>::value, int> = 0>
inline real dispatch_pow(const T &x, U &&a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    return dispatch_pow(tmp, std::forward<U>(a));
}
}

/// Binary \link mppp::real real\endlink exponentiation.
/**
 * \rststar
 * The precision of the result will be set to the largest precision among the operands.
 *
 * Non-:cpp:class:`~mppp::real` operands will be converted to :cpp:class:`~mppp::real`
 * before performing the operation. The conversion of non-:cpp:class:`~mppp::real` operands
 * to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment operator of
 * :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is either the default
 * precision, if set, or it is automatically deduced depending on the type and value of the
 * operand to be converted.
 * \endrststar
 *
 * @param op1 the base.
 * @param op2 the exponent.
 *
 * @return \p op1 raised to the power of \p op2.
 *
 * @throws unspecified any exception thrown by the generic assignment operator of \link mppp::real real\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline real pow(T &&op1, RealOpTypes<T> &&op2)
#else
template <typename T, typename U, real_op_types_enabler<T, U> = 0>
inline real pow(T &&op1, U &&op2)
#endif
{
    return dispatch_pow(std::forward<decltype(op1)>(op1), std::forward<decltype(op2)>(op2));
}

    /** @} */

    /** @defgroup real_trig real_trig
     *  @{
     */

    /// Binary \link mppp::real real\endlink sine.
    /**
     * @param rop the return value.
     * @param op the operand.
     *
     * @return a reference to \p rop.
     */
#if defined(MPPP_HAVE_CONCEPTS)
inline real &sin(real &rop, CvrReal &&op)
#else
template <typename T, cvr_real_enabler<T> = 0>
inline real &sin(real &rop, T &&op)
#endif
{
    return mpfr_nary_op(0, ::mpfr_sin, rop, std::forward<decltype(op)>(op));
}

/// Unary \link mppp::real real\endlink sine.
/**
 * @param r the operand.
 *
 * @return the sine of \p r.
 */
#if defined(MPPP_HAVE_CONCEPTS)
inline real sin(CvrReal &&r)
#else
template <typename T, cvr_real_enabler<T> = 0>
inline real sin(T &&r)
#endif
{
    return mpfr_nary_op_return(0, ::mpfr_sin, std::forward<decltype(r)>(r));
}

    /// Binary \link mppp::real real\endlink cosine.
    /**
     * @param rop the return value.
     * @param op the operand.
     *
     * @return a reference to \p rop.
     */
#if defined(MPPP_HAVE_CONCEPTS)
inline real &cos(real &rop, CvrReal &&op)
#else
template <typename T, cvr_real_enabler<T> = 0>
inline real &cos(real &rop, T &&op)
#endif
{
    return mpfr_nary_op(0, ::mpfr_cos, rop, std::forward<decltype(op)>(op));
}

/// Unary \link mppp::real real\endlink cosine.
/**
 * @param r the operand.
 *
 * @return the cosine of \p r.
 */
#if defined(MPPP_HAVE_CONCEPTS)
inline real cos(CvrReal &&r)
#else
template <typename T, cvr_real_enabler<T> = 0>
inline real cos(T &&r)
#endif
{
    return mpfr_nary_op_return(0, ::mpfr_cos, std::forward<decltype(r)>(r));
}

/** @} */

/** @defgroup real_logexp real_logexp
 *  @{
 */

/** @} */

/** @defgroup real_io real_io
 *  @{
 */

/// Output stream operator for \link mppp::real real\endlink objects.
/**
 * \rststar
 * This operator will insert into the stream ``os`` a string representation of ``r``
 * in base 10 (as returned by :cpp:func:`mppp::real::to_string()`).
 *
 * .. warning::
 *    In future versions of mp++, the behaviour of this operator will change to support the output stream's formatting
 *    flags. For the time being, users are encouraged to use the ``mpfr_get_str()`` function from the MPFR
 *    library if precise and forward-compatible control on the printing format is needed.
 *
 * \endrststar
 *
 * @param os the target stream.
 * @param r the \link mppp::real real\endlink that will be directed to \p os.
 *
 * @return a reference to \p os.
 *
 * @throws unspecified any exception thrown by mppp::real::to_string().
 */
inline std::ostream &operator<<(std::ostream &os, const real &r)
{
    mpfr_to_stream(r.get_mpfr_t(), os, 10);
    return os;
}

/// Input stream operator for \link mppp::real real\endlink objects.
/**
 * \rststar
 * This operator is equivalent to extracting a line from the stream and assigning it to ``x``.
 * \endrststar
 *
 * @param is the input stream.
 * @param x the \link mppp::real real\endlink to which the string extracted from the stream will be assigned.
 *
 * @return a reference to \p is.
 *
 * @throws unspecified any exception thrown by \link mppp::real real\endlink's assignment operator from string.
 */
inline std::istream &operator>>(std::istream &is, real &x)
{
    MPPP_MAYBE_TLS std::string tmp_str;
    std::getline(is, tmp_str);
    x = tmp_str;
    return is;
}

/** @} */

inline namespace detail
{

template <typename F>
inline real real_constant(const F &f, ::mpfr_prec_t p)
{
    ::mpfr_prec_t prec;
    if (p) {
        if (mppp_unlikely(!real_prec_check(p))) {
            throw std::invalid_argument("Cannot init a real constant with a precision of " + std::to_string(p)
                                        + ": the value must be either zero or between "
                                        + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max()));
        }
        prec = p;
    } else {
        const auto dp = real_get_default_prec();
        if (mppp_unlikely(!dp)) {
            throw std::invalid_argument("Cannot init a real constant with an automatically-deduced precision if "
                                        "the global default precision has not been set");
        }
        prec = dp;
    }
    real retval{real::ptag{}, prec, true};
    f(retval._get_mpfr_t(), MPFR_RNDN);
    return retval;
}
}

/** @defgroup real_constants real_constants
 *  @{
 */

/// \link mppp::real Real\endlink \f$\pi\f$ constant.
/**
 * \rststar
 * This function will return a :cpp:class:`~mppp::real` :math:`\pi`
 * with a precision of ``p``. If ``p`` is zero, the precision
 * will be set to the value returned by :cpp:func:`~mppp::real_get_default_prec()`.
 * If no default precision has been set, an error will be raised.
 * \endrststar
 *
 * @param p the desired precision.
 *
 * @return a \link mppp::real real\endlink \f$\pi\f$.
 *
 * @throws std::invalid_argument if \p p is not within the bounds established by
 * \link mppp::real_prec_min() real_prec_min()\endlink and \link mppp::real_prec_max() real_prec_max()\endlink,
 * or if \p p is zero but no default precision has been set.
 */
inline real real_pi(::mpfr_prec_t p = 0)
{
    return real_constant(::mpfr_const_pi, p);
}

/// Set \link mppp::real real\endlink to \f$\pi\f$.
/**
 * This function will set \p rop to \f$\pi\f$. The precision
 * of \p rop will not be altered.
 *
 * @param rop the \link mppp::real real\endlink that will be set to \f$\pi\f$.
 *
 * @return a reference to \p rop.
 */
inline real &real_pi(real &rop)
{
    ::mpfr_const_pi(rop._get_mpfr_t(), MPFR_RNDN);
    return rop;
}

/** @} */

/** @defgroup real_operators real_operators
 *  @{
 */

/// Identity operator for \link mppp::real real\endlink.
/**
 * \rststar
 * This operator will return a copy of the input :cpp:class:`~mppp::real` ``r``.
 * \endrststar
 *
 * @param r the \link mppp::real real\endlink that will be copied.
 *
 * @return a copy of \p r.
 */
#if defined(MPPP_HAVE_CONCEPTS)
inline real operator+(CvrReal &&r)
#else
template <typename T, cvr_real_enabler<T> = 0>
inline real operator+(T &&r)
#endif
{
    return std::forward<decltype(r)>(r);
}

inline namespace detail
{

template <typename T, typename U,
          enable_if_t<conjunction<std::is_same<real, uncvref_t<T>>, std::is_same<real, uncvref_t<U>>>::value, int> = 0>
inline real dispatch_binary_add(T &&a, U &&b)
{
    return mpfr_nary_op_return(0, ::mpfr_add, std::forward<T>(a), std::forward<U>(b));
}

template <typename T, typename U,
          enable_if_t<conjunction<std::is_same<real, uncvref_t<T>>, is_real_interoperable<U>>::value, int> = 0>
inline real dispatch_binary_add(T &&a, const U &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    return dispatch_binary_add(std::forward<T>(a), tmp);
}

template <typename T, typename U,
          enable_if_t<conjunction<is_real_interoperable<T>, std::is_same<real, uncvref_t<U>>>::value, int> = 0>
inline real dispatch_binary_add(const T &x, U &&a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    return dispatch_binary_add(tmp, std::forward<U>(a));
}
}

/// Binary addition involving \link mppp::real real\endlink.
/**
 * \rststar
 * The precision of the result will be set to the largest precision among the operands.
 *
 * Non-:cpp:class:`~mppp::real` operands will be converted to :cpp:class:`~mppp::real`
 * before performing the operation. The conversion of non-:cpp:class:`~mppp::real` operands
 * to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment operator of
 * :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is either the default
 * precision, if set, or it is automatically deduced depending on the type and value of the
 * operand to be converted.
 * \endrststar
 *
 * @param a the first operand.
 * @param b the second operand.
 *
 * @return \f$a+b\f$.
 *
 * @throws unspecified any exception thrown by the generic assignment operator of \link mppp::real real\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline real operator+(T &&a, RealOpTypes<T> &&b)
#else
template <typename T, typename U, real_op_types_enabler<T, U> = 0>
inline real operator+(T &&a, U &&b)
#endif
{
    // NOTE: we use forward + decltype here in order to accommodate the signature
    // when using concepts. It's equivalent to the usual perfect forwarding:
    // https://stackoverflow.com/questions/23321028/is-any-difference-between-stdforwardt-and-stdforwarddecltypet
    return dispatch_binary_add(std::forward<T>(a), std::forward<decltype(b)>(b));
}

inline namespace detail
{

template <typename T, typename U,
          enable_if_t<conjunction<std::is_same<real, unref_t<T>>, std::is_same<real, uncvref_t<U>>>::value, int> = 0>
inline void dispatch_in_place_add(T &a, U &&b)
{
    add(a, a, std::forward<U>(b));
}

template <typename T, enable_if_t<is_real_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_add(real &a, const T &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    dispatch_in_place_add(a, tmp);
}

// NOTE: split this in two parts: for C++ types and real128, we use directly the cast
// operator, for integer and rational we use the get() function.
template <typename T, typename U,
          enable_if_t<conjunction<disjunction<is_cpp_interoperable<T>
#if defined(MPPP_WITH_QUADMATH)
                                              ,
                                              std::is_same<T, real128>
#endif
                                              >,
                                  std::is_same<real, uncvref_t<U>>>::value,
                      int> = 0>
inline void dispatch_in_place_add(T &x, U &&a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    dispatch_in_place_add(tmp, std::forward<U>(a));
    x = static_cast<T>(tmp);
}

template <typename T>
inline void real_in_place_convert(T &x, const real &tmp, const real &a, const char *op)
{
    if (mppp_unlikely(!get(x, tmp))) {
        if (is_integer<T>::value) {
            // Conversion to integer can fail only if the source value is not finite.
            assert(!tmp.number_p());
            throw std::domain_error(std::string{"The result of the in-place "} + op + " of the real " + a.to_string()
                                    + " with the integer " + x.to_string() + " is the non-finite value "
                                    + tmp.to_string());
        }
        // Conversion to rational can fail if the source value is not finite, or if the conversion
        // results in overflow in the manipulation of the real exponent.
        if (!tmp.number_p()) {
            throw std::domain_error(std::string{"The result of the in-place "} + op + " of the real " + a.to_string()
                                    + " with the rational " + x.to_string() + " is the non-finite value "
                                    + tmp.to_string());
        }
        // LCOV_EXCL_START
        throw std::overflow_error("The conversion of the real " + tmp.to_string() + " to rational during the in-place "
                                  + op + " of the real " + a.to_string() + " with the rational " + x.to_string()
                                  + " triggers an internal overflow condition");
        // LCOV_EXCL_STOP
    }
}

template <typename T, typename U,
          enable_if_t<conjunction<disjunction<is_integer<T>, is_rational<T>>, std::is_same<real, uncvref_t<U>>>::value,
                      int> = 0>
inline void dispatch_in_place_add(T &x, U &&a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    dispatch_in_place_add(tmp, std::forward<U>(a));
    real_in_place_convert(x, tmp, a, "addition");
}
}

/// In-place addition involving \link mppp::real real\endlink.
/**
 * \rststar
 * If ``a`` is a :cpp:class:`~mppp::real`, then this operator is equivalent
 * to the expression:
 *
 * .. code-block:: c++
 *
 *    a = a + b;
 *
 * Otherwise, this operator is equivalent to the expression:
 *
 * .. code-block:: c++
 *
 *    a = static_cast<T>(a + b);
 *
 * That is, the operation is always performed via the corresponding binary operator
 * and the result is assigned back to ``a``, after a conversion if necessary.
 * \endrststar
 *
 * @param a the augend.
 * @param b the addend.
 *
 * @return a reference to \p a.
 *
 * @throws unspecified any exception thrown by the corresponding binary operator,
 * or by the generic conversion operator of \link mppp::real real\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline T &operator+=(T &a, RealCompoundOpTypes<T> &&b)
#else
template <typename T, typename U, real_compound_op_types_enabler<T, U> = 0>
inline T &operator+=(T &a, U &&b)
#endif
{
    dispatch_in_place_add(a, std::forward<decltype(b)>(b));
    return a;
}

    /// Negated copy for \link mppp::real real\endlink.
    /**
     * \rststar
     * This operator will return a negated copy of the input :cpp:class:`~mppp::real` ``r``.
     * \endrststar
     *
     * @param r the \link mppp::real real\endlink that will be negated.
     *
     * @return a negated copy of \p r.
     */
#if defined(MPPP_HAVE_CONCEPTS)
inline real operator-(CvrReal &&r)
#else
template <typename T, cvr_real_enabler<T> = 0>
inline real operator-(T &&r)
#endif
{
    real retval{std::forward<decltype(r)>(r)};
    retval.neg();
    return retval;
}

inline namespace detail
{

template <typename T, typename U,
          enable_if_t<conjunction<std::is_same<real, uncvref_t<T>>, std::is_same<real, uncvref_t<U>>>::value, int> = 0>
inline real dispatch_binary_sub(T &&a, U &&b)
{
    return mpfr_nary_op_return(0, ::mpfr_sub, std::forward<T>(a), std::forward<U>(b));
}

template <typename T, typename U,
          enable_if_t<conjunction<std::is_same<real, uncvref_t<T>>, is_real_interoperable<U>>::value, int> = 0>
inline real dispatch_binary_sub(T &&a, const U &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    return dispatch_binary_sub(std::forward<T>(a), tmp);
}

template <typename T, typename U,
          enable_if_t<conjunction<is_real_interoperable<T>, std::is_same<real, uncvref_t<U>>>::value, int> = 0>
inline real dispatch_binary_sub(const T &x, U &&a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    return dispatch_binary_sub(tmp, std::forward<U>(a));
}
}

/// Binary subtraction involving \link mppp::real real\endlink.
/**
 * \rststar
 * The precision of the result will be set to the largest precision among the operands.
 *
 * Non-:cpp:class:`~mppp::real` operands will be converted to :cpp:class:`~mppp::real`
 * before performing the operation. The conversion of non-:cpp:class:`~mppp::real` operands
 * to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment operator of
 * :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is either the default
 * precision, if set, or it is automatically deduced depending on the type and value of the
 * operand to be converted.
 * \endrststar
 *
 * @param a the first operand.
 * @param b the second operand.
 *
 * @return \f$a-b\f$.
 *
 * @throws unspecified any exception thrown by the generic assignment operator of \link mppp::real real\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline real operator-(T &&a, RealOpTypes<T> &&b)
#else
template <typename T, typename U, real_op_types_enabler<T, U> = 0>
inline real operator-(T &&a, U &&b)
#endif
{
    return dispatch_binary_sub(std::forward<T>(a), std::forward<decltype(b)>(b));
}

inline namespace detail
{

template <typename T, typename U,
          enable_if_t<conjunction<std::is_same<real, unref_t<T>>, std::is_same<real, uncvref_t<U>>>::value, int> = 0>
inline void dispatch_in_place_sub(T &a, U &&b)
{
    sub(a, a, std::forward<U>(b));
}

template <typename T, enable_if_t<is_real_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_sub(real &a, const T &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    dispatch_in_place_sub(a, tmp);
}

template <typename T, typename U,
          enable_if_t<conjunction<disjunction<is_cpp_interoperable<T>
#if defined(MPPP_WITH_QUADMATH)
                                              ,
                                              std::is_same<T, real128>
#endif
                                              >,
                                  std::is_same<real, uncvref_t<U>>>::value,
                      int> = 0>
inline void dispatch_in_place_sub(T &x, U &&a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    dispatch_in_place_sub(tmp, std::forward<U>(a));
    x = static_cast<T>(tmp);
}

template <typename T, typename U,
          enable_if_t<conjunction<disjunction<is_integer<T>, is_rational<T>>, std::is_same<real, uncvref_t<U>>>::value,
                      int> = 0>
inline void dispatch_in_place_sub(T &x, U &&a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    dispatch_in_place_sub(tmp, std::forward<U>(a));
    real_in_place_convert(x, tmp, a, "subtraction");
}
}

/// In-place subtraction involving \link mppp::real real\endlink.
/**
 * \rststar
 * If ``a`` is a :cpp:class:`~mppp::real`, then this operator is equivalent
 * to the expression:
 *
 * .. code-block:: c++
 *
 *    a = a - b;
 *
 * Otherwise, this operator is equivalent to the expression:
 *
 * .. code-block:: c++
 *
 *    a = static_cast<T>(a - b);
 *
 * That is, the operation is always performed via the corresponding binary operator
 * and the result is assigned back to ``a``, after a conversion if necessary.
 * \endrststar
 *
 * @param a the minuend.
 * @param b the subtrahend.
 *
 * @return a reference to \p a.
 *
 * @throws unspecified any exception thrown by the corresponding binary operator,
 * or by the generic conversion operator of \link mppp::real real\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline T &operator-=(T &a, RealCompoundOpTypes<T> &&b)
#else
template <typename T, typename U, real_compound_op_types_enabler<T, U> = 0>
inline T &operator-=(T &a, U &&b)
#endif
{
    dispatch_in_place_sub(a, std::forward<decltype(b)>(b));
    return a;
}

inline namespace detail
{

template <typename T, typename U,
          enable_if_t<conjunction<std::is_same<real, uncvref_t<T>>, std::is_same<real, uncvref_t<U>>>::value, int> = 0>
inline real dispatch_binary_mul(T &&a, U &&b)
{
    return mpfr_nary_op_return(0, ::mpfr_mul, std::forward<T>(a), std::forward<U>(b));
}

template <typename T, typename U,
          enable_if_t<conjunction<std::is_same<real, uncvref_t<T>>, is_real_interoperable<U>>::value, int> = 0>
inline real dispatch_binary_mul(T &&a, const U &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    return dispatch_binary_mul(std::forward<T>(a), tmp);
}

template <typename T, typename U,
          enable_if_t<conjunction<is_real_interoperable<T>, std::is_same<real, uncvref_t<U>>>::value, int> = 0>
inline real dispatch_binary_mul(const T &x, U &&a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    return dispatch_binary_mul(tmp, std::forward<U>(a));
}
}

/// Binary multiplication involving \link mppp::real real\endlink.
/**
 * \rststar
 * The precision of the result will be set to the largest precision among the operands.
 *
 * Non-:cpp:class:`~mppp::real` operands will be converted to :cpp:class:`~mppp::real`
 * before performing the operation. The conversion of non-:cpp:class:`~mppp::real` operands
 * to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment operator of
 * :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is either the default
 * precision, if set, or it is automatically deduced depending on the type and value of the
 * operand to be converted.
 * \endrststar
 *
 * @param a the first operand.
 * @param b the second operand.
 *
 * @return \f$a\times b\f$.
 *
 * @throws unspecified any exception thrown by the generic assignment operator of \link mppp::real real\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline real operator*(T &&a, RealOpTypes<T> &&b)
#else
template <typename T, typename U, real_op_types_enabler<T, U> = 0>
inline real operator*(T &&a, U &&b)
#endif
{
    return dispatch_binary_mul(std::forward<T>(a), std::forward<decltype(b)>(b));
}

inline namespace detail
{

template <typename T, typename U,
          enable_if_t<conjunction<std::is_same<real, unref_t<T>>, std::is_same<real, uncvref_t<U>>>::value, int> = 0>
inline void dispatch_in_place_mul(T &a, U &&b)
{
    mul(a, a, std::forward<U>(b));
}

template <typename T, enable_if_t<is_real_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_mul(real &a, const T &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    dispatch_in_place_mul(a, tmp);
}

template <typename T, typename U,
          enable_if_t<conjunction<disjunction<is_cpp_interoperable<T>
#if defined(MPPP_WITH_QUADMATH)
                                              ,
                                              std::is_same<T, real128>
#endif
                                              >,
                                  std::is_same<real, uncvref_t<U>>>::value,
                      int> = 0>
inline void dispatch_in_place_mul(T &x, U &&a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    dispatch_in_place_mul(tmp, std::forward<U>(a));
    x = static_cast<T>(tmp);
}

template <typename T, typename U,
          enable_if_t<conjunction<disjunction<is_integer<T>, is_rational<T>>, std::is_same<real, uncvref_t<U>>>::value,
                      int> = 0>
inline void dispatch_in_place_mul(T &x, U &&a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    dispatch_in_place_mul(tmp, std::forward<U>(a));
    real_in_place_convert(x, tmp, a, "multiplication");
}
}

/// In-place multiplication involving \link mppp::real real\endlink.
/**
 * \rststar
 * If ``a`` is a :cpp:class:`~mppp::real`, then this operator is equivalent
 * to the expression:
 *
 * .. code-block:: c++
 *
 *    a = a * b;
 *
 * Otherwise, this operator is equivalent to the expression:
 *
 * .. code-block:: c++
 *
 *    a = static_cast<T>(a * b);
 *
 * That is, the operation is always performed via the corresponding binary operator
 * and the result is assigned back to ``a``, after a conversion if necessary.
 * \endrststar
 *
 * @param a the multiplicand.
 * @param b the multiplicator.
 *
 * @return a reference to \p a.
 *
 * @throws unspecified any exception thrown by the corresponding binary operator,
 * or by the generic conversion operator of \link mppp::real real\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline T &operator*=(T &a, RealCompoundOpTypes<T> &&b)
#else
template <typename T, typename U, real_compound_op_types_enabler<T, U> = 0>
inline T &operator*=(T &a, U &&b)
#endif
{
    dispatch_in_place_mul(a, std::forward<decltype(b)>(b));
    return a;
}

inline namespace detail
{

template <typename T, typename U,
          enable_if_t<conjunction<std::is_same<real, uncvref_t<T>>, std::is_same<real, uncvref_t<U>>>::value, int> = 0>
inline real dispatch_binary_div(T &&a, U &&b)
{
    return mpfr_nary_op_return(0, ::mpfr_div, std::forward<T>(a), std::forward<U>(b));
}

template <typename T, typename U,
          enable_if_t<conjunction<std::is_same<real, uncvref_t<T>>, is_real_interoperable<U>>::value, int> = 0>
inline real dispatch_binary_div(T &&a, const U &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    return dispatch_binary_div(std::forward<T>(a), tmp);
}

template <typename T, typename U,
          enable_if_t<conjunction<is_real_interoperable<T>, std::is_same<real, uncvref_t<U>>>::value, int> = 0>
inline real dispatch_binary_div(const T &x, U &&a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    return dispatch_binary_div(tmp, std::forward<U>(a));
}
}

/// Binary division involving \link mppp::real real\endlink.
/**
 * \rststar
 * The precision of the result will be set to the largest precision among the operands.
 *
 * Non-:cpp:class:`~mppp::real` operands will be converted to :cpp:class:`~mppp::real`
 * before performing the operation. The conversion of non-:cpp:class:`~mppp::real` operands
 * to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment operator of
 * :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is either the default
 * precision, if set, or it is automatically deduced depending on the type and value of the
 * operand to be converted.
 * \endrststar
 *
 * @param a the first operand.
 * @param b the second operand.
 *
 * @return \f$a / b\f$.
 *
 * @throws unspecified any exception thrown by the generic assignment operator of \link mppp::real real\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline real operator/(T &&a, RealOpTypes<T> &&b)
#else
template <typename T, typename U, real_op_types_enabler<T, U> = 0>
inline real operator/(T &&a, U &&b)
#endif
{
    return dispatch_binary_div(std::forward<T>(a), std::forward<decltype(b)>(b));
}

inline namespace detail
{

template <typename T, typename U,
          enable_if_t<conjunction<std::is_same<real, unref_t<T>>, std::is_same<real, uncvref_t<U>>>::value, int> = 0>
inline void dispatch_in_place_div(T &a, U &&b)
{
    div(a, a, std::forward<U>(b));
}

template <typename T, enable_if_t<is_real_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_div(real &a, const T &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    dispatch_in_place_div(a, tmp);
}

template <typename T, typename U,
          enable_if_t<conjunction<disjunction<is_cpp_interoperable<T>
#if defined(MPPP_WITH_QUADMATH)
                                              ,
                                              std::is_same<T, real128>
#endif
                                              >,
                                  std::is_same<real, uncvref_t<U>>>::value,
                      int> = 0>
inline void dispatch_in_place_div(T &x, U &&a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    dispatch_in_place_div(tmp, std::forward<U>(a));
    x = static_cast<T>(tmp);
}

template <typename T, typename U,
          enable_if_t<conjunction<disjunction<is_integer<T>, is_rational<T>>, std::is_same<real, uncvref_t<U>>>::value,
                      int> = 0>
inline void dispatch_in_place_div(T &x, U &&a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    dispatch_in_place_div(tmp, std::forward<U>(a));
    real_in_place_convert(x, tmp, a, "division");
}
}

/// In-place division involving \link mppp::real real\endlink.
/**
 * \rststar
 * If ``a`` is a :cpp:class:`~mppp::real`, then this operator is equivalent
 * to the expression:
 *
 * .. code-block:: c++
 *
 *    a = a / b;
 *
 * Otherwise, this operator is equivalent to the expression:
 *
 * .. code-block:: c++
 *
 *    a = static_cast<T>(a / b);
 *
 * That is, the operation is always performed via the corresponding binary operator
 * and the result is assigned back to ``a``, after a conversion if necessary.
 * \endrststar
 *
 * @param a the dividend.
 * @param b the divisor.
 *
 * @return a reference to \p a.
 *
 * @throws unspecified any exception thrown by the corresponding binary operator,
 * or by the generic conversion operator of \link mppp::real real\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline T &operator/=(T &a, RealCompoundOpTypes<T> &&b)
#else
template <typename T, typename U, real_compound_op_types_enabler<T, U> = 0>
inline T &operator/=(T &a, U &&b)
#endif
{
    dispatch_in_place_div(a, std::forward<decltype(b)>(b));
    return a;
}

inline namespace detail
{

// Common dispatch functions for comparison operators involving real.
template <typename F, typename T, enable_if_t<is_real_interoperable<T>::value, int> = 0>
inline bool dispatch_real_comparison(const F &f, const real &a, const T &x)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    return f(a.get_mpfr_t(), tmp.get_mpfr_t());
}

template <typename F, typename T, enable_if_t<is_real_interoperable<T>::value, int> = 0>
inline bool dispatch_real_comparison(const F &f, const T &x, const real &a)
{
    MPPP_MAYBE_TLS real tmp;
    tmp = x;
    return f(tmp.get_mpfr_t(), a.get_mpfr_t());
}

template <typename F>
inline bool dispatch_real_comparison(const F &f, const real &a, const real &b)
{
    return f(a.get_mpfr_t(), b.get_mpfr_t()) != 0;
}
}

/// Equality operator involving \link mppp::real real\endlink.
/**
 * \rststar
 * This operator will compare ``a`` and ``b``, returning ``true`` if ``a`` is
 * equal to ``b``, ``false`` otherwise. The comparison is performed via the
 * ``mpfr_equal_p()`` function from the MPFR API. Non-:cpp:class:`~mppp::real`
 * operands will be converted to :cpp:class:`~mppp::real` before performing the operation.
 * The conversion of non-:cpp:class:`~mppp::real` operands
 * to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment operator of
 * :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is either the default
 * precision, if set, or it is automatically deduced depending on the type and value of the
 * operand to be converted.
 *
 * If at least one NaN value is involved in the comparison, ``false`` will be returned.
 * \endrststar
 *
 * @param a the first operand.
 * @param b the second operand.
 *
 * @return ``true`` if ``a`` is equal to ``b``, ``false`` otherwise.
 *
 * @throws unspecified any exception thrown by the generic assignment operator of \link mppp::real real\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline bool operator==(const T &a, const RealOpTypes<T> &b)
#else
template <typename T, typename U, real_op_types_enabler<T, U> = 0>
inline bool operator==(const T &a, const U &b)
#endif
{
    return dispatch_real_comparison(::mpfr_equal_p, a, b);
}

/// Inequality operator involving \link mppp::real real\endlink.
/**
 * \rststar
 * This operator will compare ``a`` and ``b``, returning ``true`` if ``a`` is
 * different from ``b``, ``false`` otherwise. The comparison is performed via the
 * ``mpfr_equal_p()`` function from the MPFR API. Non-:cpp:class:`~mppp::real`
 * operands will be converted to :cpp:class:`~mppp::real` before performing the operation.
 * The conversion of non-:cpp:class:`~mppp::real` operands
 * to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment operator of
 * :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is either the default
 * precision, if set, or it is automatically deduced depending on the type and value of the
 * operand to be converted.
 *
 * If at least one NaN value is involved in the comparison, ``true`` will be returned.
 * \endrststar
 *
 * @param a the first operand.
 * @param b the second operand.
 *
 * @return ``true`` if ``a`` is different from ``b``, ``false`` otherwise.
 *
 * @throws unspecified any exception thrown by the generic assignment operator of \link mppp::real real\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline bool operator!=(const T &a, const RealOpTypes<T> &b)
#else
template <typename T, typename U, real_op_types_enabler<T, U> = 0>
inline bool operator!=(const T &a, const U &b)
#endif
{
    return !(a == b);
}

/// Greater-than operator involving \link mppp::real real\endlink.
/**
 * \rststar
 * This operator will compare ``a`` and ``b``, returning ``true`` if ``a`` is
 * greater than ``b``, ``false`` otherwise. The comparison is performed via the
 * ``mpfr_greater_p()`` function from the MPFR API. Non-:cpp:class:`~mppp::real`
 * operands will be converted to :cpp:class:`~mppp::real` before performing the operation.
 * The conversion of non-:cpp:class:`~mppp::real` operands
 * to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment operator of
 * :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is either the default
 * precision, if set, or it is automatically deduced depending on the type and value of the
 * operand to be converted.
 *
 * If at least one NaN value is involved in the comparison, ``false`` will be returned.
 * \endrststar
 *
 * @param a the first operand.
 * @param b the second operand.
 *
 * @return ``true`` if ``a`` is greater than ``b``, ``false`` otherwise.
 *
 * @throws unspecified any exception thrown by the generic assignment operator of \link mppp::real real\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline bool operator>(const T &a, const RealOpTypes<T> &b)
#else
template <typename T, typename U, real_op_types_enabler<T, U> = 0>
inline bool operator>(const T &a, const U &b)
#endif
{
    return dispatch_real_comparison(::mpfr_greater_p, a, b);
}

/// Greater-than or equal operator involving \link mppp::real real\endlink.
/**
 * \rststar
 * This operator will compare ``a`` and ``b``, returning ``true`` if ``a`` is
 * greater than or equal to ``b``, ``false`` otherwise. The comparison is performed via the
 * ``mpfr_greaterequal_p()`` function from the MPFR API. Non-:cpp:class:`~mppp::real`
 * operands will be converted to :cpp:class:`~mppp::real` before performing the operation.
 * The conversion of non-:cpp:class:`~mppp::real` operands
 * to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment operator of
 * :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is either the default
 * precision, if set, or it is automatically deduced depending on the type and value of the
 * operand to be converted.
 *
 * If at least one NaN value is involved in the comparison, ``false`` will be returned.
 * \endrststar
 *
 * @param a the first operand.
 * @param b the second operand.
 *
 * @return ``true`` if ``a`` is greater than or equal to ``b``, ``false`` otherwise.
 *
 * @throws unspecified any exception thrown by the generic assignment operator of \link mppp::real real\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline bool operator>=(const T &a, const RealOpTypes<T> &b)
#else
template <typename T, typename U, real_op_types_enabler<T, U> = 0>
inline bool operator>=(const T &a, const U &b)
#endif
{
    return dispatch_real_comparison(::mpfr_greaterequal_p, a, b);
}

/// Less-than operator involving \link mppp::real real\endlink.
/**
 * \rststar
 * This operator will compare ``a`` and ``b``, returning ``true`` if ``a`` is
 * less than ``b``, ``false`` otherwise. The comparison is performed via the
 * ``mpfr_less_p()`` function from the MPFR API. Non-:cpp:class:`~mppp::real`
 * operands will be converted to :cpp:class:`~mppp::real` before performing the operation.
 * The conversion of non-:cpp:class:`~mppp::real` operands
 * to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment operator of
 * :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is either the default
 * precision, if set, or it is automatically deduced depending on the type and value of the
 * operand to be converted.
 *
 * If at least one NaN value is involved in the comparison, ``false`` will be returned.
 * \endrststar
 *
 * @param a the first operand.
 * @param b the second operand.
 *
 * @return ``true`` if ``a`` is less than ``b``, ``false`` otherwise.
 *
 * @throws unspecified any exception thrown by the generic assignment operator of \link mppp::real real\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline bool operator<(const T &a, const RealOpTypes<T> &b)
#else
template <typename T, typename U, real_op_types_enabler<T, U> = 0>
inline bool operator<(const T &a, const U &b)
#endif
{
    return dispatch_real_comparison(::mpfr_less_p, a, b);
}

/// Less-than or equal operator involving \link mppp::real real\endlink.
/**
 * \rststar
 * This operator will compare ``a`` and ``b``, returning ``true`` if ``a`` is
 * less than or equal to ``b``, ``false`` otherwise. The comparison is performed via the
 * ``mpfr_lessequal_p()`` function from the MPFR API. Non-:cpp:class:`~mppp::real`
 * operands will be converted to :cpp:class:`~mppp::real` before performing the operation.
 * The conversion of non-:cpp:class:`~mppp::real` operands
 * to :cpp:class:`~mppp::real` follows the same heuristics described in the generic assignment operator of
 * :cpp:class:`~mppp::real`. Specifically, the precision of the conversion is either the default
 * precision, if set, or it is automatically deduced depending on the type and value of the
 * operand to be converted.
 *
 * If at least one NaN value is involved in the comparison, ``false`` will be returned.
 * \endrststar
 *
 * @param a the first operand.
 * @param b the second operand.
 *
 * @return ``true`` if ``a`` is less than or equal to ``b``, ``false`` otherwise.
 *
 * @throws unspecified any exception thrown by the generic assignment operator of \link mppp::real real\endlink.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline bool operator<=(const T &a, const RealOpTypes<T> &b)
#else
template <typename T, typename U, real_op_types_enabler<T, U> = 0>
inline bool operator<=(const T &a, const U &b)
#endif
{
    return dispatch_real_comparison(::mpfr_lessequal_p, a, b);
}

/** @} */
}

#else

#error The real.hpp header was included but mp++ was not configured with the MPPP_WITH_MPFR option.

#endif

#endif
