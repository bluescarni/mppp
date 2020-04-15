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
#include <cstddef>
#include <ostream>
#include <string>
#include <type_traits>

#if defined(MPPP_HAVE_STRING_VIEW)

#include <string_view>

#endif

#include <mp++/concepts.hpp>
#include <mp++/detail/fwd_decl.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/visibility.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real128.hpp>
#include <mp++/type_name.hpp>

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

namespace detail
{

// For internal use only.
template <typename T>
using is_complex128_mppp_interoperable = disjunction<is_real128_mppp_interoperable<T>
#if defined(MPPP_WITH_MPFR)
                                                     ,
                                                     std::is_same<T, real>
#endif
                                                     >;

} // namespace detail

// Detect real-valued interoperable types.
template <typename T>
using is_complex128_interoperable = detail::disjunction<is_real128_interoperable<T>, std::is_same<T, real128>
#if defined(MPPP_WITH_MPFR)
                                                        ,
                                                        std::is_same<T, real>
#endif
                                                        >;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL complex128_interoperable = is_complex128_interoperable<T>::value;

#endif

constexpr complex128 conj(const complex128 &);

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
    // NOTE: early GCC versions seem to exhibit constexpr
    // failures if c is passed by const reference.
    constexpr explicit complex128(
#if defined(__GNUC__) && __GNUC__ < 5
        cplex128 c
#else
        const cplex128 &c
#endif
        )
        : m_value{c}
    {
    }

private:
    // Helpers to cast to __float128.
    // real128 and C++ types.
    template <typename T>
    static constexpr __float128 cast_to_f128(const T &x, std::false_type)
    {
        return static_cast<__float128>(x);
    }
    // Integer, rational and real.
    template <typename T>
    static __float128 cast_to_f128(const T &x, std::true_type)
    {
        return static_cast<real128>(x).m_value;
    }

public:
    // Generic ctor.
#if defined(MPPP_HAVE_CONCEPTS)
    template <complex128_interoperable T>
#else
    template <typename T, detail::enable_if_t<is_complex128_interoperable<T>::value, int> = 0>
#endif
    constexpr explicit complex128(const T &x) : m_value{cast_to_f128(x, detail::is_complex128_mppp_interoperable<T>{})}
    {
    }
    // Binary generic ctor.
#if defined(MPPP_HAVE_CONCEPTS)
    template <complex128_interoperable T, complex128_interoperable U>
#else
    template <typename T, typename U,
              detail::enable_if_t<
                  detail::conjunction<is_complex128_interoperable<T>, is_complex128_interoperable<U>>::value, int> = 0>
#endif
    constexpr explicit complex128(const T &re, const U &im)
        : m_value{cast_to_f128(re, detail::is_complex128_mppp_interoperable<T>{}),
                  cast_to_f128(im, detail::is_complex128_mppp_interoperable<U>{})}
    {
    }
    // Constructor from std::complex.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real128_cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_real128_cpp_complex<T>::value, int> = 0>
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
    template <string_type T>
#else
    template <typename T, detail::enable_if_t<is_string_type<T>::value, int> = 0>
#endif
    explicit complex128(const T &s) : complex128(ptag{}, s)
    {
    }
    // Constructor from range of characters.
    explicit complex128(const char *, const char *);

    // Trivial copy assignment operator.
    complex128 &operator=(const complex128 &) = default;
    // Trivial move assignment operator.
    complex128 &operator=(complex128 &&) = default;

    // Assignment from a __complex128.
    MPPP_CONSTEXPR_14 complex128 &operator=(const cplex128 &c)
    {
        m_value = c;
        return *this;
    }

    // Assignment from real-valued interoperable types.
#if defined(MPPP_HAVE_CONCEPTS)
    template <complex128_interoperable T>
#else
    template <typename T, detail::enable_if_t<is_complex128_interoperable<T>::value, int> = 0>
#endif
    MPPP_CONSTEXPR_14 complex128 &operator=(const T &x)
    {
        return *this = complex128{x};
    }

    // Assignment from std::complex.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real128_cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_real128_cpp_complex<T>::value, int> = 0>
