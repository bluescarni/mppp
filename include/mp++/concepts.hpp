// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
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
// NOTE: mppp::is_integral, for consistency with std::is_integral, will be true also for cv qualified
// integral types. is_supported_integral, however, is used in contexts where the cv qualifications
// matter, and we want this type trait not to be satisfied by integral types which are, e.g.,
// const qualified.
template <typename T>
using is_supported_integral = conjunction<is_integral<T>, std::is_same<remove_cv_t<T>, T>>;

// Type trait to check if T is a supported floating-point type.
template <typename T>
struct is_supported_float : conjunction<std::is_floating_point<T>, std::is_same<remove_cv_t<T>, T>> {
};

#if !defined(MPPP_WITH_MPFR)

// Without MPFR, we don't support long double.
template <>
struct is_supported_float<long double> : std::false_type {
};

#endif

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
