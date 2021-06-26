// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
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

TEST_CASE("real128 fma")
{
    real128 x, y, z;
    REQUIRE((fma(x, y, z).m_value == 0));
    x = -2;
    y = 3;
    z = -7;
    REQUIRE((fma(x, y, z).m_value == -13));
    x = "1.18973149535723176508575932662800702e+4932";
    y = 2;
    z = "-1.18973149535723176508575932662800702e+4932";
    REQUIRE((fma(x, y, z).m_value == x.m_value));
}

// Some tests involving constexpr result in an ICE on GCC < 6.
#if (defined(_MSC_VER) || defined(__clang__) || __GNUC__ >= 6) && MPPP_CPLUSPLUS >= 201402L

#define MPPP_ENABLE_CONSTEXPR_TESTS

#endif

#if defined(MPPP_ENABLE_CONSTEXPR_TESTS) && !defined(__INTEL_COMPILER)

static constexpr real128 test_constexpr_abs()
{
    real128 retval{-5};
    retval.abs();
    return retval;
}

static constexpr real128 test_constexpr_abs_ff()
{
    real128 retval{-5};
    return abs(retval);
}

static constexpr real128 test_constexpr_fabs()
{
    real128 retval{-5};
    retval.fabs();
    return retval;
}

static constexpr real128 test_constexpr_fabs_ff()
{
    real128 retval{-5};
    return fabs(retval);
}

#endif

#if defined(__INTEL_COMPILER)
#define MPPP_INTEL_CONSTEXPR const
#else
#define MPPP_INTEL_CONSTEXPR constexpr
#endif

TEST_CASE("real128 abs")
{
    real128 r;
    REQUIRE(!r.signbit());
    REQUIRE((abs(r).m_value == 0));
    REQUIRE(!(abs(r).signbit()));
    REQUIRE((r.abs().m_value == 0));
    REQUIRE((r.fabs().m_value == 0));
    REQUIRE((r.m_value == 0));
    REQUIRE(!r.signbit());
    r = -0.;
    REQUIRE(r.signbit());
    REQUIRE((abs(r).m_value == 0));
    REQUIRE(!(abs(r).signbit()));
    REQUIRE((r.abs().m_value == 0));
    REQUIRE((r.m_value == 0));
    REQUIRE(!r.signbit());
    r = -0.;
    REQUIRE(r.signbit());
    REQUIRE((fabs(r).m_value == 0));
    REQUIRE(!(fabs(r).signbit()));
    REQUIRE((r.fabs().m_value == 0));
    REQUIRE((r.m_value == 0));
    REQUIRE(!r.signbit());
    r = -5;
    REQUIRE((abs(r).m_value == 5));
    REQUIRE((r.abs().m_value == 5));
    REQUIRE((r.m_value == 5));
    r = -5;
    REQUIRE((fabs(r).m_value == 5));
    REQUIRE((r.fabs().m_value == 5));
    REQUIRE((r.m_value == 5));
    r = 5;
    REQUIRE((abs(r).m_value == 5));
    REQUIRE((r.abs().m_value == 5));
    REQUIRE((r.m_value == 5));
    r = 5;
    REQUIRE((fabs(r).m_value == 5));
    REQUIRE((r.fabs().m_value == 5));
    REQUIRE((r.m_value == 5));
    r = -.00005;
    REQUIRE((abs(r).m_value == .00005));
    REQUIRE((r.abs().m_value == .00005));
    REQUIRE((r.m_value == .00005));
    r = -.00005;
    REQUIRE((fabs(r).m_value == .00005));
    REQUIRE((r.fabs().m_value == .00005));
    REQUIRE((r.m_value == .00005));
    r = .00005;
    REQUIRE((abs(r).m_value == .00005));
    REQUIRE((r.abs().m_value == .00005));
    REQUIRE((r.m_value == .00005));
    r = "-inf";
    REQUIRE((abs(r).m_value == real128{"inf"}.m_value));
    REQUIRE((r.abs().m_value == real128{"inf"}.m_value));
    REQUIRE((r.m_value == real128{"inf"}.m_value));
    r = "inf";
    REQUIRE((abs(r).m_value == real128{"inf"}.m_value));
    REQUIRE((r.abs().m_value == real128{"inf"}.m_value));
    REQUIRE((r.m_value == real128{"inf"}.m_value));
    r = "nan";
    REQUIRE(abs(r).isnan());
    REQUIRE(r.abs().isnan());
    r = "-nan";
    REQUIRE(abs(r).isnan());
    REQUIRE(r.abs().isnan());
    MPPP_INTEL_CONSTEXPR auto c0 = abs(real128{-5});
    REQUIRE(c0 == 5);
    MPPP_INTEL_CONSTEXPR auto c1 = abs(real128{42});
    REQUIRE(c1 == 42);
#if defined(MPPP_ENABLE_CONSTEXPR_TESTS) && !defined(__INTEL_COMPILER)
    constexpr auto c3 = test_constexpr_abs();
    REQUIRE(c3 == 5);
    constexpr auto c4 = test_constexpr_fabs();
    REQUIRE(c4 == 5);
    constexpr auto c5 = test_constexpr_abs_ff();
    REQUIRE(c5 == 5);
    constexpr auto c6 = test_constexpr_fabs_ff();
    REQUIRE(c6 == 5);
#endif
}

