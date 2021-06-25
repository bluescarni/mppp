// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <type_traits>

#include <mp++/complex128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("sqrt")
{
    const complex128 cmp{"(2,1)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.sqrt())>::value);
    c.sqrt();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}) < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(sqrt(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{sqrt(complex128{3, 4}).m_value - cmp.m_value}) < 1E-32);
}
