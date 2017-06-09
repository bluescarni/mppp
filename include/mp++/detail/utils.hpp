// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_UTILS_HPP
#define MPPP_DETAIL_UTILS_HPP

#include <type_traits>

#include <mp++/detail/type_traits.hpp>

namespace mppp
{

inline namespace detail
{

template <typename T, enable_if_t<std::is_integral<T>::value, int> = 0>
inline int sgn(const T &n)
{
    return n ? (n > T(0) ? 1 : -1) : 0;
}

template <typename T, enable_if_t<std::is_integral<T>::value, int> = 0>
inline bool is_zero(const T &n)
{
    return n == T(0);
}
}
}

#endif
