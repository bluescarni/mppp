// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_RATIONAL_HPP
#define MPPP_RATIONAL_HPP

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <mp++/concepts.hpp>
#include <mp++/config.hpp>
#include <mp++/detail/fwd_decl.hpp>
#include <mp++/detail/gmp.hpp>
#if defined(MPPP_WITH_MPFR)
#include <mp++/detail/mpfr.hpp>
#endif
#include <mp++/detail/type_traits.hpp>
#include <mp++/exceptions.hpp>
#include <mp++/integer.hpp>

namespace mppp
{

template <typename T, std::size_t SSize>
using is_rational_interoperable = disjunction<is_cpp_interoperable<T>, std::is_same<T, integer<SSize>>>;

template <typename T, std::size_t SSize>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool RationalInteroperable = is_rational_interoperable<T, SSize>::value;
#else
using rational_interoperable_enabler = enable_if_t<is_rational_interoperable<T, SSize>::value, int>;
#endif

template <typename T, std::size_t SSize>
using is_rational_integral_interoperable
    = conjunction<is_rational_interoperable<T, SSize>, negation<std::is_floating_point<T>>>;

#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, std::size_t SSize>
concept bool RationalIntegralInteroperable = is_rational_integral_interoperable<T, SSize>::value;
#endif

// Fwd declare this as we need friendship.
template <std::size_t SSize>
int cmp(const rational<SSize> &, const rational<SSize> &);

inline namespace detail
{

// This is useful in various bits below.
template <std::size_t SSize>
inline void fix_den_sign(rational<SSize> &q)
{
    if (q.get_den().sgn() == -1) {
        q._get_num().neg();
        q._get_den().neg();
    }
}
}

/// Multiprecision rational class.
/**
 * \rststar
 * This class represents arbitrary-precision rationals. Internally, the class stores a pair of
 * :cpp:class:`integers <mppp::integer>` with static size ``SSize`` as the numerator and denonimator.
 * Rational numbers are represented in the usual canonical form:
 *
 * * numerator and denominator are coprime,
 * * the denominator is always strictly positive.
 *
 * This class has the look and feel of a C++ builtin type: it can interact with most of C++'s integral and
 * floating-point primitive types (see the :cpp:concept:`~mppp::CppInteroperable` concept for the full list)
 * and with :cpp:class:`integers <mppp::integer>` with static size ``SSize``,
 * and it provides overloaded :ref:`operators <rational_operators>`. Differently from the builtin types,
 * however, this class does not allow any implicit conversion to/from other types (apart from ``bool``): construction
 * from and  conversion to primitive types must always be requested explicitly. As a side effect, syntax such as
 *
 * .. code-block:: c++
 *
 *    rational<1> q = 5;
 *    int m = q;
 *
 * will not work, and direct initialization and explicit casting should be used instead:
 *
 * .. code-block:: c++
 *
 *    rational<1> q{5};
 *    int m = static_cast<int>(q);
 *
 * Most of the functionality is exposed via plain :ref:`functions <rational_functions>`, with the
 * general convention that the functions are named after the corresponding GMP functions minus the leading ``mpq_``
 * prefix. For instance, the GMP call
 *
 * .. code-block:: c++
 *
 *    mpq_add(rop,a,b);
 *
 * that writes the result of ``a + b`` into ``rop`` becomes simply
 *
 * .. code-block:: c++
 *
 *    add(rop,a,b);
 *
 * where the ``add()`` function is resolved via argument-dependent lookup. Function calls with overlapping arguments
 * are allowed, unless noted otherwise.
 * \endrststar
 */
// NOTEs:
// - GMP has a divexact_gcd() function that, I believe, shaves off some complexity in terms of branching
//   etc. Consider implementing it.
// - not clear if the NewRop flag helps at all. Needs to be benchmarked. If it does, its usage could
//   be expanded in mul/div.
// - we might be paying a perf penalty for dynamic storage values due to the lack of pre-allocation for
//   temporary variables in algorithms such as addsub, mul, div, etc. Needs to be investigated.
// - in the algorithm bits derived from GMP ones, we don't check for unitary GCDs because the original
//   algos do not. We should investigate if there's perf benefits to be had there.
// - cmp() can be improved (see comments there).
template <std::size_t SSize>
class rational
{
    // Set denominator to 1. To be used **exclusively** in constructors,
    // only on def-cted den.
    void fast_set_den_one()
    {
        m_den._get_union().g_st()._mp_size = 1;
        // No need to use the mask for just 1.
        m_den._get_union().g_st().m_limbs[0] = 1;
    }
#if !defined(MPPP_DOXYGEN_INVOKED)
    // Make friends with the comparison function.
    template <std::size_t S>
    friend int cmp(const rational<S> &, const rational<S> &);
#endif

public:
/// Underlying integral type.
/**
 * \rststar
 * This is the :cpp:class:`~mppp::integer` type used for the representation of numerator and
 * denominator.
 * \endrststar
 */
#if defined(MPPP_DOXYGEN_INVOKED)
    typedef integer<SSize> int_t;
#else
    using int_t = integer<SSize>;
#endif
    /// Alias for the template parameter \p SSize.
    static constexpr std::size_t ssize = SSize;
    /// Default constructor.
    /**
     * The default constructor will initialize ``this`` to 0 (represented as 0/1).
     */
    rational()
    {
        fast_set_den_one();
    }
    /// Defaulted copy constructor.
    rational(const rational &) = default;
    /// Move constructor.
    /**
     * The move constructor will leave \p other in an unspecified but valid state.
     *
     * @param other the construction argument.
     */
    rational(rational &&other) noexcept : m_num(std::move(other.m_num)), m_den(std::move(other.m_den))
    {
        // NOTE: the aim of this is to have other in a valid state. One reason is that,
        // according to the standard, for use in std::sort() (and possibly other algorithms)
        // it is required that a moved-from rational is still comparable, but if we simply move
        // construct num and den we could end up with other in noncanonical form or zero den.
        // http://stackoverflow.com/questions/26579132/what-is-the-post-condition-of-a-move-constructor
        //
        // NOTE: after move construction, other's data members are guaranteed to be static.
        assert(other.m_num.is_static());
        assert(other.m_den.is_static());
        // Set other's numerator to 0 (see integer::set_zero()).
        other.m_num.m_int.g_st()._mp_size = 0;
        std::fill(other.m_num.m_int.g_st().m_limbs.begin(), other.m_num.m_int.g_st().m_limbs.end(), ::mp_limb_t(0));
        // Set other's denominator to 1 (see integer::set_one()).
        other.m_den.m_int.g_st()._mp_size = 1;
        other.m_den.m_int.g_st().m_limbs[0] = 1;
        std::fill(other.m_den.m_int.g_st().m_limbs.begin() + 1, other.m_den.m_int.g_st().m_limbs.end(), ::mp_limb_t(0));
    }

private:
    template <typename T, enable_if_t<disjunction<std::is_integral<T>, std::is_same<T, int_t>>::value, int> = 0>
    void dispatch_generic_construction(const T &n)
    {
        m_num = n;
        fast_set_den_one();
    }
    template <typename T, enable_if_t<disjunction<std::is_same<float, T>, std::is_same<double, T>>::value, int> = 0>
    void dispatch_generic_construction(const T &x)
    {
        if (mppp_unlikely(!std::isfinite(x))) {
            throw std::domain_error("Cannot construct a rational from the non-finite floating-point value "
                                    + std::to_string(x));
        }
        MPPP_MAYBE_TLS mpq_raii q;
        ::mpq_set_d(&q.m_mpq, static_cast<double>(x));
        m_num.dispatch_mpz_ctor(mpq_numref(&q.m_mpq));
        m_den.dispatch_mpz_ctor(mpq_denref(&q.m_mpq));
    }
#if defined(MPPP_WITH_MPFR)
    void dispatch_generic_construction(const long double &x)
    {
        if (mppp_unlikely(!std::isfinite(x))) {
            throw std::domain_error("Cannot construct a rational from the non-finite floating-point value "
                                    + std::to_string(x));
        }
        // NOTE: static checks for overflows are done in mpfr.hpp.
        constexpr int d2 = std::numeric_limits<long double>::digits10 * 4;
        MPPP_MAYBE_TLS mpfr_raii mpfr(static_cast<::mpfr_prec_t>(d2));
        MPPP_MAYBE_TLS mpf_raii mpf(static_cast<::mp_bitcnt_t>(d2));
        MPPP_MAYBE_TLS mpq_raii mpq;
        // NOTE: we go through an mpfr->mpf->mpq conversion chain as
        // mpfr_get_q() does not exist.
        ::mpfr_set_ld(&mpfr.m_mpfr, x, MPFR_RNDN);
        ::mpfr_get_f(&mpf.m_mpf, &mpfr.m_mpfr, MPFR_RNDN);
        ::mpq_set_f(&mpq.m_mpq, &mpf.m_mpf);
        m_num.dispatch_mpz_ctor(mpq_numref(&mpq.m_mpq));
        m_den.dispatch_mpz_ctor(mpq_denref(&mpq.m_mpq));
    }
#endif

public:
/// Generic constructor.
/**
 * \rststar
 * This constructor will initialize a rational with the value of ``x``. The initialization is always
 * successful if ``x`` is an integral value (construction from ``bool`` yields 1 for ``true``, 0 for ``false``)
 * or an instance of :cpp:type:`~mppp::rational::int_t`.
 * If ``x`` is a floating-point value, the construction will fail if ``x`` is not finite.
 * \endrststar
 *
 * @param x value that will be used to initialize \p this.
 *
 * @throws std::domain_error if \p x is a non-finite floating-point value.
 */
#if defined(MPPP_HAVE_CONCEPTS)
    explicit rational(const RationalInteroperable<SSize> &x)
#else
    template <typename T, rational_interoperable_enabler<T, SSize> = 0>
    explicit rational(const T &x)
#endif
    {
        dispatch_generic_construction(x);
    }
/// Constructor from numerator and denominator.
/**
 * \rststar
 * This constructor is enabled only if both ``T`` and ``U`` satisfy the
 * :cpp:concept:`~mppp::RationalIntegralInteroperable` concept. The input value ``n`` will be used to initialise the
 * numerator, while ``d`` will be used to initialise the denominator.
 * \endrststar
 *
 * @param n the numerator.
 * @param d the denominator.
 *
 * @throws mppp::zero_division_error if the denominator is zero.
 */
#if defined(MPPP_HAVE_CONCEPTS)
    template <typename T, typename U>
#if !defined(MPPP_DOXYGEN_INVOKED)
    requires RationalIntegralInteroperable<T, SSize> &&RationalIntegralInteroperable<U, SSize>
#endif
#else
    template <typename T, typename U,
              enable_if_t<conjunction<is_rational_integral_interoperable<T, SSize>,
                                      is_rational_integral_interoperable<U, SSize>>::value,
                          int> = 0>
#endif
        explicit rational(const T &n, const U &d) : m_num(n), m_den(d)
    {
        if (mppp_unlikely(m_den.is_zero())) {
            throw zero_division_error("Cannot construct a rational with zero as denominator");
        }
        canonicalise();
    }
    /// Constructor from C string.
    /**
     * \rststar
     * This constructor will initialize ``this`` from the null-terminated string ``s``, which must represent
     * a rational value in base ``base``. The expected format is either a numerator-denominator pair separated
     * by the division operator ``/``, or just a numerator (in which case the denominator will be set to one).
     * The format for numerator and denominator is described in the documentation of the constructor from string
     * of :cpp:class:`~mppp::integer`.
     * \endrststar
     *
     * @param s the input string.
     * @param base the base used in the string representation.
     *
     * @throws mppp::zero_division_error if the denominator is zero.
     * @throws unspecified any exception thrown by the string constructor of mppp::integer.
     */
    explicit rational(const char *s, int base = 10)
    {
        MPPP_MAYBE_TLS std::string tmp_str;
        auto ptr = s;
        for (; *ptr != '\0' && *ptr != '/'; ++ptr) {
        }
        tmp_str.assign(s, ptr);
        m_num = int_t{tmp_str, base};
        if (*ptr == '\0') {
            fast_set_den_one();
        } else {
            tmp_str.assign(ptr + 1);
            m_den = int_t{tmp_str, base};
            if (mppp_unlikely(m_den.is_zero())) {
                throw zero_division_error(
                    "A zero denominator was detected in the constructor of a rational from string");
            }
            canonicalise();
        }
    }
    /// Constructor from C++ string (equivalent to the constructor from C string).
    /**
     * @param s the input string.
     * @param base the base used in the string representation.
     *
     * @throws unspecified any exception thrown by the constructor from C string.
     */
    explicit rational(const std::string &s, int base = 10) : rational(s.c_str(), base)
    {
    }
    /// Constructor from \p mpq_t.
    /**
     * This constructor will initialise the numerator and denominator of \p this with those of the GMP rational \p q.
     *
     * \rststar
     * .. warning::
     *
     *    It is the user's responsibility to ensure that ``q`` has been correctly initialized. Calling this constructor
     *    with an uninitialized ``q`` results in undefined behaviour. Also, this constructor will **not**
     *    canonicalise ``this``: numerator and denominator are constructed
     *    as-is from ``q``.
     * \endrststar
     *
     * @param q the input GMP rational.
     */
    explicit rational(const ::mpq_t q)
    {
        m_num.dispatch_mpz_ctor(mpq_numref(q));
        m_den.dispatch_mpz_ctor(mpq_denref(q));
    }
    /// Defaulted copy-assignment operator.
    /**
     * @return a reference to ``this``.
     */
    // NOTE: as long as copy assignment of integer cannot
    // throw, the default is good.
    rational &operator=(const rational &) = default;
    /// Defaulted move assignment operator.
    /**
     * After the assignment, \p other is left in an unspecified but valid state.
     *
     * @param other the assignment argument.
     *
     * @return a reference to ``this``.
     */
    rational &operator=(rational &&other) noexcept
    {
        // NOTE: see the rationale in the move ctor about why we don't want
        // to default this.
        //
        // NOTE: we need the self check here because otherwise we will end
        // up setting this to 0/1, in case of self assignment.
        if (mppp_likely(this != &other)) {
            m_num = std::move(other.m_num);
            m_den = std::move(other.m_den);
            other.m_num.set_zero();
            other.m_den.set_one();
        }
        return *this;
    }
    /// Assignment from \p mpq_t.
    /**
     * This assignment operator will copy into \p this the value of the GMP rational \p q.
     *
     * \rststar
     * .. warning::
     *
     *    It is the user's responsibility to ensure that ``q`` has been correctly initialized. Calling this operator
     *    with an uninitialized ``q`` results in undefined behaviour. Also, this operator will **not** canonicalise
     *    the assigned value: numerator and denominator are assigned as-is from ``q``.
     * \endrststar
     *
     * @param q the input GMP rational.
     *
     * @return a reference to \p this.
     */
    rational &operator=(const ::mpq_t q)
    {
        m_num = mpq_numref(q);
        m_den = mpq_denref(q);
        return *this;
    }
/// Generic assignment operator.
/**
 * \rststar
 * The body of this operator is equivalent to:
 *
 * .. code-block:: c++
 *
 *    return *this = rational{x};
 *
 * That is, a temporary rational is constructed from ``x`` and it is then move-assigned to ``this``.
 * \endrststar
 *
 * @param x the assignment argument.
 *
 * @return a reference to \p this.
 *
 * @throws unspecified any exception thrown by the generic constructor.
 */
#if defined(MPPP_HAVE_CONCEPTS)
    rational &operator=(const RationalInteroperable<SSize> &x)
#else
    template <typename T, rational_interoperable_enabler<T, SSize> = 0>
    rational &operator=(const T &x)
#endif
    {
        return *this = rational{x};
    }
    /// Assignment from C string.
    /**
     * \rststar
     * The body of this operator is equivalent to:
     *
     * .. code-block:: c++
     *
     *    return *this = rational{s};
     *
     * That is, a temporary rational is constructed from ``s`` and it is then move-assigned to ``this``.
     * \endrststar
     *
     * @param s the C string that will be used for the assignment.
     *
     * @return a reference to \p this.
     *
     * @throws unspecified any exception thrown by the constructor from string.
     */
    rational &operator=(const char *s)
    {
        return *this = rational{s};
    }
    /// Assignment from C++ string (equivalent to the assignment from C string).
    /**
     * @param s the C++ string that will be used for the assignment.
     *
     * @return a reference to \p this.
     *
     * @throws unspecified any exception thrown by the assignment operator from C string.
     */
    rational &operator=(const std::string &s)
    {
        return operator=(s.c_str());
    }
    /// Convert to string.
    /**
     * \rststar
     * This operator will return a string representation of ``this`` in base ``base``.
     * The string format consists of the numerator, followed by the division operator ``/`` and the
     * denominator, but only if the denominator is not unitary. Otherwise, only the numerator will be
     * represented in the returned string.
     * \endrststar
     *
     * @param base the desired base for the string representation.
     *
     * @return a string representation for ``this``.
     *
     * @throws unspecified any exception thrown by mppp::integer::to_string().
     */
    std::string to_string(int base = 10) const
    {
        if (m_den.is_one()) {
            return m_num.to_string(base);
        }
        return m_num.to_string(base) + "/" + m_den.to_string(base);
    }

private:
    // Let's keep the view machinery private for now, as it suffers from the potential aliasing
    // issues described in the mpz_view documentation. In this case, we have to fill in a shallow mpq_struct
    // in any case, as the rational class is not backed by a regular mpq_t. We could then have aliasing
    // between a real mpz_t and, say, the numerator extracted from a view which is internally pointing
    // to the same mpz_t. Example:
    //
    // rational q;
    // auto &n = q._get_num();
    // n.promote();
    // auto q_view = q.get_mpq_view();
    // mpz_add_ui(n.get_mpz_t(), mpq_numref(q_view.get()), 1);
    //
    // In the last line, mpq_numref() is extracting an mpz_struct from the view which internally
    // points to the same limbs as the mpz_t inside n. The mpz_add_ui code will try to realloc()
    // the limb pointer inside n, thus invalidating the limb pointer in the view (this would work
    // if the view's mpz_struct were the same object as n's mpz_t, as the GMP code would update
    // the new pointer after the realloc for both structs).
    struct mpq_view {
        explicit mpq_view(const rational &q) : m_mpq{*q.m_num.get_mpz_view().get(), *q.m_den.get_mpz_view().get()}
        {
        }
        operator mpq_struct_t const *() const
        {
            return get();
        }
        mpq_struct_t const *get() const
        {
            return &m_mpq;
        }
        mpq_struct_t m_mpq;
    };
    mpq_view get_mpq_view() const
    {
        return mpq_view{*this};
    }
    // Conversion to int_t.
    template <typename T, enable_if_t<std::is_same<int_t, T>::value, int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        return std::make_pair(true, m_num / m_den);
    }
    // Conversion to bool.
    template <typename T, enable_if_t<std::is_same<bool, T>::value, int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        return {true, m_num.m_int.m_st._mp_size != 0};
    }
    // Conversion to integral types other than bool.
    template <typename T,
              enable_if_t<conjunction<std::is_integral<T>, negation<std::is_same<bool, T>>>::value, int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        return static_cast<int_t>(*this).template dispatch_conversion<T>();
    }
    // Conversion to float/double.
    template <typename T, enable_if_t<disjunction<std::is_same<T, float>, std::is_same<T, double>>::value, int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        return {true, static_cast<T>(::mpq_get_d(get_mpq_view()))};
    }
