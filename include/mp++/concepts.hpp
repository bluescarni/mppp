// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_CONCEPTS_HPP
#define MPPP_CONCEPTS_HPP

#include <mp++/config.hpp>

#include <complex>
#include <cstddef>
#include <string>
#include <type_traits>

#if defined(MPPP_HAVE_STRING_VIEW)
#include <string_view>
#endif

#include <mp++/detail/type_traits.hpp>

namespace mppp
{

// Type trait to check if T is a C++ integral type, including possibly __(u)int128_t.
// NOTE: mppp::detail::is_integral, for consistency with std::is_integral, will be true also for cv qualified
// integral types. is_cpp_integral, however, is used in contexts where the cv qualifications
// matter, and we want this type trait not to be satisfied by integral types which are, e.g.,
// const qualified.
template <typename T>
using is_cpp_integral = detail::conjunction<detail::is_integral<T>, std::is_same<detail::remove_cv_t<T>, T>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL cpp_integral = is_cpp_integral<T>::value;

#endif

template <typename T>
using is_cpp_unsigned_integral = detail::conjunction<is_cpp_integral<T>, detail::is_unsigned<T>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL cpp_unsigned_integral = is_cpp_unsigned_integral<T>::value;

#endif

template <typename T>
using is_cpp_signed_integral = detail::conjunction<is_cpp_integral<T>, detail::is_signed<T>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL cpp_signed_integral = is_cpp_signed_integral<T>::value;

#endif

template <typename T>
using is_cpp_floating_point = detail::conjunction<std::is_floating_point<T>, std::is_same<detail::remove_cv_t<T>, T>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL cpp_floating_point = is_cpp_floating_point<T>::value;

#endif

template <typename T>
using is_cpp_arithmetic = detail::disjunction<is_cpp_integral<T>, is_cpp_floating_point<T>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL cpp_arithmetic = is_cpp_arithmetic<T>::value;

#endif

template <typename T>
using is_cpp_complex = detail::disjunction<std::is_same<T, std::complex<float>>, std::is_same<T, std::complex<double>>,
                                           std::is_same<T, std::complex<long double>>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL cpp_complex = is_cpp_complex<T>::value;

#endif

namespace detail
{

// NOTE: remove_pointer_t removes the top level qualifiers of the pointer as well:
// http://en.cppreference.com/w/cpp/types/remove_pointer
// After removal of pointer, we could still have a type which is cv qualified. Thus,
// we remove cv qualifications after pointer removal.
template <typename T>
using is_char_pointer = conjunction<std::is_pointer<T>, std::is_same<remove_cv_t<remove_pointer_t<T>>, char>>;

} // namespace detail

template <typename T>
using is_string_type
    = detail::disjunction<std::is_same<detail::remove_cv_t<T>, std::string>, detail::is_char_pointer<T>,
                          // NOTE: detail::remove_cv_t does remove cv qualifiers from arrays.
                          detail::conjunction<std::is_array<detail::remove_cv_t<T>>,
                                              std::is_same<detail::remove_extent_t<detail::remove_cv_t<T>>, char>>
#if defined(MPPP_HAVE_STRING_VIEW)
                          ,
                          std::is_same<detail::remove_cv_t<T>, std::string_view>
#endif
                          >;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL string_type = is_string_type<T>::value;

#endif

} // namespace mppp

#endif
