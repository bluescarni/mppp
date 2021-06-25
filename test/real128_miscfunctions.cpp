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

TEST_CASE("real128 erf")
{
    REQUIRE(erf(real128{}) == 0);
    real128 x;
    REQUIRE(x.erf() == 0);
    REQUIRE(abs(erf(real128{"1.234"}) - real128{"0.9190394169576684157198123662625681813"}) < 1E-34);
}

TEST_CASE("real128 erfc")
{
    REQUIRE(erfc(real128{}) == 1);
    real128 x;
    REQUIRE(x.erfc() == 1);
    REQUIRE(abs(erfc(real128{4}) - 0.0000000154172579002800188521596734868840499_rq) < 1E-34);
}

TEST_CASE("real128 lgamma")
{
    REQUIRE(lgamma(real128{1}) == 0);
    real128 x(1);
    REQUIRE(x.lgamma() == 0);
    REQUIRE(abs(lgamma(real128{"1.234"}) - real128{"-0.094478407681159572584826666218660204"}) < 1E-34);
}

TEST_CASE("real128 tgamma")
{
    REQUIRE(tgamma(real128{1}) == 1);
    real128 x(1);
    REQUIRE(x.tgamma() == 1);
    REQUIRE(abs(tgamma(real128{"1.234"}) - real128{"0.909847380128661106639623421763461722"}) < 1E-34);
}
