// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstdint>
#include <limits>
#include <random>
#include <utility>

#include <mp++/mp++.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

using int_t = integer<1>;

static int ntries = 1000;

static std::mt19937 rng;

TEST_CASE("real128 constructors")
{
    real128 r;
    REQUIRE((r.m_value == 0));
    r.m_value = 12;
    real128 r2{r};
    REQUIRE((r2.m_value == 12));
    real128 r3{std::move(r)};
    REQUIRE((r3.m_value == 12));
    REQUIRE((r.m_value == 12));
    real128 r4{::__float128(-56)};
    REQUIRE((r4.m_value == -56));
    real128 r5{-123};
    REQUIRE((r5.m_value == -123));
    real128 r6{124ull};
    REQUIRE((r6.m_value == 124));
    real128 r7{-0.5};
    REQUIRE((r7.m_value == -0.5));
    real128 r8{1.5f};
    REQUIRE((r8.m_value == 1.5f));
#if defined(MPPP_WITH_MPFR)
    real128 r8a{1.5l};
    REQUIRE((r8a.m_value == 1.5l));
#endif
    // Construction from integer.
    REQUIRE((real128{int_t{0}}.m_value == 0));
    int_t n{123};
    REQUIRE((real128{n}.m_value == 123));
    n = -123;
    n.promote();
    REQUIRE((real128{n}.m_value == -123));
    // Use a couple of limbs, nbits does not divide GMP_NUMB_BITS exactly.
    n = -1;
    n <<= GMP_NUMB_BITS + 1;
    REQUIRE((real128{n}.m_value == ::scalbnq(::__float128(-1), GMP_NUMB_BITS + 1)));
    n.promote();
    n.neg();
    REQUIRE((real128{n}.m_value == ::scalbnq(::__float128(1), GMP_NUMB_BITS + 1)));
    // Use two limbs, nbits dividing exactly.
    n = -2;
    n <<= 2 * GMP_NUMB_BITS - 1;
    REQUIRE((real128{n}.m_value == ::scalbnq(::__float128(-2), 2 * GMP_NUMB_BITS - 1)));
    n.promote();
    n.neg();
    REQUIRE((real128{n}.m_value == ::scalbnq(::__float128(2), 2 * GMP_NUMB_BITS - 1)));
    // Random testing.
    constexpr auto delta64 = std::numeric_limits<std::uint_least64_t>::digits - 64;
    constexpr auto delta49 = std::numeric_limits<std::uint_least64_t>::digits - 49;
    std::uniform_int_distribution<std::uint_least64_t> dist64(0u, (std::uint_least64_t(-1) << delta64) >> delta64);
    std::uniform_int_distribution<std::uint_least64_t> dist49(0u, (std::uint_least64_t(-1) << delta49) >> delta49);
    std::uniform_int_distribution<int> sdist(0, 1);
    std::uniform_int_distribution<int> extra_bits(0, 8);
    for (int i = 0; i < ntries; ++i) {
        const auto hi = dist49(rng);
        const auto lo = dist64(rng);
        const auto sign = sdist(rng) ? 1 : -1;
        const auto ebits = extra_bits(rng);
        auto tmp_r = real128{((int_t{hi} << 64) * sign + lo) << ebits};
        auto cmp_r = ::scalbnq(::scalbnq(::__float128(hi) * sign, 64) + lo, ebits);
        REQUIRE((tmp_r.m_value == cmp_r));
        REQUIRE(static_cast<int_t>(tmp_r) == ((int_t{hi} << 64) * sign + lo) << ebits);
    }
}
