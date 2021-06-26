// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <type_traits>

#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("real128 ceil")
{
    REQUIRE(ceil(1_rq) == 1);
    REQUIRE(ceil(1.1_rq) == 2);
    REQUIRE(ceil(-1_rq) == -1);
    REQUIRE(ceil(-1.1_rq) == -1);

    auto r = 1_rq;
    REQUIRE(r.ceil() == 1);
    r = 1.1_rq;
    REQUIRE(r.ceil() == 2);
    r = -1_rq;
    REQUIRE(r.ceil() == -1);
    r = -1.1_rq;
    REQUIRE(r.ceil() == -1);
}

TEST_CASE("real128 floor")
{
    REQUIRE(floor(1_rq) == 1);
    REQUIRE(floor(1.1_rq) == 1);
    REQUIRE(floor(-1_rq) == -1);
    REQUIRE(floor(-1.1_rq) == -2);

    auto r = 1_rq;
    REQUIRE(r.floor() == 1);
    r = 1.1_rq;
    REQUIRE(r.floor() == 1);
    r = -1_rq;
    REQUIRE(r.floor() == -1);
    r = -1.1_rq;
    REQUIRE(r.floor() == -2);
}

TEST_CASE("real128 nearbyint")
{
    REQUIRE(nearbyint(1_rq) == 1);
    REQUIRE(nearbyint(1.1_rq) == 1);
    REQUIRE(nearbyint(1.5_rq) == 2);
    REQUIRE(nearbyint(2.5_rq) == 2);
    REQUIRE(nearbyint(-1_rq) == -1);
    REQUIRE(nearbyint(-1.1_rq) == -1);
    REQUIRE(nearbyint(-1.5_rq) == -2);
    REQUIRE(nearbyint(-2.5_rq) == -2);

    auto r = 1_rq;
    REQUIRE(r.nearbyint() == 1);
    r = 1.1_rq;
    REQUIRE(r.nearbyint() == 1);
    r = 1.5_rq;
    REQUIRE(r.nearbyint() == 2);
    r = 2.5_rq;
    REQUIRE(r.nearbyint() == 2);
    r = -1_rq;
    REQUIRE(r.nearbyint() == -1);
    r = -1.1_rq;
    REQUIRE(r.nearbyint() == -1);
    r = -1.5_rq;
    REQUIRE(r.nearbyint() == -2);
    r = -2.5_rq;
    REQUIRE(r.nearbyint() == -2);
}

TEST_CASE("real128 rint")
{
    REQUIRE(rint(1_rq) == 1);
    REQUIRE(rint(1.1_rq) == 1);
    REQUIRE(rint(1.5_rq) == 2);
    REQUIRE(rint(2.5_rq) == 2);
    REQUIRE(rint(-1_rq) == -1);
    REQUIRE(rint(-1.1_rq) == -1);
    REQUIRE(rint(-1.5_rq) == -2);
    REQUIRE(rint(-2.5_rq) == -2);

    auto r = 1_rq;
    REQUIRE(r.rint() == 1);
    r = 1.1_rq;
    REQUIRE(r.rint() == 1);
    r = 1.5_rq;
    REQUIRE(r.rint() == 2);
    r = 2.5_rq;
    REQUIRE(r.rint() == 2);
    r = -1_rq;
    REQUIRE(r.rint() == -1);
    r = -1.1_rq;
    REQUIRE(r.rint() == -1);
    r = -1.5_rq;
    REQUIRE(r.rint() == -2);
    r = -2.5_rq;
    REQUIRE(r.rint() == -2);
}

TEST_CASE("real128 round")
{
    REQUIRE(round(1_rq) == 1);
    REQUIRE(round(1.1_rq) == 1);
    REQUIRE(round(1.5_rq) == 2);
    REQUIRE(round(2.5_rq) == 3);
    REQUIRE(round(-1_rq) == -1);
    REQUIRE(round(-1.1_rq) == -1);
    REQUIRE(round(-1.5_rq) == -2);
    REQUIRE(round(-2.5_rq) == -3);

    auto r = 1_rq;
    REQUIRE(r.round() == 1);
    r = 1.1_rq;
    REQUIRE(r.round() == 1);
    r = 1.5_rq;
    REQUIRE(r.round() == 2);
    r = 2.5_rq;
    REQUIRE(r.round() == 3);
    r = -1_rq;
    REQUIRE(r.round() == -1);
    r = -1.1_rq;
    REQUIRE(r.round() == -1);
    r = -1.5_rq;
    REQUIRE(r.round() == -2);
    r = -2.5_rq;
    REQUIRE(r.round() == -3);
}

TEST_CASE("real128 trunc")
{
    REQUIRE(trunc(1_rq) == 1);
    REQUIRE(trunc(1.1_rq) == 1);
    REQUIRE(trunc(1.5_rq) == 1);
    REQUIRE(trunc(2.5_rq) == 2);
    REQUIRE(trunc(-1_rq) == -1);
    REQUIRE(trunc(-1.1_rq) == -1);
    REQUIRE(trunc(-1.5_rq) == -1);
    REQUIRE(trunc(-2.5_rq) == -2);

    auto r = 1_rq;
    REQUIRE(r.trunc() == 1);
    r = 1.1_rq;
    REQUIRE(r.trunc() == 1);
    r = 1.5_rq;
    REQUIRE(r.trunc() == 1);
    r = 2.5_rq;
    REQUIRE(r.trunc() == 2);
    r = -1_rq;
    REQUIRE(r.trunc() == -1);
    r = -1.1_rq;
    REQUIRE(r.trunc() == -1);
    r = -1.5_rq;
    REQUIRE(r.trunc() == -1);
    r = -2.5_rq;
    REQUIRE(r.trunc() == -2);
}

