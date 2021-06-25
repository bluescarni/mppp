// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
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
using atan2_t = decltype(mppp::atan2(std::declval<const T &>(), std::declval<const U &>()));

TEST_CASE("real128 sincos")
{
    REQUIRE(cos(real128{}) == 1);
    REQUIRE(sin(real128{}) == 0);
    real128 x;
    REQUIRE(x.cos() == 1);
    REQUIRE(x.sin() != 0);
    x = 0;
    REQUIRE(x.sin() == 0);
    REQUIRE(abs(sin(real128{"1.234"}) - real128{"0.943818209374633704861751006156827573"}) < 1E-34);
    REQUIRE(abs(cos(real128{"1.234"}) - real128{"0.330465108071729857403280772789927239"}) < 1E-34);
}

TEST_CASE("real128 tan")
{
    REQUIRE(tan(real128{}) == 0);
    real128 x;
    REQUIRE(x.tan() == 0);
    x = 0;
    REQUIRE(x.tan() == 0);
    REQUIRE(abs(tan(real128{"1.234"}) - real128{"2.85602983891954817746307080725818826776"}) < 1E-33);
}

TEST_CASE("real128 inversefunctions")
{
    {
        REQUIRE(acos(real128{}) == real128_pi() / 2.);
        real128 x;
        REQUIRE(x.acos() == real128_pi() / 2.);
        x = 0;
        REQUIRE(x.acos() == real128_pi() / 2.);
        REQUIRE(abs(acos(cos(real128{"0.234"})) - real128{"0.234"}) < 1E-33);
    }
    {
        REQUIRE(asin(real128{}) == 0.);
        real128 x;
        REQUIRE(x.asin() == 0.);
        x = 0;
        REQUIRE(x.asin() == 0.);
        REQUIRE(abs(asin(sin(real128{"0.234"})) - real128{"0.234"}) < 1E-33);
    }
    {
        REQUIRE(atan(real128{}) == 0.);
        real128 x;
        REQUIRE(x.atan() == 0.);
        x = 0;
        REQUIRE(x.atan() == 0.);
        REQUIRE(abs(atan(tan(real128{"0.234"})) - real128{"0.234"}) < 1E-33);
    }
}

TEST_CASE("real128 atan2")
{
    const auto cmp1 = 0.588002603547567551245611080625085457_rq;
    const auto cmp2 = 2.55359005004222568721703230265441744_rq;

    REQUIRE((std::is_same<real128, decltype(atan2(real128{}, real128{}))>::value));
    REQUIRE((std::is_same<real128, decltype(atan2(real128{}, 0))>::value));
    REQUIRE((std::is_same<real128, decltype(atan2(0., real128{}))>::value));
    REQUIRE((std::is_same<real128, decltype(atan2(int_t{}, real128{}))>::value));
    REQUIRE((std::is_same<real128, decltype(atan2(real128{}, rat_t{}))>::value));
    REQUIRE((atan2(real128{}, real128{}) == 0));
    REQUIRE(abs(atan2(real128{2}, real128{3}) - cmp1) < pow(2_rq, -110));
    REQUIRE(abs(atan2(real128{2}, real128{-3}) - cmp2) < pow(2_rq, -110));
    REQUIRE((atan2(real128{}, 0).m_value == 0));
    REQUIRE((atan2(0.f, real128{}).m_value == 0));
    REQUIRE(abs(atan2(real128{2}, 3ll) - cmp1) < 1E-33);
    REQUIRE(abs(atan2(2u, real128{3}) - cmp1) < 1E-33);
    REQUIRE((atan2(real128{2}, static_cast<signed char>(-3)) - cmp2) < 1E-33);
    REQUIRE(abs(atan2(2., real128{-3}) - cmp2) < 1E-33);
    REQUIRE((atan2(real128{}, int_t{}).m_value == 0));
    REQUIRE((atan2(int_t{}, real128{}).m_value == 0));
    REQUIRE(abs(atan2(real128{2}, int_t{3}) - cmp1) < 1E-33);
    REQUIRE(abs(atan2(int_t{2}, real128{3}) - cmp1) < 1E-33);
    REQUIRE(abs(atan2(real128{2}, int_t{-3}) - cmp2) < 1E-33);
    REQUIRE(abs(atan2(int_t{2}, real128{-3}) - cmp2) < 1E-33);
    REQUIRE((atan2(real128{}, rat_t{}).m_value == 0));
    REQUIRE((atan2(rat_t{}, real128{}).m_value == 0));
    REQUIRE(abs(atan2(real128{2}, rat_t{3}) - cmp1) < 1E-33);
    REQUIRE(abs(atan2(rat_t{2}, real128{3}) - cmp1) < 1E-33);
    REQUIRE((atan2(real128{2}, rat_t{-3}) - cmp2) < 1E-33);
    REQUIRE((atan2(rat_t{2}, real128{-3}) - cmp2) < 1E-33);
    REQUIRE((atan2(rat_t{1, 2}, real128{2}) - cmp2) < 1E-33);
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE(abs(atan2(real128{2}, __int128_t{3}) - cmp1) < 1E-33);
    REQUIRE(abs(atan2(__int128_t{2}, real128{3}) - cmp1) < 1E-33);
    REQUIRE(abs(atan2(real128{2}, __uint128_t{3}) - cmp1) < 1E-33);
    REQUIRE(abs(atan2(__uint128_t{2}, real128{3}) - cmp1) < 1E-33);
#endif
#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    REQUIRE(abs(atan2(real128{2}, 3.l) - cmp1) < 1E-33);
    REQUIRE(abs(atan2(2.l, real128{3}) - cmp1) < 1E-33);
#else
    REQUIRE(!detail::is_detected<atan2_t, real128, long double>::value);
    REQUIRE(!detail::is_detected<atan2_t, long double, real128>::value);
#endif
}

TEST_CASE("real128 sincos sim")
{
    real128 s, c;
    sincos(1.2_rq, &s, &c);

    REQUIRE(s == sin(1.2_rq));
    REQUIRE(c == cos(1.2_rq));
}
