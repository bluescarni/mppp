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
/// Constructor from numerator and denominator.
/**
 * \rststar
 * This constructor is enabled only if both ``T`` and ``U`` satisfy the
 * :cpp:concept:`~mppp::RationalIntegralInteroperable` concept. The input value ``n`` will be used to initialise the
 * numerator, while ``d`` will be used to initialise the denominator. The constructor will call
 * :cpp:func:`~mppp::rational::canonicalise()` after the construction of numerator and denominator.
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
#if defined(MPPP_HAVE_CONCEPTS)
    explicit rational(const RationalInteroperable<SSize> &x)
#else
    template <typename T, rational_interoperable_enabler<T, SSize> = 0>
    explicit rational(const T &x)
#endif
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
    rational &operator=(const ::mpq_t q)
    {
        m_num = mpq_numref(q);
        m_den = mpq_denref(q);
        return *this;
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
