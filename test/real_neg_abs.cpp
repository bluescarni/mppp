// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <type_traits>

#include <mp++/real.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("real neg")
{
    real r0;
    REQUIRE(!r0.signbit());
    REQUIRE((std::is_same<decltype(r0.neg()), real &>::value));
    r0.neg();
    REQUIRE(r0.signbit());
    r0 = -1;
    REQUIRE(r0.neg() == real{1});
    REQUIRE(neg(real{-42}) == real{42});
    r0 = 53;
    REQUIRE(neg(r0) == real{-53});
    neg(r0, real{-53, 8});
    REQUIRE(std::is_same<real &, decltype(neg(r0, real{-53, 8}))>::value);
    REQUIRE(r0 == real{53});
    REQUIRE(r0.get_prec() == 8);
    real r1{123};
    neg(r0, r1);
    REQUIRE(r0 == real{-123});
    REQUIRE(r0.get_prec() == detail::nl_digits<int>() + 1);
}

TEST_CASE("real abs")
{
    real r0{"-0", 50};
    REQUIRE(r0.signbit());
    REQUIRE((std::is_same<decltype(r0.abs()), real &>::value));
    r0.abs();
    REQUIRE(!r0.signbit());
    r0 = -1;
    REQUIRE(r0.abs() == real{1});
    REQUIRE(abs(real{-42}) == real{42});
    r0 = -53;
    REQUIRE(abs(r0) == real{53});
    abs(r0, real{-53, 8});
    REQUIRE(std::is_same<real &, decltype(abs(r0, real{-53, 8}))>::value);
    REQUIRE(r0 == real{53});
    REQUIRE(r0.get_prec() == 8);
    real r1{-123};
    abs(r0, r1);
    REQUIRE(r0 == real{123});
    REQUIRE(r0.get_prec() == detail::nl_digits<int>() + 1);
}
