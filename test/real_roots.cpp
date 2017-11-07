// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
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

TEST_CASE("real sqrt")
{
    real r0{0};
    r0.sqrt();
    REQUIRE(r0.get_prec() == real_deduce_precision(0));
    REQUIRE(r0.zero_p());
    real rop;
    REQUIRE(sqrt(rop, r0).zero_p());
    REQUIRE(rop.get_prec() == real_deduce_precision(0));
    REQUIRE(sqrt(r0).zero_p());
    REQUIRE(sqrt(std::move(r0)).zero_p());
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}
