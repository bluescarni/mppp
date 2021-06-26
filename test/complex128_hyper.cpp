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

TEST_CASE("sinh")
{
    const complex128 cmp{"(-6.54812004091100164776681101883532441,-7.61923172032141020848713573680431168)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.sinh())>::value);
    c.sinh();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}) < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(sinh(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{sinh(complex128{3, 4}).m_value - cmp.m_value}) < 1E-32);
}

TEST_CASE("cosh")
{
    const complex128 cmp{"(-6.58066304055115643256074412653880334,-7.5815527427465443537163452865384257)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.cosh())>::value);
    c.cosh();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}) < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(cosh(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{cosh(complex128{3, 4}).m_value - cmp.m_value}) < 1E-32);
}

TEST_CASE("tanh")
{
    const complex128 cmp{"(1.00070953606723293932958547240417269,0.00490825806749606025907878692993276652)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.tanh())>::value);
    c.tanh();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}) < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(tanh(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{tanh(complex128{3, 4}).m_value - cmp.m_value}) < 1E-32);
}

TEST_CASE("asinh")
{
    const complex128 cmp{"(2.29991404087926964995578963066317545,0.91761685335147865575986274867017455)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.asinh())>::value);
    c.asinh();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}) < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(asinh(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{asinh(complex128{3, 4}).m_value - cmp.m_value}) < 1E-32);
}

TEST_CASE("acosh")
{
    const complex128 cmp{"(2.30550903124347694204183593813343083,0.936812461155719902912524576575608903)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.acosh())>::value);
    c.acosh();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}) < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(acosh(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{acosh(complex128{3, 4}).m_value - cmp.m_value}) < 1E-32);
}

TEST_CASE("atanh")
{
    const complex128 cmp{"(0.117500907311433888412734257787085516,1.40992104959657552253061938446042078)"};

    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.atanh())>::value);
    c.atanh();
    REQUIRE(abs(complex128{c.m_value - cmp.m_value}) < 1E-32);
    REQUIRE(std::is_same<complex128, decltype(atanh(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{atanh(complex128{3, 4}).m_value - cmp.m_value}) < 1E-32);
}