TEST_CASE("real128 llrint")
{
    REQUIRE(std::is_same<long long, decltype(llrint(1.1_rq))>::value);
    REQUIRE(llrint(1.1_rq) == 1);
    REQUIRE(llrint(1.5_rq) == 2);
    REQUIRE(llrint(-1.1_rq) == -1);
    REQUIRE(llrint(-1.5_rq) == -2);
    REQUIRE(llrint(2.1_rq) == 2);
    REQUIRE(llrint(2.5_rq) == 2);
    REQUIRE(llrint(-2.1_rq) == -2);
    REQUIRE(llrint(-2.5_rq) == -2);
}

TEST_CASE("real128 lrint")
{
    REQUIRE(std::is_same<long, decltype(lrint(1.1_rq))>::value);
    REQUIRE(lrint(1.1_rq) == 1);
    REQUIRE(lrint(1.5_rq) == 2);
    REQUIRE(lrint(-1.1_rq) == -1);
    REQUIRE(lrint(-1.5_rq) == -2);
    REQUIRE(lrint(2.1_rq) == 2);
    REQUIRE(lrint(2.5_rq) == 2);
    REQUIRE(lrint(-2.1_rq) == -2);
    REQUIRE(lrint(-2.5_rq) == -2);
}

TEST_CASE("real128 llround")
{
    REQUIRE(std::is_same<long long, decltype(llround(1.1_rq))>::value);
    REQUIRE(llround(1.1_rq) == 1);
    REQUIRE(llround(1.5_rq) == 2);
    REQUIRE(llround(-1.1_rq) == -1);
    REQUIRE(llround(-1.5_rq) == -2);
    REQUIRE(llround(2.1_rq) == 2);
    REQUIRE(llround(2.5_rq) == 3);
    REQUIRE(llround(-2.1_rq) == -2);
    REQUIRE(llround(-2.5_rq) == -3);
}

TEST_CASE("real128 lround")
{
    REQUIRE(std::is_same<long, decltype(lround(1.1_rq))>::value);
    REQUIRE(lround(1.1_rq) == 1);
    REQUIRE(lround(1.5_rq) == 2);
    REQUIRE(lround(-1.1_rq) == -1);
    REQUIRE(lround(-1.5_rq) == -2);
    REQUIRE(lround(2.1_rq) == 2);
    REQUIRE(lround(2.5_rq) == 3);
    REQUIRE(lround(-2.1_rq) == -2);
    REQUIRE(lround(-2.5_rq) == -3);
}

TEST_CASE("real128 fmod")
{
    REQUIRE(fmod(3_rq, 2.5_rq) == .5_rq);
    REQUIRE(fmod(-3_rq, -2.5_rq) == -.5_rq);
    REQUIRE(fmod(-1_rq, 2_rq) == -1);
    REQUIRE(fmod(1_rq, 2_rq) == 1);

    // Couple of small tests with non-real128 args.
    REQUIRE(fmod(3, 2.5_rq) == .5_rq);
    REQUIRE(fmod(-3., -2.5_rq) == -.5_rq);
    REQUIRE(fmod(-1_rq, 2_z1) == -1);
    REQUIRE(fmod(1_rq, 2_q1) == 1);
}

TEST_CASE("real128 remainder")
{
    REQUIRE(remainder(3_rq, 2.5_rq) == .5_rq);
    REQUIRE(remainder(4_rq, 1.5_rq) == -.5_rq);
    REQUIRE(remainder(-3_rq, -2.5_rq) == -.5_rq);
    REQUIRE(remainder(-1_rq, 2_rq) == -1);
    REQUIRE(remainder(1_rq, 2_rq) == 1);

    // Couple of small tests with non-real128 args.
    REQUIRE(remainder(3, 2.5_rq) == .5_rq);
    REQUIRE(remainder(4, 1.5_rq) == -.5_rq);
    REQUIRE(remainder(-3., -2.5_rq) == -.5_rq);
    REQUIRE(remainder(-1_rq, 2_z1) == -1);
    REQUIRE(remainder(1_rq, 2_q1) == 1);
    REQUIRE(remainder(10_rq, 3_q1) == 1);
}

TEST_CASE("real128 modf")
{
    real128 out;

    REQUIRE(modf(1.25_rq, &out) == .25_rq);
    REQUIRE(out == 1);

    REQUIRE(modf(-1.25_rq, &out) == -.25_rq);
    REQUIRE(out == -1);

    // Try overlap.
    out = 1.25_rq;
    REQUIRE(modf(out, &out) == .25_rq);
    REQUIRE(out == 1);
}

TEST_CASE("real128 remquo")
{
    int quo = 0;

    REQUIRE(remquo(3_rq, 2.5_rq, &quo) == .5_rq);
    REQUIRE(remquo(4_rq, 1.5_rq, &quo) == -.5_rq);
    REQUIRE(remquo(-3_rq, -2.5_rq, &quo) == -.5_rq);
    REQUIRE(remquo(-1_rq, 2_rq, &quo) == -1);
    REQUIRE(remquo(1_rq, 2_rq, &quo) == 1);
}
