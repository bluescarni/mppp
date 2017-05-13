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
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <utility>

#include <mp++/concepts.hpp>
#include <mp++/config.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/exceptions.hpp>
#include <mp++/integer.hpp>

namespace mppp
{

template <typename T, typename U, std::size_t SSize>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool RationalNumDenCtorTypes
    = (std::is_same<T, integer<SSize>>::value || (CppInteroperable<T> && std::is_integral<T>::value))
      && (std::is_same<U, integer<SSize>>::value || (CppInteroperable<U> && std::is_integral<U>::value));
#else
using rational_num_den_ctor_types_enabler
    = enable_if_t<conjunction<disjunction<std::is_same<T, integer<SSize>>,
                                          conjunction<is_cpp_interoperable<T>, std::is_integral<T>>>,
                              disjunction<std::is_same<U, integer<SSize>>,
                                          conjunction<is_cpp_interoperable<U>, std::is_integral<U>>>>::value,
                  int>;
#endif

template <typename T, std::size_t SSize>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool RationalInteroperable = CppInteroperable<T> || std::is_same<T, integer<SSize>>::value;
#else
using rational_interoperable_enabler
    = enable_if_t<disjunction<is_cpp_interoperable<T>, std::is_same<T, integer<SSize>>>::value, int>;
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
/// Constructor from numerator and denominator.
/**
 * \rststar
 * This constructor is enabled only if ``T`` and ``U`` satisfy the :cpp:concept:`~mppp::RationalNumDenCtorTypes`
 * concept. The input value ``n`` will be used to initialise the numerator, while ``d`` will be used to
 * initialise the denominator. The constructor will call :cpp:func:`~mppp::rational::canonicalise()` after
 * the construction of numerator and denominator.
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
    requires RationalNumDenCtorTypes<T, U, SSize>
#endif
#else
    template <typename T, typename U, rational_num_den_ctor_types_enabler<T, U, SSize> = 0>
#endif
        explicit rational(T &&n, U &&d) : m_num(std::forward<T>(n)), m_den(std::forward<U>(d))
    {
        if (mppp_unlikely(m_den.is_zero())) {
            throw zero_division_error("Cannot create a rational with zero as denominator");
        }
        canonicalise();
    }

private:
    template <typename T, enable_if_t<disjunction<std::is_integral<T>, std::is_same<T, int_t>>::value, int> = 0>
    void dispatch_generic_construction(const T &n)
    {
        m_num = n;
        fast_set_den_one();
    }
    template <typename T, enable_if_t<disjunction<std::is_same<float, T>, std::is_same<double, T>>::value, int> = 0>
    void dispatch_generic_construction(T x)
    {
        MPPP_MAYBE_TLS mpq_raii q;
        ::mpq_set_d(&q.m_mpq, static_cast<double>(x));
        m_num = mpq_numref(&q.m_mpq);
        m_den = mpq_denref(&q.m_mpq);
    }

public:
    /// Generic constructor.
    explicit rational(const RationalInteroperable<SSize> &x)
    {
        dispatch_generic_construction(x);
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
    const int_t &_get_num() const
    {
        return m_num;
    }
    const int_t &_get_den() const
    {
        return m_den;
    }
    int_t &_get_num()
    {
        return m_num;
    }
    int_t &_get_den()
    {
        return m_den;
    }
    /// Canonicalise.
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

template <std::size_t SSize>
inline std::ostream &operator<<(std::ostream &os, const rational<SSize> &q)
{
    if (q._get_den().is_one()) {
        return os << q._get_num();
    }
    return os << q._get_num() << "/" << q._get_den();
}
}

#endif
