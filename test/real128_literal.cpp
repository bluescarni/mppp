// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stdexcept>
#include <string>
#include <type_traits>

#include <mp++/config.hpp>
#include <mp++/real128.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("real128_literal_tests")
{
    REQUIRE(std::is_same<real128, decltype(123_rq)>::value);
    REQUIRE(123_rq == 123);
    REQUIRE(-123._rq == -123);
    REQUIRE(-.1_rq == -real128{"0.1"});
    REQUIRE(-0.1_rq == -real128{"0.1"});
    REQUIRE(0._rq == -real128{});
    REQUIRE(0_rq == -real128{});
    REQUIRE(-.123e-7_rq == -real128{".123e-7"});
#if MPPP_CPLUSPLUS >= 201703L
    // Hex literals are supported as well.
    REQUIRE(0x123.p-7_rq == real128{"0x123.p-7"});
    REQUIRE(-0X123.p-7_rq == -real128{"0x123.p-7"});
    REQUIRE(0x123.P-7_rq == real128{"0x123.p-7"});
    REQUIRE(-0X123.P-7_rq == -real128{"0x123.p-7"});
    REQUIRE(-0X0.123P-7_rq == -real128{"0x0.123p-7"});
#endif

    // Runtime failures.
    real128 r;
#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE_THROWS_PREDICATE(r = 0b010010_rq, std::invalid_argument, [](const std::invalid_argument &ia) {
        return ia.what() == std::string("A real128 cannot be constructed from binary or octal literals");
    });
    REQUIRE_THROWS_PREDICATE(r = 0B010010_rq, std::invalid_argument, [](const std::invalid_argument &ia) {
        return ia.what() == std::string("A real128 cannot be constructed from binary or octal literals");
    });
#endif
    REQUIRE_THROWS_PREDICATE(r = 04552627_rq, std::invalid_argument, [](const std::invalid_argument &ia) {
        return ia.what() == std::string("A real128 cannot be constructed from binary or octal literals");
    });
}
