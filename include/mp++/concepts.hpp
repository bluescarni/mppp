// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
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

// Type trait to check if T is a supported integral type.
// NOTE: mppp::detail::is_integral, for consistency with std::is_integral, will be true also for cv qualified
// integral types. is_cpp_integral_interoperable, however, is used in contexts where the cv qualifications
// matter, and we want this type trait not to be satisfied by integral types which are, e.g.,
// const qualified.
template <typename T>
using is_cpp_integral_interoperable
    = detail::conjunction<detail::is_integral<T>, std::is_same<detail::remove_cv_t<T>, T>>;

template <typename T>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool CppIntegralInteroperable = is_cpp_integral_interoperable<T>::value;
#else
using cpp_integral_interoperable_enabler = detail::enable_if_t<is_cpp_integral_interoperable<T>::value, int>;
#endif

// Type trait to check if T is a supported unsigned integral type.
template <typename T>
using is_cpp_unsigned_integral_interoperable
    = detail::conjunction<is_cpp_integral_interoperable<T>, detail::is_unsigned<T>>;

template <typename T>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool CppUnsignedIntegralInteroperable = is_cpp_unsigned_integral_interoperable<T>::value;
#else
using cpp_unsigned_integral_interoperable_enabler
    = detail::enable_if_t<is_cpp_unsigned_integral_interoperable<T>::value, int>;
#endif

// Type trait to check if T is a supported signed integral type.
template <typename T>
using is_cpp_signed_integral_interoperable
    = detail::conjunction<is_cpp_integral_interoperable<T>, detail::is_signed<T>>;

template <typename T>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool CppSignedIntegralInteroperable = is_cpp_signed_integral_interoperable<T>::value;
#else
using cpp_signed_integral_interoperable_enabler
    = detail::enable_if_t<is_cpp_signed_integral_interoperable<T>::value, int>;
#endif

// Type trait to check if T is a supported floating-point type.
template <typename T>
using is_cpp_floating_point_interoperable
    = detail::conjunction<std::is_same<detail::remove_cv_t<T>, T>, std::is_floating_point<T>
#if !defined(MPPP_WITH_MPFR)
                          ,
                          detail::negation<std::is_same<T, long double>>
#endif
                          >;

template <typename T>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool CppFloatingPointInteroperable = is_cpp_floating_point_interoperable<T>::value;
#else
using cpp_floating_point_interoperable_enabler
    = detail::enable_if_t<is_cpp_floating_point_interoperable<T>::value, int>;
#endif

template <typename T>
using is_cpp_interoperable
    = detail::disjunction<is_cpp_integral_interoperable<T>, is_cpp_floating_point_interoperable<T>>;

template <typename T>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool CppInteroperable = is_cpp_interoperable<T>::value;
#else
using cpp_interoperable_enabler = detail::enable_if_t<is_cpp_interoperable<T>::value, int>;
#endif

namespace detail
{

// NOTE: remove_pointer_t removes the top level qualifiers of the pointer as well:
// http://en.cppreference.com/w/cpp/types/remove_pointer
// After removal of pointer, we could still have a type which is cv-qualified. Thus,
// we remove cv-qualifications after pointer removal.
template <typename T>
using is_char_pointer = conjunction<std::is_pointer<T>, std::is_same<remove_cv_t<remove_pointer_t<T>>, char>>;

} // namespace detail

template <typename T>
using is_string_type
    = detail::disjunction<std::is_same<detail::remove_cv_t<T>, std::string>, detail::is_char_pointer<T>,
                          // NOTE: detail::remove_cv_t does remove cv qualifiers from arrays.
                          detail::conjunction<std::is_array<detail::remove_cv_t<T>>,
                                              std::is_same<detail::remove_extent_t<detail::remove_cv_t<T>>, char>>
#if MPPP_CPLUSPLUS >= 201703L
                          ,
                          std::is_same<detail::remove_cv_t<T>, std::string_view>
#endif
                          >;

template <typename T>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool StringType = is_string_type<T>::value;
#else
using string_type_enabler = detail::enable_if_t<is_string_type<T>::value, int>;
#endif
} // namespace mppp

#endif
