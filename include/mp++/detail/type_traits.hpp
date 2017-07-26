// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_TYPE_TRAITS_HPP
#define MPPP_DETAIL_TYPE_TRAITS_HPP

#include <type_traits>

namespace mppp
{

inline namespace detail
{

// A bunch of useful utilities from C++14/C++17.

// http://en.cppreference.com/w/cpp/types/void_t
template <typename... Ts>
struct make_void {
    typedef void type;
};

template <typename... Ts>
using void_t = typename make_void<Ts...>::type;

// http://en.cppreference.com/w/cpp/experimental/is_detected
template <class Default, class AlwaysVoid, template <class...> class Op, class... Args>
struct detector {
    using value_t = std::false_type;
    using type = Default;
};

template <class Default, template <class...> class Op, class... Args>
struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
    using type = Op<Args...>;
};

// http://en.cppreference.com/w/cpp/experimental/nonesuch
struct nonesuch {
    nonesuch() = delete;
    ~nonesuch() = delete;
    nonesuch(nonesuch const &) = delete;
    void operator=(nonesuch const &) = delete;
};

template <template <class...> class Op, class... Args>
using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;

template <template <class...> class Op, class... Args>
using detected_t = typename detector<nonesuch, void, Op, Args...>::type;

// http://en.cppreference.com/w/cpp/types/conjunction
template <class...>
struct conjunction : std::true_type {
};

template <class B1>
struct conjunction<B1> : B1 {
};

template <class B1, class... Bn>
struct conjunction<B1, Bn...> : std::conditional<B1::value != false, conjunction<Bn...>, B1>::type {
};

// http://en.cppreference.com/w/cpp/types/disjunction
template <class...>
struct disjunction : std::false_type {
};

template <class B1>
struct disjunction<B1> : B1 {
};

template <class B1, class... Bn>
struct disjunction<B1, Bn...> : std::conditional<B1::value != false, B1, disjunction<Bn...>>::type {
};

// http://en.cppreference.com/w/cpp/types/negation
template <class B>
struct negation : std::integral_constant<bool, !B::value> {
};

// Some handy aliases.
template <typename T>
using uncvref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

// Small helpers, like C++14.
template <bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <typename T>
using make_unsigned = typename std::make_unsigned<T>::type;
}
}

#endif