#if defined(MPPP_WITH_MPFR)
    // Conversion to long double.
    template <typename T, enable_if_t<std::is_same<T, long double>::value, int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        // NOTE: static checks for overflows are done in mpfr.hpp.
        constexpr int d2 = std::numeric_limits<long double>::digits10 * 4;
        MPPP_MAYBE_TLS mpfr_raii mpfr(static_cast<::mpfr_prec_t>(d2));
        MPPP_MAYBE_TLS mpf_raii mpf(static_cast<::mp_bitcnt_t>(d2));
        ::mpf_set_q(&mpf.m_mpf, get_mpq_view());
        ::mpfr_set_f(&mpfr.m_mpfr, &mpf.m_mpf, MPFR_RNDN);
        return {true, ::mpfr_get_ld(&mpfr.m_mpfr, MPFR_RNDN)};
    }
#endif

public:
/// Generic conversion operator.
/**
 * \rststar
 * This operator will convert ``this`` to a :cpp:concept:`~mppp::RationalInteroperable` type.
 * Conversion to ``bool`` yields ``false`` if ``this`` is zero,
 * ``true`` otherwise. Conversion to other integral types and to :cpp:type:`~mppp::rational::int_t`
 * yields the result of the truncated division of the numerator by the denominator, if representable by the target
 * :cpp:concept:`~mppp::RationalInteroperable` type. Conversion to floating-point types might yield inexact values and
 * infinities.
 * \endrststar
 *
 * @return \p this converted to the target type.
 *
 * @throws std::overflow_error if the target type is an integral type and the value of ``this`` cannot be
 * represented by it.
 */
