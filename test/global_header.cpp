// Copyright 2016-2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/mp++.hpp>

#include "catch.hpp"

// NOLINTBEGIN(misc-const-correctness,bugprone-chained-comparison,misc-use-anonymous-namespace,misc-use-internal-linkage,bugprone-random-generator-seed,bugprone-throwing-static-initialization)

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("global_test")
{
    auto n = 123_z1;
    REQUIRE(n == 123);

    auto q = 123_q1;
    REQUIRE(q == 123);

#if defined(MPPP_WITH_QUADMATH)
    auto rq = 123_rq;
    REQUIRE(rq == 123);

    auto cq = 123_rq + 321_icq;
    REQUIRE(cq == complex128{123, 321});
#endif

#if defined(MPPP_WITH_MPFR)
    auto r = 123_r256;
    REQUIRE(r == 123);
#endif

#if defined(MPPP_WITH_MPC)
    auto c = complex{123};
    REQUIRE(c == 123);
#endif
}

// NOLINTEND(misc-const-correctness,bugprone-chained-comparison,misc-use-anonymous-namespace,misc-use-internal-linkage,bugprone-random-generator-seed,bugprone-throwing-static-initialization)
