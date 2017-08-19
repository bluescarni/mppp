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

TEST_CASE("real128 signbit")
{
    real128 r;
    REQUIRE(r.finite());
    REQUIRE(finite(r));
    REQUIRE(!r.isinf());
    REQUIRE(!isinf(r));
    REQUIRE(!r.isnan());
    REQUIRE(!isnan(r));
    r = -1;
    REQUIRE(r.finite());
    REQUIRE(finite(r));
    REQUIRE(!r.isinf());
    REQUIRE(!isinf(r));
    REQUIRE(!r.isnan());
    REQUIRE(!isnan(r));
    r = 123;
    REQUIRE(r.finite());
    REQUIRE(finite(r));
    REQUIRE(!r.isinf());
    REQUIRE(!isinf(r));
    REQUIRE(!r.isnan());
    REQUIRE(!isnan(r));
    r = "inf";
    REQUIRE(!r.finite());
    REQUIRE(!finite(r));
    REQUIRE(r.isinf());
    REQUIRE(isinf(r));
    REQUIRE(!r.isnan());
    REQUIRE(!isnan(r));
    r = "-inf";
    REQUIRE(!r.finite());
    REQUIRE(!finite(r));
    REQUIRE(r.isinf());
    REQUIRE(isinf(r));
    REQUIRE(!r.isnan());
    REQUIRE(!isnan(r));
    r = "nan";
    REQUIRE(!r.finite());
    REQUIRE(!finite(r));
    REQUIRE(!r.isinf());
    REQUIRE(!isinf(r));
    REQUIRE(r.isnan());
    REQUIRE(isnan(r));
}