#if defined(MPPP_HAVE_CONCEPTS)
    template <RationalInteroperable<SSize> T>
#else
    template <typename T, rational_interoperable_enabler<T, SSize> = 0>
#endif
    explicit operator T() const
    {
        auto retval = dispatch_conversion<T>();
        if (mppp_unlikely(!retval.first)) {
            throw std::overflow_error("Conversion of the rational " + to_string() + " to the type " + typeid(T).name()
                                      + " results in overflow");
        }
        return std::move(retval.second);
    }
    /// Const numerator getter.
    /**
     * @return a const reference to the numerator.
     */
    const int_t &get_num() const
    {
        return m_num;
    }
    /// Const denominator getter.
    /**
     * @return a const reference to the denominator.
     */
    const int_t &get_den() const
    {
        return m_den;
    }
    /// Mutable numerator getter.
    /**
     * \rststar
     * .. warning::
     *
     *    It is the user's responsibility to ensure that, after changing the numerator
     *    via this getter, the rational is kept in canonical form.
     * \endrststar
     *
     * @return a mutable reference to the numerator.
     */
    int_t &_get_num()
    {
        return m_num;
    }
    /// Mutable denominator getter.
    /**
     * \rststar
     * .. warning::
     *
     *    It is the user's responsibility to ensure that, after changing the denominator
     *    via this getter, the rational is kept in canonical form.
     * \endrststar
     *
     * @return a mutable reference to the denominator.
     */
    int_t &_get_den()
    {
        return m_den;
    }
    /// Canonicalise.
    /**
     * \rststar
     * This method will put ``this`` in canonical form. In particular, this method
     * will make sure that:
     *
     * * if the numerator is zero, the denominator is set to 1,
     * * the numerator and denominator are coprime (dividing them by their GCD,
     *   if necessary),
     * * the denominator is strictly positive.
     *
     * In general, it is not necessary to call explicitly this method, as the public
     * API of :cpp:class:`~mppp::rational` ensures that rationals are kept in canonical
     * form. Calling this method, however, might be necessary if the numerator and/or denominator
     * are modified manually, or when constructing/assigning from non-canonical ``mpq_t``
     * values.
     *
     * .. warning::
     *
     *    Calling this method with on a rational with null denominator will result in undefined
     *    behaviour.
     * \endrststar
     *
     * @return a reference to \p this.
     */
    rational &canonicalise()
    {
        if (m_num.is_zero()) {
            m_den = 1;
            return *this;
        }
        // NOTE: this is best in case of small m_num/m_den.
        // For dynamically allocated num/den, it would be better
        // to have a TLS integer and re-use that accross calls to
        // canonicalise. Eventually, we could consider branching
        // this bit out depending on whether num/den are static
        // or not. Let's keep it simple for now.
        // NOTE: gcd() always gets a positive value.
        const auto g = gcd(m_num, m_den);
        // This can be zero only if both num and den are zero.
        assert(!g.is_zero());
        if (!g.is_one()) {
            divexact(m_num, m_num, g);
            divexact(m_den, m_den, g);
        }
        // Fix mismatch in signs.
        fix_den_sign(*this);
        // NOTE: consider attempting demoting num/den. Let's KIS for now.
        return *this;
    }
    /// Check canonical form.
    /**
     * @return \p true if \p this is the canonical form for rational numbers, \p false otherwise.
     */
    bool is_canonical() const
    {
        if (m_num.is_zero()) {
            // If num is zero, den must be one.
            return m_den.is_one();
        }
        if (m_den.sgn() != 1) {
            // Den must be strictly positive.
            return false;
        }
        if (m_den.is_one()) {
            // The rational is an integer.
            return true;
        }
        // Num and den must be coprime.
        return gcd(m_num, m_den).is_one();
    }
    /// Sign.
    /**
     * @return 0 if \p this is zero, 1 if \p this is positive, -1 if \p this is negative.
     */
    int sgn() const
    {
        return mppp::sgn(m_num);
    }
    /// Negate in-place.
    /**
     * This method will set \p this to <tt>-this</tt>.
     *
     * @return a reference to \p this.
     */
    rational &neg()
    {
        m_num.neg();
        return *this;
    }
    /// In-place absolute value.
    /**
     * This method will set \p this to its absolute value.
     *
     * @return a reference to \p this.
     */
    rational &abs()
    {
        m_num.abs();
        return *this;
    }
    /// In-place inversion.
    /**
     * This method will set \p this to its inverse.
     *
     * @return a reference to \p this.
     *
     * @throws zero_division_error if \p this is zero.
     */
    rational &inv()
    {
        if (mppp_unlikely(is_zero())) {
            throw zero_division_error("Cannot invert a zero rational");
        }
        std::swap(m_num, m_den);
        fix_den_sign(*this);
        return *this;
    }
    /// Test if the value is zero.
    /**
     * @return \p true if the value represented by \p this is 0, \p false otherwise.
     */
    bool is_zero() const
    {
        return mppp::is_zero(m_num);
    }
    /// Test if the value is one.
    /**
     * @return \p true if the value represented by \p this is 1, \p false otherwise.
     */
    bool is_one() const
    {
        return mppp::is_one(m_num) && mppp::is_one(m_den);
    }
    /// Test if the value is minus one.
    /**
     * @return \p true if the value represented by \p this is -1, \p false otherwise.
     */
    bool is_negative_one() const
    {
        return mppp::is_negative_one(m_num) && mppp::is_one(m_den);
    }

