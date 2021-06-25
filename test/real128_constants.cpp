// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <tuple>

#include <mp++/real128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
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
    constexpr auto max = real128_max();
    REQUIRE(max == real128{"1.18973149535723176508575932662800702e+4932"});
    REQUIRE(get<0>(max.get_ieee()) == 0u);
    REQUIRE(get<1>(max.get_ieee()) == 32766ul);
    REQUIRE(get<2>(max.get_ieee()) == 281474976710655ull);
    REQUIRE(get<3>(max.get_ieee()) == 18446744073709551615ull);
    constexpr auto min = real128_min();
    REQUIRE(min == real128{"3.36210314311209350626267781732175260e-4932"});
    REQUIRE(get<0>(min.get_ieee()) == 0u);
    REQUIRE(get<1>(min.get_ieee()) == 1u);
    REQUIRE(get<2>(min.get_ieee()) == 0u);
    REQUIRE(get<3>(min.get_ieee()) == 0u);
    constexpr auto eps = real128_epsilon();
    REQUIRE(eps == real128{"1.92592994438723585305597794258492732e-34"});
    REQUIRE(get<0>(eps.get_ieee()) == 0u);
    REQUIRE(get<1>(eps.get_ieee()) == 16271u);
    REQUIRE(get<2>(eps.get_ieee()) == 0u);
    REQUIRE(get<3>(eps.get_ieee()) == 0u);
    constexpr auto dmin = real128_denorm_min();
    REQUIRE(dmin == real128{"6.47517511943802511092443895822764655e-4966"});
    REQUIRE(get<0>(dmin.get_ieee()) == 0u);
    REQUIRE(get<1>(dmin.get_ieee()) == 0u);
    REQUIRE(get<2>(dmin.get_ieee()) == 0u);
    REQUIRE(get<3>(dmin.get_ieee()) == 1u);
#if MPPP_CPLUSPLUS >= 201703L
    REQUIRE(sig_digits_128 == 113u);
    REQUIRE((pi_128.m_value == real128_pi().m_value));
    REQUIRE((e_128.m_value == real128_e().m_value));
    REQUIRE((sqrt2_128.m_value == real128_sqrt2().m_value));
    REQUIRE((inf_128.m_value == real128_inf().m_value));
    REQUIRE(nan_128.isnan());
    REQUIRE((max_128.m_value == real128_max().m_value));
    REQUIRE((min_128.m_value == real128_min().m_value));
    REQUIRE((epsilon_128.m_value == real128_epsilon().m_value));
    REQUIRE((denorm_min_128.m_value == real128_denorm_min().m_value));
#endif
}
