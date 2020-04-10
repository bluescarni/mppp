// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <complex>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(MPPP_HAVE_STRING_VIEW)

#include <string_view>

#endif

#include <mp++/complex128.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real128.hpp>
#include <mp++/type_name.hpp>

#if defined(MPPP_WITH_MPFR)

#include <mp++/real.hpp>

#endif

#include "catch.hpp"

using namespace mppp;

#if MPPP_CPLUSPLUS >= 201402L

constexpr auto test_constexpr_cm_assignment(const complex128 &c)
{
    complex128 c2{}, c3{};
    c2 = c;
    c3 = std::move(c);
    return std::make_pair(c2, c3);
}

constexpr auto test_constexpr_c128_assignment(const cplex128 &c)
{
    complex128 c2{};
    c2 = c;
    return c2;
}

constexpr auto test_constexpr_interop_assignment(int n)
{
    complex128 c2{};
    c2 = n;
    return c2;
}

constexpr auto test_constexpr_cpp_complex_assignment(const std::complex<double> &c)
{
    complex128 c2{};
    c2 = c;
    return c2;
}

constexpr auto test_constexpr_setters(double re, double im)
{
    complex128 c2{};
    c2.set_real(real128{re});
    c2.set_imag(real128{im});
    return c2;
}

#endif

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
#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    constexpr complex128 c6a{4.l};
    REQUIRE(c6a.m_value == 4);
    // NOTE: even if MPPP_FLOAT128_WITH_LONG_DOUBLE
    // is not defined, the construction from long double
    // will still work because the assignment
    // from __complex128 is picked up instead.
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
#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    constexpr complex128 c9a{4.l, 3};
    REQUIRE(c9a.m_value == cplex128{4, 3});
#else
    REQUIRE(!std::is_constructible<complex128, long double, int>::value);
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
#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    MPPP_CONSTEXPR_14 complex128 c12{std::complex<long double>{1, 2}};
    REQUIRE(c12.m_value == cplex128{1, 2});
#else
    REQUIRE(!std::is_constructible<complex128, std::complex<long double>>::value);
#endif
}

TEST_CASE("string constructors")
{
    using Catch::Matchers::Message;
#if defined(MPPP_HAVE_STRING_VIEW)
    using namespace std::literals;
#endif

    std::vector<char> buffer;

    // Empty strings.
    REQUIRE_THROWS_MATCHES(complex128{""}, std::invalid_argument,
                           Message("The string '' is not a valid representation of a complex128"));
    REQUIRE_THROWS_MATCHES(complex128{" "}, std::invalid_argument,
                           Message("The string ' ' is not a valid representation of a complex128"));
    REQUIRE_THROWS_MATCHES(complex128{"  "}, std::invalid_argument,
                           Message("The string '  ' is not a valid representation of a complex128"));
    REQUIRE_THROWS_MATCHES((complex128{buffer.data(), buffer.data()}), std::invalid_argument,
                           Message("The string '' is not a valid representation of a complex128"));

    // Only the real value, no brackets.
    REQUIRE(complex128{"123"}.m_value == 123);
    REQUIRE(complex128{std::string("123")}.m_value == 123);
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
    buffer = {'1', '2', '3'};
    REQUIRE(complex128{buffer.data(), buffer.data() + 3}.m_value == 123);
    buffer = {'1', '2', '3', '4', '5', '6'};
    REQUIRE(complex128{buffer.data() + 3, buffer.data() + 6}.m_value == 456);
    buffer = {'1', '2', '3', ' '};
    REQUIRE_THROWS_MATCHES(
        (complex128{buffer.data(), buffer.data() + 4}), std::invalid_argument,
        Message("The string '123 ' does not represent a valid quadruple-precision floating-point value"));

    // Strings with brackets and only the real component.
    REQUIRE(complex128{"(123)"}.m_value == 123);
    REQUIRE(complex128{std::string("(123)")}.m_value == 123);
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
    buffer = {'(', '1', '2', '3', ')'};
    REQUIRE(complex128{buffer.data(), buffer.data() + 5}.m_value == 123);
    buffer = {'(', '1', '2', '3', ')', '5', '6'};
    REQUIRE(complex128{buffer.data(), buffer.data() + 5}.m_value == 123);
    buffer = {'(', '2', '3', '4'};
    REQUIRE_THROWS_MATCHES((complex128{buffer.data(), buffer.data() + 4}), std::invalid_argument,
                           Message("The string '(234' is not a valid representation of a complex128"));

    // Real and imaginary components.
    REQUIRE(complex128{"(123,12)"}.m_value == cplex128{123, 12});
    REQUIRE(complex128{std::string("(123,12)")}.m_value == cplex128{123, 12});
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
    buffer = {'(', '1', '2', '3', ',', '1', '2', ')'};
    REQUIRE(complex128{buffer.data(), buffer.data() + 8}.m_value == cplex128{123, 12});
    buffer = {'(', '1', '2', '3', ',', '1', '2', ')', '4', '5'};
    REQUIRE(complex128{buffer.data(), buffer.data() + 8}.m_value == cplex128{123, 12});
    buffer = {'(', '1', '2', '3', ',', '1'};
    REQUIRE_THROWS_MATCHES((complex128{buffer.data(), buffer.data() + 6}), std::invalid_argument,
                           Message("The string '(123,1' is not a valid representation of a complex128"));
}

