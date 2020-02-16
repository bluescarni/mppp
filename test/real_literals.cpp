// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stdexcept>
#include <string>
#include <type_traits>

#include <mp++/real.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

using namespace mppp;

TEST_CASE("real_literals_tests")
{
    REQUIRE(std::is_same<real, decltype(123_r128)>::value);
    REQUIRE((123_r128).get_prec() == 128);
    REQUIRE(123_r128 == 123);
    REQUIRE(-123._r128 == -123);
    REQUIRE(-.1_r128 == -real{"0.1", 128});
    REQUIRE(-.123e-7_r128 == -real{".123e-7", 128});
    // Hex literals are supported as well.
    REQUIRE(0x123.p-7_r128 == real{"0x123.p-7", 16, 128});
    REQUIRE(-0X123.p-7_r128 == -real{"0x123.p-7", 16, 128});
    REQUIRE(0x123.P-7_r128 == real{"0x123.p-7", 16, 128});
    REQUIRE(-0X123.P-7_r128 == -real{"0x123.p-7", 16, 128});

    // Runtime failures.
    real r;
    REQUIRE_THROWS_PREDICATE(r = 0b010010_r128, std::invalid_argument, [](const std::invalid_argument &ia) {
        return ia.what() == std::string("A real cannot be constructed from binary or octal literals");
    });
    REQUIRE_THROWS_PREDICATE(r = 0B010010_r128, std::invalid_argument, [](const std::invalid_argument &ia) {
        return ia.what() == std::string("A real cannot be constructed from binary or octal literals");
    });
    REQUIRE_THROWS_PREDICATE(r = 04552627_r128, std::invalid_argument, [](const std::invalid_argument &ia) {
        return ia.what() == std::string("A real cannot be constructed from binary or octal literals");
    });

    REQUIRE(std::is_same<real, decltype(123_r256)>::value);
    REQUIRE((123_r256).get_prec() == 256);
    REQUIRE(123_r256 == 123);
    REQUIRE(-123._r256 == -123);
    REQUIRE(-.1_r256 == -real{"0.1", 256});
    REQUIRE(-.123e-7_r256 == -real{".123e-7", 256});
    // Hex literals are supported as well.
    REQUIRE(0x123.p-7_r256 == real{"0x123.p-7", 16, 256});
    REQUIRE(-0X123.p-7_r256 == -real{"0x123.p-7", 16, 256});
    REQUIRE(0x123.P-7_r256 == real{"0x123.p-7", 16, 256});
    REQUIRE(-0X123.P-7_r256 == -real{"0x123.p-7", 16, 256});

    // Runtime failures.
    REQUIRE_THROWS_PREDICATE(r = 0b010010_r256, std::invalid_argument, [](const std::invalid_argument &ia) {
        return ia.what() == std::string("A real cannot be constructed from binary or octal literals");
    });
    REQUIRE_THROWS_PREDICATE(r = 0B010010_r256, std::invalid_argument, [](const std::invalid_argument &ia) {
        return ia.what() == std::string("A real cannot be constructed from binary or octal literals");
    });
    REQUIRE_THROWS_PREDICATE(r = 04552627_r256, std::invalid_argument, [](const std::invalid_argument &ia) {
        return ia.what() == std::string("A real cannot be constructed from binary or octal literals");
    });

    REQUIRE(std::is_same<real, decltype(123_r512)>::value);
    REQUIRE((123_r512).get_prec() == 512);
    REQUIRE(123_r512 == 123);
    REQUIRE(-123._r512 == -123);
    REQUIRE(-.1_r512 == -real{"0.1", 512});
    REQUIRE(-.123e-7_r512 == -real{".123e-7", 512});
    // Hex literals are supported as well.
    REQUIRE(0x123.p-7_r512 == real{"0x123.p-7", 16, 512});
    REQUIRE(-0X123.p-7_r512 == -real{"0x123.p-7", 16, 512});
    REQUIRE(0x123.P-7_r512 == real{"0x123.p-7", 16, 512});
    REQUIRE(-0X123.P-7_r512 == -real{"0x123.p-7", 16, 512});

    // Runtime failures.
    REQUIRE_THROWS_PREDICATE(r = 0b010010_r512, std::invalid_argument, [](const std::invalid_argument &ia) {
        return ia.what() == std::string("A real cannot be constructed from binary or octal literals");
    });
    REQUIRE_THROWS_PREDICATE(r = 0B010010_r512, std::invalid_argument, [](const std::invalid_argument &ia) {
        return ia.what() == std::string("A real cannot be constructed from binary or octal literals");
    });
    REQUIRE_THROWS_PREDICATE(r = 04552627_r512, std::invalid_argument, [](const std::invalid_argument &ia) {
        return ia.what() == std::string("A real cannot be constructed from binary or octal literals");
    });

    REQUIRE(std::is_same<real, decltype(123_r1024)>::value);
    REQUIRE((123_r1024).get_prec() == 1024);
    REQUIRE(123_r1024 == 123);
    REQUIRE(-123._r1024 == -123);
    REQUIRE(-.1_r1024 == -real{"0.1", 1024});
    REQUIRE(-.123e-7_r1024 == -real{".123e-7", 1024});
    // Hex literals are supported as well.
    REQUIRE(0x123.p-7_r1024 == real{"0x123.p-7", 16, 1024});
    REQUIRE(-0X123.p-7_r1024 == -real{"0x123.p-7", 16, 1024});
    REQUIRE(0x123.P-7_r1024 == real{"0x123.p-7", 16, 1024});
    REQUIRE(-0X123.P-7_r1024 == -real{"0x123.p-7", 16, 1024});

    // Runtime failures.
    REQUIRE_THROWS_PREDICATE(r = 0b010010_r1024, std::invalid_argument, [](const std::invalid_argument &ia) {
        return ia.what() == std::string("A real cannot be constructed from binary or octal literals");
    });
    REQUIRE_THROWS_PREDICATE(r = 0B010010_r1024, std::invalid_argument, [](const std::invalid_argument &ia) {
        return ia.what() == std::string("A real cannot be constructed from binary or octal literals");
    });
    REQUIRE_THROWS_PREDICATE(r = 04552627_r1024, std::invalid_argument, [](const std::invalid_argument &ia) {
        return ia.what() == std::string("A real cannot be constructed from binary or octal literals");
    });
}
