// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#if defined(__clang__) || defined(__GNUC__)

#pragma GCC diagnostic ignored "-Wconversion"

#endif

#include <type_traits>

#include <quadmath.h>

#include <mp++/config.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real128.hpp>

#include "catch.hpp"

using namespace mppp;

using int_t = integer<1>;
using rat_t = rational<1>;

TEST_CASE("real128 pow")
{
    REQUIRE((std::is_same<real128, decltype(pow(real128{}, real128{}))>::value));
    REQUIRE((std::is_same<real128, decltype(pow(real128{}, 0))>::value));
    REQUIRE((std::is_same<real128, decltype(pow(0., real128{}))>::value));
    REQUIRE((std::is_same<real128, decltype(pow(int_t{}, real128{}))>::value));
    REQUIRE((std::is_same<real128, decltype(pow(real128{}, rat_t{}))>::value));
    REQUIRE((pow(real128{}, real128{}).m_value == 1.));
    REQUIRE((pow(real128{2}, real128{3}).m_value == 8.));
    REQUIRE((pow(real128{2}, real128{-3}).m_value == real128{".125"}.m_value));
    REQUIRE((pow(real128{}, 0).m_value == 1.));
    REQUIRE((pow(0.f, real128{}).m_value == 1.));
    REQUIRE((pow(real128{2}, 3ll).m_value == 8.));
    REQUIRE((pow(2u, real128{3}).m_value == 8.));
    REQUIRE((pow(real128{2}, static_cast<signed char>(-3)).m_value == real128{".125"}.m_value));
    REQUIRE((pow(2., real128{-3}).m_value == real128{".125"}.m_value));
    REQUIRE((pow(real128{}, int_t{}).m_value == 1.));
    REQUIRE((pow(int_t{}, real128{}).m_value == 1.));
    REQUIRE((pow(real128{2}, int_t{3}).m_value == 8.));
    REQUIRE((pow(int_t{2}, real128{3}).m_value == 8.));
    REQUIRE((pow(real128{2}, int_t{-3}).m_value == real128{".125"}.m_value));
    REQUIRE((pow(int_t{2}, real128{-3}).m_value == real128{".125"}.m_value));
    REQUIRE((pow(real128{}, rat_t{}).m_value == 1.));
    REQUIRE((pow(rat_t{}, real128{}).m_value == 1.));
    REQUIRE((pow(real128{2}, rat_t{3}).m_value == 8.));
    REQUIRE((pow(rat_t{2}, real128{3}).m_value == 8.));
    REQUIRE((pow(real128{2}, rat_t{-3}).m_value == real128{".125"}.m_value));
    REQUIRE((pow(rat_t{2}, real128{-3}).m_value == real128{".125"}.m_value));
    REQUIRE((abs(pow(real128{2}, rat_t{1, 2}) - real128_sqrt2()).m_value < 1E-30));
    REQUIRE((pow(rat_t{1, 2}, real128{2}).m_value == real128{".25"}.m_value));
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE(pow(real128{5}, __int128_t{2}) == 25);
    REQUIRE(pow(__int128_t{2}, real128{5}) == 32);
    REQUIRE(pow(real128{5}, __uint128_t{2}) == 25);
    REQUIRE(pow(__uint128_t{2}, real128{5}) == 32);
#endif
}
