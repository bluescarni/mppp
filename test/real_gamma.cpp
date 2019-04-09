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

#include "test_utils.hpp"

using namespace mppp;

TEST_CASE("real gamma")
{
    real r0{1};
    r0.gamma();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    real rop;
    r0 = real{1};
    REQUIRE(gamma(rop, r0) == 1);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(gamma(r0) == 1);
    REQUIRE(gamma(std::move(r0)) == 1);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real lgamma")
{
    real r0{1};
    r0.lgamma();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    real rop;
    r0 = real{1};
    REQUIRE(lgamma(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(lgamma(r0) == 0);
    REQUIRE(lgamma(std::move(r0)) == 0);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}