TEST_CASE("assignment operators")
{
    using Catch::Matchers::Message;

    // Trivial copy/move.
    complex128 a{1, 2}, b, c;
    b = a;
    REQUIRE(b.real() == 1);
    REQUIRE(b.imag() == 2);
    c = std::move(b);
    REQUIRE(c.real() == 1);
    REQUIRE(c.imag() == 2);

    // NOTE: icpc ICEs on this test.
#if MPPP_CPLUSPLUS >= 201402L && !defined(__INTEL_COMPILER)
    {
        constexpr auto tca = test_constexpr_cm_assignment(complex128{-1, 3});
        REQUIRE(tca.first == complex128{-1, 3});
        REQUIRE(tca.second == complex128{-1, 3});
    }
#endif

    // Assignment from __complex128.
    c = cplex128{4, -5};
    REQUIRE(c.real() == 4);
    REQUIRE(c.imag() == -5);

#if MPPP_CPLUSPLUS >= 201402L
    {
        constexpr auto tca = test_constexpr_c128_assignment(cplex128{4, 5});
        REQUIRE(tca.real() == 4);
        REQUIRE(tca.imag() == 5);
    }
#endif

    // Assignment from interoperable types.
    c = 4;
    REQUIRE(c.real() == 4);
    REQUIRE(c.imag() == 0);
    c = -25.;
    REQUIRE(c.real() == -25);
    REQUIRE(c.imag() == 0);
    c = -1234_rq;
    REQUIRE(c.real() == -1234);
    REQUIRE(c.imag() == 0);
    c = 1234_z1;
    REQUIRE(c.real() == 1234);
    REQUIRE(c.imag() == 0);
    c = -4321_q1;
    REQUIRE(c.real() == -4321);
    REQUIRE(c.imag() == 0);
#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    c = -4321.l;
    REQUIRE(c.real() == -4321);
    REQUIRE(c.imag() == 0);
    // NOTE: even if MPPP_FLOAT128_WITH_LONG_DOUBLE
    // is not defined, the assignment from long double
    // will still work because the assignment
    // from __complex128 is picked up instead.
#endif
#if defined(MPPP_WITH_MPFR)
    c = 789_r256;
    REQUIRE(c.real() == 789);
    REQUIRE(c.imag() == 0);
#endif

#if MPPP_CPLUSPLUS >= 201402L
    {
        constexpr auto tca = test_constexpr_interop_assignment(42);
        REQUIRE(tca.real() == 42);
        REQUIRE(tca.imag() == 0);
    }
#endif

    // Assignment from C++ complex.
    c = std::complex<float>{4, 5};
    REQUIRE(c.real() == 4);
    REQUIRE(c.imag() == 5);
    c = std::complex<double>{-4, -5};
    REQUIRE(c.real() == -4);
    REQUIRE(c.imag() == -5);
#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
    c = std::complex<long double>{4, -5};
    REQUIRE(c.real() == 4);
    REQUIRE(c.imag() == -5);
#else
    REQUIRE(!std::is_assignable<complex128 &, std::complex<long double>>::value);
#endif

#if MPPP_CPLUSPLUS >= 201402L
    {
        constexpr auto tca = test_constexpr_cpp_complex_assignment(std::complex<double>{1, 2});
        REQUIRE(tca.real() == 1);
        REQUIRE(tca.imag() == 2);
    }
#endif

    // Assignment from strings.
    c = "123";
    REQUIRE(c.m_value == 123);
    c = "(-123)";
    REQUIRE(c.m_value == -123);
    c = "(-123,456)";
    REQUIRE(c.m_value == cplex128{-123, 456});
    REQUIRE_THROWS_MATCHES(
        c = complex128{"123 "}, std::invalid_argument,
        Message("The string '123 ' does not represent a valid quadruple-precision floating-point value"));
}