private:
    int_t m_num;
    int_t m_den;
};

template <std::size_t SSize>
constexpr std::size_t rational<SSize>::ssize;

inline namespace detail
{

// Machinery for the determination of the result of a binary operation involving rational.
// Default is empty for SFINAE.
template <typename, typename, typename = void>
struct rational_common_type {
};

template <std::size_t SSize>
struct rational_common_type<rational<SSize>, rational<SSize>> {
    using type = rational<SSize>;
};

template <std::size_t SSize>
struct rational_common_type<rational<SSize>, integer<SSize>> {
    using type = rational<SSize>;
};

template <std::size_t SSize>
struct rational_common_type<integer<SSize>, rational<SSize>> {
    using type = rational<SSize>;
};

template <std::size_t SSize, typename U>
struct rational_common_type<rational<SSize>, U, enable_if_t<is_supported_integral<U>::value>> {
    using type = rational<SSize>;
};

template <std::size_t SSize, typename T>
struct rational_common_type<T, rational<SSize>, enable_if_t<is_supported_integral<T>::value>> {
    using type = rational<SSize>;
};

template <std::size_t SSize, typename U>
struct rational_common_type<rational<SSize>, U, enable_if_t<is_supported_float<U>::value>> {
    using type = U;
};

template <std::size_t SSize, typename T>
struct rational_common_type<T, rational<SSize>, enable_if_t<is_supported_float<T>::value>> {
    using type = T;
};

template <typename T, typename U>
using rational_common_t = typename rational_common_type<T, U>::type;

// Implementation of the rational op types concept, used in various operators.
template <typename T, typename U, typename = void>
struct are_rational_op_types : std::false_type {
};

template <std::size_t SSize>
struct are_rational_op_types<rational<SSize>, rational<SSize>> : std::true_type {
};

template <std::size_t SSize, typename T>
struct are_rational_op_types<rational<SSize>, T, enable_if_t<is_rational_interoperable<T, SSize>::value>>
    : std::true_type {
};

template <typename T, std::size_t SSize>
struct are_rational_op_types<T, rational<SSize>, enable_if_t<is_rational_interoperable<T, SSize>::value>>
    : std::true_type {
};

// Implementation of binary add/sub. The NewRop flag indicates that
// rop is a def-cted rational distinct from op1 and op2.
template <bool AddOrSub, bool NewRop, std::size_t SSize>
inline void addsub_impl(rational<SSize> &rop, const rational<SSize> &op1, const rational<SSize> &op2)
{
    assert(!NewRop || (rop.is_zero() && &rop != &op1 && &rop != &op2));
    const bool u1 = op1.get_den().is_one(), u2 = op2.get_den().is_one();
    // NOTE: it's important here to take care about overlapping arguments: we cannot use
    // rop as a "temporary" storage space, because if it overlaps with op1/op2 we will be
    // altering op1/op2 as well.
    if (u1 && u2) {
        // add/sub() are fine with overlapping args.
        AddOrSub ? add(rop._get_num(), op1.get_num(), op2.get_num())
                 : sub(rop._get_num(), op1.get_num(), op2.get_num());
        if (!NewRop) {
            // Set rop's den to 1, if rop is not new (otherwise it's 1 already).
            rop._get_den().set_one();
        }
    } else if (u1) {
        if (NewRop) {
            // rop is separate, can write into it directly.
            rop._get_num() = op2.get_num();
            if (AddOrSub) {
                addmul(rop._get_num(), op1.get_num(), op2.get_den());
            } else {
                submul(rop._get_num(), op1.get_num(), op2.get_den());
                rop._get_num().neg();
            }
            // NOTE: gcd(a+m*b,b) == gcd(a,b) for every integer m, no need to canonicalise the result.
            rop._get_den() = op2.get_den();
        } else {
            integer<SSize> tmp{op2.get_num()};
            // Ok, tmp is a separate variable, won't modify ops.
            if (AddOrSub) {
                addmul(tmp, op1.get_num(), op2.get_den());
            } else {
                submul(tmp, op1.get_num(), op2.get_den());
                tmp.neg();
            }
            // Final assignments, potential for self-assignment
            // in the second one.
            rop._get_num() = std::move(tmp);
            rop._get_den() = op2.get_den();
        }
    } else if (u2) {
        // Mirror of the above.
        if (NewRop) {
            rop._get_num() = op1.get_num();
            AddOrSub ? addmul(rop._get_num(), op2.get_num(), op1.get_den())
                     : submul(rop._get_num(), op2.get_num(), op1.get_den());
            rop._get_den() = op1.get_den();
        } else {
            integer<SSize> tmp{op1.get_num()};
            AddOrSub ? addmul(tmp, op2.get_num(), op1.get_den()) : submul(tmp, op2.get_num(), op1.get_den());
            rop._get_num() = std::move(tmp);
            rop._get_den() = op1.get_den();
        }
    } else if (op1.get_den() == op2.get_den()) {
        // add()/sub() are fine with overlapping args.
        AddOrSub ? add(rop._get_num(), op1.get_num(), op2.get_num())
                 : sub(rop._get_num(), op1.get_num(), op2.get_num());
        // Set rop's den to the common den.
        rop._get_den() = op1.get_den();
        rop.canonicalise();
    } else {
        // NOTE: the algorithm here is taken from GMP's aors.c for mpq. The idea
        // is, as usual, to avoid large canonicalisations and to try to keep
        // the values as small as possible at every step. We need to do some
        // testing about this, as I am not 100% sure that this is going to be
        // a win for our small-operands focus. Let's bookmark here the previous
        // implementation, at git commit:
        // a8a397d67d6e2af43592aa99061016398a1457ad
        auto g = gcd(op1.get_den(), op2.get_den());
        if (g.is_one()) {
            // This is the case in which the two dens are coprime.
            AddOrSub ? add(rop._get_num(), op1.get_num() * op2.get_den(), op2.get_num() * op1.get_den())
                     : sub(rop._get_num(), op1.get_num() * op2.get_den(), op2.get_num() * op1.get_den());
            mul(rop._get_den(), op1.get_den(), op2.get_den());
        } else {
            // Eliminate common factors between the dens.
            auto t = divexact(op2.get_den(), g);
            auto tmp2 = divexact(op1.get_den(), g);

            // Compute the numerator (will be t).
            auto tmp1 = op1.get_num() * t;
            mul(t, op2.get_num(), tmp2);
            AddOrSub ? add(t, tmp1, t) : sub(t, tmp1, t);

            // Check if the numerator and the den GCD are coprime.
            gcd(g, t, g);
            if (g.is_one()) {
                // They are coprime: assign the num and compute the final den.
                rop._get_num() = std::move(t);
                mul(rop._get_den(), op2.get_den(), tmp2);
            } else {
                // Assign numerator, reduced by the new gcd.
                divexact(rop._get_num(), t, g);
                // Reduced version of the second den.
                divexact(tmp1, op2.get_den(), g);
                // Assign final den: tmp1 x the reduced den1.
                mul(rop._get_den(), tmp1, tmp2);
            }
        }
    }
}
}

