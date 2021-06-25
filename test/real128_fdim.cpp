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

TEST_CASE("real128 fdim")
{
    REQUIRE(fdim(-1_rq, -1_rq) == 0);
    REQUIRE(fdim(1_rq, -1_rq) == 2);
    REQUIRE(fdim(-1_rq, 1_rq) == 0);
    REQUIRE(fdim(1_rq, 1_rq) == 0);

    // Couple of small tests with non-real128 args.
    REQUIRE(fdim(-1_rq, -1) == 0);
    REQUIRE(fdim(1., -1_rq) == 2);
    REQUIRE(fdim(-1_rq, 1_z1) == 0);
    REQUIRE(fdim(1_q1, 1_rq) == 0);
}
