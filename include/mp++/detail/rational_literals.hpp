// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_RATIONAL_LITERALS_HPP
#define MPPP_DETAIL_RATIONAL_LITERALS_HPP

#include <mp++/integer.hpp>

namespace mppp
{

inline namespace literals
{

#define MPPP_DECLARE_RATIONAL_UDL(n)                                                                                   \
    template <char... Chars>                                                                                           \
    inline rational<n> operator"" _q##n()                                                                              \
    {                                                                                                                  \
        return rational<n>{detail::integer_literal_impl<n, Chars...>()};                                               \
    }

MPPP_DECLARE_RATIONAL_UDL(1)
MPPP_DECLARE_RATIONAL_UDL(2)
MPPP_DECLARE_RATIONAL_UDL(3)

#undef MPPP_DECLARE_RATIONAL_UDL

} // namespace literals

} // namespace mppp

#endif
