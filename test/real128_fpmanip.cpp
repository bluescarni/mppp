// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/real128.hpp>

#include "catch.hpp"

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