template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool RationalOpTypes = are_rational_op_types<T, U>::value;
#else
using rational_op_types_enabler = enable_if_t<are_rational_op_types<T, U>::value, int>;
#endif

/** @defgroup rational_arithmetic rational_arithmetic
 *  @{
 */

/// Ternary addition.
/**
 * This function will set \p rop to <tt>op1 + op2</tt>.
 *
 * @param rop the return value.
 * @param op1 the first argument.
 * @param op2 the second argument.
 */
template <std::size_t SSize>
inline void add(rational<SSize> &rop, const rational<SSize> &op1, const rational<SSize> &op2)
{
    addsub_impl<true, false>(rop, op1, op2);
}

/// Ternary subtraction.
/**
 * This function will set \p rop to <tt>op1 - op2</tt>.
 *
 * @param rop the return value.
 * @param op1 the first argument.
 * @param op2 the second argument.
 */
template <std::size_t SSize>
inline void sub(rational<SSize> &rop, const rational<SSize> &op1, const rational<SSize> &op2)
{
    addsub_impl<false, false>(rop, op1, op2);
}

inline namespace detail
{
template <bool NewRop, std::size_t SSize>
inline void mul_impl(rational<SSize> &rop, const rational<SSize> &op1, const rational<SSize> &op2)
{
    assert(!NewRop || rop.is_zero());
    const bool u1 = op1.get_den().is_one(), u2 = op2.get_den().is_one();
    // NOTE: it's important here to take care about overlapping arguments: we cannot use
    // rop as a "temporary" storage space, because if it overlaps with op1/op2 we will be
    // altering op1/op2 as well.
    if (u1 && u2) {
        // mul() is fine with overlapping args.
        mul(rop._get_num(), op1.get_num(), op2.get_num());
        if (!NewRop) {
            // Set rop's den to 1, if rop is not a new value (in that case, den is 1 already).
            rop._get_den().set_one();
        }
    } else if (op1.get_den() == op2.get_den()) {
        // Special case: equal dens do not require canonicalisation.
        mul(rop._get_num(), op1.get_num(), op2.get_num());
        // NOTE: we can use a squaring function here, once implemented in integer.
        mul(rop._get_den(), op1.get_den(), op2.get_den());
    } else if (u1) {
        // This is a * (b/c). Instead of doing (ab)/c and then canonicalise,
        // we remove the common factors from a and c and we perform
        // a normal multiplication (without canonicalisation). This allows us to
        // perform a gcd with smaller operands.
        auto g = gcd(op1.get_num(), op2.get_den());
        if (g.is_one()) {
            // NOTE: after this line, all nums are tainted.
            mul(rop._get_num(), op2.get_num(), op1.get_num());
            rop._get_den() = op2.get_den();
        } else {
            // NOTE: after this line, all dens are tainted.
            divexact(rop._get_den(), op2.get_den(), g);
            // Re-use g.
            divexact(g, op1.get_num(), g);
            mul(rop._get_num(), op2.get_num(), g);
        }
    } else if (u2) {
        // Mirror of the above.
        auto g = gcd(op2.get_num(), op1.get_den());
        if (g.is_one()) {
            mul(rop._get_num(), op1.get_num(), op2.get_num());
            rop._get_den() = op1.get_den();
        } else {
            divexact(rop._get_den(), op1.get_den(), g);
            divexact(g, op2.get_num(), g);
            mul(rop._get_num(), op1.get_num(), g);
        }
    } else {
        // General case: a/b * c/d
        // NOTE: like above, we don't want to canonicalise (ac)/(bd),
        // and we trade one big gcd with two smaller gcds.
        // Compute gcd(a,d) and gcd(b,c).
        const auto g1 = gcd(op1.get_num(), op2.get_den());
        const auto g2 = gcd(op1.get_den(), op2.get_num());
        // Remove common factors from the nums.
        auto tmp1 = divexact(op1.get_num(), g1);
        auto tmp2 = divexact(op2.get_num(), g2);
        // Compute rop's numerator.
        // NOTE: after this line, all nums are tainted.
        mul(rop._get_num(), tmp1, tmp2);
        // Remove common factors from the dens.
        divexact(tmp1, op2.get_den(), g1);
        divexact(tmp2, op1.get_den(), g2);
        // Compute rop's denominator.
        mul(rop._get_den(), tmp1, tmp2);
    }
}
}

/// Ternary multiplication.
/**
 * This function will set \p rop to <tt>op1 * op2</tt>.
 *
 * @param rop the return value.
 * @param op1 the first argument.
 * @param op2 the second argument.
 */
template <std::size_t SSize>
inline void mul(rational<SSize> &rop, const rational<SSize> &op1, const rational<SSize> &op2)
{
    mul_impl<false>(rop, op1, op2);
}

/// Ternary division.
/**
 * This function will set \p rop to <tt>op1 / op2</tt>.
 *
 * @param rop the return value.
 * @param op1 the first argument.
 * @param op2 the second argument.
 *
 * @throws zero_division_error if \p op2 is zero.
 */
