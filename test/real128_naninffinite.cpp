// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cmath>

#include <mp++/real128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

#if defined(__INTEL_COMPILER)
#define MPPP_INTEL_CONSTEXPR const
#else
#define MPPP_INTEL_CONSTEXPR constexpr
#endif

TEST_CASE("real128 naninffinite")
{
    real128 r;
    REQUIRE(r.finite());
    REQUIRE(r.isfinite());
    REQUIRE(!r.isnormal());
    REQUIRE(finite(r));
    REQUIRE(isfinite(r));
    REQUIRE(!isnormal(r));
    REQUIRE(!r.isinf());
    REQUIRE(!isinf(r));
    REQUIRE(!r.isnan());
    REQUIRE(!isnan(r));
    MPPP_INTEL_CONSTEXPR bool c0 = real128{}.isnan();
    REQUIRE(!c0);
    MPPP_INTEL_CONSTEXPR bool c1 = isnan(real128{});
    REQUIRE(!c1);
    MPPP_INTEL_CONSTEXPR int c2 = real128{}.fpclassify();
    REQUIRE(c2 == FP_ZERO);
    REQUIRE(c2 == fpclassify(real128{}));
    MPPP_INTEL_CONSTEXPR bool c3 = isinf(real128{});
    REQUIRE(!c3);
    MPPP_INTEL_CONSTEXPR bool c4 = real128{}.isinf();
    REQUIRE(!c4);
    MPPP_INTEL_CONSTEXPR bool c5 = finite(real128{});
    REQUIRE(c5);
    MPPP_INTEL_CONSTEXPR bool c6 = real128{}.finite();
    REQUIRE(c6);
    r = -1;
    REQUIRE(r.finite());
    REQUIRE(r.isfinite());
    REQUIRE(r.isnormal());
    REQUIRE(finite(r));
    REQUIRE(isfinite(r));
    REQUIRE(isnormal(r));
    REQUIRE(!r.isinf());
    REQUIRE(!isinf(r));
    REQUIRE(!r.isnan());
    REQUIRE(!isnan(r));
    MPPP_INTEL_CONSTEXPR bool d0 = real128{1}.isnan();
    REQUIRE(!d0);
    MPPP_INTEL_CONSTEXPR bool d1 = isnan(real128{1});
    REQUIRE(!d1);
    MPPP_INTEL_CONSTEXPR int d2 = fpclassify(real128{-1});
    REQUIRE(d2 == FP_NORMAL);
    REQUIRE(d2 == real128{-1}.fpclassify());
    MPPP_INTEL_CONSTEXPR bool d3 = isinf(real128{});
    REQUIRE(!d3);
    MPPP_INTEL_CONSTEXPR bool d4 = real128{}.isinf();
    REQUIRE(!d4);
    MPPP_INTEL_CONSTEXPR bool d5 = finite(real128{});
    REQUIRE(d5);
    MPPP_INTEL_CONSTEXPR bool d6 = real128{}.finite();
    REQUIRE(d6);
    r = 123;
    REQUIRE(r.finite());
    REQUIRE(r.isfinite());
    REQUIRE(r.isnormal());
    REQUIRE(finite(r));
    REQUIRE(isfinite(r));
    REQUIRE(isnormal(r));
    REQUIRE(!r.isinf());
    REQUIRE(!isinf(r));
    REQUIRE(!r.isnan());
    REQUIRE(!isnan(r));
    r = "inf";
    REQUIRE(!r.finite());
    REQUIRE(!r.isfinite());
    REQUIRE(!r.isnormal());
    REQUIRE(!finite(r));
    REQUIRE(!isfinite(r));
    REQUIRE(!isnormal(r));
    REQUIRE(r.isinf());
    REQUIRE(isinf(r));
    REQUIRE(!r.isnan());
    REQUIRE(!isnan(r));
    MPPP_INTEL_CONSTEXPR bool e0 = real128_inf().isnan();
    REQUIRE(!e0);
    MPPP_INTEL_CONSTEXPR bool e1 = isnan(real128_inf());
    REQUIRE(!e1);
    MPPP_INTEL_CONSTEXPR int e2 = fpclassify(real128_inf());
    REQUIRE(e2 == FP_INFINITE);
    REQUIRE(e2 == real128{"inf"}.fpclassify());
    MPPP_INTEL_CONSTEXPR bool e3 = isinf(real128_inf());
    REQUIRE(e3);
    MPPP_INTEL_CONSTEXPR bool e4 = real128_inf().isinf();
    REQUIRE(e4);
    MPPP_INTEL_CONSTEXPR bool e5 = finite(real128_inf());
    REQUIRE(!e5);
    MPPP_INTEL_CONSTEXPR bool e6 = real128_inf().finite();
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
    REQUIRE(!r.isfinite());
    REQUIRE(!r.isnormal());
    REQUIRE(!finite(r));
    REQUIRE(!isfinite(r));
    REQUIRE(!isnormal(r));
    REQUIRE(!r.isinf());
    REQUIRE(!isinf(r));
    REQUIRE(r.isnan());
    REQUIRE(isnan(r));
    MPPP_INTEL_CONSTEXPR bool f0 = real128_nan().isnan();
    REQUIRE(f0);
    MPPP_INTEL_CONSTEXPR bool f1 = isnan(real128_nan());
    REQUIRE(f1);
    MPPP_INTEL_CONSTEXPR int f2 = fpclassify(real128_nan());
    REQUIRE(f2 == FP_NAN);
    REQUIRE(f2 == real128{"-nan"}.fpclassify());
    MPPP_INTEL_CONSTEXPR bool f3 = isinf(real128_nan());
    REQUIRE(!f3);
    MPPP_INTEL_CONSTEXPR bool f4 = real128_nan().isinf();
    REQUIRE(!f4);
    MPPP_INTEL_CONSTEXPR bool f5 = finite(real128_nan());
    REQUIRE(!f5);
    MPPP_INTEL_CONSTEXPR bool f6 = real128_nan().finite();
    REQUIRE(!f6);
    // Subnormals.
    REQUIRE(fpclassify(real128{"1E-4940"}) == FP_SUBNORMAL);
    REQUIRE(fpclassify(real128{"-1E-4940"}) == FP_SUBNORMAL);
    REQUIRE(real128{"1E-4940"}.finite());
    REQUIRE(real128{"1E-4940"}.isfinite());
    REQUIRE(!real128{"1E-4940"}.isnormal());
    REQUIRE(finite(real128{"1E-4940"}));
    REQUIRE(isfinite(real128{"1E-4940"}));
    REQUIRE(!isnormal(real128{"1E-4940"}));
    // Large but not infinite.
    REQUIRE(fpclassify(real128{"1E4930"}) == FP_NORMAL);
    REQUIRE(fpclassify(-real128{"1E4930"}) == FP_NORMAL);
    REQUIRE(real128{"1E4930"}.finite());
    REQUIRE(real128{"1E4930"}.isfinite());
    REQUIRE(real128{"1E4930"}.isnormal());
    REQUIRE(finite(real128{"1E4930"}));
    REQUIRE(isfinite(real128{"1E4930"}));
    REQUIRE(isnormal(real128{"1E4930"}));
    // Small but not subnormal.
    REQUIRE(fpclassify(real128{"1E-4931"}) == FP_NORMAL);
    REQUIRE(fpclassify(-real128{"1E-4931"}) == FP_NORMAL);
}
