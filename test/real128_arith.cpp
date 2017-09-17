// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/real128.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

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

#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)

static constexpr real128 test_constexpr_abs()
{
    real128 retval{-5};
    retval.abs();
    return retval;
}

#endif

TEST_CASE("real128 abs")
{
    real128 r;
    REQUIRE(!r.signbit());
    REQUIRE((abs(r).m_value == 0));
    REQUIRE(!(abs(r).signbit()));
    REQUIRE((r.abs().m_value == 0));
    REQUIRE((r.m_value == 0));
    REQUIRE(!r.signbit());
    r = -0.;
    REQUIRE(r.signbit());
    REQUIRE((abs(r).m_value == 0));
    REQUIRE(!(abs(r).signbit()));
    REQUIRE((r.abs().m_value == 0));
    REQUIRE((r.m_value == 0));
    REQUIRE(!r.signbit());
    r = -5;
    REQUIRE((abs(r).m_value == 5));
    REQUIRE((r.abs().m_value == 5));
    REQUIRE((r.m_value == 5));
    r = 5;
    REQUIRE((abs(r).m_value == 5));
    REQUIRE((r.abs().m_value == 5));
    REQUIRE((r.m_value == 5));
    r = -.00005;
    REQUIRE((abs(r).m_value == .00005));
    REQUIRE((r.abs().m_value == .00005));
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
    constexpr auto c0 = abs(real128{-5});
    REQUIRE(c0 == 5);
    constexpr auto c1 = abs(real128{42});
    REQUIRE(c1 == 42);
#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)
    constexpr auto c3 = test_constexpr_abs();
    REQUIRE(c3 == 5);
#endif
}
