// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstdint>
#include <tuple>

#include <mp++/config.hpp>
#include <mp++/integer.hpp>
#include <mp++/real128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

using int_t = integer<1>;

TEST_CASE("real128 get_ieee()")
{
    // NOTE: not sure why, but we get
    // a compiler error with ICC in C++17
    // on these tuple operations below.
#if !defined(__INTEL_COMPILER) || MPPP_CPLUSPLUS < 201703L
    std::uint_least8_t sign{};
    std::uint_least16_t exp{};
    std::uint_least64_t hi{};
    std::uint_least64_t lo{};
    std::tie(sign, exp, hi, lo) = real128{}.get_ieee();
    REQUIRE(!sign);
    REQUIRE(!exp);
    REQUIRE(!hi);
    REQUIRE(!lo);
    std::tie(sign, exp, hi, lo) = real128{-0.}.get_ieee();
    REQUIRE(sign);
    REQUIRE(!exp);
    REQUIRE(!hi);
    REQUIRE(!lo);
    std::tie(sign, exp, hi, lo) = real128{42}.get_ieee();
    REQUIRE(!sign);
    REQUIRE(exp == 16388ul);
    REQUIRE(hi == 10ull << (48 - 5));
    REQUIRE(!lo);
    std::tie(sign, exp, hi, lo) = real128{-42}.get_ieee();
    REQUIRE(sign);
    REQUIRE(exp == 16388ul);
    REQUIRE(hi == 10ull << (48 - 5));
    REQUIRE(!lo);
    std::tie(sign, exp, hi, lo) = real128_nan().get_ieee();
    REQUIRE(exp == 32767ul);
    REQUIRE((hi || lo));
    std::tie(sign, exp, hi, lo) = real128_inf().get_ieee();
    REQUIRE(!sign);
    REQUIRE(exp == 32767ul);
    REQUIRE((!hi && !lo));
    std::tie(sign, exp, hi, lo) = (-real128_inf()).get_ieee();
    REQUIRE(sign);
    REQUIRE(exp == 32767ul);
    REQUIRE((!hi && !lo));
    std::tie(sign, exp, hi, lo) = real128{"1.189731495357231765085759326628007e4932"}.get_ieee();
    REQUIRE(!sign);
    REQUIRE(exp == 32766ul);
    REQUIRE(hi == std::uint_least64_t(-1) % (1ull << 48));
    REQUIRE(lo == std::uint_least64_t(-1) % (int_t(1) << 64));
    std::tie(sign, exp, hi, lo) = real128{"-1.189731495357231765085759326628007e4932"}.get_ieee();
    REQUIRE(sign);
    REQUIRE(exp == 32766ul);
    REQUIRE(hi == std::uint_least64_t(-1) % (1ull << 48));
    REQUIRE(lo == std::uint_least64_t(-1) % (int_t(1) << 64));
    std::tie(sign, exp, hi, lo) = real128{"6.47517511943802511092443895822764655e-4966"}.get_ieee();
    REQUIRE(!sign);
    REQUIRE(!exp);
    REQUIRE(!hi);
    REQUIRE(lo == 1u);
    std::tie(sign, exp, hi, lo) = real128{"-6.47517511943802511092443895822764655e-4966"}.get_ieee();
    REQUIRE(sign);
    REQUIRE(!exp);
    REQUIRE(!hi);
    REQUIRE(lo == 1u);
#endif
}
