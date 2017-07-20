// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_UTILS_HPP
#define MPPP_DETAIL_UTILS_HPP

#include <limits>
#include <string>
#include <type_traits>

#include <mp++/detail/type_traits.hpp>

namespace mppp
{

inline namespace detail
{

// These are overloads useful to treat in a generic way mppp classes and standard numeric types.

// Sign function for integral types.
template <typename T, enable_if_t<std::is_integral<T>::value, int> = 0>
inline int sgn(const T &n)
{
    return n ? (n > T(0) ? 1 : -1) : 0;
}

// Zero detection for integral types.
template <typename T, enable_if_t<std::is_integral<T>::value, int> = 0>
inline bool is_zero(const T &n)
{
    return n == T(0);
}

// Generic string conversion utility - will use std::to_string() for arithmetic types,
// x.to_string() otherwise.
template <typename T, enable_if_t<std::is_arithmetic<T>::value, int> = 0>
inline std::string to_string(const T &x)
{
    return std::to_string(x);
}

template <typename T, enable_if_t<!std::is_arithmetic<T>::value, int> = 0>
inline std::string to_string(const T &x)
{
    return x.to_string();
}

#if defined(_MSC_VER)

// Disable some warnings for MSVC.
#pragma warning(push)
#pragma warning(disable : 4146)

#endif

// Compute the absolute value of a negative integer, returning the result as an instance
// of the corresponding unsigned type. Requires T to be a signed integral type and n
// to be negative.
// NOTE: here we are using cast to unsigned + unary minus to extract the abs value of a signed negative
// integral. See:
// http://stackoverflow.com/questions/4536095/unary-minus-and-signed-to-unsigned-conversion
// This technique is not 100% portable: it requires an implementation
// of signed integers such that the absolute value of the minimum (negative) value is not greater than
// the maximum value of the unsigned counterpart. This is guaranteed on all computer architectures in use today,
// but in theory there could be architectures where the assumption is not respected. See for instance the
// discussion here:
// http://stackoverflow.com/questions/11372044/unsigned-vs-signed-range-guarantees
// Note that in any case we never run into UB, the only consequence is that for very large negative values
// we could init the integer with the wrong value, and we should be able to detect this in the unit tests.
// Let's keep this in mind in the remote case this ever becomes a problem.
template <typename T>
constexpr typename std::make_unsigned<T>::type nint_abs(T n)
{
    // NOTE: we should assert about negative n, but this is guaranteed to work properly only
    // from C++17:
    // https://stackoverflow.com/questions/26072709/alternative-to-asserts-for-constexpr-functions
    static_assert(std::is_integral<T>::value && std::is_signed<T>::value,
                  "The sint_abs() function can be used only with signed integral types.");
    using uT = typename std::make_unsigned<T>::type;
    // NOTE: the potential cast to "unsigned", rather than uT, is for when uT is a short integral type.
    // In such a case, the unary minus will trigger integral promotion to int/unsigned
    // int, and I am *not* 100% sure in this case the technique still works. Written like this, the cast
    // is never to a type narrower than "unsigned".
    return static_cast<uT>(
        -static_cast<typename std::conditional<(std::numeric_limits<uT>::max() <= std::numeric_limits<unsigned>::max()),
                                               unsigned, uT>::type>(n));
}

#if defined(_MSC_VER)

#pragma warning(pop)

#endif

// constexpr max/min implementations with copy semantics.
template <typename T>
constexpr T c_max(T a, T b)
{
    return a > b ? a : b;
}

template <typename T>
constexpr T c_min(T a, T b)
{
    return a < b ? a : b;
}
}
}

#endif
