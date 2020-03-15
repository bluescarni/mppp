// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <complex>
#include <stdexcept>
#include <utility>

#if defined(MPPP_HAVE_STRING_VIEW)

#include <string_view>

#endif

#include <mp++/complex128.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real128.hpp>

#if defined(MPPP_WITH_MPFR)

#include <mp++/real.hpp>

#endif

#include "catch.hpp"

using namespace mppp;

TEST_CASE("basic constructors")
{
    // Default ctor.
    complex128 c0;
    REQUIRE(c0.m_value == 0);
    constexpr complex128 c0a{4};

    // Copy and move.
    constexpr auto c1(c0a);
    REQUIRE(c1.m_value == 4);
    constexpr auto c2(std::move(c1));
    REQUIRE(c2.m_value == 4);

    // From __complex128.
    constexpr complex128 c3(cplex128{1, 2});
    REQUIRE(c3.m_value == cplex128{1, 2});

    // The generic ctor.
    constexpr complex128 c4{4};
    REQUIRE(c4.m_value == 4);
    constexpr complex128 c5{-3.f};
    REQUIRE(c5.m_value == -3);
    constexpr complex128 c6{real128{42}};
    REQUIRE(c6.m_value == 42);
#if !defined(__clang__)
    constexpr complex128 c6a{4.l};
    REQUIRE(c6a.m_value == 4);
#endif
    REQUIRE(complex128{integer<1>{-48}}.m_value == -48);
    REQUIRE(complex128{rational<1>{5, 2}}.m_value == complex128{5}.m_value / 2);

#if defined(MPPP_WITH_MPFR)
    REQUIRE(complex128{real{123}}.m_value == 123);
#endif

    // Binary generic ctor.
    constexpr complex128 c7{4, char(5)};
    REQUIRE(c7.m_value == cplex128{4, 5});
    constexpr complex128 c8{-4., -5.f};
    REQUIRE(c8.m_value == cplex128{-4, -5});
    constexpr complex128 c9{1, real128{12}};
    REQUIRE(c9.m_value == cplex128{1, 12});
#if !defined(__clang__)
    constexpr complex128 c9a{4.l, 3};
    REQUIRE(c9a.m_value == cplex128{4, 3});
#endif
    REQUIRE(complex128{-48_z1, 66_z1}.m_value == cplex128{-48, 66});
    REQUIRE(complex128{-5_q1 / 2, 3_q1 / 2}.m_value == cplex128{real128{"-2.5"}.m_value, real128{"1.5"}.m_value});
    REQUIRE(complex128{-48_z1, 66}.m_value == cplex128{-48, 66});
    REQUIRE(complex128{3, 3_q1 / 2}.m_value == cplex128{3, real128{"1.5"}.m_value});

#if defined(MPPP_WITH_MPFR)
    REQUIRE(complex128{real{123}, -real{124}}.m_value == cplex128{123, -124});
    REQUIRE(complex128{real{123}, 124_z1}.m_value == cplex128{123, 124});
    REQUIRE(complex128{-123_q1, -real{124}}.m_value == cplex128{-123, -124});
    REQUIRE(complex128{real{123}, -124}.m_value == cplex128{123, -124});
    REQUIRE(complex128{123.f, -real{124}}.m_value == cplex128{123, -124});
#endif

    // Ctor from std::complex.
    MPPP_CONSTEXPR_14 complex128 c10{std::complex<double>{1, 2}};
    REQUIRE(c10.m_value == cplex128{1, 2});
    MPPP_CONSTEXPR_14 complex128 c11{std::complex<float>{-1, -3}};
    REQUIRE(c11.m_value == cplex128{-1, -3});
}

