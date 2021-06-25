// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_PARSE_COMPLEX_HPP
#define MPPP_DETAIL_PARSE_COMPLEX_HPP

#include <array>

namespace mppp
{

namespace detail
{

std::array<const char *, 4> parse_complex(const char *);

} // namespace detail

} // namespace mppp

#endif
