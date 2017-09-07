// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cmath>
#include <quadmath.h>

#include <mp++/mp++.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

TEST_CASE("real128 naninffinite")
{
    real128 r;
    REQUIRE(r.finite());
    REQUIRE(finite(r));
    REQUIRE(!r.isinf());
    REQUIRE(!isinf(r));
    REQUIRE(!r.isnan());
    REQUIRE(!isnan(r));
    constexpr bool c0 = real128{}.isnan();
    REQUIRE(!c0);
    constexpr bool c1 = isnan(real128{});
    REQUIRE(!c1);
    constexpr int c2 = real128{}.fpclassify();
    REQUIRE(c2 == FP_ZERO);
    REQUIRE(c2 == fpclassify(real128{}));
    constexpr bool c3 = isinf(real128{});
    REQUIRE(!c3);
    constexpr bool c4 = real128{}.isinf();
    REQUIRE(!c4);
    constexpr bool c5 = finite(real128{});
    REQUIRE(c5);
    constexpr bool c6 = real128{}.finite();
    REQUIRE(c6);
    r = -1;
    REQUIRE(r.finite());
    REQUIRE(finite(r));
    REQUIRE(!r.isinf());
    REQUIRE(!isinf(r));
    REQUIRE(!r.isnan());
    REQUIRE(!isnan(r));
    constexpr bool d0 = real128{1}.isnan();
    REQUIRE(!d0);
    constexpr bool d1 = isnan(real128{1});
    REQUIRE(!d1);
    constexpr int d2 = fpclassify(real128{-1});
    REQUIRE(d2 == FP_NORMAL);
    REQUIRE(d2 == real128{-1}.fpclassify());
    constexpr bool d3 = isinf(real128{});
    REQUIRE(!d3);
    constexpr bool d4 = real128{}.isinf();
    REQUIRE(!d4);
    constexpr bool d5 = finite(real128{});
    REQUIRE(d5);
    constexpr bool d6 = real128{}.finite();
    REQUIRE(d6);
    r = 123;
    REQUIRE(r.finite());
    REQUIRE(finite(r));
    REQUIRE(!r.isinf());
    REQUIRE(!isinf(r));
    REQUIRE(!r.isnan());
    REQUIRE(!isnan(r));
    r = "inf";
    REQUIRE(!r.finite());
    REQUIRE(!finite(r));
    REQUIRE(r.isinf());
    REQUIRE(isinf(r));
    REQUIRE(!r.isnan());
    REQUIRE(!isnan(r));
    constexpr bool e0 = real128_inf().isnan();
    REQUIRE(!e0);
    constexpr bool e1 = isnan(real128_inf());
    REQUIRE(!e1);
    constexpr int e2 = fpclassify(real128_inf());
    REQUIRE(e2 == FP_INFINITE);
    REQUIRE(e2 == real128{"inf"}.fpclassify());
    constexpr bool e3 = isinf(real128_inf());
    REQUIRE(e3);
    constexpr bool e4 = real128_inf().isinf();
    REQUIRE(e4);
    constexpr bool e5 = finite(real128_inf());
    REQUIRE(!e5);
    constexpr bool e6 = real128_inf().finite();
    REQUIRE(!e6);
    r = "-inf";
    REQUIRE(!r.finite());
    REQUIRE(!finite(r));
    REQUIRE(r.isinf());
    REQUIRE(isinf(r));
    REQUIRE(!r.isnan());
    REQUIRE(!isnan(r));
    r = "nan";
    REQUIRE(!r.finite());
    REQUIRE(!finite(r));
    REQUIRE(!r.isinf());
    REQUIRE(!isinf(r));
    REQUIRE(r.isnan());
    REQUIRE(isnan(r));
    constexpr bool f0 = real128_nan().isnan();
    REQUIRE(f0);
    constexpr bool f1 = isnan(real128_nan());
    REQUIRE(f1);
    constexpr int f2 = fpclassify(real128_nan());
    REQUIRE(f2 == FP_NAN);
    REQUIRE(f2 == real128{"-nan"}.fpclassify());
    constexpr bool f3 = isinf(real128_nan());
    REQUIRE(!f3);
    constexpr bool f4 = real128_nan().isinf();
    REQUIRE(!f4);
    constexpr bool f5 = finite(real128_nan());
    REQUIRE(!f5);
    constexpr bool f6 = real128_nan().finite();
    REQUIRE(!f6);
    // Subnormals.
    REQUIRE(fpclassify(real128{"1E-4940"}) == FP_SUBNORMAL);
}
