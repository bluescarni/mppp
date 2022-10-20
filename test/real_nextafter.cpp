// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cmath>
#include <utility>

#include <mp++/real.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("real nextafter ternary")
{
    real rop;
    real x{1.1};
    nextafter(rop, x, real{0});
    REQUIRE(rop.get_prec() == x.get_prec());
    REQUIRE(rop == std::nextafter(1.1, 0.));

    // Check that the precision of y does not have any influence.
    rop = real{};
    REQUIRE(&rop == &nextafter(rop, x, real{0, 512}));
    REQUIRE(rop.get_prec() == x.get_prec());
    REQUIRE(rop == std::nextafter(1.1, 0.));

    // Check with y overlapping x.
    rop = real{};
    nextafter(rop, x, x);
    REQUIRE(rop.get_prec() == x.get_prec());
    REQUIRE(rop == x);

    // Try moving in x.
    rop = real{};
    auto x2 = x;
    nextafter(rop, std::move(x2), x);
    REQUIRE(rop.get_prec() == x.get_prec());
    REQUIRE(rop == x);
    REQUIRE(x2 == real{});
    REQUIRE(x2.get_prec() == real{}.get_prec());

    // Identical arguments.
    rop = real{};
    nextafter(rop, x, x);
    REQUIRE(rop.get_prec() == x.get_prec());
    REQUIRE(rop == x);

    // Identical arguments, first one moved in.
    rop = real{};
    x2 = x;
    nextafter(rop, std::move(x), x);
    REQUIRE(rop.get_prec() == x2.get_prec());
    REQUIRE(rop == x2);
    REQUIRE(x == real{});
    REQUIRE(x.get_prec() == real{}.get_prec());

    rop = real{};
    x = real{1.1};

    // NaN testing.
    nextafter(rop, real{"nan", 23}, x);
    REQUIRE(isnan(rop));
    REQUIRE(rop.get_prec() == 23);

    rop = real{};
    nextafter(rop, x, real{"nan", 23});
    REQUIRE(isnan(rop));
    REQUIRE(rop.get_prec() == x.get_prec());
}

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("real nextafter binary")
{
    real x{1.1};
    auto rop = nextafter(x, real{0});
    REQUIRE(rop.get_prec() == x.get_prec());
    REQUIRE(rop == std::nextafter(1.1, 0.));

    // Check that the precision of y does not have any influence.
    rop = nextafter(x, real{0, 512});
    REQUIRE(rop.get_prec() == x.get_prec());
    REQUIRE(rop == std::nextafter(1.1, 0.));

    // Check with y overlapping x.
    rop = nextafter(x, x);
    REQUIRE(rop.get_prec() == x.get_prec());
    REQUIRE(rop == x);

    // Try moving in x.
    auto x2 = x;
    rop = nextafter(std::move(x2), x);
    REQUIRE(rop.get_prec() == x.get_prec());
    REQUIRE(rop == x);
    REQUIRE(!x2.is_valid());

    // Identical arguments.
    rop = nextafter(x, x);
    REQUIRE(rop.get_prec() == x.get_prec());
    REQUIRE(rop == x);

    // Identical arguments, first one moved in.
    x2 = x;
    rop = nextafter(std::move(x), x);
    REQUIRE(rop.get_prec() == x2.get_prec());
    REQUIRE(rop == x2);
    REQUIRE(!x.is_valid());

    x = real{1.1};

    // NaN testing.
    rop = nextafter(real{"nan", 23}, x);
    REQUIRE(isnan(rop));
    REQUIRE(rop.get_prec() == 23);

    rop = nextafter(x, real{"nan", 23});
    REQUIRE(isnan(rop));
    REQUIRE(rop.get_prec() == x.get_prec());
}
