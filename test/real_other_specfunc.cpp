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

TEST_CASE("real eint")
{
    real r0{1};
    r0.eint();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(r0 - 1.89511781) < 1E-5);
    real rop;
    r0 = real{1};
    REQUIRE(abs(eint(rop, r0) - 1.89511781) < 1E-5);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(eint(r0) - 1.89511781) < 1E-5);
    REQUIRE(abs(eint(std::move(r0)) - 1.89511781) < 1E-5);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real li2")
{
    real r0{-1};
    r0.li2();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(r0 + 0.8224670334241132) < 1E-5);
    real rop;
    r0 = real{-1};
    REQUIRE(abs(li2(rop, r0) + 0.8224670334241132) < 1E-5);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(li2(r0) + 0.8224670334241132) < 1E-5);
    REQUIRE(abs(li2(std::move(r0)) + 0.8224670334241132) < 1E-5);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real zeta")
{
    real r0{-1};
    r0.zeta();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(r0 + 1. / 12) < 1E-5);
    real rop;
    r0 = real{-1};
    REQUIRE(abs(zeta(rop, r0) + 1. / 12) < 1E-5);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(zeta(r0) + 1. / 12) < 1E-5);
    REQUIRE(abs(zeta(std::move(r0)) + 1. / 12) < 1E-5);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real erf")
{
    real r0{1};
    r0.erf();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(r0 - 0.84270079295) < 1E-5);
    real rop;
    r0 = real{1};
    REQUIRE(abs(erf(rop, r0) - 0.84270079295) < 1E-5);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(erf(r0) - 0.84270079295) < 1E-5);
    REQUIRE(abs(erf(std::move(r0)) - 0.84270079295) < 1E-5);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real erfc")
{
    real r0{1};
    r0.erfc();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(r0 - 0.15729920705) < 1E-5);
    real rop;
    r0 = real{1};
    REQUIRE(abs(erfc(rop, r0) - 0.15729920705) < 1E-5);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(erfc(r0) - 0.15729920705) < 1E-5);
    REQUIRE(abs(erfc(std::move(r0)) - 0.15729920705) < 1E-5);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}
