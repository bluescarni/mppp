// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_RATIONAL_HPP
#define MPPP_RATIONAL_HPP

#include <cassert>
#include <cmath>
#include <cstddef>
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

/// Multiprecision rational class.
/**
 * \rststar
 * *#include <mp++/rational.hpp>*
 *
 * .. versionadded:: 0.3
 *
 * This class represents arbitrary-precision rationals. Internally, the class stores a pair of
 * :cpp:class:`integers <mppp::integer>` with static size ``SSize`` as the numerator and denonimator.
 * Rational numbers are represented in the usual canonical form:
 *
 * * 0 is represented as 0/1,
 * * numerator and denominator are coprime,
 * * the denominator is always strictly positive.
 * \endrststar
 */
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
    /// Defaulted move constructor.
    rational(rational &&) = default;

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
        MPPP_MAYBE_TLS mpfr_raii mpfr;
        MPPP_MAYBE_TLS mpf_raii mpf;
        MPPP_MAYBE_TLS mpq_raii mpq;
        // NOTE: static checks for overflows are done in mpfr.hpp.
        constexpr int d2 = std::numeric_limits<long double>::digits10 * 4;
        ::mpfr_set_prec(&mpfr.m_mpfr, static_cast<::mpfr_prec_t>(d2));
        ::mpf_set_prec(&mpf.m_mpf, static_cast<::mp_bitcnt_t>(d2));
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
        explicit rational(T &&n, U &&d) : m_num(std::forward<T>(n)), m_den(std::forward<U>(d))
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
     * The format of numerator and denominator is described in the documentation of the constructor from string
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
     *    with an uninitialized ``q`` is undefined behaviour. Also, this constructor will **not**
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
     * @return a reference to ``this``.
     */
    rational &operator=(rational &&) = default;
    /// Assignment from \p mpq_t.
    /**
     * This assignment operator will copy into \p this the value of the GMP rational \p q.
     *
     * \rststar
     * .. warning::
     *
     *    It is the user's responsibility to ensure that ``q`` has been correctly initialized. Calling this operator
     *    with an uninitialized ``q`` is undefined behaviour. Also, this operator will **not** canonicalise
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
    /// Convert to string.
    /**
     * \rststar
     * This operator will return a string representation of ``this`` in base ``base``.
     * The string format consists of the numerator, followed by the division operator ``/`` and the
     * denominator, but only if the denominator is not unitary. Otherwise, only the numerator will be
     * present in the returned string.
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
    // Conversion to bool.
    template <typename T, enable_if_t<std::is_same<bool, T>::value, int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        return {true, m_num.m_int.m_st._mp_size != 0};
    }
    // Conversion to int_t.
    template <typename T, enable_if_t<std::is_same<int_t, T>::value, int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        return std::make_pair(true, m_num / m_den);
    }
    // Conversion to integral types other than bool.
    template <typename T,
              enable_if_t<conjunction<std::is_integral<T>, negation<std::is_same<bool, T>>>::value, int> = 0>
    std::pair<bool, T> dispatch_conversion() const
    {
        return static_cast<int_t>(*this).dispatch_conversion<T>();
    }

public:
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
     * \endrststar
     */
    void canonicalise()
    {
        if (m_num.is_zero()) {
            m_den = 1;
            return;
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
        if (sgn(m_den) == -1) {
            m_num.neg();
            m_den.neg();
        }
        // NOTE: consider attempting demoting num/den. Let's KIS for now.
    }

private:
    int_t m_num;
    int_t m_den;
};

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
    const bool u1 = op1.get_den().is_one(), u2 = op2.get_den().is_one();
    // NOTE: it's important here to take care about overlapping arguments: we cannot use
    // rop as a "temporary" storage space, because if it overlaps with op1/op2 we will be
    // altering op1/op2 as well.
    if (u1 && u2) {
        // add() is fine with overlapping args.
        add(rop._get_num(), op1.get_num(), op2.get_num());
    } else if (u1) {
        integer<SSize> tmp{op2.get_num()};
        // Ok, tmp is a separate variable, won't modify ops.
        addmul(tmp, op1.get_num(), op2.get_den());
        // Final assignments, potential for self-assignment
        // in the second one.
        rop._get_num() = std::move(tmp);
        rop._get_den() = op2.get_den();
        // NOTE: gcd(a+m*b,b) == gcd(a,b) for every integer m, no need to canonicalise the result.
    } else if (u2) {
        // Mirror of the above.
        integer<SSize> tmp{op1.get_num()};
        addmul(tmp, op2.get_num(), op1.get_den());
        rop._get_num() = std::move(tmp);
        rop._get_den() = op1.get_den();
    } else if (op1.get_den() == op2.get_den()) {
        // add() is fine with overlapping args.
        add(rop._get_num(), op1.get_num(), op2.get_num());
        rop.canonicalise();
    } else {
        integer<SSize> tmp;
        // These two steps are ok, tmp is a separate variable.
        // NOTE: these implement a*d+b*c. We might have a primitive operation
        // for that down the line.
        mul(tmp, op1.get_num(), op2.get_den());
        addmul(tmp, op1.get_den(), op2.get_num());
        // After this line, op1/op2's num is tainted, and it cannot be used.
        rop._get_num() = std::move(tmp);
        // Ok, we are using only the dens and mul() is ok with overlapping args.
        mul(rop._get_den(), op1.get_den(), op2.get_den());
        rop.canonicalise();
    }
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
    const bool u1 = op1.get_den().is_one(), u2 = op2.get_den().is_one();
    // NOTE: it's important here to take care about overlapping arguments: we cannot use
    // rop as a "temporary" storage space, because if it overlaps with op1/op2 we will be
    // altering op1/op2 as well.
    if (u1 && u2) {
        // sub() is fine with overlapping args.
        sub(rop._get_num(), op1.get_num(), op2.get_num());
    } else if (u1) {
        integer<SSize> tmp{op2.get_num()};
        tmp.neg();
        // Ok, tmp is a separate variable, won't modify ops.
        addmul(tmp, op1.get_num(), op2.get_den());
        // Final assignments, potential for self-assignment
        // in the second one.
        rop._get_num() = std::move(tmp);
        rop._get_den() = op2.get_den();
        // NOTE: gcd(a+m*b,b) == gcd(a,b) for every integer m, no need to canonicalise the result.
    } else if (u2) {
        integer<SSize> tmp{op1.get_num()};
        submul(tmp, op2.get_num(), op1.get_den());
        rop._get_num() = std::move(tmp);
        rop._get_den() = op1.get_den();
    } else if (op1.get_den() == op2.get_den()) {
        // sub() is fine with overlapping args.
        sub(rop._get_num(), op1.get_num(), op2.get_num());
        rop.canonicalise();
    } else {
        integer<SSize> tmp;
        // These two steps are ok, tmp is a separate variable.
        // NOTE: these implement a*d+b*c. We might have a primitive operation
        // for that down the line.
        mul(tmp, op1.get_num(), op2.get_den());
        submul(tmp, op1.get_den(), op2.get_num());
        // After this line, op1/op2's num is tainted, and it cannot be used.
        rop._get_num() = std::move(tmp);
        // Ok, we are using only the dens and mul() is ok with overlapping args.
        mul(rop._get_den(), op1.get_den(), op2.get_den());
        rop.canonicalise();
    }
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
    return os << q.to_string(10);
}

/** @} */
}

#endif
