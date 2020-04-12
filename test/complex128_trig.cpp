// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <type_traits>

#include <mp++/complex128.hpp>

#include "catch.hpp"

using namespace mppp;

TEST_CASE("sin")
{
    const complex128 cmp{"(3.85373803791937732161752894046373053,-27.0168132580039344880975437549921532)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.sin())>::value);
    c.sin();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}).real().m_value < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(sin(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{sin(complex128{3, 4}).m_value - cmp.m_value}).real().m_value < 1E-32);
}

TEST_CASE("cos")
{
    const complex128 cmp{"(-27.0349456030742246476948026682709123,-3.85115333481177753656333712305312452)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.cos())>::value);
    c.cos();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}).real().m_value < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(cos(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{cos(complex128{3, 4}).m_value - cmp.m_value}).real().m_value < 1E-32);
}

TEST_CASE("tan")
{
    const complex128 cmp{"(-0.000187346204629478426224255637728218093,0.999355987381473141391649630320133042)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.tan())>::value);
    c.tan();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}).real().m_value < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(tan(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{tan(complex128{3, 4}).m_value - cmp.m_value}).real().m_value < 1E-32);
}
