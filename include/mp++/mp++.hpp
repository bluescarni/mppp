// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_MPPP_HPP
#define MPPP_MPPP_HPP

#include <mp++/config.hpp>
#include <mp++/exceptions.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/type_name.hpp>

#if defined(MPPP_WITH_MPFR)
#include <mp++/real.hpp>
#endif

#if defined(MPPP_WITH_MPC)
#include <mp++/complex.hpp>
#endif

#if defined(MPPP_WITH_QUADMATH)
#include <mp++/complex128.hpp>
#include <mp++/real128.hpp>
#endif

#endif
