// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <mp++/config.hpp>

#include <mp++/detail/type_traits.hpp>

using namespace mppp;

TEST_CASE("type traits")
{
    REQUIRE(is_integral<int>::value);
    REQUIRE(is_integral<unsigned long long>::value);
    REQUIRE(is_integral<char>::value);
    REQUIRE(is_integral<const char>::value);
    REQUIRE(!is_integral<char &>::value);
    REQUIRE(!is_integral<const char &>::value);
    REQUIRE(is_signed<int>::value);
    REQUIRE(is_signed<const int>::value);
    REQUIRE(!is_signed<const int &>::value);
    REQUIRE(!is_signed<unsigned>::value);
    REQUIRE(!is_signed<const unsigned>::value);
    REQUIRE(!is_signed<const unsigned &>::value);
    REQUIRE(!is_unsigned<int>::value);
    REQUIRE(!is_unsigned<const int>::value);
    REQUIRE(!is_unsigned<const int &>::value);
    REQUIRE(is_unsigned<unsigned>::value);
    REQUIRE(is_unsigned<const unsigned>::value);
    REQUIRE(!is_unsigned<const unsigned &>::value);
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE(is_integral<__int128_t>::value);
    REQUIRE(is_integral<__uint128_t>::value);
    REQUIRE(is_integral<const __int128_t>::value);
    REQUIRE(is_integral<const __uint128_t>::value);
    REQUIRE(!is_integral<const __uint128_t &>::value);
    REQUIRE(is_signed<__int128_t>::value);
    REQUIRE(is_signed<const __int128_t>::value);
    REQUIRE(!is_signed<__int128_t &&>::value);
    REQUIRE(!is_unsigned<__int128_t>::value);
    REQUIRE(!is_unsigned<const __int128_t>::value);
    REQUIRE(!is_unsigned<__int128_t &&>::value);
    REQUIRE(!is_signed<__uint128_t>::value);
    REQUIRE(!is_signed<const __uint128_t>::value);
    REQUIRE(!is_signed<__uint128_t &&>::value);
    REQUIRE(is_unsigned<__uint128_t>::value);
    REQUIRE(is_unsigned<const __uint128_t>::value);
    REQUIRE(!is_unsigned<__uint128_t &&>::value);
#endif
}
