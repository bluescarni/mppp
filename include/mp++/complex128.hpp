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

#include <cstddef>
#include <ostream>
#include <string>
#include <type_traits>

#if defined(MPPP_HAVE_STRING_VIEW)

#include <string_view>

#endif

#include <mp++/concepts.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/visibility.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real128.hpp>

#if defined(MPPP_WITH_MPFR)

#include <mp++/real.hpp>

#endif

namespace mppp
{

// Re-define in the mppp namespace the __complex128 type
// (using another name). This allows us to avoid having
// to include quadmath.h in the public API.
// NOTE: check regularly the definition of __complex128 in GCC:
// https://github.com/gcc-mirror/gcc/blob/master/libquadmath/quadmath.h
// See also:
// https://gcc.gnu.org/onlinedocs/gcc/Floating-Types.html
#if (!defined(_ARCH_PPC)) || defined(__LONG_DOUBLE_IEEE128__)
typedef _Complex float __attribute__((mode(TC))) cplex128;
#else
typedef _Complex float __attribute__((mode(KC))) cplex128;
#endif

// TODO what about std::complex?
// Should this be a concept only for *real*
// interoperable types?
template <typename T>
using is_complex128_interoperable = detail::disjunction<is_real128_interoperable<T>, std::is_same<T, real128>
#if defined(MPPP_WITH_MPFR)
                                                        ,
                                                        std::is_same<T, real>
#endif
                                                        >;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL Complex128Interoperable = is_complex128_interoperable<T>::value;

#endif

class MPPP_DLL_PUBLIC complex128
{
public:
    cplex128 m_value;

    // Default constructor.
    constexpr complex128() : m_value{0} {}
    // Trivial copy constructor.
    constexpr complex128(const complex128 &) = default;
    // Trivial move constructor.
    constexpr complex128(complex128 &&) = default;

    // Constructor from __complex128.
    constexpr explicit complex128(cplex128 x) : m_value{x} {}

private:
    // Helpers to cast to __float128.
    // real128 and C++ types.
    template <typename T>
    static constexpr __float128 cast_to_f128(const T &x)
    {
        return static_cast<__float128>(x);
    }
    // Integer, rational and real.
    template <std::size_t SSize>
    static __float128 cast_to_f128(const integer<SSize> &n)
    {
        return static_cast<real128>(n).m_value;
    }
    template <std::size_t SSize>
    static __float128 cast_to_f128(const rational<SSize> &q)
    {
        return static_cast<real128>(q).m_value;
    }
#if defined(MPPP_WITH_MPFR)
    static __float128 cast_to_f128(const real &);
#endif

public:
    // Generic ctor.
#if defined(MPPP_HAVE_CONCEPTS)
    template <Complex128Interoperable T>
#else
    template <typename T, detail::enable_if_t<is_complex128_interoperable<T>::value, int> = 0>
#endif
    constexpr explicit complex128(const T &x) : m_value{cast_to_f128(x)}
    {
    }
    // Binary generic ctor.
#if defined(MPPP_HAVE_CONCEPTS)
    template <Complex128Interoperable T, Complex128Interoperable U>
#else
    template <typename T, typename U,
              detail::enable_if_t<
                  detail::conjunction<is_complex128_interoperable<T>, is_complex128_interoperable<U>>::value, int> = 0>
#endif
    constexpr explicit complex128(const T &re, const U &im) : m_value{cast_to_f128(re), cast_to_f128(im)}
    {
    }
    // Constructor from std::complex.
#if defined(MPPP_HAVE_CONCEPTS)
    template <CppComplex T>
#else
    template <typename T, detail::enable_if_t<is_cpp_complex<T>::value, int> = 0>
#endif
    MPPP_CONSTEXPR_14 explicit complex128(const T &c)
        : m_value{static_cast<__float128>(c.real()), static_cast<__float128>(c.imag())}
    {
    }

private:
    // A tag to call private ctors.
    struct ptag {
    };
    explicit complex128(const ptag &, const char *);
    explicit complex128(const ptag &, const std::string &);
#if defined(MPPP_HAVE_STRING_VIEW)
    explicit complex128(const ptag &, const std::string_view &);
#endif
    MPPP_DLL_LOCAL void construct_from_nts(const char *);

public:
    // Constructor from string.
#if defined(MPPP_HAVE_CONCEPTS)
    template <StringType T>
#else
    template <typename T, string_type_enabler<T> = 0>
#endif
    explicit complex128(const T &s) : complex128(ptag{}, s)
    {
    }
    // Constructor from range of characters.
    explicit complex128(const char *, const char *);

