// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <mp++/detail/mpfr.hpp>
#include <mp++/real.hpp>
#include <utility>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

TEST_CASE("real arith nary rop")
{
    real r1, r2, r3;
    auto p = std::make_pair(true, r2.get_prec());
    mpfr_examine_precs(p, r1, r3);
    REQUIRE(p.first);
    REQUIRE(p.second == r1.get_prec());
    r1.set_prec(r1.get_prec() + 1);
    p = std::make_pair(r1.get_prec() == r2.get_prec(), r2.get_prec());
    mpfr_examine_precs(p, r1, r3);
    REQUIRE(!p.first);
    REQUIRE(p.second == r2.get_prec());
    r2.set_prec(r2.get_prec() + 1);
    p = std::make_pair(r1.get_prec() == r2.get_prec(), r2.get_prec());
    mpfr_examine_precs(p, r1, r3);
    REQUIRE(!p.first);
    REQUIRE(p.second == r3.get_prec() + 1);
    r3.set_prec(r3.get_prec() + 1);
    p = std::make_pair(r1.get_prec() == r2.get_prec(), r2.get_prec());
    mpfr_examine_precs(p, r1, r3);
    REQUIRE(p.first);
    REQUIRE(p.second == r1.get_prec());
    r3.set_prec(r3.get_prec() + 1);
    p = std::make_pair(r1.get_prec() == r2.get_prec(), r2.get_prec());
    mpfr_examine_precs(p, r1, r3);
    REQUIRE(!p.first);
    REQUIRE(p.second == r3.get_prec());
    r2.set_prec(r3.get_prec() + 2);
    p = std::make_pair(r1.get_prec() == r2.get_prec(), r2.get_prec());
    mpfr_examine_precs(p, r1, r3);
    REQUIRE(!p.first);
    REQUIRE(p.second == r2.get_prec());
    // Try with only 1 operand as well.
    r1 = real{};
    r2 = real{};
    p = std::make_pair(true, r2.get_prec());
    mpfr_examine_precs(p, r1);
    REQUIRE(p.first);
    REQUIRE(p.second == r1.get_prec());
    r1.set_prec(r1.get_prec() + 1);
    p = std::make_pair(r1.get_prec() == r2.get_prec(), r2.get_prec());
    mpfr_examine_precs(p, r1);
    REQUIRE(!p.first);
    REQUIRE(p.second == r2.get_prec());
    r2.set_prec(r2.get_prec() + 1);
    p = std::make_pair(r1.get_prec() == r2.get_prec(), r2.get_prec());
    mpfr_examine_precs(p, r1);
    REQUIRE(p.first);
    REQUIRE(p.second == r2.get_prec());
}