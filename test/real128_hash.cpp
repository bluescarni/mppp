// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>

#include <mp++/real128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("real128 hash")
{
    REQUIRE(hash(real128{"nan"}) == std::size_t(-1));
    REQUIRE(hash(real128{"-nan"}) == std::size_t(-1));
    REQUIRE(hash(real128{0}) == 0u);
    REQUIRE(hash(real128{0.}) == hash(real128{-0.}));
    REQUIRE(hash(real128{42}) != 0u);
    REQUIRE(hash(real128{42}) != std::size_t(-1));
    REQUIRE(hash(real128{-42}) != 0u);
    REQUIRE(hash(real128{-42}) != std::size_t(-1));
    REQUIRE(hash(real128{"inf"}) != 0u);
    REQUIRE(hash(real128{"inf"}) != std::size_t(-1));
    REQUIRE(hash(real128{"-inf"}) != 0u);
    REQUIRE(hash(real128{"-inf"}) != std::size_t(-1));
    REQUIRE(hash(real128{"inf"}) != hash(real128{"-inf"}));
    REQUIRE(hash(real128_min()) != 0u);
    REQUIRE(hash(real128_min()) != std::size_t(-1));
    REQUIRE(hash(-real128_min()) != 0u);
    REQUIRE(hash(-real128_min()) != std::size_t(-1));
    REQUIRE(hash(real128_max()) != 0u);
    REQUIRE(hash(real128_max()) != std::size_t(-1));
    REQUIRE(hash(-real128_max()) != 0u);
    REQUIRE(hash(-real128_max()) != std::size_t(-1));
}
