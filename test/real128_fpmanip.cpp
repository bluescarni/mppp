// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("real128 nextafter")
{
    REQUIRE(nextafter(real128{0}, real128{1}) > 0);
    REQUIRE(nextafter(real128{0}, real128{-1}) < 0);
    REQUIRE(nextafter(real128{0}, real128{0}) == 0);
    REQUIRE(nextafter(real128{1}, real128{1}) == 1);
    REQUIRE(nextafter(real128{-1}, real128{-1}) == -1);
    REQUIRE(isinf(nextafter(real128{"inf"}, real128{"inf"})));
    REQUIRE(isinf(nextafter(real128{"-inf"}, real128{"-inf"})));
    REQUIRE(!isinf(nextafter(real128{"-inf"}, real128{1})));
}

TEST_CASE("real128 copysign")
{
    REQUIRE(copysign(-1_rq, -1_rq) == -1);
    REQUIRE(copysign(1_rq, -1_rq) == -1);
    REQUIRE(copysign(-1_rq, 1_rq) == 1);
    REQUIRE(copysign(1_rq, 1_rq) == 1);

    // Couple of small tests with non-real128 args.
    REQUIRE(copysign(-1_rq, -1) == -1);
    REQUIRE(copysign(1., -1_rq) == -1);
    REQUIRE(copysign(-1_rq, 1_z1) == 1);
    REQUIRE(copysign(1_q1, 1_rq) == 1);
}
