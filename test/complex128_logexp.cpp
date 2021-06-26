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

TEST_CASE("exp")
{
    const complex128 cmp{"(-13.1287830814621580803275551453741285,-15.2007844630679545622034810233427382)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.exp())>::value);
    c.exp();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}) < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(exp(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{exp(complex128{3, 4}).m_value - cmp.m_value}) < 1E-32);
}

TEST_CASE("log")
{
    const complex128 cmp{"(1.6094379124341003746007593332261876,0.927295218001612232428512462922428794)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.log())>::value);
    c.log();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}) < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(log(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{log(complex128{3, 4}).m_value - cmp.m_value}) < 1E-32);
}

TEST_CASE("log10")
{
    const complex128 cmp{"(0.698970004336018804786261105275506932,0.402719196273373142077900460387457914)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.log10())>::value);
    c.log10();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}) < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(log10(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{log10(complex128{3, 4}).m_value - cmp.m_value}) < 1E-32);
}
