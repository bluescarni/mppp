// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <mp++/config.hpp>

#include <limits>
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
    REQUIRE((std::is_same<unsigned, make_unsigned_t<int>>::value));
    REQUIRE((std::is_same<unsigned, make_unsigned_t<unsigned>>::value));
    REQUIRE((std::is_same<const unsigned, make_unsigned_t<const int>>::value));
    REQUIRE((std::is_same<const unsigned, make_unsigned_t<const unsigned>>::value));
    REQUIRE((std::is_same<volatile unsigned, make_unsigned_t<volatile int>>::value));
    REQUIRE((std::is_same<volatile unsigned, make_unsigned_t<volatile unsigned>>::value));
    REQUIRE((std::is_same<const volatile unsigned, make_unsigned_t<volatile const int>>::value));
    REQUIRE((std::is_same<const volatile unsigned, make_unsigned_t<volatile const unsigned>>::value));
    REQUIRE(std::numeric_limits<int>::digits == nl_digits<int>());
    REQUIRE(std::numeric_limits<int>::max() == nl_max<int>());
    REQUIRE(std::numeric_limits<int>::min() == nl_min<int>());
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
    REQUIRE((std::is_same<__uint128_t, make_unsigned_t<__int128_t>>::value));
    REQUIRE((std::is_same<__uint128_t, make_unsigned_t<__uint128_t>>::value));
    REQUIRE((std::is_same<const __uint128_t, make_unsigned_t<const __int128_t>>::value));
    REQUIRE((std::is_same<const __uint128_t, make_unsigned_t<const __uint128_t>>::value));
    REQUIRE((std::is_same<volatile __uint128_t, make_unsigned_t<volatile __int128_t>>::value));
    REQUIRE((std::is_same<volatile __uint128_t, make_unsigned_t<volatile __uint128_t>>::value));
    REQUIRE((std::is_same<const volatile __uint128_t, make_unsigned_t<volatile const __int128_t>>::value));
    REQUIRE((std::is_same<const volatile __uint128_t, make_unsigned_t<volatile const __uint128_t>>::value));
    REQUIRE(128 == nl_digits<__uint128_t>());
    REQUIRE(~__uint128_t(0) == nl_max<__uint128_t>());
    REQUIRE(0u == nl_min<__uint128_t>());
    REQUIRE(127 == nl_digits<__int128_t>());
    REQUIRE(__int128_t(18446744073709551615ull) + (__int128_t(9223372036854775807ull) << 64) == nl_max<__int128_t>());
    REQUIRE(static_cast<__int128_t>(__uint128_t(1) << 127) == nl_min<__int128_t>());
#endif
}
