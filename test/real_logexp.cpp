// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <mp++/real.hpp>
#include <utility>

using namespace mppp;

TEST_CASE("real exp")
{
    real r0{0};
    r0.exp();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    real rop;
    r0 = real{0};
    REQUIRE(exp(rop, r0) == 1);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(exp(r0) == 1);
    REQUIRE(exp(std::move(r0)) == 1);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real exp2")
{
    real r0{0};
    r0.exp2();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    real rop;
    r0 = real{0};
    REQUIRE(exp2(rop, r0) == 1);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(exp2(r0) == 1);
    REQUIRE(exp2(std::move(r0)) == 1);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    REQUIRE(exp2(real{4}) == 16);
    REQUIRE(exp2(real{-4}) == 1 / exp2(real{4}));
}

TEST_CASE("real exp10")
{
    real r0{0};
    r0.exp10();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    real rop;
    r0 = real{0};
    REQUIRE(exp10(rop, r0) == 1);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(exp10(r0) == 1);
    REQUIRE(exp10(std::move(r0)) == 1);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    REQUIRE(exp10(real{4}) == 10000);
    REQUIRE(exp10(real{-4}) == 1 / exp10(real{4}));
}

TEST_CASE("real expm1")
{
    real r0{0};
    r0.expm1();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    real rop;
    r0 = real{0};
    REQUIRE(expm1(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(expm1(r0) == 0);
    REQUIRE(expm1(std::move(r0)) == 0);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    REQUIRE(expm1(real{4}) == exp(real{4}) - 1);
    REQUIRE(expm1(real{-4}) == exp(real{-4}) - 1);
}
