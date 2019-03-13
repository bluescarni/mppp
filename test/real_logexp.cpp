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
    REQUIRE(r0.get_prec() == real_deduce_precision(0));
    REQUIRE(r0 == 1);
    real rop;
    r0 = real{0};
    REQUIRE(exp(rop, r0) == 1);
    REQUIRE(rop.get_prec() == real_deduce_precision(0));
    REQUIRE(exp(r0) == 1);
    REQUIRE(exp(std::move(r0)) == 1);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}