TEST_CASE("string constructors")
{
    using Catch::Matchers::Message;
    using namespace std::literals;

    // Empty strings.
    REQUIRE_THROWS_MATCHES(complex128{""}, std::invalid_argument,
                           Message("The string '' is not a valid representation of a complex128"));
    REQUIRE_THROWS_MATCHES(complex128{" "}, std::invalid_argument,
                           Message("The string ' ' is not a valid representation of a complex128"));
    REQUIRE_THROWS_MATCHES(complex128{"  "}, std::invalid_argument,
                           Message("The string '  ' is not a valid representation of a complex128"));

    // Only the real value, no brackets.
    REQUIRE(complex128{"123"}.m_value == 123);
    REQUIRE(complex128{"123"s}.m_value == 123);
    constexpr char str1[] = "123456";
    REQUIRE(complex128{str1, str1 + 3}.m_value == 123);
    REQUIRE(complex128{str1 + 3, str1 + 6}.m_value == 456);
#if defined(MPPP_HAVE_STRING_VIEW)
    REQUIRE(complex128{"123"sv}.m_value == 123);
#endif
    REQUIRE(complex128{" 456"}.m_value == 456);
    REQUIRE(complex128{"  789"}.m_value == 789);
    REQUIRE(complex128{"  -0x2f2"}.m_value == -754);
    REQUIRE_THROWS_MATCHES(
        complex128{"123 "}, std::invalid_argument,
        Message("The string '123 ' does not represent a valid quadruple-precision floating-point value"));
    REQUIRE_THROWS_MATCHES(
        complex128{"  123 "}, std::invalid_argument,
        Message("The string '123 ' does not represent a valid quadruple-precision floating-point value"));
    REQUIRE_THROWS_MATCHES(
        complex128{"  hello world "}, std::invalid_argument,
        Message("The string 'hello world ' does not represent a valid quadruple-precision floating-point value"));

    // Strings with brackets and only the real component.
    REQUIRE(complex128{"(123)"}.m_value == 123);
    REQUIRE(complex128{"(123)"s}.m_value == 123);
    constexpr char str2[] = "(123)(456)";
    REQUIRE(complex128{str2, str2 + 5}.m_value == 123);
    REQUIRE(complex128{str2 + 5, str2 + 10}.m_value == 456);
#if defined(MPPP_HAVE_STRING_VIEW)
    REQUIRE(complex128{"(123)"sv}.m_value == 123);
#endif
    REQUIRE(complex128{" (123)"}.m_value == 123);
    REQUIRE(complex128{" ( 123)"}.m_value == 123);
    REQUIRE(complex128{"  ( -0x2f2)"}.m_value == -754);
    REQUIRE_THROWS_MATCHES(complex128{" (123) "}, std::invalid_argument,
                           Message("The string ' (123) ' is not a valid representation of a complex128"));
    REQUIRE_THROWS_MATCHES(
        complex128{" (123 )"}, std::invalid_argument,
        Message("The string '123 ' does not represent a valid quadruple-precision floating-point value"));
    REQUIRE_THROWS_MATCHES(complex128{" (123"}, std::invalid_argument,
                           Message("The string ' (123' is not a valid representation of a complex128"));
    REQUIRE_THROWS_MATCHES(
        complex128{" (123as)"}, std::invalid_argument,
        Message("The string '123as' does not represent a valid quadruple-precision floating-point value"));

    // Real and imaginary components.
    REQUIRE(complex128{"(123,12)"}.m_value == cplex128{123, 12});
    REQUIRE(complex128{"(123,12)"s}.m_value == cplex128{123, 12});
    constexpr char str3[] = "(123,456)(-123,-456)";
    REQUIRE(complex128{str3, str3 + 9}.m_value == cplex128{123, 456});
    REQUIRE(complex128{str3 + 9, str3 + 20}.m_value == -cplex128{123, 456});
#if defined(MPPP_HAVE_STRING_VIEW)
    REQUIRE(complex128{"(123,12)"sv}.m_value == cplex128{123, 12});
#endif
    REQUIRE(complex128{" (123,12)"}.m_value == cplex128{123, 12});
    REQUIRE(complex128{" ( 123,12)"}.m_value == cplex128{123, 12});
    REQUIRE(complex128{" ( 123, 12)"}.m_value == cplex128{123, 12});
    REQUIRE(complex128{" ( 0x10.1p0, 12)"}.m_value == cplex128{(16.0625_rq).m_value, 12});
    REQUIRE(complex128{" ( 0x10.1p0, 0x10.1p0)"}.m_value == cplex128{(16.0625_rq).m_value, (16.0625_rq).m_value});
    REQUIRE(complex128{" ( 12, 0x10.1p0)"}.m_value == cplex128{12, (16.0625_rq).m_value});
    REQUIRE_THROWS_MATCHES(complex128{" (123,12) "}, std::invalid_argument,
                           Message("The string ' (123,12) ' is not a valid representation of a complex128"));
    REQUIRE_THROWS_MATCHES(
        complex128{" (123 ,12)"}, std::invalid_argument,
        Message("The string '123 ' does not represent a valid quadruple-precision floating-point value"));
    REQUIRE_THROWS_MATCHES(
        complex128{" (123, 12 )"}, std::invalid_argument,
        Message("The string ' 12 ' does not represent a valid quadruple-precision floating-point value"));
    REQUIRE_THROWS_MATCHES(complex128{" (123,"}, std::invalid_argument,
                           Message("The string ' (123,' is not a valid representation of a complex128"));
    REQUIRE_THROWS_MATCHES(complex128{" (123, "}, std::invalid_argument,
                           Message("The string ' (123, ' is not a valid representation of a complex128"));
    REQUIRE_THROWS_MATCHES(complex128{" (123,1"}, std::invalid_argument,
                           Message("The string ' (123,1' is not a valid representation of a complex128"));
    REQUIRE_THROWS_MATCHES(complex128{" (123, 1"}, std::invalid_argument,
                           Message("The string ' (123, 1' is not a valid representation of a complex128"));
    REQUIRE_THROWS_MATCHES(complex128{" (123,1 "}, std::invalid_argument,
                           Message("The string ' (123,1 ' is not a valid representation of a complex128"));
    REQUIRE_THROWS_MATCHES(
        complex128{" (hello,12)"}, std::invalid_argument,
        Message("The string 'hello' does not represent a valid quadruple-precision floating-point value"));
    REQUIRE_THROWS_MATCHES(
        complex128{" (12,world)"}, std::invalid_argument,
        Message("The string 'world' does not represent a valid quadruple-precision floating-point value"));
    REQUIRE_THROWS_MATCHES(
        complex128{" (12,)"}, std::invalid_argument,
        Message("The string '' does not represent a valid quadruple-precision floating-point value"));
    REQUIRE_THROWS_MATCHES(
        complex128{" (,12)"}, std::invalid_argument,
        Message("The string '' does not represent a valid quadruple-precision floating-point value"));
    REQUIRE_THROWS_MATCHES(
        complex128{"(,)"}, std::invalid_argument,
        Message("The string '' does not represent a valid quadruple-precision floating-point value"));
}
