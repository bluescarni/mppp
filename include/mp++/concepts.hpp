// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_CONCEPTS_HPP
#define MPPP_CONCEPTS_HPP

#include <mp++/config.hpp>

#include <cstddef>
#include <string>
#if MPPP_CPLUSPLUS >= 201703L
#include <string_view>
#endif
#include <type_traits>

#include <mp++/detail/type_traits.hpp>

namespace mppp
{

inline namespace detail
{

// Type trait to check if T is a supported integral type.
template <typename T>
using is_supported_integral
    = disjunction<std::is_same<T, bool>, std::is_same<T, char>, std::is_same<T, signed char>,
                  std::is_same<T, unsigned char>, std::is_same<T, short>, std::is_same<T, unsigned short>,
                  std::is_same<T, int>, std::is_same<T, unsigned>, std::is_same<T, long>,
                  std::is_same<T, unsigned long>, std::is_same<T, long long>, std::is_same<T, unsigned long long>
#if defined(MPPP_HAVE_GCC_INT128)
                  ,
                  std::is_same<T, __int128_t>, std::is_same<T, __uint128_t>
#endif
                  >;

// Type trait to check if T is a supported floating-point type.
template <typename T>
using is_supported_float = disjunction<std::is_same<T, float>, std::is_same<T, double>
#if defined(MPPP_WITH_MPFR)
                                       ,
                                       std::is_same<T, long double>
#endif
                                       >;

template <typename T>
using is_cpp_interoperable = disjunction<is_supported_integral<T>, is_supported_float<T>>;
}

template <typename T>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool CppInteroperable = is_cpp_interoperable<T>::value;
#else
using cpp_interoperable_enabler = enable_if_t<is_cpp_interoperable<T>::value, int>;
#endif

inline namespace detail
{

template <typename T>
using is_string_type = disjunction<std::is_same<T, std::string>, std::is_same<T, const char *>, std::is_same<T, char *>,
                                   conjunction<std::is_array<T>, std::is_same<remove_extent_t<T>, char>>
#if MPPP_CPLUSPLUS >= 201703L
                                   ,
                                   std::is_same<T, std::string_view>
#endif
                                   >;
}

template <typename T>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool StringType = is_string_type<T>::value;
#else
using string_type_enabler = enable_if_t<is_string_type<T>::value, int>;
#endif

template <typename T>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool CppIntegralInteroperable = is_supported_integral<T>::value;
#else
using cpp_integral_interoperable_enabler = enable_if_t<is_supported_integral<T>::value, int>;
#endif
}

#endif
