// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/mp++.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

TEST_CASE("real128 constants")
{
    constexpr auto sig_digits = real128_sig_digits();
    REQUIRE(sig_digits == 113u);
    REQUIRE(real128_sig_digits() == 113u);
    constexpr auto pi = real128_pi();
    REQUIRE((pi.m_value == real128{"3.14159265358979323846264338327950280"}.m_value));
    constexpr auto e = real128_e();
    REQUIRE((e.m_value == real128{"2.71828182845904523536028747135266231e+00"}.m_value));
    constexpr auto sqrt2 = real128_sqrt2();
    REQUIRE((sqrt2.m_value == real128{"1.41421356237309504880168872420969798"}.m_value));
    constexpr auto inf = real128_inf();
    REQUIRE((inf.m_value == real128{"inf"}.m_value));
    constexpr auto minf = -real128_inf();
    REQUIRE((minf.m_value == real128{"-inf"}.m_value));
    constexpr auto nan = real128_nan();
    REQUIRE(nan.isnan());
    constexpr auto mnan = -real128_nan();
    REQUIRE(mnan.isnan());
#if MPPP_CPLUSPLUS >= 201703L
    REQUIRE(sig_digits_128 == 113u);
    REQUIRE((pi_128.m_value == real128_pi().m_value));
    REQUIRE((e_128.m_value == real128_e().m_value));
    REQUIRE((sqrt2_128.m_value == real128_sqrt2().m_value));
    REQUIRE((inf_128.m_value == real128_inf().m_value));
    REQUIRE(nan_128.isnan());
#endif
}
