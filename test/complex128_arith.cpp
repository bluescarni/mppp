// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <type_traits>

#include <mp++/complex128.hpp>
#include <mp++/config.hpp>
#include <mp++/real128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

#if MPPP_CPLUSPLUS >= 201402L

constexpr auto test_constexpr_conj(complex128 c)
{
    return c.conj();
}

#endif

TEST_CASE("abs")
{
    complex128 c{3, 4};
    REQUIRE(std::is_same<complex128 &, decltype(c.abs())>::value);
    REQUIRE(c.abs().m_value == 5);
    REQUIRE(std::is_same<real128, decltype(abs(complex128{3, 4}))>::value);
    REQUIRE(abs(complex128{3, 4}) == 5);
}

TEST_CASE("arg")
{
    complex128 c{1, 0};
    REQUIRE(std::is_same<complex128 &, decltype(c.arg())>::value);
    REQUIRE(c.arg().m_value == 0);
    REQUIRE(std::is_same<complex128, decltype(arg(complex128{1, 0}))>::value);
    REQUIRE(arg(complex128{1, 0}).m_value == 0);
}

TEST_CASE("conj")
{
    constexpr auto cnj1 = conj(complex128{1, 3});
    REQUIRE(std::is_same<complex128, decltype(conj(complex128{3, 4}))>::value);
    REQUIRE(cnj1.m_value == cplex128{1, -3});
    REQUIRE(std::is_same<const complex128, decltype(cnj1)>::value);

    complex128 c{3, -3};
    c.conj();
    REQUIRE(c == complex128{3, 3});
    REQUIRE(std::is_same<complex128 &, decltype(c.conj())>::value);

    // NOTE: it looks like on ICC there might
    // be a codegen issue that discards the sign
    // of a negative zero in certain circumstances.
#if !defined(__INTEL_COMPILER)
    c = complex128{1, 0};
    REQUIRE(!c.imag().signbit());
    c.conj();
    REQUIRE(c.imag().signbit());
#endif

#if MPPP_CPLUSPLUS >= 201402L && !defined(__INTEL_COMPILER)
    constexpr auto cnj2 = test_constexpr_conj(complex128{0, 3});
    REQUIRE(cnj2 == complex128{0, -3});
#endif
}

TEST_CASE("proj")
{
    complex128 c{42, -43};
    REQUIRE(std::is_same<complex128 &, decltype(c.proj())>::value);
    REQUIRE(c.proj().m_value == cplex128{42, -43});
    REQUIRE(std::is_same<complex128, decltype(proj(complex128{1, 2}))>::value);
    REQUIRE(proj(complex128{1, 2}).m_value == cplex128{1, 2});

    REQUIRE(proj(complex128{real128_inf(), 123}).m_value == cplex128{real128_inf().m_value, 0});
    REQUIRE(!proj(complex128{real128_inf(), 123}).imag().signbit());
    REQUIRE(proj(complex128{real128_inf(), -123}).m_value == cplex128{real128_inf().m_value, 0});
    REQUIRE(proj(complex128{real128_inf(), -123}).imag().signbit());
}