template <std::size_t SSize>
inline void div(rational<SSize> &rop, const rational<SSize> &op1, const rational<SSize> &op2)
{
    if (mppp_unlikely(op2.is_zero())) {
        throw zero_division_error("Zero divisor in rational division");
    }
    if (mppp_unlikely(&rop == &op2)) {
        // Following the GMP algorithm, special case in which rop and op2 are the same object.
        // This allows us to use op2.get_num() safely later, even after setting rop's num, as
        // we will be sure that rop and op2 do not overlap.
        if (mppp_unlikely(&rop == &op1)) {
            // x = x/x = 1.
            rop._get_num().set_one();
            rop._get_den().set_one();
            return;
        }
        // Set rop to 1/rop by swapping num/den.
        // NOTE: we already checked that op2 is nonzero, so inverting it
        // does not yield division by zero.
        std::swap(rop._get_num(), rop._get_den());
        // Fix den sign.
        fix_den_sign(rop);
        // Multiply by op1.
        mul(rop, rop, op1);
        return;
    }
    const bool u1 = op1.get_den().is_one(), u2 = op2.get_den().is_one();
    if ((u1 && u2) || (op1.get_den() == op2.get_den())) {
        const auto g = gcd(op1.get_num(), op2.get_num());
        if (g.is_one()) {
            rop._get_num() = op1.get_num();
            // NOTE: op2 not tainted, as it's separate from rop.
            rop._get_den() = op2.get_num();
        } else {
            divexact(rop._get_num(), op1.get_num(), g);
            divexact(rop._get_den(), op2.get_num(), g);
        }
    } else if (u1) {
        // Same idea as in the mul().
        auto g = gcd(op1.get_num(), op2.get_num());
        if (g.is_one()) {
            mul(rop._get_num(), op2.get_den(), op1.get_num());
            // NOTE: op2's num is not tainted, as op2 is distinct
            // from rop.
            rop._get_den() = op2.get_num();
        } else {
            // NOTE: dens tainted after this, apart from op2's
            // as we have established earlier that rop and op2
            // are distinct.
            divexact(rop._get_den(), op2.get_num(), g);
            divexact(g, op1.get_num(), g);
            mul(rop._get_num(), op2.get_den(), g);
        }
    } else if (u2) {
        auto g = gcd(op1.get_num(), op2.get_num());
        if (g.is_one()) {
            rop._get_num() = op1.get_num();
            mul(rop._get_den(), op1.get_den(), op2.get_num());
        } else {
            divexact(rop._get_num(), op1.get_num(), g);
            divexact(g, op2.get_num(), g);
            mul(rop._get_den(), op1.get_den(), g);
        }
    } else {
        // (a/b) / (c/d) -> a/b * d/c
        // The two gcds.
        const auto g1 = gcd(op1.get_num(), op2.get_num());
        const auto g2 = gcd(op1.get_den(), op2.get_den());
        // Remove common factors.
        auto tmp1 = divexact(op1.get_num(), g1);
        auto tmp2 = divexact(op2.get_den(), g2);
        // Compute the numerator.
        mul(rop._get_num(), tmp1, tmp2);
        // Remove common factors.
        divexact(tmp1, op2.get_num(), g1);
        divexact(tmp2, op1.get_den(), g2);
        // Denominator.
        mul(rop._get_den(), tmp1, tmp2);
    }
    // Fix wrong sign in the den.
    fix_den_sign(rop);
}

/// Binary negation.
/**
 * This method will set \p rop to <tt>-q</tt>.
 *
 * @param rop the return value.
 * @param q the rational that will be negated.
 */
template <std::size_t SSize>
inline void neg(rational<SSize> &rop, const rational<SSize> &q)
{
    rop = q;
    rop.neg();
}

/// Unary negation.
/**
 * @param q the rational that will be negated.
 *
 * @return <tt>-q</tt>.
 */
template <std::size_t SSize>
inline rational<SSize> neg(const rational<SSize> &q)
{
    rational<SSize> ret(q);
    ret.neg();
    return ret;
}

/// Binary absolute value.
/**
 * This function will set \p rop to the absolute value of \p q.
 *
 * @param rop the return value.
 * @param q the argument.
 */
template <std::size_t SSize>
inline void abs(rational<SSize> &rop, const rational<SSize> &q)
{
    rop = q;
    rop.abs();
}

/// Unary absolute value.
/**
 * @param q the argument.
 *
 * @return the absolute value of \p q.
 */
template <std::size_t SSize>
inline rational<SSize> abs(const rational<SSize> &q)
{
    rational<SSize> ret(q);
    ret.abs();
    return ret;
}

/// Binary inversion.
/**
 * This function will set \p rop to the inverse of \p q.
 *
 * @param rop the return value.
 * @param q the argument.
 *
 * @throws unspecified any exception thrown by mppp::rational::inv().
 */
template <std::size_t SSize>
inline void inv(rational<SSize> &rop, const rational<SSize> &q)
{
    rop = q;
    rop.inv();
}

/// Unary inversion.
/**
 * @param q the argument.
 *
 * @return the inverse of \p q.
 *
 * @throws unspecified any exception thrown by mppp::rational::inv().
 */
template <std::size_t SSize>
inline rational<SSize> inv(const rational<SSize> &q)
{
    rational<SSize> ret(q);
    ret.inv();
    return ret;
}

/** @} */

/** @defgroup rational_io rational_io
 *  @{
 */

/// Output stream operator.
/**
 * \rststar
 * This operator will print to the stream ``os`` the :cpp:class:`~mppp::rational` ``q`` in base 10.
 * The printing format is described in :cpp:func:`mppp::rational::to_string()`.
 * \endrststar
 *
 * @param os the target stream.
 * @param q the input rational.
 *
 * @return a reference to \p os.
 *
 * @throws unspecified any exception thrown by mppp::rational::to_string().
 */
template <std::size_t SSize>
inline std::ostream &operator<<(std::ostream &os, const rational<SSize> &q)
{
    return os << q.to_string();
}

/// Input stream operator.
/**
 * \rststar
 * This operator is equivalent to extracting a line from the stream, using it to construct a temporary
 * :cpp:class:`~mppp::rational` and then assigning the temporary to ``q``.
 * \endrststar
 *
 * @param is input stream.
 * @param q rational to which the contents of the stream will be assigned.
 *
 * @return a reference to \p is.
 *
 * @throws unspecified any exception thrown by the constructor from string of rational.
 */
template <std::size_t SSize>
inline std::istream &operator>>(std::istream &is, rational<SSize> &q)
{
    MPPP_MAYBE_TLS std::string tmp_str;
    std::getline(is, tmp_str);
    q = rational<SSize>{tmp_str};
    return is;
}

/** @} */

/** @defgroup rational_operators rational_operators
 *  @{
 */

/// Identity operator.
/**
 * @param q the rational that will be copied.
 *
 * @return a copy of \p q.
 */
template <std::size_t SSize>
inline rational<SSize> operator+(const rational<SSize> &q)
{
    return q;
}

inline namespace detail
{

// Dispatching for the binary addition operator.
template <std::size_t SSize>
inline rational<SSize> dispatch_binary_add(const rational<SSize> &op1, const rational<SSize> &op2)
{
    rational<SSize> retval;
    addsub_impl<true, true>(retval, op1, op2);
    return retval;
}

template <std::size_t SSize>
inline rational<SSize> dispatch_binary_add(const rational<SSize> &op1, const integer<SSize> &op2)
{
    rational<SSize> retval{op1};
    if (op1.get_den().is_one()) {
        add(retval._get_num(), retval._get_num(), op2);
    } else {
        addmul(retval._get_num(), retval.get_den(), op2);
    }
    return retval;
}

template <std::size_t SSize>
inline rational<SSize> dispatch_binary_add(const integer<SSize> &op1, const rational<SSize> &op2)
{
    return dispatch_binary_add(op2, op1);
}

template <std::size_t SSize, typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_add(const rational<SSize> &op1, T n)
{
    return dispatch_binary_add(op1, integer<SSize>{n});
}

template <std::size_t SSize, typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_add(T n, const rational<SSize> &op2)
{
    return dispatch_binary_add(op2, n);
}

template <std::size_t SSize, typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
inline T dispatch_binary_add(const rational<SSize> &op1, T x)
{
    return static_cast<T>(op1) + x;
}

template <std::size_t SSize, typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
inline T dispatch_binary_add(T x, const rational<SSize> &op2)
{
    return dispatch_binary_add(op2, x);
}
}

