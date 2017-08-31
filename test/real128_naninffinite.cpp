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

TEST_CASE("real128 naninffinite")
{
    real128 r;
    REQUIRE(r.finite());
    REQUIRE(finite(r));
    REQUIRE(!r.isinf());
    REQUIRE(!isinf(r));
    REQUIRE(!r.isnan());
    REQUIRE(!isnan(r));
    constexpr bool c0 = real128{}.isnan();
    REQUIRE(!c0);
    constexpr bool c1 = isnan(real128{});
    REQUIRE(!c1);
    r = -1;
    REQUIRE(r.finite());
    REQUIRE(finite(r));
    REQUIRE(!r.isinf());
    REQUIRE(!isinf(r));
    REQUIRE(!r.isnan());
    REQUIRE(!isnan(r));
    constexpr bool d0 = real128{1}.isnan();
    REQUIRE(!c0);
    constexpr bool d1 = isnan(real128{1});
    REQUIRE(!c1);
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