#endif
    MPPP_CONSTEXPR_14 complex128 &operator=(const T &c)
    {
        return *this = complex128{c};
    }

    // Assignment from string.
#if defined(MPPP_HAVE_CONCEPTS)
    template <string_type T>
#else
    template <typename T, detail::enable_if_t<is_string_type<T>::value, int> = 0>
#endif
    complex128 &operator=(const T &s)
    {
        return *this = complex128{s};
    }

    // Getters for the real/imaginary parts.
    constexpr real128 real() const
    {
        return real128{__real__ m_value};
    }
    constexpr real128 imag() const
    {
        return real128{__imag__ m_value};
    }

    // Setters for the real/imaginary parts.
    MPPP_CONSTEXPR_14 complex128 &set_real(const real128 &re)
    {
        return set_real(re.m_value);
    }
    MPPP_CONSTEXPR_14 complex128 &set_imag(const real128 &im)
    {
        return set_imag(im.m_value);
    }

private:
    // Private setters in terms of __float128.
    MPPP_CONSTEXPR_14 complex128 &set_real(const __float128 &re)
    {
        // NOTE: use this idiom, instead of setting
        // directly __real__ m_value, because
        // GCC does not allow constexpr setting of
        // real/imaginary parts. Luckily, this idiom
        // produces identically-efficient code:
        // https://godbolt.org/z/Pjsx8h
        // NOTE: because here we are reading
        // from m_value when fetching the current imaginary
        // part, we CANNOT use the setters on
        // uninitialised complex128 objects! (e.g., during
        // construction).
        m_value = cplex128{re, __imag__ m_value};

        return *this;
    }
    MPPP_CONSTEXPR_14 complex128 &set_imag(const __float128 &im)
    {
        m_value = cplex128{__real__ m_value, im};

        return *this;
    }

public:
    // Conversion operator to __complex128.
    constexpr explicit operator cplex128() const
    {
        return m_value;
    }

    // Conversion operator to real-valued interoperable types.
#if defined(MPPP_HAVE_CONCEPTS)
    template <complex128_interoperable T>
#else
    template <typename T, detail::enable_if_t<is_complex128_interoperable<T>::value, int> = 0>
#endif
    constexpr explicit operator T() const
    {
        return imag() == 0 ? static_cast<T>(real())
                           : throw std::domain_error("Cannot convert a complex128 with a nonzero imaginary part of "
                                                     + imag().to_string() + " to the real-valued type '"
                                                     + type_name<T>() + "'");
    }

    // Conversion to complex C++ types.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real128_cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_real128_cpp_complex<T>::value, int> = 0>
#endif
    MPPP_CONSTEXPR_14 explicit operator T() const
    {
        using value_t = typename T::value_type;

        return T{static_cast<value_t>(real()), static_cast<value_t>(imag())};
    }

private:
    // get() implementation for real128.
    static MPPP_CONSTEXPR_14 bool get_impl(real128 &rop, const real128 &r)
    {
        rop = r;
        return true;
    }
#if defined(MPPP_WITH_MPFR)
    // get() implementation for real.
    static bool get_impl(mppp::real &, const real128 &);
#endif
    // get() implementation for fundamental C++ types, integer
    // and rational.
    template <typename T>
    static MPPP_CONSTEXPR_14 bool get_impl(T &rop, const real128 &r)
    {
        return r.get(rop);
    }

public:
    // Conversion member function to real-valued interoperable types.
#if defined(MPPP_HAVE_CONCEPTS)
    template <complex128_interoperable T>
#else
    template <typename T, detail::enable_if_t<is_complex128_interoperable<T>::value, int> = 0>
#endif
    MPPP_CONSTEXPR_14 bool get(T &rop) const
    {
        if (imag() == 0) {
            return get_impl(rop, real());
        } else {
            return false;
        }
    }
    // Conversion member function to complex C++ types.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real128_cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_real128_cpp_complex<T>::value, int> = 0>