/// Binary addition operator.
/**
 * \rststar
 * This operator is enabled only if ``T`` and ``U`` satisfy :cpp:concept:`~mppp::RationalOpTypes`.
 * The return type is determined as follows:
 *
 * * if the non-:cpp:class:`~mppp::rational` argument is a floating-point type ``F``, then the
 *   type of the result is ``F``; otherwise,
 * * the type of the result is :cpp:class:`~mppp::rational`.
 *
 * \endrststar
 *
 * @param op1 the first summand.
 * @param op2 the second summand.
 *
 * @return <tt>op1 + op2</tt>.
 */
template <typename T, typename U>
inline rational_common_t<T, U> operator+(const T &op1, const U &op2)
{
    return dispatch_binary_add(op1, op2);
}

/// Negated copy.
/**
 * @param q the rational that will be negated.
 *
 * @return a negated copy of \p q.
 */
template <std::size_t SSize>
inline rational<SSize> operator-(const rational<SSize> &q)
{
    auto retval(q);
    retval.neg();
    return retval;
}

inline namespace detail
{

// Dispatching for the binary subtraction operator.
template <std::size_t SSize>
inline rational<SSize> dispatch_binary_sub(const rational<SSize> &op1, const rational<SSize> &op2)
{
    rational<SSize> retval;
    addsub_impl<false, true>(retval, op1, op2);
    return retval;
}

template <std::size_t SSize>
inline rational<SSize> dispatch_binary_sub(const rational<SSize> &op1, const integer<SSize> &op2)
{
    rational<SSize> retval{op1};
    if (op1.get_den().is_one()) {
        sub(retval._get_num(), retval._get_num(), op2);
    } else {
        submul(retval._get_num(), retval.get_den(), op2);
    }
    return retval;
}

template <std::size_t SSize>
inline rational<SSize> dispatch_binary_sub(const integer<SSize> &op1, const rational<SSize> &op2)
{
    auto retval = dispatch_binary_sub(op2, op1);
    retval.neg();
    return retval;
}

template <std::size_t SSize, typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_sub(const rational<SSize> &op1, T n)
{
    return dispatch_binary_sub(op1, integer<SSize>{n});
}

template <std::size_t SSize, typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_sub(T n, const rational<SSize> &op2)
{
    auto retval = dispatch_binary_sub(op2, n);
    retval.neg();
    return retval;
}

template <std::size_t SSize, typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
inline T dispatch_binary_sub(const rational<SSize> &op1, T x)
{
    return static_cast<T>(op1) - x;
}

template <std::size_t SSize, typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
inline T dispatch_binary_sub(T x, const rational<SSize> &op2)
{
    return -dispatch_binary_sub(op2, x);
}
}

/// Binary subtraction operator.
/**
 * \rststar
 * This operator is enabled only if ``T`` and ``U`` satisfy :cpp:concept:`~mppp::RationalOpTypes`.
 * The return type is determined as follows:
 *
 * * if the non-:cpp:class:`~mppp::rational` argument is a floating-point type ``F``, then the
 *   type of the result is ``F``; otherwise,
 * * the type of the result is :cpp:class:`~mppp::rational`.
 *
 * \endrststar
 *
 * @param op1 the first operand.
 * @param op2 the second operand.
 *
 * @return <tt>op1 - op2</tt>.
 */
template <typename T, typename U>
inline rational_common_t<T, U> operator-(const T &op1, const U &op2)
{
    return dispatch_binary_sub(op1, op2);
}

inline namespace detail
{

// Dispatching for the binary multiplication operator.
template <std::size_t SSize>
inline rational<SSize> dispatch_binary_mul(const rational<SSize> &op1, const rational<SSize> &op2)
{
    rational<SSize> retval;
    mul_impl<true>(retval, op1, op2);
    return retval;
}

template <std::size_t SSize>
inline rational<SSize> dispatch_binary_mul(const rational<SSize> &op1, const integer<SSize> &op2)
{
    rational<SSize> retval;
    if (op1.get_den().is_one()) {
        mul(retval._get_num(), op1.get_num(), op2);
    } else {
        auto g = gcd(op1.get_den(), op2);
        if (g.is_one()) {
            // Nums will be tainted after this.
            mul(retval._get_num(), op1.get_num(), op2);
            retval._get_den() = op1.get_den();
        } else {
            // Set the den first. Dens tainted after this.
            divexact(retval._get_den(), op1.get_den(), g);
            // Re-use the g variable as tmp storage.
            divexact(g, op2, g);
            mul(retval._get_num(), op1.get_num(), g);
        }
    }
    return retval;
}

template <std::size_t SSize>
inline rational<SSize> dispatch_binary_mul(const integer<SSize> &op1, const rational<SSize> &op2)
{
    return dispatch_binary_mul(op2, op1);
}

template <std::size_t SSize, typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_mul(const rational<SSize> &op1, T n)
{
    return dispatch_binary_mul(op1, integer<SSize>{n});
}

template <std::size_t SSize, typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_mul(T n, const rational<SSize> &op2)
{
    return dispatch_binary_mul(op2, n);
}

template <std::size_t SSize, typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
inline T dispatch_binary_mul(const rational<SSize> &op1, T x)
{
    return static_cast<T>(op1) * x;
}

template <std::size_t SSize, typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
inline T dispatch_binary_mul(T x, const rational<SSize> &op2)
{
    return dispatch_binary_mul(op2, x);
}
}

/// Binary multiplication operator.
/**
 * \rststar
 * This operator is enabled only if ``T`` and ``U`` satisfy :cpp:concept:`~mppp::RationalOpTypes`.
 * The return type is determined as follows:
 *
 * * if the non-:cpp:class:`~mppp::rational` argument is a floating-point type ``F``, then the
 *   type of the result is ``F``; otherwise,
 * * the type of the result is :cpp:class:`~mppp::rational`.
 *
 * \endrststar
 *
 * @param op1 the first factor.
 * @param op2 the second factor.
 *
 * @return <tt>op1 * op2</tt>.
 */
template <typename T, typename U>
inline rational_common_t<T, U> operator*(const T &op1, const U &op2)
{
    return dispatch_binary_mul(op1, op2);
}

inline namespace detail
{

// Dispatching for the binary division operator.
template <std::size_t SSize>
inline rational<SSize> dispatch_binary_div(const rational<SSize> &op1, const rational<SSize> &op2)
{
    rational<SSize> retval;
    div(retval, op1, op2);
    return retval;
}

template <std::size_t SSize>
inline rational<SSize> dispatch_binary_div(const rational<SSize> &op1, const integer<SSize> &op2)
{
    if (mppp_unlikely(op2.is_zero())) {
        throw zero_division_error("Zero divisor in rational division");
    }
    rational<SSize> retval;
    auto g = gcd(op1.get_num(), op2);
    if (op1.get_den().is_one()) {
        if (g.is_one()) {
            retval._get_num() = op1.get_num();
            retval._get_den() = op2;
        } else {
            divexact(retval._get_num(), op1.get_num(), g);
            divexact(retval._get_den(), op2, g);
        }
    } else {
        if (g.is_one()) {
            retval._get_num() = op1.get_num();
            mul(retval._get_den(), op1.get_den(), op2);
        } else {
            // Set the num first.
            divexact(retval._get_num(), op1.get_num(), g);
            // Re-use the g variable as tmp storage.
            divexact(g, op2, g);
            mul(retval._get_den(), op1.get_den(), g);
        }
    }
    // Fix den sign.
    fix_den_sign(retval);
    return retval;
}

// NOTE: could not find a way to easily share the implementation above, so there's some repetition here.
template <std::size_t SSize>
inline rational<SSize> dispatch_binary_div(const integer<SSize> &op1, const rational<SSize> &op2)
{
    if (mppp_unlikely(op2.is_zero())) {
        throw zero_division_error("Zero divisor in rational division");
    }
    rational<SSize> retval;
    auto g = gcd(op1, op2.get_num());
    if (op2.get_den().is_one()) {
        if (g.is_one()) {
            retval._get_num() = op1;
            retval._get_den() = op2.get_num();
        } else {
            divexact(retval._get_num(), op1, g);
            divexact(retval._get_den(), op2.get_num(), g);
        }
    } else {
        if (g.is_one()) {
            mul(retval._get_num(), op1, op2.get_den());
            retval._get_den() = op2.get_num();
        } else {
            divexact(retval._get_den(), op2.get_num(), g);
            divexact(g, op1, g);
            mul(retval._get_num(), op2.get_den(), g);
        }
    }
    // Fix den sign.
    fix_den_sign(retval);
    return retval;
}

template <std::size_t SSize, typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_div(const rational<SSize> &op1, T n)
{
    return dispatch_binary_div(op1, integer<SSize>{n});
}

template <std::size_t SSize, typename T, enable_if_t<is_supported_integral<T>::value, int> = 0>
inline rational<SSize> dispatch_binary_div(T n, const rational<SSize> &op2)
{
    return dispatch_binary_div(integer<SSize>{n}, op2);
}

template <std::size_t SSize, typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
inline T dispatch_binary_div(const rational<SSize> &op1, T x)
{
    return static_cast<T>(op1) / x;
}

template <std::size_t SSize, typename T, enable_if_t<is_supported_float<T>::value, int> = 0>
inline T dispatch_binary_div(T x, const rational<SSize> &op2)
{
    return x / static_cast<T>(op2);
}
}

