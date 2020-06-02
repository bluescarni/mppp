// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/real128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("real128 logexp")
{
    REQUIRE(exp(real128{}) == 1);
    REQUIRE(abs(exp(real128{1}) - real128_e()) < 1E-32);
    REQUIRE(abs(exp(real128{-1}) - 1 / real128_e()) < 1E-32);
    REQUIRE(abs(exp(real128{2}) - real128_e() * real128_e()) < 1e-32);
    real128 x{2};
    x.exp();
    REQUIRE(x == exp(real128{2}));

    REQUIRE(log(real128{}) == -real128_inf());
    REQUIRE(log(real128{1}) == 0);
    REQUIRE(log(real128{-1}).isnan());
    REQUIRE(abs(log(real128{2}) - real128{"0.693147180559945309417232121458176575"}) < 1e-32);
    x = 2;
    x.log();
    REQUIRE(x == log(real128{2}));

    REQUIRE(log10(real128{}) == -real128_inf());
    REQUIRE(log10(real128{1}) == 0);
    REQUIRE(log10(real128{-1}).isnan());
    REQUIRE(abs(log10(real128{2}) - real128{"0.30102999566398119521373889472449302"}) < 1e-32);
    x = 2;
    x.log10();
    REQUIRE(x == log10(real128{2}));

    REQUIRE(log2(real128{}) == -real128_inf());
    REQUIRE(log2(real128{1}) == 0);
    REQUIRE(log2(real128{-1}).isnan());
    REQUIRE(abs(log2(real128{2}) - real128{1}) < 1e-32);
    x = 2;
    x.log2();
    REQUIRE(x == log2(real128{2}));
}
