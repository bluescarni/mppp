// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/complex.hpp>

#include "catch.hpp"

using namespace mppp;

TEST_CASE("neg")
{
    complex c{1, 2};
    c.neg();
    REQUIRE(c == complex{-1, -2});
}

TEST_CASE("conj")
{
    complex c{1, 2};
    c.conj();
    REQUIRE(c == complex{1, -2});
}

TEST_CASE("abs")
{
    complex c{3, 4};
    c.abs();
    REQUIRE(c == complex{5, 0});
}

TEST_CASE("norm")
{
    complex c{3, 4};
    c.norm();
    REQUIRE(c == complex{25, 0});
}
