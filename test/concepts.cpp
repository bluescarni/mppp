// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <mp++/concepts.hpp>
#if MPPP_CPLUSPLUS >= 201703L
#include <string_view>
#endif
#include <string>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

TEST_CASE("concepts")
{
    REQUIRE(is_string_type<char *>::value);
    REQUIRE(is_string_type<const char *>::value);
    REQUIRE(!is_string_type<char>::value);
    REQUIRE(!is_string_type<const char>::value);
    REQUIRE(!is_string_type<int>::value);
    REQUIRE(is_string_type<std::string>::value);
    REQUIRE(!is_string_type<std::string &>::value);
    REQUIRE(!is_string_type<const std::string &>::value);
    REQUIRE(!is_string_type<const std::string>::value);
#if MPPP_CPLUSPLUS >= 201703L
    REQUIRE(is_string_type<std::string_view>::value);
    REQUIRE(!is_string_type<std::string_view &>::value);
    REQUIRE(!is_string_type<const std::string_view &>::value);
    REQUIRE(!is_string_type<const std::string_view>::value);
#endif
}
