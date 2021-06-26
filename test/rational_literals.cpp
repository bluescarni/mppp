// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <type_traits>

#include <mp++/config.hpp>
#include <mp++/rational.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

// Just a few simple tests, as the rational
// literal is built on top of the integer
// literal.
TEST_CASE("rational_literals_tests")
{
    REQUIRE(std::is_same<rational<1>, decltype(123_q1)>::value);
    REQUIRE(123_q1 == 123);
    REQUIRE(-456_q1 == -456);
#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(-0b1_q1 == -1);
#endif
    REQUIRE(07_q1 == 7);
    REQUIRE(-0xf_q1 == -15);
    REQUIRE(4_q1 / -6 == -rational<1>{2, 3});

    REQUIRE(std::is_same<rational<2>, decltype(123_q2)>::value);
    REQUIRE(123_q2 == 123);
    REQUIRE(-456_q2 == -456);
#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(-0b1_q2 == -1);
#endif
    REQUIRE(07_q2 == 7);
    REQUIRE(-0xf_q2 == -15);
    REQUIRE(4_q2 / -6 == -rational<2>{2, 3});

    REQUIRE(std::is_same<rational<3>, decltype(123_q3)>::value);
    REQUIRE(123_q3 == 123);
    REQUIRE(-456_q3 == -456);
#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(-0b1_q3 == -1);
#endif
    REQUIRE(07_q3 == 7);
    REQUIRE(-0xf_q3 == -15);
    REQUIRE(4_q3 / -6 == -rational<3>{2, 3});
}
