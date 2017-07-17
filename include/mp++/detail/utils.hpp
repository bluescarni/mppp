// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_UTILS_HPP
#define MPPP_DETAIL_UTILS_HPP

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

// Compute a power of 2 value of the signed integer type T that can be safely negated.
template <typename T>
constexpr T safe_abs(T cur_p = T(1), T cur_n = T(-1))
{
    static_assert(std::is_integral<T>::value && std::is_signed<T>::value, "Invalid type.");
    return (cur_p > std::numeric_limits<T>::max() / T(2) || cur_n < std::numeric_limits<T>::min() / T(2))
               ? cur_p
               : safe_abs(static_cast<T>(cur_p * 2), static_cast<T>(cur_n * 2));
}
}
}

#endif