/// Binary division operator.
/**
 * \rststar
 * This operator is enabled only if ``T`` and ``U`` satisfy :cpp:concept:`~mppp::RationalOpTypes`.
 * The return type is determined as follows:
 *
 * * if the non-:cpp:class:`~mppp::rational` argument is a floating-point type ``F``, then the
 *   type of the result is ``F``; otherwise,
 * * the type of the result is :cpp:class:`~mppp::rational`.
 *
 * \endrststar
 *
 * @param op1 the dividend.
 * @param op2 the divisor.
 *
 * @return <tt>op1 / op2</tt>.
 *
 * @throws zero_division_error if the division does not involve floating-point types and \p op2 is zero.
 */
template <typename T, typename U>
inline rational_common_t<T, U> operator/(const T &op1, const U &op2)
{
    return dispatch_binary_div(op1, op2);
}

inline namespace detail
{

template <std::size_t SSize>
inline bool dispatch_equality(const rational<SSize> &op1, const rational<SSize> &op2)
{
    return op1.get_num() == op2.get_num() && op1.get_den() == op2.get_den();
}

template <std::size_t SSize, typename T, enable_if_t<is_rational_integral_interoperable<T, SSize>::value, int> = 0>
inline bool dispatch_equality(const rational<SSize> &op1, const T &op2)
{
    return op1.get_den().is_one() && op1.get_num() == op2;
}

template <std::size_t SSize, typename T, enable_if_t<is_rational_integral_interoperable<T, SSize>::value, int> = 0>
inline bool dispatch_equality(const T &op1, const rational<SSize> &op2)
{
    return dispatch_equality(op2, op1);
}

template <std::size_t SSize, typename T, enable_if_t<std::is_floating_point<T>::value, int> = 0>
inline bool dispatch_equality(const rational<SSize> &op1, const T &op2)
{
    return static_cast<T>(op1) == op2;
}

template <std::size_t SSize, typename T, enable_if_t<std::is_floating_point<T>::value, int> = 0>
inline bool dispatch_equality(const T &op1, const rational<SSize> &op2)
{
    return dispatch_equality(op2, op1);
}
}

/// Equality operator.
/**
 * @param op1 first argument.
 * @param op2 second argument.
 *
 * @return \p true if <tt>op1 == op2</tt>, \p false otherwise.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline bool operator==(const T &op1, const RationalOpTypes<T> &op2)
#else
template <typename T, typename U, rational_op_types_enabler<T, U> = 0>
inline bool operator==(const T &op1, const U &op2)
#endif
{
    return dispatch_equality(op1, op2);
}

/// Inequality operator.
/**
 * @param op1 first argument.
 * @param op2 second argument.
 *
 * @return \p true if <tt>op1 != op2</tt>, \p false otherwise.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline bool operator!=(const T &op1, const RationalOpTypes<T> &op2)
#else
template <typename T, typename U, rational_op_types_enabler<T, U> = 0>
inline bool operator!=(const T &op1, const U &op2)
#endif
{
    return !(op1 == op2);
}

/** @} */

/** @defgroup rational_comparison rational_comparison
 *  @{
 */

/// Comparison function for rationals.
/**
 * @param op1 first argument.
 * @param op2 second argument.
 *
 * @return \p 0 if <tt>op1 == op2</tt>, a negative value if <tt>op1 < op2</tt>, a positive value if
 * <tt>op1 > op2</tt>.
 */
template <std::size_t SSize>
inline int cmp(const rational<SSize> &op1, const rational<SSize> &op2)
{
    // NOTE: here we have potential for 2 views referring to the same underlying
    // object. The same potential issues as described in the mpz_view class may arise.
    // Keep an eye on it.
    // NOTE: this can probably be improved by implementing the same strategy as in ::mpq_cmp()
    // but based on our primitives:
    // - if op1 and op2 are integers, compare the nums,
    // - try to see if the limb/bit sizes of nums and dens can tell use immediately which
    //   number is larger,
    // - otherwise, do the two multiplications and compare.
    return ::mpq_cmp(op1.get_mpq_view(), op2.get_mpq_view());
}

/// Sign function.
/**
 * @param q the rational whose sign will be computed.
 *
 * @return 0 if \p q is zero, 1 if \p q is positive, -1 if \p q is negative.
 */
template <std::size_t SSize>
int sgn(const rational<SSize> &q)
{
    return q.sgn();
}

/// Test if a rational is one.
/**
 * @param q the rational to be tested.
 *
 * @return \p true if \p q is 1, \p false otherwise.
 */
template <std::size_t SSize>
inline bool is_one(const rational<SSize> &q)
{
    return q.is_one();
}

/// Test if a rational is minus one.
/**
 * @param q the rational to be tested.
 *
 * @return \p true if \p q is -1, \p false otherwise.
 */
template <std::size_t SSize>
inline bool is_negative_one(const rational<SSize> &q)
{
    return q.is_negative_one();
}

/// Test if a rational is zero.
/**
 * @param q the rational to be tested.
 *
 * @return \p true if \p q is 0, \p false otherwise.
 */
template <std::size_t SSize>
inline bool is_zero(const rational<SSize> &q)
{
    return q.is_zero();
}

/** @} */

/** @defgroup rational_other rational_other
 *  @{
 */

/// Canonicalise.
/**
 * \rststar
 * This function will put ``q`` in canonical form. Internally, this function will employ
 * :cpp:func:`mppp::rational::canonicalise()`.
 * \endrststar
 *
 * @param q the rational that will be canonicalised.
 */
template <std::size_t SSize>
inline void canonicalise(rational<SSize> &q)
{
    q.canonicalise();
}

/// Hash value.
/**
 * \rststar
 * This function will return a hash value for ``q``.
 *
 * A specialisation of the standard ``std::hash`` functor is also provided, so that it is possible to use
 * :cpp:class:`~mppp::rational` in standard unordered associative containers out of the box.
 * \endrststar
 *
 * @param q the rational whose hash value will be computed.
 *
 * @return a hash value for \p q.
 */
template <std::size_t SSize>
inline std::size_t hash(const rational<SSize> &q)
{
    // NOTE: just return the sum of the hashes. We are already doing
    // some hashing in the integers, hopefully this is enough to obtain
    // decent hashing on the rational as well.
    return hash(q.get_num()) + hash(q.get_den());
}

/** @} */
}

namespace std
{

template <size_t SSize>
struct hash<mppp::rational<SSize>> {
    using argument_type = mppp::rational<SSize>;
    using result_type = size_t;
    result_type operator()(const argument_type &q) const
    {
        return mppp::hash(q);
    }
};
}

#endif