TEST_CASE("setters getters")
{
    constexpr auto c1 = complex128{1, 2};
    constexpr bool b1 = c1.real() == 1;
    REQUIRE(b1);
    constexpr bool b2 = c1.imag() == 2;
    REQUIRE(b2);

    auto c2 = complex128{4, 5};
    c2.set_real(real128{-4});
    c2.set_imag(real128{-5});
    REQUIRE(c2.real() == -4);
    REQUIRE(c2.imag() == -5);

#if MPPP_CPLUSPLUS >= 201402L
    {
        constexpr auto tca = test_constexpr_setters(6., 7.);
        REQUIRE(tca.real() == 6);
        REQUIRE(tca.imag() == 7);
    }
#endif
}

TEST_CASE("conversions")
{
    using Catch::Matchers::Message;

    // NOTE: not sure why, but clang complains
    // that the conversion here is not a constexpr expression.
    // This looks like a bug, because a regular
    // function (instead of the conversion operator)
    // works fine.
#if !defined(__clang__)
    {
        // To __complex128.
        constexpr auto c = complex128{1, 2};
        constexpr auto cc = static_cast<cplex128>(c);
        REQUIRE(cc == cplex128{1, 2});
    }
#endif

    {
        // To C++ arithmetic types.
        constexpr auto c = complex128{-42, 0};
        constexpr auto ci = static_cast<int>(c);
        REQUIRE(ci == -42);
        constexpr auto cd = static_cast<double>(c);
        REQUIRE(cd == -42);
#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
        constexpr auto cld = static_cast<long double>(c);
        REQUIRE(cld == -42);
#else
        REQUIRE(!std::is_convertible<complex128, long double>::value);
#endif

        // To real128.
        constexpr auto crq = static_cast<real128>(c);
        REQUIRE(crq == -42);

        // mp++ types.
        REQUIRE(static_cast<integer<1>>(complex128{41, 0}) == 41);
        REQUIRE(static_cast<rational<1>>(complex128{-3, 0}) == -3);

#if defined(MPPP_WITH_MPFR)
        // To real.
        auto cr = static_cast<real>(complex128{45, 0});
        REQUIRE(cr == 45);
        REQUIRE(cr.get_prec() == 113);
#endif

        // Failure modes.
        REQUIRE_THROWS_MATCHES(static_cast<int>(complex128{1, 1}), std::domain_error,
                               Message("Cannot convert a complex128 with a nonzero imaginary part of "
                                       + real128{1}.to_string() + " to the real-valued type '" + type_name<int>()
                                       + "'"));
        REQUIRE_THROWS_MATCHES(static_cast<integer<1>>(complex128{1, 2}), std::domain_error,
                               Message("Cannot convert a complex128 with a nonzero imaginary part of "
                                       + real128{2}.to_string() + " to the real-valued type '" + type_name<integer<1>>()
                                       + "'"));
    }

    {
        // To c++ complex.
        constexpr auto c = complex128{1, 2};
        MPPP_CONSTEXPR_14 auto cf = static_cast<std::complex<float>>(c);
        REQUIRE(cf == std::complex<float>{1, 2});
        MPPP_CONSTEXPR_14 auto cd = static_cast<std::complex<double>>(c);
        REQUIRE(cd == std::complex<double>{1, 2});
#if defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
        MPPP_CONSTEXPR_14 auto cld = static_cast<std::complex<long double>>(c);
        REQUIRE(cld == std::complex<long double>{1, 2});
#else
        REQUIRE(!std::is_convertible<complex128, std::complex<long double>>::value);
#endif
    }
}

TEST_CASE("to_string")
{
    complex128 c{1, 2};
    REQUIRE(c.to_string() == "(" + c.real().to_string() + "," + c.imag().to_string() + ")");
}
