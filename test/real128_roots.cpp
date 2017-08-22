// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <quadmath.h>

#include <mp++/mp++.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

TEST_CASE("real128 sqrt")
{
    real128 r;
    r.sqrt();
    REQUIRE((r.m_value == 0));
    r = 4;
    r.sqrt();
    REQUIRE((r.m_value == 2));
    r = 2;
    r.sqrt();
    REQUIRE((::fabsq(real128{"1.41421356237309504880168872420969807856967187537694807317667973799073247"}.m_value
                     - r.m_value)
             < 1E-32));
    r = -2;
    r.sqrt();
    REQUIRE(isnan(r));
    r.sqrt().sqrt();
    REQUIRE(isnan(r));
}
