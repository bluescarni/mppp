// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <utility>

#include <mp++/complex.hpp>

#include "catch.hpp"

using namespace mppp;

TEST_CASE("identity")
{
    complex r1{4, 5};
    REQUIRE(+r1 == r1);

    // Check stealing.
    const auto p = r1.get_prec();
    auto r2 = +std::move(r1);
    REQUIRE(r2.get_prec() == p);
    REQUIRE(r2 == complex{4, 5});
    REQUIRE(!r1.is_valid());
}

TEST_CASE("binary plus")
{
    // complex-complex.
    {
        complex r1{4, 5}, r2{-4, 7};
        const auto p = r1.get_prec();
        auto ret = r1 + r2;
        REQUIRE(ret == complex{0, 12});
        REQUIRE(ret.get_prec() == r1.get_prec());

        // Test moves.
        ret = std::move(r1) + r2;
        REQUIRE(ret == complex{0, 12});
        REQUIRE(ret.get_prec() == p);
        REQUIRE(!r1.is_valid());

        r1 = complex{4, 5};
        ret = r1 + std::move(r2);
        REQUIRE(ret == complex{0, 12});
        REQUIRE(ret.get_prec() == p);
        REQUIRE(!r2.is_valid());

        r2 = complex{-4, 7};
        ret = std::move(r1) + std::move(r2);
        REQUIRE(ret == complex{0, 12});
        REQUIRE(ret.get_prec() == p);
        REQUIRE((!r1.is_valid() || !r2.is_valid()));
    }
    // complex-unsigned integral.
    {
        complex r1{4, 5};
        r1 + 5u;
    }
}

TEST_CASE("negation")
{
    complex r1{4, 5};
    REQUIRE(-r1 == complex{-4, -5});

    // Check stealing.
    const auto p = r1.get_prec();
    auto r2 = -std::move(r1);
    REQUIRE(r2.get_prec() == p);
    REQUIRE(r2 == complex{-4, -5});
    REQUIRE(!r1.is_valid());
}
