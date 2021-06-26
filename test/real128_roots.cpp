// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <type_traits>
#include <utility>

#include <mp++/config.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

using int_t = integer<1>;
using rat_t = rational<1>;

template <typename T, typename U>
using hypot_t = decltype(mppp::hypot(std::declval<const T &>(), std::declval<const U &>()));

TEST_CASE("real128 sqrt")
{
    real128 r;
    REQUIRE(sqrt(r) == 0);
    r.sqrt();
    REQUIRE((r.m_value == 0));
    r = -0.;
    r.sqrt();
    REQUIRE((r.m_value == 0));
    r = 4;
    REQUIRE(sqrt(r) == 2);
    r.sqrt();
    REQUIRE((r.m_value == 2));
    r = 2;
    r.sqrt();
    REQUIRE((abs(real128{"1.41421356237309504880168872420969807856967187537694807317667973799073247"} - r) < 1E-32));
    r = -2;
    r.sqrt();
    REQUIRE(isnan(r));
    r.sqrt().sqrt();
    REQUIRE(isnan(r));
}

TEST_CASE("real128 cbrt")
{
    real128 r;
    r.cbrt();
    REQUIRE((r.m_value == 0));
    r = 8;
    REQUIRE(cbrt(r) == 2);
    r.cbrt();
    REQUIRE((r.m_value == 2));
    r = -8;
    REQUIRE(cbrt(r) == -2);
    r.cbrt();
    REQUIRE((r.m_value == -2));
    r = 2;
    r.cbrt();
    REQUIRE((abs(real128{"1.25992104989487316476721060727822835057025146470150798008197"} - r) < 1E-32));
    r = -2;
    r.cbrt();
    REQUIRE((abs(real128{"-1.25992104989487316476721060727822835057025146470150798008197"} - r) < 1E-32));
    r.cbrt().cbrt();
}

TEST_CASE("real128 hypot")
{
    const auto cmp1 = 3.60555127546398929311922126747049613_rq;
    const auto cmp2 = 3.60555127546398929311922126747049613_rq;

    REQUIRE((std::is_same<real128, decltype(hypot(real128{}, real128{}))>::value));
    REQUIRE((std::is_same<real128, decltype(hypot(real128{}, 0))>::value));
    REQUIRE((std::is_same<real128, decltype(hypot(0., real128{}))>::value));
    REQUIRE((std::is_same<real128, decltype(hypot(int_t{}, real128{}))>::value));
    REQUIRE((std::is_same<real128, decltype(hypot(real128{}, rat_t{}))>::value));
    REQUIRE((hypot(real128{}, real128{}) == 0));
    REQUIRE(abs(hypot(real128{2}, real128{3}) - cmp1) < pow(2_rq, -110));
    REQUIRE(abs(hypot(real128{2}, real128{-3}) - cmp2) < pow(2_rq, -110));
    REQUIRE((hypot(real128{}, 0).m_value == 0));
    REQUIRE((hypot(0.f, real128{}).m_value == 0));
    REQUIRE(abs(hypot(real128{2}, 3ll) - cmp1) < 1E-33);
    REQUIRE(abs(hypot(2u, real128{3}) - cmp1) < 1E-33);
    REQUIRE((hypot(real128{2}, static_cast<signed char>(-3)) - cmp2) < 1E-33);
    REQUIRE(abs(hypot(2., real128{-3}) - cmp2) < 1E-33);
    REQUIRE((hypot(real128{}, int_t{}).m_value == 0));
    REQUIRE((hypot(int_t{}, real128{}).m_value == 0));
    REQUIRE(abs(hypot(real128{2}, int_t{3}) - cmp1) < 1E-33);
    REQUIRE(abs(hypot(int_t{2}, real128{3}) - cmp1) < 1E-33);
    REQUIRE(abs(hypot(real128{2}, int_t{-3}) - cmp2) < 1E-33);
    REQUIRE(abs(hypot(int_t{2}, real128{-3}) - cmp2) < 1E-33);
    REQUIRE((hypot(real128{}, rat_t{}).m_value == 0));
    REQUIRE((hypot(rat_t{}, real128{}).m_value == 0));
    REQUIRE(abs(hypot(real128{2}, rat_t{3}) - cmp1) < 1E-33);
    REQUIRE(abs(hypot(rat_t{2}, real128{3}) - cmp1) < 1E-33);
    REQUIRE((hypot(real128{2}, rat_t{-3}) - cmp2) < 1E-33);
    REQUIRE((hypot(rat_t{2}, real128{-3}) - cmp2) < 1E-33);
    REQUIRE((hypot(rat_t{1, 2}, real128{2}) - cmp2) < 1E-33);
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE(abs(hypot(real128{2}, __int128_t{3}) - cmp1) < 1E-33);
    REQUIRE(abs(hypot(__int128_t{2}, real128{3}) - cmp1) < 1E-33);
    REQUIRE(abs(hypot(real128{2}, __uint128_t{3}) - cmp1) < 1E-33);
    REQUIRE(abs(hypot(__uint128_t{2}, real128{3}) - cmp1) < 1E-33);
#endif
#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    REQUIRE(abs(hypot(real128{2}, 3.l) - cmp1) < 1E-33);
    REQUIRE(abs(hypot(2.l, real128{3}) - cmp1) < 1E-33);
#else
    REQUIRE(!detail::is_detected<hypot_t, real128, long double>::value);
    REQUIRE(!detail::is_detected<hypot_t, long double, real128>::value);
#endif
}