#endif
    MPPP_CONSTEXPR_20 bool get(T &rop) const
    {
        using value_type = typename T::value_type;

        // NOTE: constexpr mutation of a std::complex
        // seems to be available only since C++20:
        // https://en.cppreference.com/w/cpp/numeric/complex/real
        // https://en.cppreference.com/w/cpp/numeric/complex/operator%3D
        rop.real(static_cast<value_type>(real()));
        rop.imag(static_cast<value_type>(imag()));

        return true;
    }

    // Conversion to string.
    std::string to_string() const;

    // Complex absolute value.
    complex128 &abs();
    // Complex argument.
    complex128 &arg();
    // Complex conjugate.
    MPPP_CONSTEXPR_14 complex128 &conj()
    {
        return *this = mppp::conj(*this);
    }
    // Project into Riemann sphere.
    complex128 &proj();

    // Sqrt.
    complex128 &sqrt();

    // Trigonometric functions.
    complex128 &sin();
    complex128 &cos();
    complex128 &tan();

    // Inverse trigonometric functions.
    complex128 &asin();
    complex128 &acos();
    complex128 &atan();

    // Hyperbolic functions.
    complex128 &sinh();
    complex128 &cosh();
    complex128 &tanh();

    // Inverse hyperbolic functions.
    complex128 &asinh();
    complex128 &acosh();
    complex128 &atanh();

    // Exponentials and logarithms.
    complex128 &exp();
    complex128 &log();
    complex128 &log10();
};

// Getters for real/imaginary parts.
constexpr real128 creal(const complex128 &c)
{
    return c.real();
}

constexpr real128 cimag(const complex128 &c)
{
    return c.imag();
}

// Setters for real/imaginary parts.
inline MPPP_CONSTEXPR_14 complex128 &set_real(complex128 &c, const real128 &re)
{
    return c.set_real(re);
}

inline MPPP_CONSTEXPR_14 complex128 &set_imag(complex128 &c, const real128 &im)
{
    return c.set_imag(im);
}

// Conversion function.
template <typename T>
inline MPPP_CONSTEXPR_14 auto get(T &rop, const complex128 &c) -> decltype(c.get(rop))
{
    return c.get(rop);
}

// Absolute value.
MPPP_DLL_PUBLIC complex128 abs(const complex128 &);
// Complex argument.
MPPP_DLL_PUBLIC complex128 arg(const complex128 &);
// Project into Riemann sphere.
MPPP_DLL_PUBLIC complex128 proj(const complex128 &);
// Complex conjugate.
constexpr complex128 conj(const complex128 &c)
{
    return complex128{c.real(), -c.imag()};
}

// Square root.
MPPP_DLL_PUBLIC complex128 sqrt(const complex128 &);

// Trigonometric functions.
MPPP_DLL_PUBLIC complex128 sin(const complex128 &);
MPPP_DLL_PUBLIC complex128 cos(const complex128 &);
MPPP_DLL_PUBLIC complex128 tan(const complex128 &);

// Inverse trigonometric functions.
MPPP_DLL_PUBLIC complex128 asin(const complex128 &);
MPPP_DLL_PUBLIC complex128 acos(const complex128 &);
MPPP_DLL_PUBLIC complex128 atan(const complex128 &);

// Hyperbolic functions.
MPPP_DLL_PUBLIC complex128 sinh(const complex128 &);
MPPP_DLL_PUBLIC complex128 cosh(const complex128 &);
MPPP_DLL_PUBLIC complex128 tanh(const complex128 &);

// Inverse hyperbolic functions.
MPPP_DLL_PUBLIC complex128 asinh(const complex128 &);
MPPP_DLL_PUBLIC complex128 acosh(const complex128 &);
MPPP_DLL_PUBLIC complex128 atanh(const complex128 &);

// Exponentials and logarithms.
MPPP_DLL_PUBLIC complex128 exp(const complex128 &);
MPPP_DLL_PUBLIC complex128 log(const complex128 &);
MPPP_DLL_PUBLIC complex128 log10(const complex128 &);