TEST_CASE("real128 scalb")
{
    real128 r;
    REQUIRE(scalbn(r, 0) == 0);
    REQUIRE(ldexp(r, 0) == 0);
    REQUIRE(scalbn(r, 10) == 0);
    REQUIRE(ldexp(r, 10) == 0);
    REQUIRE(scalbn(r, -10) == 0);
    REQUIRE(ldexp(r, -10) == 0);
    REQUIRE(!scalbn(r, -10).signbit());
    REQUIRE(!ldexp(r, -10).signbit());
    REQUIRE(scalbln(r, 0) == 0);
    REQUIRE(scalbln(r, 10) == 0);
    REQUIRE(scalbln(r, -10) == 0);
    REQUIRE(!scalbln(r, -10).signbit());
    r = -r;
    REQUIRE(scalbn(r, 0) == 0);
    REQUIRE(ldexp(r, 0) == 0);
    REQUIRE(scalbn(r, 10) == 0);
    REQUIRE(ldexp(r, 10) == 0);
    REQUIRE(scalbn(r, -10) == 0);
    REQUIRE(ldexp(r, -10) == 0);
    REQUIRE(scalbn(r, -10).signbit());
    REQUIRE(ldexp(r, -10).signbit());
    REQUIRE(scalbln(r, 0) == 0);
    REQUIRE(scalbln(r, 10) == 0);
    REQUIRE(scalbln(r, -10) == 0);
    REQUIRE(scalbln(r, -10).signbit());
    r = 10;
    REQUIRE(scalbn(r, 0) == 10);
    REQUIRE(ldexp(r, 0) == 10);
    REQUIRE(scalbn(r, 2) == 40);
    REQUIRE(ldexp(r, 2) == 40);
    REQUIRE(scalbln(r, 4) == 160);
    REQUIRE(scalbn(r, -2) == real128{10} / 4);
    REQUIRE(ldexp(r, -2) == real128{10} / 4);
    REQUIRE(scalbn(r, -4) == real128{10} / 16);
    REQUIRE(ldexp(r, -4) == real128{10} / 16);
    // Some non-finite tests.
    REQUIRE(scalbn(real128_inf(), 0) == real128_inf());
    REQUIRE(ldexp(real128_inf(), 0) == real128_inf());
    REQUIRE(scalbn(real128_inf(), -3) == real128_inf());
    REQUIRE(ldexp(real128_inf(), -3) == real128_inf());
    REQUIRE(scalbln(real128_inf(), 3) == real128_inf());
    REQUIRE(scalbn(-real128_inf(), 0) == -real128_inf());
    REQUIRE(ldexp(-real128_inf(), 0) == -real128_inf());
    REQUIRE(scalbn(-real128_inf(), -3) == -real128_inf());
    REQUIRE(ldexp(-real128_inf(), -3) == -real128_inf());
    REQUIRE(scalbln(-real128_inf(), 3) == -real128_inf());
    REQUIRE(scalbn(real128_nan(), 0).isnan());
    REQUIRE(ldexp(real128_nan(), 0).isnan());
    REQUIRE(scalbn(real128_nan(), 1).isnan());
    REQUIRE(ldexp(real128_nan(), 1).isnan());
    REQUIRE(scalbn(real128_nan(), -1).isnan());
    REQUIRE(ldexp(real128_nan(), -1).isnan());
    REQUIRE(scalbn(-real128_nan(), 0).isnan());
    REQUIRE(ldexp(-real128_nan(), 0).isnan());
    REQUIRE(scalbn(-real128_nan(), 1).isnan());
    REQUIRE(ldexp(-real128_nan(), 1).isnan());
    REQUIRE(scalbn(-real128_nan(), -1).isnan());
    REQUIRE(ldexp(-real128_nan(), -1).isnan());
}
