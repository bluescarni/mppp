// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <mp++/integer.hpp>

#include "catch.hpp"

using namespace mppp;

#if MPPP_CPLUSPLUS >= 201703L

TEST_CASE("integer_literal_check_str_test")
{
    // Decimal.
    {
        constexpr char str[] = {'0', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 10);
    }
    {
        constexpr char str[] = {'9', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 10);
    }
    {
        constexpr char str[] = {'9', '1', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 10);
    }
    {
        constexpr char str[] = {'9', '1', '5', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 10);
    }
    {
        constexpr char str[] = {'9', '1', '5', '0', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 10);
    }
    {
        char str[] = {'a', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'a', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'1', 'a', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'a', '1', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'1', '2', '.', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'1', '2', 'e', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }

    // Binary.
    {
        constexpr char str[] = {'0', 'b', '1', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 2);
    }
    {
        constexpr char str[] = {'0', 'B', '1', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 2);
    }
    {
        constexpr char str[] = {'0', 'b', '0', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 2);
    }
    {
        constexpr char str[] = {'0', 'B', '0', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 2);
    }
    {
        constexpr char str[] = {'0', 'b', '0', '1', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 2);
    }
    {
        constexpr char str[] = {'0', 'B', '0', '0', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 2);
    }
    {
        char str[] = {'0', 'b', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'0', 'B', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'0', 'b', '2', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'0', 'B', '2', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'0', 'b', 'a', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'0', 'B', 'f', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }

    // Octal.
    {
        constexpr char str[] = {'0', '1', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 8);
    }
    {
        constexpr char str[] = {'0', '2', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 8);
    }
    {
        constexpr char str[] = {'0', '1', '2', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 8);
    }
    {
        constexpr char str[] = {'0', '0', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 8);
    }
    {
        constexpr char str[] = {'0', '0', '7', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 8);
    }
    {
        char str[] = {'0', '9', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'0', 'a', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'0', '1', 'a', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'0', '7', '8', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'0', '1', '.', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'0', '3', 'e', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }

    // Hex.
    {
        constexpr char str[] = {'0', 'x', '1', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 16);
    }
    {
        constexpr char str[] = {'0', 'X', 'f', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 16);
    }
    {
        constexpr char str[] = {'0', 'x', 'F', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 16);
    }
    {
        constexpr char str[] = {'0', 'X', '0', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 16);
    }
    {
        constexpr char str[] = {'0', 'x', 'a', 'B', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 16);
    }
    {
        constexpr char str[] = {'0', 'X', 'C', 'd', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 16);
    }
    {
        char str[] = {'0', 'x', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'0', 'X', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'0', 'x', 'g', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'0', 'X', 'G', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'0', 'x', '.', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
    {
        char str[] = {'0', 'X', 'p', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), detail::invalid_integer_literal);
    }
}

TEST_CASE("z1_test")
{
    REQUIRE(0b1101010010101001010100100101_z1 == integer<1>{"1101010010101001010100100101", 2});
}

#else

TEST_CASE("null_test") {}

#endif
