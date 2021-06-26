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

TEST_CASE("real128 j0")
{
    REQUIRE(j0(real128{}) == 1);
    real128 x;
    REQUIRE(x.j0() == 1);
    REQUIRE(abs(j0(real128{1}) - 0.765197686557966551449717526102663221_rq) < 1E-34);
}

TEST_CASE("real128 j1")
{
    REQUIRE(j1(real128{}) == 0);
    real128 x;
    REQUIRE(x.j1() == 0);
    REQUIRE(abs(j1(real128{1}) - 0.440050585744933515959682203718914913_rq) < 1E-34);
}

TEST_CASE("real128 jn")
{
    REQUIRE(abs(jn(7, real128{1}) - 0.00000150232581743680821221863346804840184_rq) < 1E-34);
}

TEST_CASE("real128 y0")
{
    REQUIRE(y0(real128{}) == real128{"-inf"});
    real128 x;
    REQUIRE(x.y0() == real128{"-inf"});
    REQUIRE(abs(y0(real128{1}) - 0.0882569642156769579829267660235151621_rq) < 1E-34);
}

TEST_CASE("real128 y1")
{
    REQUIRE(y1(real128{}) == real128{"-inf"});
    real128 x;
    REQUIRE(x.y1() == real128{"-inf"});
    REQUIRE(abs(y1(real128{1}) - -0.78121282130028871654715000004796479_rq) < 1E-34);
}

TEST_CASE("real128 yn")
{
    REQUIRE(abs(yn(7, real128{1}) - -30588.9570521239888408800675925647278_rq) < 1E-34);
}
