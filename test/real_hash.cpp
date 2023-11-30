// Copyright 2016-2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <functional>

#include <mp++/real.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("real hash")
{
    // NaNs.
    REQUIRE(hash(real{"nan", 32}) == hash(real{"-nan", 32}));
    REQUIRE(hash(real{"nan", 640}) == hash(real{"-nan", 32}));
    REQUIRE(hash(real{"nan", 32}) == hash(real{"-nan", 640}));

    // Zeroes.
    REQUIRE(hash(real{"0", 32}) == hash(real{"-0", 32}));
    REQUIRE(hash(real{"0", 640}) == hash(real{"-0", 32}));
    REQUIRE(hash(real{"0", 32}) == hash(real{"-0", 640}));

    // Infinities.
    REQUIRE(hash(real{"inf", 32}) == hash(real{"inf", 32}));
    REQUIRE(hash(real{"-inf", 640}) == hash(real{"-inf", 640}));
    REQUIRE(hash(real{"inf", 32}) != hash(real{"-inf", 32}));
    REQUIRE(hash(real{"inf", 32}) == hash(real{"inf", 640}));
    REQUIRE(hash(real{"-inf", 640}) == hash(real{"-inf", 32}));

    // Normal numbers. The important thing to check here is that
    // trailing zero limbs are ignored in the computation of the hash.
    REQUIRE(hash(real{1, 1137}) == hash(real{1, 31}));
    real tmp{"-1.1", 113};
    tmp.prec_round(2371);
    REQUIRE(hash(tmp) == hash(real{"-1.1", 113}));
    REQUIRE(hash(real{1, 64}) == hash(real{1, 65}));
    REQUIRE(hash(real{1, 64}) == hash(real{1, 63}));

    // Test also the std::hash specialisation.
    REQUIRE(std::hash<real>{}(real{"-1.1", 113}) == hash(real{"-1.1", 113}));

    // A test changing the exponent.
    REQUIRE(std::hash<real>{}(real{"-1.1", 113}) != hash(real{"-1.1", 113} * 16 * 16));
}
