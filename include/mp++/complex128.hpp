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

#include <complex>
#include <ostream>

#include <mp++/concepts.hpp>
#include <mp++/detail/visibility.hpp>
#include <mp++/real128.hpp>

namespace mppp
{

// NOTE: lifted from quadmath.h, so that we can avoid
// including the header.
// Define the complex type corresponding to __float128
// ("_Complex __float128" is not allowed).
#if (!defined(_ARCH_PPC)) || defined(__LONG_DOUBLE_IEEE128__)
typedef _Complex float __attribute__((mode(TC))) __complex128;
#else
typedef _Complex float __attribute__((mode(KC))) __complex128;
#endif

class MPPP_DLL_PUBLIC complex128
{
public:
    __complex128 m_value;

    // Default constructor.
    constexpr complex128() : m_value{0} {}
    // Trivial copy constructor.
    constexpr complex128(const complex128 &other) = default;
    // Trivial move constructor.
    constexpr complex128(complex128 &&other) = default;

    // Constructor from __complex128.
    constexpr explicit complex128(__complex128 x) : m_value{x} {}

#if defined(MPPP_HAVE_CONCEPTS)
    template <Real128CppInteroperable T>
#else
    template <typename T, real128_cpp_interoperable_enabler<T> = 0>
#endif
    constexpr explicit complex128(const T &x) : m_value{x}
    {
    }
#if defined(MPPP_HAVE_CONCEPTS)
    template <Real128CppInteroperable T, Real128CppInteroperable U>
#else
    template <typename T, typename U,
              detail::enable_if_t<detail::conjunction<detail::is_real128_cpp_interoperable<T>,
                                                      detail::is_real128_cpp_interoperable<U>>::value,
                                  int> = 0>
#endif
    constexpr explicit complex128(const T &re, const U &im)
        : m_value{static_cast<__float128>(re), static_cast<__float128>(im)}
    {
    }

public:
#if defined(MPPP_HAVE_CONCEPTS)
    template <CppComplex T>
#else
    template <typename T, cpp_complex_enabler<T> = 0>
#endif
    MPPP_CONSTEXPR_14 explicit complex128(const T &c)
        : m_value{static_cast<__float128>(c.real()), static_cast<__float128>(c.imag())}
    {
    }

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
