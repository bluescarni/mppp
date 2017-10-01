// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <mp++/detail/mpfr.hpp>
#include <mp++/real.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

TEST_CASE("real io")
{
    REQUIRE((real{42, 32}.to_string() == "4.2000000000e+1"));
    REQUIRE(::mpfr_equal_p(real{"4.2000000000e+1", 32}.get_mpfr_t(), real{42, 32}.get_mpfr_t()));
}
