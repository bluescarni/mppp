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
    REQUIRE(sqrt(r) == 0);
    r.sqrt();
    REQUIRE((r.m_value == 0));
    r = -0.;
    r.sqrt();
    REQUIRE((r.m_value == 0));
    r = 4;
    REQUIRE(sqrt(r) == 2);
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

TEST_CASE("real128 cbrt")
{
    real128 r;
    r.cbrt();
    REQUIRE((r.m_value == 0));
    r = 8;
    REQUIRE(cbrt(r) == 2);
    r.cbrt();
    REQUIRE((r.m_value == 2));
    r = -8;
    REQUIRE(cbrt(r) == -2);
    r.cbrt();
    REQUIRE((r.m_value == -2));
    r = 2;
    r.cbrt();
    REQUIRE((::fabsq(real128{"1.25992104989487316476721060727822835057025146470150798008197"}.m_value - r.m_value)
             < 1E-32));
    r = -2;
    r.cbrt();
    REQUIRE((::fabsq(real128{"-1.25992104989487316476721060727822835057025146470150798008197"}.m_value - r.m_value)
             < 1E-32));
    r.cbrt().cbrt();
}

TEST_CASE("real128 hypot")
{
    REQUIRE((hypot(real128{}, real128{}).m_value == 0));
    REQUIRE((hypot(real128{4}, real128{3}).m_value == 5));
    REQUIRE((hypot(real128{4}, real128{-3}).m_value == 5));
    REQUIRE((hypot(real128{-4}, real128{-3}).m_value == 5));
    REQUIRE((hypot(real128{-4}, real128{3}).m_value == 5));
    REQUIRE((::fabsq(hypot(real128{"1.2"}, real128{"2.3"}).m_value
                     - real128("2.5942243542145694689507875815343180229805121340196627480426331876639493063").m_value)
             < 1E-32));
}
