// Copyright 2016-2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_RATIONAL_LITERALS_HPP
#define MPPP_DETAIL_RATIONAL_LITERALS_HPP

#include <mp++/integer.hpp>

MPPP_BEGIN_NAMESPACE

inline namespace literals
{

template <char... Chars>
inline rational<1> operator""_q1()
{
    return rational<1>{detail::integer_literal_impl<1, Chars...>()};
}
template <char... Chars>
inline rational<2> operator""_q2()
{
    return rational<2>{detail::integer_literal_impl<2, Chars...>()};
}

template <char... Chars>
inline rational<3> operator""_q3()
{
    return rational<3>{detail::integer_literal_impl<3, Chars...>()};
}

} // namespace literals

MPPP_END_NAMESPACE

#endif
