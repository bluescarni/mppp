// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_FWD_HPP
#define MPPP_FWD_HPP

#include <cstddef>

#include <mp++/config.hpp>

namespace mppp
{

template <std::size_t>
class integer;

template <std::size_t>
class rational;

#if defined(MPPP_WITH_MPFR)

class real;

#endif

#if defined(MPPP_WITH_MPC)

class complex;

#endif

#if defined(MPPP_WITH_QUADMATH)

class real128;
class complex128;

#endif

} // namespace mppp

#endif
