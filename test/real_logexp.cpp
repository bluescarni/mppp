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
    r0 = real{4};
    r0.exp2();
    REQUIRE(r0 == 16);
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
    r0 = real{4};
    r0.exp10();
    REQUIRE(r0 == 10000);
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
    r0 = real{4};
    r0.expm1();
    REQUIRE(r0 == exp(real{4}) - 1);
}

TEST_CASE("real log")
{
    real r0{1};
    r0.log();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    real rop;
    r0 = real{1};
    REQUIRE(log(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(log(r0) == 0);
    REQUIRE(log(std::move(r0)) == 0);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real log2")
{
    real r0{1};
    r0.log2();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    real rop;
    r0 = real{1};
    REQUIRE(log2(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(log2(r0) == 0);
    REQUIRE(log2(std::move(r0)) == 0);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    REQUIRE(log2(real{4}) == 2);
    REQUIRE(log2(real{-4}).nan_p());
    r0 = real{4};
    r0.log2();
    REQUIRE(r0 == 2);
}

TEST_CASE("real log10")
{
    real r0{1};
    r0.log10();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    real rop;
    r0 = real{1};
    REQUIRE(log10(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(log10(r0) == 0);
    REQUIRE(log10(std::move(r0)) == 0);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    REQUIRE(log10(real{100}) == 2);
    REQUIRE(log10(real{-100}).nan_p());
    r0 = real{100};
    r0.log10();
    REQUIRE(r0 == 2);
}

TEST_CASE("real log1p")
{
    real r0{0};
    r0.log1p();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    real rop;
    r0 = real{0};
    REQUIRE(log1p(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(log1p(r0) == 0);
    REQUIRE(log1p(std::move(r0)) == 0);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    REQUIRE(log1p(real{99}) == log(real{100}));
    REQUIRE(log1p(real{-99}).nan_p());
    r0 = real{99};
    r0.log1p();
    REQUIRE(r0 == log(real{100}));
}
