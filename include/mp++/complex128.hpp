// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_COMPLEX128_HPP
#define MPPP_COMPLEX128_HPP

#include <mp++/config.hpp>

#if defined(MPPP_WITH_QUADMATH)

#include <ostream>
#include <type_traits>

#include <mp++/concepts.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/visibility.hpp>
#include <mp++/real128.hpp>

namespace mppp
{

// NOTE: lifted from quadmath.h, so that we can avoid
// including the header.
// NOTE: the definition changed in GCC 8, not sure
// about the consequences for clang.
#if defined(__GNUC__) && __GNUC__ >= 8

// Define the complex type corresponding to __float128
// ("_Complex __float128" is not allowed).
#if (!defined(_ARCH_PPC)) || defined(__LONG_DOUBLE_IEEE128__)
typedef _Complex float __attribute__((mode(TC))) cplex128;
#else
typedef _Complex float __attribute__((mode(KC))) cplex128;
#endif

#else

typedef _Complex float __attribute__((mode(TC))) cplex128;

#endif

namespace detail
{

template <typename T>
using is_complex128_literal_interoperable = disjunction<is_real128_cpp_interoperable<T>, std::is_same<T, real128>>;

} // namespace detail

template <typename T>
#if defined(MPPP_HAVE_CONCEPTS)
MPPP_CONCEPT_DECL Complex128LiteralInteroperable = detail::is_complex128_literal_interoperable<T>::value;
#else
using complex128_literal_interoperable_enabler
    = detail::enable_if_t<detail::is_complex128_literal_interoperable<T>::value, int>;
#endif

class MPPP_DLL_PUBLIC complex128
{
public:
    cplex128 m_value;

    // Default constructor.
    constexpr complex128() : m_value{0} {}
    // Trivial copy constructor.
    constexpr complex128(const complex128 &other) = default;
    // Trivial move constructor.
    constexpr complex128(complex128 &&other) = default;

    // Constructor from __complex128.
    constexpr explicit complex128(cplex128 x) : m_value{x} {}

    // Constructor from a literal type.
#if defined(MPPP_HAVE_CONCEPTS)
    template <Complex128LiteralInteroperable T>
#else
    template <typename T, complex128_literal_interoperable_enabler<T> = 0>
#endif
    constexpr explicit complex128(const T &x) : m_value{static_cast<__float128>(x)}
    {
    }
    // Constructor from a pair of literal types.
#if defined(MPPP_HAVE_CONCEPTS)
    template <Complex128LiteralInteroperable T, Complex128LiteralInteroperable U>
#else
    template <typename T, typename U,
              detail::enable_if_t<detail::conjunction<detail::is_complex128_literal_interoperable<T>,
                                                      detail::is_complex128_literal_interoperable<U>>::value,
                                  int> = 0>
#endif
    constexpr explicit complex128(const T &re, const U &im)
        : m_value{static_cast<__float128>(re), static_cast<__float128>(im)}
    {
    }
    // Constructor from std::complex.
#if defined(MPPP_HAVE_CONCEPTS)
    template <CppComplex T>
#else
    template <typename T, cpp_complex_enabler<T> = 0>
#endif
    MPPP_CONSTEXPR_14 explicit complex128(const T &c)
        : m_value{static_cast<__float128>(c.real()), static_cast<__float128>(c.imag())}
    {
    }

    // Constructor from an mp++ type.
#if defined(MPPP_HAVE_CONCEPTS)
    template <Real128MpppInteroperable T>
#else
    template <typename T, real128_mppp_interoperable_enabler<T> = 0>
#endif
    explicit complex128(const T &x) : m_value{real128{x}.m_value}
    {
    }
    // Constructor from a pair of mp++ types.
#if defined(MPPP_HAVE_CONCEPTS)
    template <Real128MpppInteroperable T, Real128MpppInteroperable U>
#else
    template <typename T, typename U,
              detail::enable_if_t<detail::conjunction<detail::is_real128_mppp_interoperable<T>,
                                                      detail::is_real128_mppp_interoperable<U>>::value,
                                  int> = 0>
#endif
    explicit complex128(const T &re, const U &im) : m_value{real128{re}.m_value, real128{im}.m_value}
    {
    }

    // Getters for real/imaginary parts.
    constexpr real128 creal() const
    {
        return real128{__real__ m_value};
    }
    constexpr real128 cimag() const
    {
        return real128{__imag__ m_value};
    }
};

constexpr real128 creal(const complex128 &c)
{
    return c.creal();
}

constexpr real128 cimag(const complex128 &c)
{
    return c.cimag();
}

MPPP_DLL_PUBLIC std::ostream &operator<<(std::ostream &, const complex128 &);

} // namespace mppp

#else

#error The complex128.hpp header was included but mp++ was not configured with the MPPP_WITH_QUADMATH option.

#endif

#endif
