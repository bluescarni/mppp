// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <quadmath.h>
#include <random>
#include <sstream>

#include <mp++/real128.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

static int ntries = 1000;

static std::mt19937 rng;

static inline void check_round_trip(const real128 &r)
{
    const auto tmp = r.to_string();
    real128 r2{tmp};
    REQUIRE(((r.m_value == r2.m_value) || (r.isnan() && r2.isnan() && r.signbit() == r2.signbit())));
    std::ostringstream oss;
    oss << r;
    REQUIRE(oss.str() == tmp);
}

TEST_CASE("real128 io")
{
    check_round_trip(real128{});
    check_round_trip(real128{1.23});
    check_round_trip(real128{-4.56});
    check_round_trip(real128{"1.1"});
    check_round_trip(real128{"-1.1"});
    check_round_trip(real128{"inf"});
    check_round_trip(real128{"-inf"});
    check_round_trip(real128{"nan"});
    check_round_trip(real128{"-nan"});
    std::uniform_int_distribution<int> sdist(0, 1);
    std::uniform_real_distribution<double> dist1(100., 1000.);
    for (int i = 0; i < ntries; ++i) {
        check_round_trip(real128{::nextafterq(dist1(rng), 10000.) * (sdist(rng) ? 1 : -1)});
    }
    std::uniform_real_distribution<double> dist2(1E-6, 1E-1);
    for (int i = 0; i < ntries; ++i) {
        check_round_trip(real128{::nextafterq(dist2(rng), 1.) * (sdist(rng) ? 1 : -1)});
    }
    std::uniform_real_distribution<double> dist3(1E100, 1E120);
    for (int i = 0; i < ntries; ++i) {
        check_round_trip(real128{::nextafterq(dist3(rng), 1E121) * (sdist(rng) ? 1 : -1)});
    }
    // Some subnormals.
    check_round_trip(real128{"1E-4960"});
    check_round_trip(real128{"-1E-4960"});
    // Small tests for the input stream.
    real128 r;
    std::istringstream iss;
    iss.str("123.456");
    iss >> r;
    REQUIRE((r.m_value == real128{"123.456"}.m_value));
    iss.clear();
    iss.str("-1E-4000");
    iss >> r;
    REQUIRE((r.m_value == real128{"-1E-4000"}.m_value));
}
