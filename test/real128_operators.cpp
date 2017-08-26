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

using int_t = integer<1>;
using rat_t = rational<1>;

TEST_CASE("real128 plus")
{
    real128 x;
    REQUIRE(((+x).m_value == 0));
    x = -145;
    REQUIRE(((+x).m_value == -145));
    real128 y{12};
    x = -5;
    REQUIRE(((x + y).m_value == 7));
    constexpr auto z1 = real128{56} + real128{3};
    constexpr auto z1a = +z1;
    REQUIRE((z1.m_value == 59));
    REQUIRE((z1a.m_value == 59));
    REQUIRE(((x + 3).m_value == -2));
    REQUIRE(((x + 2.).m_value == -3));
    REQUIRE(((3 + x).m_value == -2));
    REQUIRE(((2. + x).m_value == -3));
    constexpr auto z2 = real128{56} + 3;
    REQUIRE((z2.m_value == 59));
    constexpr auto z3 = 3.f + real128{56};
    REQUIRE((z3.m_value == 59));
    REQUIRE(((x + int_t{3}).m_value == -2));
    REQUIRE(((int_t{3} + x).m_value == -2));
    REQUIRE(((x + rat_t{3, 2}).m_value == real128{"-3.5"}.m_value));
    REQUIRE(((rat_t{3, 2} + x).m_value == real128{"-3.5"}.m_value));
}
