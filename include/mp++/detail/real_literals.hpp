// Copyright 2016-2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_REAL_LITERALS_HPP
#define MPPP_DETAIL_REAL_LITERALS_HPP

#include <mp++/detail/mpfr.hpp>

MPPP_BEGIN_NAMESPACE

namespace detail
{

template <char... Chars>
inline real real_literal_impl(::mpfr_prec_t prec)
{
    // Turn the sequence of input chars
    // into a null-terminated char array.
    constexpr char arr[] = {Chars..., '\0'};

    // Pre-check for binary/octal literals.
    if (sizeof...(Chars) >= 2u && arr[0] == '0'
        && (arr[1] == 'b' || arr[1] == 'B' || (arr[1] >= '0' && arr[1] <= '7'))) {
        throw std::invalid_argument("A real cannot be constructed from binary or octal literals");
    }

    // Infer the base. Default is 10, if the string
    // contains 'x'/'X' then we assume base 16.
    int base = 10;
    for (auto c : arr) {
        if (c == 'x' || c == 'X') {
            base = 16;
            break;
        }
    }

    return real{arr, base, prec};
}

} // namespace detail

inline namespace literals
{

template <char... Chars>
inline real operator""_r128()
{
    return detail::real_literal_impl<Chars...>(128);
}

template <char... Chars>
inline real operator""_r256()
{
    return detail::real_literal_impl<Chars...>(256);
}

template <char... Chars>
inline real operator""_r512()
{
    return detail::real_literal_impl<Chars...>(512);
}

template <char... Chars>
inline real operator""_r1024()
{
    return detail::real_literal_impl<Chars...>(1024);
}

} // namespace literals

MPPP_END_NAMESPACE

#endif
