// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>
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

#if defined(MPPP_QUADMATH_HAVE_EXP2Q)
    // exp2.
    REQUIRE(exp2(real128{}) == 1);
    REQUIRE(abs(exp2(real128{1}) - 2) < 1E-32);
    REQUIRE(abs(exp2(real128{-1}) - 1_rq / 2) < 1E-32);
    REQUIRE(abs(exp2(real128{2}) - 2 * 2) < 1e-32);
    x = 2;
    x.exp2();
    REQUIRE(x == exp2(real128{2}));
#endif

    // expm1.
    REQUIRE(expm1(real128{}) == 0);
    REQUIRE(abs(expm1(real128{1}) - real128_e() + 1) < 1E-32);
    REQUIRE(abs(expm1(real128{-1}) + 1 - 1 / real128_e()) < 1E-32);
    REQUIRE(abs(expm1(real128{2}) + 1 - (real128_e() * real128_e())) < 1e-32);
    x = 2;
    x.expm1();
    REQUIRE(x == expm1(real128{2}));

    // log1p.
    REQUIRE(log1p(real128{}) == 0);
    REQUIRE(log1p(real128{1}) == log(2_rq));
    REQUIRE(log1p(real128{-2}).isnan());
    REQUIRE(abs(log1p(real128{2}) - 1.09861228866810969139524523692252561_rq) < 1e-32);
    x = 2;
    x.log1p();
    REQUIRE(x == log1p(real128{2}));
}
