// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/mp++.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

TEST_CASE("real128 abs")
{
    real128 r;
    REQUIRE((r.abs().m_value == 0));
    REQUIRE((abs(r).m_value == 0));
    REQUIRE((r.m_value == 0));
    r = -5;
    REQUIRE((abs(r).m_value == 5));
    REQUIRE((r.abs().m_value == 5));
    REQUIRE((r.m_value == 5));
    r = 5;
    REQUIRE((abs(r).m_value == 5));
    REQUIRE((r.abs().m_value == 5));
    REQUIRE((r.m_value == 5));
    r = -.00005;
    REQUIRE((abs(r).m_value == .00005));
    REQUIRE((r.abs().m_value == .00005));
    REQUIRE((r.m_value == .00005));
    r = .00005;
    REQUIRE((abs(r).m_value == .00005));
    REQUIRE((r.abs().m_value == .00005));
    REQUIRE((r.m_value == .00005));
    r = "-inf";
    REQUIRE((abs(r).m_value == real128{"inf"}.m_value));
    REQUIRE((r.abs().m_value == real128{"inf"}.m_value));
    REQUIRE((r.m_value == real128{"inf"}.m_value));
    r = "inf";
    REQUIRE((abs(r).m_value == real128{"inf"}.m_value));
    REQUIRE((r.abs().m_value == real128{"inf"}.m_value));
    REQUIRE((r.m_value == real128{"inf"}.m_value));
}