namespace detail
{

// The other type in binary operations when one
// argument is complex128 and the other isn't.
template <typename T>
using is_complex128_op_other_type
    = disjunction<is_real128_interoperable<T>, std::is_same<T, real128>, is_real128_cpp_complex<T>>;

} // namespace detail

// Detect types for use in mathematical operators
// involving complex128.
template <typename T, typename U>
using are_complex128_op_types
    = detail::disjunction<detail::conjunction<std::is_same<T, complex128>, std::is_same<U, complex128>>,
                          detail::conjunction<std::is_same<T, complex128>, detail::is_complex128_op_other_type<U>>,
                          detail::conjunction<std::is_same<U, complex128>, detail::is_complex128_op_other_type<T>>,
                          detail::conjunction<std::is_same<real128, T>, is_real128_cpp_complex<U>>,
                          detail::conjunction<std::is_same<real128, U>, is_real128_cpp_complex<T>>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, typename U>
MPPP_CONCEPT_DECL complex128_op_types = are_complex128_op_types<T, U>::value;

#endif

namespace detail
{

MPPP_DLL_PUBLIC complex128 pow_impl(const complex128 &, const complex128 &);

template <typename T>
inline complex128 pow_impl(const complex128 &x, const T &y)
{
    return pow_impl(x, static_cast<complex128>(y));
}

template <typename T>
inline complex128 pow_impl(const T &x, const complex128 &y)
{
    return pow_impl(static_cast<complex128>(x), y);
}

template <typename T>
inline complex128 pow_impl(const real128 &x, const std::complex<T> &y)
{
    return pow_impl(static_cast<complex128>(x), static_cast<complex128>(y));
}

template <typename T>
inline complex128 pow_impl(const std::complex<T> &x, const real128 &y)
{
    return pow_impl(static_cast<complex128>(x), static_cast<complex128>(y));
}

} // namespace detail

#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires complex128_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_complex128_op_types<T, U>::value, int> = 0>
#endif
    inline complex128 pow(const T &x, const U &y)
{
    return detail::pow_impl(x, y);
}

// Streaming operator.
MPPP_DLL_PUBLIC std::ostream &operator<<(std::ostream &, const complex128 &);

// Identity operator.
constexpr complex128 operator+(const complex128 &c)
{
    return c;
}

// Prefix increment.
inline MPPP_CONSTEXPR_14 complex128 &operator++(complex128 &x)
{
    x.m_value += 1;
    return x;
}

// Suffix increment.
inline MPPP_CONSTEXPR_14 complex128 operator++(complex128 &x, int)
{
    auto retval(x);
    ++x;
    return retval;
}

namespace detail
{

constexpr complex128 binary_add_impl(const complex128 &c1, const complex128 &c2)
{
    return complex128{c1.m_value + c2.m_value};
}

constexpr complex128 binary_add_impl(const complex128 &c, const real128 &x)
{
    return complex128{c.m_value + x.m_value};
}

constexpr complex128 binary_add_impl(const real128 &x, const complex128 &c)
{
    return complex128{x.m_value + c.m_value};
}

// NOTE: this covers all real128_interoperable types.
template <typename T>
constexpr complex128 binary_add_impl(const complex128 &c, const T &x)
{
    return binary_add_impl(c, static_cast<real128>(x));
}

template <typename T>
constexpr complex128 binary_add_impl(const T &x, const complex128 &c)
{
    return binary_add_impl(static_cast<real128>(x), c);
}

template <typename T>
inline MPPP_CONSTEXPR_14 complex128 binary_add_impl(const complex128 &c1, const std::complex<T> &c2)
{
    return binary_add_impl(c1, static_cast<complex128>(c2));
}

template <typename T>
inline MPPP_CONSTEXPR_14 complex128 binary_add_impl(const std::complex<T> &c1, const complex128 &c2)
{
    return binary_add_impl(static_cast<complex128>(c1), c2);
}

template <typename T>
inline MPPP_CONSTEXPR_14 complex128 binary_add_impl(const real128 &x, const std::complex<T> &c)
{
    return binary_add_impl(x, static_cast<complex128>(c));
}

template <typename T>
inline MPPP_CONSTEXPR_14 complex128 binary_add_impl(const std::complex<T> &c, const real128 &x)
{
    return binary_add_impl(static_cast<complex128>(c), x);
}

} // namespace detail