    // Getters for the real/imaginary parts.
    constexpr real128 creal() const
    {
        return real128{__real__ m_value};
    }
    constexpr real128 cimag() const
    {
        return real128{__imag__ m_value};
    }

    // Setters for real/imaginary parts.
    MPPP_CONSTEXPR_14 void set_real(const real128 &re)
    {
        __real__ m_value = re.m_value;
    }
    MPPP_CONSTEXPR_14 void set_imag(const real128 &im)
    {
        __imag__ m_value = im.m_value;
    }

    // Conversion to string.
    std::string to_string() const;
};

// Getters for real/imaginary parts.
constexpr real128 creal(const complex128 &c)
{
    return c.creal();
}

constexpr real128 cimag(const complex128 &c)
{
    return c.cimag();
}

// Setters for real/imaginary parts.
inline MPPP_CONSTEXPR_14 void set_real(complex128 &c, const real128 &re)
{
    c.set_real(re);
}

inline MPPP_CONSTEXPR_14 void set_imag(complex128 &c, const real128 &im)
{
    c.set_imag(im);
}

// Streaming operator.
MPPP_DLL_PUBLIC std::ostream &operator<<(std::ostream &, const complex128 &);

// Identity operator.
constexpr complex128 operator+(const complex128 &c)
{
    return c;
}

// Negation operator.
constexpr complex128 operator-(const complex128 &c)
{
    return complex128{-c.m_value};
}

// TODO: what about std::complex?
// TODO: this also includes mppp::real?
// TODO: split arith_op_types and cmp_op_types (i.e., for comparing to real)?
template <typename T, typename U>
using are_complex128_op_types
    = detail::disjunction<detail::conjunction<std::is_same<T, complex128>, std::is_same<U, complex128>>,
                          detail::conjunction<std::is_same<T, complex128>, is_complex128_interoperable<U>>,
                          detail::conjunction<std::is_same<U, complex128>, is_complex128_interoperable<T>>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, typename U>
MPPP_CONCEPT_DECL Complex128OpTypes = are_complex128_op_types<T, U>::value;

#endif

namespace detail
{

constexpr bool dispatch_eq(const complex128 &c1, const complex128 &c2)
{
    return c1.m_value == c2.m_value;
}

constexpr bool dispatch_eq(const complex128 &c, const real128 &x)
{
    return c.m_value == x.m_value;
}

constexpr bool dispatch_eq(const real128 &x, const complex128 &c)
{
    return dispatch_eq(c, x);
}

template <typename T>
constexpr bool dispatch_eq(const complex128 &c, const T &x)
{
    return c.m_value == x;
}

template <typename T>
constexpr bool dispatch_eq(const T &x, const complex128 &c)
{
    return dispatch_eq(c, x);
}

} // namespace detail

#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Complex128OpTypes<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_complex128_op_types<T, U>::value, int> = 0>
#endif
    constexpr bool operator==(const T &x, const U &y)
{
    return detail::dispatch_eq(x, y);
}

#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Complex128OpTypes<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_complex128_op_types<T, U>::value, int> = 0>
#endif
    constexpr bool operator!=(const T &x, const U &y)
{
    return !(x == y);
}

inline namespace literals
{

template <char... Chars>
inline complex128 operator"" _cq()
{
    return complex128{operator"" _rq<Chars...>()};
}

template <char... Chars>
constexpr complex128 operator"" _irq()
{
    return complex128{0, operator"" _rq<Chars...>()};
}

template <>
constexpr complex128 operator"" _irq<'1'>()
{
    return complex128{0, 1};
}

} // namespace literals

} // namespace mppp

#else

#error The complex128.hpp header was included but mp++ was not configured with the MPPP_WITH_QUADMATH option.

#endif

#endif
