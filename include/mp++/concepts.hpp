// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_CONCEPTS_HPP
#define MPPP_CONCEPTS_HPP

#include <type_traits>

#include <mp++/config.hpp>
#include <mp++/detail/type_traits.hpp>

namespace mppp
{

inline namespace detail
{

// Type trait to check if T is a supported integral type.
template <typename T>
using is_supported_integral
    = std::integral_constant<bool, disjunction<std::is_same<T, bool>, std::is_same<T, char>,
                                               std::is_same<T, signed char>, std::is_same<T, unsigned char>,
                                               std::is_same<T, short>, std::is_same<T, unsigned short>,
                                               std::is_same<T, int>, std::is_same<T, unsigned>, std::is_same<T, long>,
                                               std::is_same<T, unsigned long>, std::is_same<T, long long>,
                                               std::is_same<T, unsigned long long>>::value>;

// Type trait to check if T is a supported floating-point type.
template <typename T>
using is_supported_float = std::integral_constant<bool, disjunction<std::is_same<T, float>, std::is_same<T, double>
#if defined(MPPP_WITH_MPFR)
                                                                    ,
                                                                    std::is_same<T, long double>
#endif
                                                                    >::value>;

template <typename T>
using is_cpp_interoperable
    = std::integral_constant<bool, disjunction<is_supported_integral<T>, is_supported_float<T>>::value>;
}

template <typename T>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool CppInteroperable = is_cpp_interoperable<T>::value;
#else
using cpp_interoperable_enabler = enable_if_t<is_cpp_interoperable<T>::value, int>;
#endif
}

#endif
