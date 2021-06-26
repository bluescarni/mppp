// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/real128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("real128 sinhcosh")
{
    REQUIRE(cosh(real128{}) == 1);
    REQUIRE(sinh(real128{}) == 0);
    real128 x;
    REQUIRE(x.cosh() == 1);
    REQUIRE(x.sinh() != 0);
    x = 0;
    REQUIRE(x.sinh() == 0);
    REQUIRE(abs(sinh(real128{"1.234"}) - real128{"1.571908059102337379176699145503416648"}) < 1E-33);
    REQUIRE(abs(cosh(real128{"1.234"}) - real128{"1.863033801698422589073643750256062085"}) < 1E-33);
}

TEST_CASE("real128 tanh")
{
    REQUIRE(tanh(real128{}) == 0);
    real128 x;
    REQUIRE(x.tanh() == 0);
    x = 0;
    REQUIRE(x.tanh() == 0);
    REQUIRE(abs(tanh(real128{"1.234"}) - real128{"0.84373566258933019391702000004355143214"}) < 1E-33);
}

TEST_CASE("real128 inversefunctions")
{
    {
        REQUIRE(acosh(real128{1}) == 0.);
        real128 x{1};
        REQUIRE(x.acosh() == 0.);
        x = 1;
        REQUIRE(x.acosh() == 0.);
        REQUIRE(abs(acos(cos(real128{"1.234"})) - real128{"1.234"}) < 1E-33);
    }
    {
        REQUIRE(asinh(real128{}) == 0.);
        real128 x;
        REQUIRE(x.asinh() == 0.);
        x = 0;
        REQUIRE(x.asinh() == 0.);
        REQUIRE(abs(asinh(sinh(real128{"0.234"})) - real128{"0.234"}) < 1E-33);
    }
    {
        REQUIRE(atanh(real128{}) == 0.);
        real128 x;
        REQUIRE(x.atanh() == 0.);
        x = 0;
        REQUIRE(x.atanh() == 0.);
        REQUIRE(abs(atanh(tanh(real128{"0.234"})) - real128{"0.234"}) < 1E-33);
    }
}
