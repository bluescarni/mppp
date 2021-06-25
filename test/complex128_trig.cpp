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

TEST_CASE("sin")
{
    const complex128 cmp{"(3.85373803791937732161752894046373053,-27.0168132580039344880975437549921532)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.sin())>::value);
    c.sin();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}) < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(sin(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{sin(complex128{3, 4}).m_value - cmp.m_value}) < 1E-32);
}

TEST_CASE("cos")
{
    const complex128 cmp{"(-27.0349456030742246476948026682709123,-3.85115333481177753656333712305312452)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.cos())>::value);
    c.cos();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}) < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(cos(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{cos(complex128{3, 4}).m_value - cmp.m_value}) < 1E-32);
}

TEST_CASE("tan")
{
    const complex128 cmp{"(-0.000187346204629478426224255637728218093,0.999355987381473141391649630320133042)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.tan())>::value);
    c.tan();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}) < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(tan(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{tan(complex128{3, 4}).m_value - cmp.m_value}) < 1E-32);
}

TEST_CASE("asin")
{
    const complex128 cmp{"(0.633983865639176716318797115064142495,2.30550903124347694204183593813343083)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.asin())>::value);
    c.asin();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}) < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(asin(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{asin(complex128{3, 4}).m_value - cmp.m_value}) < 1E-32);
}

TEST_CASE("acos")
{
    const complex128 cmp{"(0.936812461155719902912524576575608903,-2.30550903124347694204183593813343083)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.acos())>::value);
    c.acos();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}) < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(acos(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{acos(complex128{3, 4}).m_value - cmp.m_value}) < 1E-32);
}

TEST_CASE("atan")
{
    const complex128 cmp{"(1.44830699523146454214528045103411354,0.158997191679999174364761036007018789)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.atan())>::value);
    c.atan();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}) < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(atan(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{atan(complex128{3, 4}).m_value - cmp.m_value}) < 1E-32);
}
