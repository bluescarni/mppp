// Copyright 2016-2026 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/complex128.hpp>

#include "catch.hpp"

// NOLINTBEGIN(misc-const-correctness,bugprone-chained-comparison,misc-use-anonymous-namespace,misc-use-internal-linkage,bugprone-random-generator-seed,bugprone-throwing-static-initialization,readability-function-size,google-readability-function-size,bugprone-use-after-move,clang-analyzer-cplusplus.Move,bugprone-signed-bitwise)

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("literal")
{
    REQUIRE(1.1_icq == complex128{0, real128{"1.1"}});
    REQUIRE(1.1_rq + 1.1_icq == complex128{real128{"1.1"}, real128{"1.1"}});
}

// NOLINTEND(misc-const-correctness,bugprone-chained-comparison,misc-use-anonymous-namespace,misc-use-internal-linkage,bugprone-random-generator-seed,bugprone-throwing-static-initialization,readability-function-size,google-readability-function-size,bugprone-use-after-move,clang-analyzer-cplusplus.Move,bugprone-signed-bitwise)
