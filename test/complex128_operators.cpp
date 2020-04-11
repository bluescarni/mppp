// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/complex128.hpp>

#include "catch.hpp"

using namespace mppp;

TEST_CASE("identity")
{
    REQUIRE(+complex128{3, 4}.m_value == cplex128{3, 4});
    REQUIRE((+complex128{3, -0.}).imag().signbit());

    constexpr auto c = +complex128{3, 4};
    REQUIRE(c.m_value == cplex128{3, 4});
}

TEST_CASE("negation")
{
    REQUIRE(-complex128{3, 4}.m_value == cplex128{-3, -4});
    REQUIRE(!(-complex128{3, -0.}).imag().signbit());
    REQUIRE(!(-complex128{-0., 3}).real().signbit());
    REQUIRE((-complex128{3, 0.}).imag().signbit());
    REQUIRE((-complex128{0., 3}).real().signbit());

    constexpr auto c = -complex128{3, 4};
    REQUIRE(c.m_value == cplex128{-3, -4});
}
