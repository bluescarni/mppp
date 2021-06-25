// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_MPC_HPP
#define MPPP_DETAIL_MPC_HPP

#include <type_traits>

#include <mpc.h>

namespace mppp
{

// The MPC structure underlying mpc_t.
using mpc_struct_t = std::remove_extent<::mpc_t>::type;

} // namespace mppp

#endif
