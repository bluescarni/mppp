// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <type_traits>
#include <utility>

#include <mp++/complex.hpp>
#include <mp++/config.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("agm1")
{
    const auto cmp1
        = 1.2049597176136955190833988540153239038944_r128 + 1.006180300341415795767582267103891529043_icr128;
    {
        // Member function.
        auto c = 1.1_r128 + 2.3_icr128;
        c.agm1();
        REQUIRE(abs(c - cmp1) < pow(2_r128, -125));
        REQUIRE(c.get_prec() == 128);
    }
    {
        // rop overload.
        complex c1, c2 = 1.1_r128 + 2.3_icr128;
        const auto p = c2.get_prec();
        REQUIRE(&agm1(c1, c2) == &c1);
        REQUIRE(std::is_same<decltype(agm1(c1, c2)), complex &>::value);
        REQUIRE(abs(c1 - cmp1) < pow(2_r128, -125));
        REQUIRE(c1.get_prec() == p);

        // Move, but won't steal because rop
        // has higher precision.
        c1 = complex{0, complex_prec_t(c2.get_prec() + 1)};
        agm1(c1, std::move(c2));
        REQUIRE(abs(c1 - cmp1) < pow(2_r128, -125));
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == 1.1_r128 + 2.3_icr128);

        // Move, will steal.
        c1 = complex{};
        agm1(c1, std::move(c2));
        REQUIRE(abs(c1 - cmp1) < pow(2_r128, -125));
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == complex{});
    }
    {
        // return overload.
        REQUIRE(abs(agm1(1.1_r128 + 2.3_icr128) - cmp1) < pow(2_r128, -125));
        REQUIRE(std::is_same<decltype(agm1(complex{1, 2})), complex>::value);

        // move, will steal.
        complex c1 = 1.1_r128 + 2.3_icr128;
        const auto p = c1.get_prec();
        auto c2 = agm1(std::move(c1));
        REQUIRE(abs(c2 - cmp1) < pow(2_r128, -125));
        REQUIRE(c2.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }
}