// Binary plus.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires complex128_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_complex128_op_types<T, U>::value, int> = 0>
#endif
    constexpr complex128 operator+(const T &x, const U &y)
{
    return detail::binary_add_impl(x, y);
}

// Negation operator.
constexpr complex128 operator-(const complex128 &c)
{
    return complex128{-c.m_value};
}

// Prefix decrement.
inline MPPP_CONSTEXPR_14 complex128 &operator--(complex128 &x)
{
    x.m_value -= 1;
    return x;
}

// Suffix decrement.
inline MPPP_CONSTEXPR_14 complex128 operator--(complex128 &x, int)
{
    auto retval(x);
    --x;
    return retval;
}

// Detect types for use in the comparison operators
// involving complex128.
template <typename T, typename U>
using are_complex128_cmp_op_types
    = detail::disjunction<detail::conjunction<std::is_same<T, complex128>, std::is_same<U, complex128>>,
                          detail::conjunction<std::is_same<T, complex128>, is_complex128_interoperable<U>>,
                          detail::conjunction<std::is_same<U, complex128>, is_complex128_interoperable<T>>,
                          detail::conjunction<std::is_same<T, complex128>, is_real128_cpp_complex<U>>,
                          detail::conjunction<std::is_same<U, complex128>, is_real128_cpp_complex<T>>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, typename U>
MPPP_CONCEPT_DECL complex128_cmp_op_types = are_complex128_cmp_op_types<T, U>::value;

#endif

namespace detail
{

// complex128-complex128.
constexpr bool dispatch_eq(const complex128 &c1, const complex128 &c2)
{
    return c1.m_value == c2.m_value;
}

// complex128-real128.
constexpr bool dispatch_eq(const complex128 &c, const real128 &x)
{
    return c.m_value == x.m_value;
}

constexpr bool dispatch_eq(const real128 &x, const complex128 &c)
{
    return dispatch_eq(c, x);
}

// complex128-mppp.
template <typename T, enable_if_t<is_complex128_mppp_interoperable<T>::value, int> = 0>
inline bool dispatch_eq(const complex128 &x, const T &y)
{
    return x.imag() == 0 && x.real() == y;
}

template <typename T, enable_if_t<is_complex128_mppp_interoperable<T>::value, int> = 0>
inline bool dispatch_eq(const T &y, const complex128 &x)
{
    return dispatch_eq(x, y);
}

// complex128-C++.
template <typename T, enable_if_t<is_cpp_arithmetic<T>::value, int> = 0>
constexpr bool dispatch_eq(const complex128 &c, const T &x)
{
    return c.m_value == x;
}

template <typename T, enable_if_t<is_cpp_arithmetic<T>::value, int> = 0>
constexpr bool dispatch_eq(const T &x, const complex128 &c)
{
    return dispatch_eq(c, x);
}

// complex128-C++ complex.
template <typename T>
inline MPPP_CONSTEXPR_14 bool dispatch_eq(const complex128 &c1, const std::complex<T> &c2)
{
    return c1.real() == c2.real() && c1.imag() == c2.imag();
}

template <typename T>
inline MPPP_CONSTEXPR_14 bool dispatch_eq(const std::complex<T> &c2, const complex128 &c1)
{
    return dispatch_eq(c1, c2);
}

} // namespace detail

#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires complex128_cmp_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_complex128_cmp_op_types<T, U>::value, int> = 0>
#endif
    constexpr bool operator==(const T &x, const U &y)
{
    return detail::dispatch_eq(x, y);
}

#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires complex128_cmp_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_complex128_cmp_op_types<T, U>::value, int> = 0>
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
