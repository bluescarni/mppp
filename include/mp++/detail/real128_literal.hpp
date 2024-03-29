// Copyright 2016-2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_REAL128_LITERAL_HPP
#define MPPP_DETAIL_REAL128_LITERAL_HPP

#include <stdexcept>

MPPP_BEGIN_NAMESPACE

inline namespace literals
{

template <char... Chars>
inline real128 operator""_rq()
{
    // Turn the sequence of input chars
    // into a null-terminated char array.
    constexpr char arr[] = {Chars..., '\0'};

    // Pre-check for binary/octal literals.
    if (sizeof...(Chars) >= 2u && arr[0] == '0'
        && (arr[1] == 'b' || arr[1] == 'B' || (arr[1] >= '0' && arr[1] <= '7'))) {
        throw std::invalid_argument("A real128 cannot be constructed from binary or octal literals");
    }

    return real128(arr);
}

} // namespace literals

MPPP_END_NAMESPACE

#endif
