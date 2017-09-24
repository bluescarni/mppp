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

// Clamp the precision between the min and max allowed values. This is used in the generic constructor.
constexpr ::mpfr_prec_t clamp_mpfr_prec(::mpfr_prec_t p)
{
    return real_prec_check(p) ? p : (p < real_prec_min() ? real_prec_min() : real_prec_max());
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
template <typename F, typename... Args>
void mpfr_nary_op(const F &, real &, const real &, const Args &...);

template <typename F, typename Arg0, typename... Args>
real mpfr_nary_op_return(const F &, Arg0 &&, Args &&...);

template <typename F>
real real_constant(const F &, ::mpfr_prec_t);
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

#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
concept bool CvrReal = std::is_same<uncvref_t<T>, real>::value;
#else
template <typename... Args>
using cvr_real_enabler = enable_if_t<conjunction<std::is_same<uncvref_t<Args>, real>...>::value, int>;
#endif

/// Get the default precision for \link mppp::real real \endlink objects.
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
 * @return the value of the default precision for \link mppp::real real \endlink objects.
 */
inline mpfr_prec_t real_get_default_prec()
{
    return real_constants<>::default_prec.load();
}

/// Set the default precision for \link mppp::real real \endlink objects.
/**
 * \ingroup real_prec
 * \rststar
 * See :cpp:func:`~mppp::real_get_default_prec()` for an explanation of how the default precision value
 * is used.
 * \endrststar
 *
 * @param p the desired value for the default precision for \link mppp::real real \endlink objects.
 *
 * @throws std::invalid_argument if \p p is nonzero and not in the range established by
 * \link mppp::real_prec_min() real_prec_min() \endlink and \link mppp::real_prec_max() real_prec_max() \endlink.
 */
inline void real_set_default_prec(::mpfr_prec_t p)
{
    if (mppp_unlikely(p && !real_prec_check(p))) {
        throw std::invalid_argument("Cannot set the default precision to " + std::to_string(p)
                                    + ": the value must be either zero or between " + std::to_string(real_prec_min())
                                    + " and " + std::to_string(real_prec_max()));
    }
    real_constants<>::default_prec.store(p);
}

/// Reset the default precision for \link mppp::real real \endlink objects.
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

/// Auxiliary class for initialising a \link mppp::real real \endlink from a precision value.
/**
 * \rststar
 * This class is used as the only construction argument for a specific :cpp:class:`~mppp::real`
 * constructor. It will initialise the calling :cpp:class:`~mppp::real` with a precision
 * equal to the value stored in this object, and a value of NaN.
 * \endrststar
 */
struct real_prec {
    /// Constructor.
    /**
     * @param p the desired precision value.
     */
    explicit real_prec(::mpfr_prec_t p) : value{p} {}
    /// The precision value specified on construction.
    const ::mpfr_prec_t value;
};

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
 * \endrststar
 */
class real
{
#if !defined(MPPP_DOXYGEN_INVOKED)
    // Make friends, for accessing the non-checking prec setting funcs.
    template <typename F, typename... Args>
    friend void detail::mpfr_nary_op(const F &, real &, const real &, const Args &...);
    template <typename F, typename Arg0, typename... Args>
    friend real detail::mpfr_nary_op_return(const F &, Arg0 &&, Args &&...);
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
    /// Constructor from precision.
    /**
     * \rststar
     * This constructor will initialise a :cpp:class:`~mppp::real` with a precision equal to
     * the :cpp:member:`mppp::real_prec::value` member of the input argument ``p``, and a value
     * of NaN.
     *
     * E.g., the following code,
     *
     * .. code-block:: c++
     *
     *    real x{real_prec{42}};
     *
     * will initialise ``x`` with 42 bits of precision and a value of NaN.
     * \endrststar
     *
     * @param p the \link mppp::real_prec real_prec \endlink that wraps the desired precision value.
     *
     * @throws std::invalid_argument if the precision value is outside the range established by
     * \link mppp::real_prec_min() real_prec_min() \endlink and \link mppp::real_prec_max() real_prec_max() \endlink.
     */
    explicit real(real_prec p)
    {
        ::mpfr_init2(&m_mpfr, check_init_prec(p.value));
    }

private:
    // A private version of the above without prec checking.
    explicit real(real_prec p, bool ignore_prec)
    {
        assert(ignore_prec);
        (void)ignore_prec;
        ::mpfr_init2(&m_mpfr, p.value);
    }

public:
    /// Copy constructor.
    /**
     * The copy constructor performs an exact deep copy of the input object.
     *
     * @param other the \link mppp::real real \endlink that will be copied.
     */
    real(const real &other)
    {
        // Init with the same precision as other, and then set.
        ::mpfr_init2(&m_mpfr, other.get_prec());
        ::mpfr_set(&m_mpfr, &other.m_mpfr, MPFR_RNDN);
    }
    /// Copy constructor with custom precision.
    /**
     * This constructor will set \p this to a copy of \p other with precision \p p. If \p p
     * is smaller than the precision of \p other, a rounding operation will be performed,
     * otherwise the value will be copied exactly.
     *
     * @param other the \link mppp::real real \endlink that will be copied.
     * @param p the desired precision.
     *
     * @throws std::invalid_argument if \p p is outside the range established by
     * \link mppp::real_prec_min() real_prec_min() \endlink and \link mppp::real_prec_max() real_prec_max() \endlink.
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
     * @param other the \link mppp::real real \endlink that will be moved.
     */
    real(real &&other) noexcept
    {
        // Shallow copy other.
        m_mpfr = other.m_mpfr;
        // Mark the other as moved-from.
        other.m_mpfr._mpfr_d = nullptr;
    }

private:
    // A helper to determine the precision to use in the generic constructors. The precision
    // can be manually provided, taken from the default global value, or deduced according
    // to the properties of the type. A deducer will implement the logic to determine the deduced
    // precision for the type.
    template <typename Deducer>
    static ::mpfr_prec_t compute_init_precision(::mpfr_prec_t provided, const Deducer &d)
    {
        if (provided) {
            // Provided precision trumps everything. Check it and return it.
            return check_init_prec(provided);
        }
        // Check if we have a default precision.
        // NOTE: this is guaranteed to be a valid precision value.
        const auto dp = real_get_default_prec();
        // Return default precision if nonzero, otherwise return the clamped deduced precision.
        return dp ? dp : clamp_mpfr_prec(d());
    }
    // Construction from FPs.
    // Alias for the MPFR assignment functions from FP types.
    template <typename T>
    using fp_a_ptr = int (*)(::mpfr_t, T, ::mpfr_rnd_t);
    template <typename T>
    void dispatch_fp_construction(fp_a_ptr<T> ptr, const T &x, ::mpfr_prec_t p)
    {
        static_assert(std::numeric_limits<T>::digits <= std::numeric_limits<::mpfr_prec_t>::max(), "Overflow error.");
        ::mpfr_init2(&m_mpfr, compute_init_precision(p, []() {
            return std::numeric_limits<T>::radix == 2 ? static_cast<::mpfr_prec_t>(std::numeric_limits<T>::digits)
                                                      : dig2mpfr_prec<T>();
        }));
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
        ::mpfr_init2(&m_mpfr, compute_init_precision(
                                  p, []() { return static_cast<::mpfr_prec_t>(std::numeric_limits<T>::digits); }));
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
            // NOTE: here and elsewhere let's use a 2-limb integer, in the hope
            // of avoiding dynamic memory allocation.
            ::mpfr_set_z(&m_mpfr, integer<2>(n).get_mpz_view(), MPFR_RNDN);
        }
    }
    template <typename T, enable_if_t<conjunction<std::is_integral<T>, std::is_signed<T>>::value, int> = 0>
    void dispatch_construction(const T &n, ::mpfr_prec_t p)
    {
        dispatch_integral_init<T>(p);
        if (n <= std::numeric_limits<long>::max() && n >= std::numeric_limits<long>::min()) {
            ::mpfr_set_si(&m_mpfr, static_cast<long>(n), MPFR_RNDN);
        } else {
            ::mpfr_set_z(&m_mpfr, integer<2>(n).get_mpz_view(), MPFR_RNDN);
        }
    }
    template <std::size_t SSize>
    void dispatch_construction(const integer<SSize> &n, ::mpfr_prec_t p)
    {
        ::mpfr_init2(&m_mpfr, compute_init_precision(p, [&n]() -> ::mpfr_prec_t {
            // Infer the precision from the bit size of n.
            const auto ls = n.size();
            // Check that ls * GMP_NUMB_BITS is representable by mpfr_prec_t.
            if (mppp_unlikely(ls
                              > static_cast<make_unsigned_t<::mpfr_prec_t>>(std::numeric_limits<::mpfr_prec_t>::max())
                                    / unsigned(GMP_NUMB_BITS))) {
                throw std::overflow_error("The deduced precision for a real constructed from an integer is too large");
            }
            return static_cast<::mpfr_prec_t>(static_cast<::mpfr_prec_t>(ls) * GMP_NUMB_BITS);
        }));
        ::mpfr_set_z(&m_mpfr, n.get_mpz_view(), MPFR_RNDN);
    }
    template <std::size_t SSize>
    void dispatch_construction(const rational<SSize> &q, ::mpfr_prec_t p)
    {
        ::mpfr_init2(&m_mpfr, compute_init_precision(p, [&q]() -> ::mpfr_prec_t {
            // Infer the precision from the bit size of num/den.
            const auto n_size = q.get_num().size();
            const auto d_size = q.get_den().size();
            // Overflow checks.
            if (mppp_unlikely(
                    // Overflow in total size.
                    (n_size > std::numeric_limits<decltype(q.get_num().size())>::max() - d_size)
                    // Check that tot_size * GMP_NUMB_BITS is representable by mpfr_prec_t.
                    || ((n_size + d_size)
                        > static_cast<make_unsigned_t<::mpfr_prec_t>>(std::numeric_limits<::mpfr_prec_t>::max())
                              / unsigned(GMP_NUMB_BITS)))) {
                throw std::overflow_error("The deduced precision for a real constructed from a rational is too large");
            }
            return static_cast<::mpfr_prec_t>(static_cast<::mpfr_prec_t>(n_size + d_size) * GMP_NUMB_BITS);
        }));
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
        ::mpfr_init2(&m_mpfr, compute_init_precision(p, []() { return ::mpfr_prec_t(113); }));
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
     * \link mppp::real_prec_min() real_prec_min() \endlink and \link mppp::real_prec_max() real_prec_max() \endlink.
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
    /// Destructor.
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
     * @param other the \link mppp::real real \endlink that will be copied into \p this.
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
     * @param other the \link mppp::real real \endlink that will be moved into \p this.
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
    /// Swap \link mppp::real real \endlink objects.
    /**
     * This function will efficiently swap the content of \p a and \p b.
     *
     * @param a the first operand.
     * @param b the second operand.
     */
    friend void swap(real &a, real &b) noexcept
    {
        ::mpfr_swap(&a.m_mpfr, &b.m_mpfr);
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
     *    of the :cpp:class:`~mppp::real` class.
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
     * \link mppp::real_prec_min() real_prec_min() \endlink and \link mppp::real_prec_max() real_prec_max() \endlink.
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
     * \link mppp::real_prec_min() real_prec_min() \endlink and \link mppp::real_prec_max() real_prec_max() \endlink.
     */
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
        // Clear the range error flag before attempting the conversion.
        ::mpfr_clear_erangeflag();
        // NOTE: this call can fail if the exponent of this is very close to the upper/lower limits of the exponent
        // type. If the call fails (signalled by a range flag being set), we will raise an error.
        MPPP_MAYBE_TLS mpz_raii mpz;
        const ::mpfr_exp_t exp2 = ::mpfr_get_z_2exp(&mpz.m_mpz, &m_mpfr);
        if (mppp_unlikely(::mpfr_erangeflag_p())) {
            // Let's first reset the error flag.
            ::mpfr_clear_erangeflag();
            throw std::overflow_error("The exponent of a real is too large for conversion to rational");
        }
        // The conversion to n * 2**exp succeeded. We will build a rational
        // from n and exp.
        using int_t = typename T::int_t;
        int_t num{&mpz.m_mpz};
        if (exp2 >= ::mpfr_exp_t(0)) {
            // The output value will be an integer.
            num <<= exp2;
            return T{std::move(num)};
        }
        // The output value will be a rational. Canonicalisation will be needed.
        int_t den{1};
        den <<= nint_abs(exp2);
        return T{std::move(num), std::move(den)};
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
                    return static_cast<T>(static_cast<integer<2>>(*this));
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
        if (mppp_likely(candidate <= std::numeric_limits<T>::max())) {
            return static_cast<T>(candidate);
        }
        raise_overflow_error<T>();
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
                    return static_cast<T>(static_cast<integer<2>>(*this));
                } catch (const std::overflow_error &) {
                }
            }
            // We end up here because either:
            // - the range of the target type is not greater than long's,
            // - the range of the target type is greater than long's but the conversion
            //   via integer failed anyway.
            raise_overflow_error<T>();
        }
        if (mppp_likely(candidate >= std::numeric_limits<T>::min() && candidate <= std::numeric_limits<T>::max())) {
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
        //
        // NOTE: the reason why we do these checks with large positive and negative exponents
        // is that they ensure we can safely convert _mpfr_exp to long later.
        if (inf_p() || m_mpfr._mpfr_exp > (1l << 18)) {
            return sgn() > 0 ? real128_inf() : -real128_inf();
        }
        if (zero_p() || m_mpfr._mpfr_exp < -(1l << 18)) {
            // Preserve the signedness of zero.
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
        // of GMP integers (which have padding on the left, i.e., in the top limb).
        //
        // NOTE: contrary to real128, the MPFR format does not have a hidden bit on top.
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
     *   (e.g., when trying to convert non-finite values);
     * - if ``T`` if a C++ floating-point type, the conversion calls directly the low-level MPFR functions (e.g.,
     *   ``mpfr_get_d()``), and might yield infinities for finite input values;
     * - if ``T`` is :cpp:class:`~mppp::integer`, the conversion rounds to zero and might fail due to domain errors,
     *   but it will never overflow;
     * - if ``T`` is :cpp:class:`~mppp::rational`, the conversion, if successful, is exact;
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
    /// Convert to string.
    /**
     * \rststar
     * This method will convert ``this`` to a string representation in base 10. The returned string is guaranteed to
     * produce exactly the original value when used as a construction argument for :cpp:class:`~mppp::real`.
     * \endrststar
     *
     * @return \p this converted to a string.
     */
    std::string to_string() const
    {
        std::ostringstream oss;
        mpfr_to_stream(&m_mpfr, oss);
        return oss.str();
    }

private:
    mpfr_struct_t m_mpfr;
};

/// Destructively set the precision of a \link mppp::real real \endlink.
/**
 * \ingroup real_prec
 * \rststar
 * This function will set the precision of ``r`` to exactly ``p`` bits. The value
 * of ``r`` will be set to NaN.
 * \endrststar
 *
 * @param r the \link mppp::real real \endlink whose precision will be set.
 * @param p the desired precision.
 *
 * @throws unspecified any exception thrown by mppp::real::set_prec().
 */
inline void set_prec(real &r, ::mpfr_prec_t p)
{
    r.set_prec(p);
}

/// Set the precision of a \link mppp::real real \endlink maintaining the current value.
/**
 * \ingroup real_prec
 * \rststar
 * This function will set the precision of ``r`` to exactly ``p`` bits. If ``p``
 * is smaller than the current precision of ``r``, a rounding operation will be performed,
 * otherwise the value will be preserved exactly.
 * \endrststar
 *
 * @param r the \link mppp::real real \endlink whose precision will be set.
 * @param p the desired precision.
 *
 * @throws unspecified any exception thrown by mppp::real::prec_round().
 */
inline void prec_round(real &r, ::mpfr_prec_t p)
{
    r.prec_round(p);
}

inline namespace detail
{

// A recursive function to examine, in an MPFR n-ary function call, the precisions
// of rop and of the arguments. It will write into a pair in which the first element is a boolean
// flag signalling if rop and args all have the same precision, and the second element
// contains the maximum precision among the args.
inline void mpfr_examine_precs(std::pair<bool, ::mpfr_prec_t> &, const real &) {}

template <typename... Args>
inline void mpfr_examine_precs(std::pair<bool, ::mpfr_prec_t> &p, const real &rop, const real &arg0,
                               const Args &... args)
{
    const auto prec0 = arg0.get_prec();
    p.first = p.first && (rop.get_prec() == prec0);
    p.second = (prec0 > p.second) ? prec0 : p.second;
    mpfr_examine_precs(p, rop, args...);
}

// Apply the MPFR n-ary function F with return value rop and real arguments (arg0, args...).
// The precision of rop will be set to the maximum of the precision among the arguments.
template <typename F, typename... Args>
inline void mpfr_nary_op(const F &f, real &rop, const real &arg0, const Args &... args)
{
    auto p = std::make_pair(rop.get_prec() == arg0.get_prec(), arg0.get_prec());
    mpfr_examine_precs(p, rop, args...);
    if (mppp_unlikely(!p.first)) {
        // NOTE: no need for prec checking, we assume all precisions in the operands are valid.
        rop.set_prec_impl<false>(p.second);
    }
    // Invoke the MPFR function.
    f(rop._get_mpfr_t(), arg0.get_mpfr_t(), args.get_mpfr_t()..., MPFR_RNDN);
}

// A recursive function to determine from which arguments, in an MPFR function call,
// we can steal resources, and the max precision among the arguments.
inline void mpfr_nary_op_check_steal(std::pair<real *, ::mpfr_prec_t> &) {}

template <typename Arg0, typename... Args>
inline void mpfr_nary_op_check_steal(std::pair<real *, ::mpfr_prec_t> &p, Arg0 &&arg0, Args &&... args)
{
    const auto prec0 = arg0.get_prec();
    if (is_ncvrvr<Arg0 &&>::value && (!p.first || prec0 > p.first->get_prec())) {
        // The current argument arg0 is a non-const rvalue reference, and either it's
        // the first argument we encounter we can steal from, or it has a precision
        // larger than the current candidate for stealing resources from. This means that
        // arg0 is the new candidate.
        p.first = &arg0;
    }
    // Update the max precision among the arguments, if necessary.
    p.second = (prec0 > p.second) ? prec0 : p.second;
    mpfr_nary_op_check_steal(p, std::forward<Args>(args)...);
}

// Invoke an MPFR function with arguments (arg0, args...), and store the result
// in a value to be created by this function. If possible, this function will try
// to re-use the storage provided by the input arguments, if one or more of these
// arguments are rvalue references and their precision is large enough. As usual,
// the precision of the return value will be the max precision among the operands.
template <typename F, typename Arg0, typename... Args>
inline real mpfr_nary_op_return(const F &f, Arg0 &&arg0, Args &&... args)
{
    // This pair contains:
    // - a pointer to the largest-precision real from which we can steal resources (may be nullptr),
    // - the largest precision among all arguments.
    // It's inited with arg0's precision, and a pointer to arg0, if arg0 is a nonconst rvalue ref.
    auto p = std::make_pair(is_ncvrvr<Arg0 &&>::value ? &arg0 : static_cast<real *>(nullptr), arg0.get_prec());
    // Examine the remaining arguments.
    mpfr_nary_op_check_steal(p, std::forward<Args>(args)...);
    if (p.first && p.first->get_prec() == p.second) {
        // There's at least one arg we can steal from, and its precision is large enough
        // to contain the result. Use it.
        f(p.first->_get_mpfr_t(), arg0.get_mpfr_t(), args.get_mpfr_t()..., MPFR_RNDN);
        return std::move(*p.first);
    }
    // Either we cannot steal from any arg, or the candidate(s) have not enough precision.
    // Init a new value and use it instead.
    real retval{real_prec{p.second}, true};
    f(retval._get_mpfr_t(), arg0.get_mpfr_t(), args.get_mpfr_t()..., MPFR_RNDN);
    return retval;
}
}

/** @defgroup real_arithmetic real_arithmetic
 *  @{
 */

inline void add(real &rop, const real &op1, const real &op2)
{
    mpfr_nary_op(::mpfr_add, rop, op1, op2);
}

inline void sub(real &rop, const real &op1, const real &op2)
{
    mpfr_nary_op(::mpfr_sub, rop, op1, op2);
}

inline void fma(real &rop, const real &op1, const real &op2, const real &op3)
{
    mpfr_nary_op(::mpfr_fma, rop, op1, op2, op3);
}

#if defined(MPPP_HAVE_CONCEPTS)
template <CvrReal T, CvrReal U, CvrReal V>
#else
template <typename T, typename U, typename V, cvr_real_enabler<T, U, V> = 0>
#endif
inline real fma(T &&a, U &&b, V &&c)
{
    return mpfr_nary_op_return(::mpfr_fma, std::forward<T>(a), std::forward<U>(b), std::forward<V>(c));
}

inline void fms(real &rop, const real &op1, const real &op2, const real &op3)
{
    mpfr_nary_op(::mpfr_fms, rop, op1, op2, op3);
}

#if defined(MPPP_HAVE_CONCEPTS)
template <CvrReal T, CvrReal U, CvrReal V>
#else
template <typename T, typename U, typename V, cvr_real_enabler<T, U, V> = 0>
#endif
inline real fms(T &&a, U &&b, V &&c)
{
    return mpfr_nary_op_return(::mpfr_fms, std::forward<T>(a), std::forward<U>(b), std::forward<V>(c));
}

/** @} */

/** @defgroup real_comparison real_comparison
 *  @{
 */

/// Detect if a \link mppp::real real \endlink is NaN.
/**
 * @param r the \link mppp::real real \endlink that will be examined.
 *
 * @return \p true if \p r is NaN, \p false otherwise.
 */
inline bool nan_p(const real &r)
{
    return r.nan_p();
}

/// Detect if a \link mppp::real real \endlink is infinity.
/**
 * @param r the \link mppp::real real \endlink that will be examined.
 *
 * @return \p true if \p r is an infinity, \p false otherwise.
 */
inline bool inf_p(const real &r)
{
    return r.inf_p();
}

/// Detect if \link mppp::real real \endlink is a finite number.
/**
 * @param r the \link mppp::real real \endlink that will be examined.
 *
 * @return \p true if \p r is a finite number (i.e., not NaN or infinity), \p false otherwise.
 */
inline bool number_p(const real &r)
{
    return r.number_p();
}

/// Detect if a \link mppp::real real \endlink is zero.
/**
 * @param r the \link mppp::real real \endlink that will be examined.
 *
 * @return \p true if \p r is zero, \p false otherwise.
 */
inline bool zero_p(const real &r)
{
    return r.zero_p();
}

/// Detect if a \link mppp::real real \endlink is a regular number.
/**
 * @param r the \link mppp::real real \endlink that will be examined.
 *
 * @return \p true if \p r is a regular number (i.e., not NaN, infinity or zero), \p false otherwise.
 */
inline bool regular_p(const real &r)
{
    return r.regular_p();
}

/// Detect the sign of a \link mppp::real real \endlink.
/**
 * @param r the \link mppp::real real \endlink that will be examined.
 *
 * @return a positive value if \p r is positive, zero if \p r is zero, a negative value if \p thris
 * is negative.
 *
 * @throws std::domain_error if \p r is NaN.
 */
inline int sgn(const real &r)
{
    return r.sgn();
}

/// Get the sign bit of a \link mppp::real real \endlink.
/**
 * @param r the \link mppp::real real \endlink that will be examined.
 *
 * @return the sign bit of \p r.
 */
inline bool signbit(const real &r)
{
    return r.signbit();
}

/** @} */

/** @defgroup real_roots real_roots
 *  @{
 */

inline void sqrt(real &rop, const real &op)
{
    mpfr_nary_op(::mpfr_sqrt, rop, op);
}

#if defined(MPPP_HAVE_CONCEPTS)
template <CvrReal T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real sqrt(T &&r)
{
    return mpfr_nary_op_return(::mpfr_sqrt, std::forward<T>(r));
}

/** @} */

/** @defgroup real_exponentiation real_exponentiation
 *  @{
 */

/** @} */

/** @defgroup real_trig real_trig
 *  @{
 */

inline void sin(real &rop, const real &op)
{
    mpfr_nary_op(::mpfr_sin, rop, op);
}

#if defined(MPPP_HAVE_CONCEPTS)
template <CvrReal T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real sin(T &&r)
{
    return mpfr_nary_op_return(::mpfr_sin, std::forward<T>(r));
}

inline void cos(real &rop, const real &op)
{
    mpfr_nary_op(::mpfr_cos, rop, op);
}

#if defined(MPPP_HAVE_CONCEPTS)
template <CvrReal T>
#else
template <typename T, cvr_real_enabler<T> = 0>
#endif
inline real cos(T &&r)
{
    return mpfr_nary_op_return(::mpfr_cos, std::forward<T>(r));
}

/** @} */

/** @defgroup real_logexp real_logexp
 *  @{
 */

/** @} */

/** @defgroup real_io real_io
 *  @{
 */

inline std::ostream &operator<<(std::ostream &os, const real &r)
{
    mpfr_to_stream(r.get_mpfr_t(), os);
    return os;
}

/** @} */

inline namespace detail
{

template <typename F>
inline real real_constant(const F &f, ::mpfr_prec_t p)
{
    ::mpfr_prec_t prec;
    if (p) {
        // TODO need prec checking here.
        prec = p;
    } else {
        const auto dp = real_get_default_prec();
        if (!dp) {
            // TODO.
            throw;
        }
        prec = dp;
    }
    real retval{real_prec{prec}, true};
    f(retval);
    return retval;
}
}

/** @defgroup real_constants real_constants
 *  @{
 */

inline real real_nan(::mpfr_prec_t p = 0)
{
    return real_constant([](real &) {}, p);
}

/** @} */

/** @defgroup real_operators real_operators
 *  @{
 */

/** @} */
}

#else

#error The real.hpp header was included but mp++ was not configured with the MPPP_WITH_MPFR option.

#endif

#endif
