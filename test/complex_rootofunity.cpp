// Copyright 2016-2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <type_traits>

#include <mp++/complex.hpp>

#include "catch.hpp"

// NOLINTBEGIN(misc-const-correctness,bugprone-chained-comparison,misc-use-anonymous-namespace,misc-use-internal-linkage,bugprone-random-generator-seed,bugprone-throwing-static-initialization)

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("rootofunity")
{
    complex c{1, 2, complex_prec_t(128)};
    REQUIRE(std::is_same<complex &, decltype(set_rootofunity(c, 1, 1))>::value);
    REQUIRE(&set_rootofunity(c, 10, 8) == &c);
    REQUIRE(abs(mppp::pow(c, 10) - 1) < mppp::pow(2_r128, -126));
}

// NOLINTEND(misc-const-correctness,bugprone-chained-comparison,misc-use-anonymous-namespace,misc-use-internal-linkage,bugprone-random-generator-seed,bugprone-throwing-static-initialization)
