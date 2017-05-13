// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_RATIONAL_HPP
#define MPPP_RATIONAL_HPP

#include <cstddef>
#include <iostream>
#include <type_traits>
#include <utility>

#include <mp++/concepts.hpp>
#include <mp++/config.hpp>
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

/// Multiprecision integer class.
/**
 * \rststar
 * *#include <mp++/rational.hpp>*
 *
 * .. versionadded:: 0.3
 *
 * This class represent arbitrary-precision signed integers. It acts as a wrapper around the GMP ``mpz_t`` type,
 * with
 * a small value optimisation: integers whose size is up to ``SSize`` limbs are stored directly in the storage
 * occupied by the :cpp:class:`~mppp::integer` object, without resorting to dynamic memory allocation. The value
 * of
 * ``SSize`` must be at least 1 and less than an implementation-defined upper limit.
 * \endrststar
 */
template <std::size_t SSize>
class rational
{
public:
/// Underlying integral type.
/**
* This is the type used for the representation of numerator and
* denominator.
*/
#if defined(MPPP_DOXYGEN_INVOKED)
    typedef integer<SSize> int_t;
#else
    using int_t = integer<SSize>;
#endif
    /// Default constructor.
    /**
     * The default constructor will initialize ``this`` to 1 (represented as 0/1).
     */
    rational()
    {
        // Go in and set den to 1 in a fast manner.
        m_den._get_union().g_st()._mp_size = 1;
        // No need to use the mask for just 1.
        m_den._get_union().g_st().m_limbs[0] = 1;
    }
    /// Defaulted copy constructor.
    rational(const rational &) = default;
    /// Defaulted move constructor.
    rational(rational &&) = default;
/// Constructor from numerator and denominator.
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
    void canonicalise()
    {
        if (m_num.is_zero()) {
            m_den = 1;
            return;
        }
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
