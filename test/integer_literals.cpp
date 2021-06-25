// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <initializer_list>
#include <stdexcept>
#include <vector>

#include <mp++/config.hpp>
#include <mp++/integer.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

#if MPPP_CPLUSPLUS >= 201703L && (!defined(_MSC_VER) || defined(__clang__))

TEST_CASE("integer_literal_check_str_test")
{
    // Decimal.
    {
        constexpr char str[] = {'0', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 10);
    }
    {
        constexpr char str[] = {'9', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 10);
    }
    {
        constexpr char str[] = {'9', '1', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 10);
    }
    {
        constexpr char str[] = {'9', '1', '5', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 10);
    }
    {
        constexpr char str[] = {'9', '1', '5', '0', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 10);
    }
    {
        char str[] = {'a', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'a', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'1', 'a', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'a', '1', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'1', '2', '.', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'1', '2', 'e', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }

    // Binary.
    {
        constexpr char str[] = {'0', 'b', '1', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 2);
    }
    {
        constexpr char str[] = {'0', 'B', '1', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 2);
    }
    {
        constexpr char str[] = {'0', 'b', '0', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 2);
    }
    {
        constexpr char str[] = {'0', 'B', '0', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 2);
    }
    {
        constexpr char str[] = {'0', 'b', '0', '1', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 2);
    }
    {
        constexpr char str[] = {'0', 'B', '0', '0', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 2);
    }
    {
        char str[] = {'0', 'b', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'0', 'B', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'0', 'b', '2', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'0', 'B', '2', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'0', 'b', 'a', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'0', 'B', 'f', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }

    // Octal.
    {
        constexpr char str[] = {'0', '1', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 8);
    }
    {
        constexpr char str[] = {'0', '2', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 8);
    }
    {
        constexpr char str[] = {'0', '1', '2', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 8);
    }
    {
        constexpr char str[] = {'0', '0', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 8);
    }
    {
        constexpr char str[] = {'0', '0', '7', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 8);
    }
    {
        char str[] = {'0', '9', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'0', 'a', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'0', '1', 'a', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'0', '7', '8', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'0', '1', '.', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'0', '3', 'e', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }

    // Hex.
    {
        constexpr char str[] = {'0', 'x', '1', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 16);
    }
    {
        constexpr char str[] = {'0', 'X', 'f', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 16);
    }
    {
        constexpr char str[] = {'0', 'x', 'F', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 16);
    }
    {
        constexpr char str[] = {'0', 'X', '0', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 16);
    }
    {
        constexpr char str[] = {'0', 'x', 'a', 'B', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 16);
    }
    {
        constexpr char str[] = {'0', 'X', 'C', 'd', '\0'};
        constexpr auto b = detail::integer_literal_check_str(str);
        REQUIRE(b == 16);
    }
    {
        char str[] = {'0', 'x', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'0', 'X', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'0', 'x', 'g', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'0', 'X', 'G', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'0', 'x', '.', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
    {
        char str[] = {'0', 'X', 'p', '\0'};
        REQUIRE_THROWS_AS(detail::integer_literal_check_str(str), std::invalid_argument);
    }
}

#endif

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("z1_test")
{
    // A few simple tests.
#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b0_z1 == 0);
#endif
    REQUIRE(00_z1 == 0);
    REQUIRE(0_z1 == 0);
    REQUIRE(0x0_z1 == 0);

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b0_z2 == 0);
#endif
    REQUIRE(00_z2 == 0);
    REQUIRE(0_z2 == 0);
    REQUIRE(0x0_z2 == 0);

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b0_z3 == 0);
#endif
    REQUIRE(00_z3 == 0);
    REQUIRE(0_z3 == 0);
    REQUIRE(0x0_z3 == 0);

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b1_z1 == 1);
#endif
    REQUIRE(01_z1 == 1);
    REQUIRE(1_z1 == 1);
    REQUIRE(0x1_z1 == 1);

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b1_z2 == 1);
#endif
    REQUIRE(01_z2 == 1);
    REQUIRE(1_z2 == 1);
    REQUIRE(0x1_z2 == 1);

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b1_z3 == 1);
#endif
    REQUIRE(01_z3 == 1);
    REQUIRE(1_z3 == 1);
    REQUIRE(0x1_z3 == 1);

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b10_z1 == 2);
#endif
    REQUIRE(02_z1 == 2);
    REQUIRE(2_z1 == 2);
    REQUIRE(0x2_z1 == 2);

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b10_z2 == 2);
#endif
    REQUIRE(02_z2 == 2);
    REQUIRE(2_z2 == 2);
    REQUIRE(0x2_z2 == 2);

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b10_z3 == 2);
#endif
    REQUIRE(02_z3 == 2);
    REQUIRE(2_z3 == 2);
    REQUIRE(0x2_z3 == 2);

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b101010_z1 == 42);
#endif
    REQUIRE(052_z1 == 42);
    REQUIRE(42_z1 == 42);
    REQUIRE(0x2a_z1 == 42);

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b101010_z2 == 42);
#endif
    REQUIRE(052_z2 == 42);
    REQUIRE(42_z2 == 42);
    REQUIRE(0x2a_z2 == 42);

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b101010_z2 == 42);
#endif
    REQUIRE(052_z2 == 42);
    REQUIRE(42_z2 == 42);
    REQUIRE(0x2a_z2 == 42);

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b101010_z3 == 42);
#endif
    REQUIRE(052_z3 == 42);
    REQUIRE(42_z3 == 42);
    REQUIRE(0x2a_z3 == 42);

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(-0b101010_z1 == -42);
#endif
    REQUIRE(-052_z1 == -42);
    REQUIRE(-42_z1 == -42);
    REQUIRE(-0x2a_z1 == -42);

    // Tests with exactly 1 64-bit limb.
#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b1001010000010100011000010100111000100000110111001001110111111011_z1
            == integer<1>{"10670260405334220283"});
#endif
    REQUIRE(01120243024704067116773_z1 == integer<1>{"10670260405334220283"});
    REQUIRE(10670260405334220283_z1 == integer<1>{"10670260405334220283"});
    REQUIRE(0x9414614e20dc9dfb_z1 == integer<1>{"10670260405334220283"});

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(-0b1001010000010100011000010100111000100000110111001001110111111011_z1
            == integer<1>{"-10670260405334220283"});
#endif
    REQUIRE(-01120243024704067116773_z1 == integer<1>{"-10670260405334220283"});
    REQUIRE(-10670260405334220283_z1 == integer<1>{"-10670260405334220283"});
    REQUIRE(-0x9414614e20dc9dfb_z1 == integer<1>{"-10670260405334220283"});

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b1001010000010100011000010100111000100000110111001001110111111011_z2
            == integer<2>{"10670260405334220283"});
#endif
    REQUIRE(01120243024704067116773_z2 == integer<2>{"10670260405334220283"});
    REQUIRE(10670260405334220283_z2 == integer<2>{"10670260405334220283"});
    REQUIRE(0x9414614e20dc9dfb_z2 == integer<2>{"10670260405334220283"});

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(-0b1001010000010100011000010100111000100000110111001001110111111011_z2
            == integer<2>{"-10670260405334220283"});
#endif
    REQUIRE(-01120243024704067116773_z2 == integer<2>{"-10670260405334220283"});
    REQUIRE(-10670260405334220283_z2 == integer<2>{"-10670260405334220283"});
    REQUIRE(-0x9414614e20dc9dfb_z2 == integer<2>{"-10670260405334220283"});

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b1001010000010100011000010100111000100000110111001001110111111011_z3
            == integer<3>{"10670260405334220283"});
#endif
    REQUIRE(01120243024704067116773_z3 == integer<3>{"10670260405334220283"});
    REQUIRE(10670260405334220283_z3 == integer<3>{"10670260405334220283"});
    REQUIRE(0x9414614e20dc9dfb_z3 == integer<3>{"10670260405334220283"});

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(-0b1001010000010100011000010100111000100000110111001001110111111011_z3
            == integer<3>{"-10670260405334220283"});
#endif
    REQUIRE(-01120243024704067116773_z3 == integer<3>{"-10670260405334220283"});
    REQUIRE(-10670260405334220283_z3 == integer<3>{"-10670260405334220283"});
    REQUIRE(-0x9414614e20dc9dfb_z3 == integer<3>{"-10670260405334220283"});

    // 1 limb and a bit.
#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b1010100111001001001010001011000000011110110010001100111011011100011011_z1
            == integer<1>("782998694375967667995"));
#endif
    REQUIRE(0124711121300366214733433_z1 == integer<1>("782998694375967667995"));
    REQUIRE(782998694375967667995_z1 == integer<1>("782998694375967667995"));
    REQUIRE(0x2a724a2c07b233b71b_z1 == integer<1>("782998694375967667995"));

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0b1010100111001001001010001011000000011110110010001100111011011100011011_z2
            == integer<2>("782998694375967667995"));
#endif
    REQUIRE(0124711121300366214733433_z2 == integer<2>("782998694375967667995"));
    REQUIRE(782998694375967667995_z2 == integer<2>("782998694375967667995"));
    REQUIRE(0x2a724a2c07b233b71b_z2 == integer<2>("782998694375967667995"));

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(0B1010100111001001001010001011000000011110110010001100111011011100011011_z3
            == integer<3>("782998694375967667995"));
#endif
    REQUIRE(0124711121300366214733433_z3 == integer<3>("782998694375967667995"));
    REQUIRE(782998694375967667995_z3 == integer<3>("782998694375967667995"));
    REQUIRE(0x2a724a2c07b233b71b_z3 == integer<3>("782998694375967667995"));

    // Exactly 2 limbs.
#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(
        0b10111111111101110110010111011111110101011011001000010110110010000001001000000100001111100000110011011011110001111101111100111000_z1
        == integer<1>{"255167110776362847119081855462662463288"});
#endif
    REQUIRE(02777566273765331026620110041740633361757470_z1 == integer<1>{"255167110776362847119081855462662463288"});
    REQUIRE(255167110776362847119081855462662463288_z1 == integer<1>{"255167110776362847119081855462662463288"});
    REQUIRE(0Xbff765dfd5b216c812043e0cdbc7df38_z1 == integer<1>{"255167110776362847119081855462662463288"});

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(
        0b10111111111101110110010111011111110101011011001000010110110010000001001000000100001111100000110011011011110001111101111100111000_z2
        == integer<2>{"255167110776362847119081855462662463288"});
#endif
    REQUIRE(02777566273765331026620110041740633361757470_z2 == integer<2>{"255167110776362847119081855462662463288"});
    REQUIRE(255167110776362847119081855462662463288_z2 == integer<2>{"255167110776362847119081855462662463288"});
    REQUIRE(0Xbff765dfd5b216c812043e0cdbc7df38_z2 == integer<2>{"255167110776362847119081855462662463288"});

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(
        0b10111111111101110110010111011111110101011011001000010110110010000001001000000100001111100000110011011011110001111101111100111000_z3
        == integer<3>{"255167110776362847119081855462662463288"});
#endif
    REQUIRE(02777566273765331026620110041740633361757470_z3 == integer<3>{"255167110776362847119081855462662463288"});
    REQUIRE(255167110776362847119081855462662463288_z3 == integer<3>{"255167110776362847119081855462662463288"});
    REQUIRE(0xbff765dfd5b216c812043e0cdbc7df38_z3 == integer<3>{"255167110776362847119081855462662463288"});

    // Two limbs and a bit.
#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(
        0b11011000101111000000000011010001111011111111010000110101111111001010011100011101100100000011010011011110010111110000111110000111111000101101010100101000_z1
        == integer<1>{"4833338351692348778406678639329659114009842984"});
#endif
    REQUIRE(0330570003217377206577123435440323362760760770552450_z1
            == integer<1>{"4833338351692348778406678639329659114009842984"});
    REQUIRE(4833338351692348778406678639329659114009842984_z1
            == integer<1>{"4833338351692348778406678639329659114009842984"});
    REQUIRE(0xd8bc00d1eff435fca71d9034de5f0f87e2d528_z1
            == integer<1>{"4833338351692348778406678639329659114009842984"});

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(
        0b11011000101111000000000011010001111011111111010000110101111111001010011100011101100100000011010011011110010111110000111110000111111000101101010100101000_z2
        == integer<2>{"4833338351692348778406678639329659114009842984"});
#endif
    REQUIRE(0330570003217377206577123435440323362760760770552450_z2
            == integer<2>{"4833338351692348778406678639329659114009842984"});
    REQUIRE(4833338351692348778406678639329659114009842984_z2
            == integer<2>{"4833338351692348778406678639329659114009842984"});
    REQUIRE(0xd8bc00d1eff435fca71d9034de5f0f87e2d528_z2
            == integer<2>{"4833338351692348778406678639329659114009842984"});

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(
        0b11011000101111000000000011010001111011111111010000110101111111001010011100011101100100000011010011011110010111110000111110000111111000101101010100101000_z3
        == integer<3>{"4833338351692348778406678639329659114009842984"});
#endif
    REQUIRE(0330570003217377206577123435440323362760760770552450_z3
            == integer<3>{"4833338351692348778406678639329659114009842984"});
    REQUIRE(4833338351692348778406678639329659114009842984_z3
            == integer<3>{"4833338351692348778406678639329659114009842984"});
    REQUIRE(0xd8bc00d1eff435fca71d9034de5f0f87e2d528_z3
            == integer<3>{"4833338351692348778406678639329659114009842984"});

    // Exactly 3 limbs.
#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(
        0b101001000011010011111110110000000110100111011011110100001011000010000111010001111010011011010011100100010100110100011001100101000000001010001100111001000010101011011100010100111101100111110101_z1
        == integer<1>{"4026344223635032748846650355469673371363642177767033002485"});
#endif
    REQUIRE(05103237660064733641302072172332344246431450012147102533424754765_z1
            == integer<1>{"4026344223635032748846650355469673371363642177767033002485"});
    REQUIRE(4026344223635032748846650355469673371363642177767033002485_z1
            == integer<1>{"4026344223635032748846650355469673371363642177767033002485"});
    REQUIRE(0xa434fec069dbd0b08747a6d3914d1994028ce42adc53d9f5_z1
            == integer<1>{"4026344223635032748846650355469673371363642177767033002485"});

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(
        0b101001000011010011111110110000000110100111011011110100001011000010000111010001111010011011010011100100010100110100011001100101000000001010001100111001000010101011011100010100111101100111110101_z2
        == integer<2>{"4026344223635032748846650355469673371363642177767033002485"});
#endif
    REQUIRE(05103237660064733641302072172332344246431450012147102533424754765_z2
            == integer<2>{"4026344223635032748846650355469673371363642177767033002485"});
    REQUIRE(4026344223635032748846650355469673371363642177767033002485_z2
            == integer<2>{"4026344223635032748846650355469673371363642177767033002485"});
    REQUIRE(0xa434fec069dbd0b08747a6d3914d1994028ce42adc53d9f5_z2
            == integer<2>{"4026344223635032748846650355469673371363642177767033002485"});

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(
        0b101001000011010011111110110000000110100111011011110100001011000010000111010001111010011011010011100100010100110100011001100101000000001010001100111001000010101011011100010100111101100111110101_z3
        == integer<3>{"4026344223635032748846650355469673371363642177767033002485"});
#endif
    REQUIRE(05103237660064733641302072172332344246431450012147102533424754765_z3
            == integer<3>{"4026344223635032748846650355469673371363642177767033002485"});
    REQUIRE(4026344223635032748846650355469673371363642177767033002485_z3
            == integer<3>{"4026344223635032748846650355469673371363642177767033002485"});
    REQUIRE(0xa434fec069dbd0b08747a6d3914d1994028ce42adc53d9f5_z3
            == integer<3>{"4026344223635032748846650355469673371363642177767033002485"});

    // 3 limbs and a bit.
#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(
        0b11110001110000011001100010111011111001000111011110010100101000100001111111111000111000011010111101111000100010111011100000010000110010001011111001001000011001100000010010111010101011001001110101010010100_z1
        == integer<1>{"12140227947719468759797663285047392109678128832104635964910228"});
#endif
    REQUIRE(036160314273710736245041777070327570427340206213711031402272531165224_z1
            == integer<1>{"12140227947719468759797663285047392109678128832104635964910228"});
    REQUIRE(12140227947719468759797663285047392109678128832104635964910228_z1
            == integer<1>{"12140227947719468759797663285047392109678128832104635964910228"});
    REQUIRE(0x78e0cc5df23bca510ffc70d7bc45dc08645f2433025d564ea94_z1
            == integer<1>{"12140227947719468759797663285047392109678128832104635964910228"});

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(
        0b11110001110000011001100010111011111001000111011110010100101000100001111111111000111000011010111101111000100010111011100000010000110010001011111001001000011001100000010010111010101011001001110101010010100_z2
        == integer<2>{"12140227947719468759797663285047392109678128832104635964910228"});
#endif
    REQUIRE(036160314273710736245041777070327570427340206213711031402272531165224_z2
            == integer<2>{"12140227947719468759797663285047392109678128832104635964910228"});
    REQUIRE(12140227947719468759797663285047392109678128832104635964910228_z2
            == integer<2>{"12140227947719468759797663285047392109678128832104635964910228"});
    REQUIRE(0x78e0cc5df23bca510ffc70d7bc45dc08645f2433025d564ea94_z2
            == integer<2>{"12140227947719468759797663285047392109678128832104635964910228"});

#if MPPP_CPLUSPLUS >= 201402L
    REQUIRE(
        0b11110001110000011001100010111011111001000111011110010100101000100001111111111000111000011010111101111000100010111011100000010000110010001011111001001000011001100000010010111010101011001001110101010010100_z3
        == integer<3>{"12140227947719468759797663285047392109678128832104635964910228"});
#endif
    REQUIRE(036160314273710736245041777070327570427340206213711031402272531165224_z3
            == integer<3>{"12140227947719468759797663285047392109678128832104635964910228"});
    REQUIRE(12140227947719468759797663285047392109678128832104635964910228_z3
            == integer<3>{"12140227947719468759797663285047392109678128832104635964910228"});
    REQUIRE(0x78e0cc5df23bca510ffc70d7bc45dc08645f2433025d564ea94_z3
            == integer<3>{"12140227947719468759797663285047392109678128832104635964910228"});
}

// Code from the tutorial.
TEST_CASE("integer_literals_tutorial")
{
    // NOLINTNEXTLINE(google-build-using-namespace)
    using namespace mppp::literals;

    auto n1 = 123_z1; // n1 has 1 limb of static storage,
                      // and it contains the value 123.

#if MPPP_CPLUSPLUS >= 201402L
    auto n2 = -0b10011_z2; // n2 has 2 limbs of static storage,
                           // and it contains the value -19
                           // (-10011 in base 2).
#endif

    auto n3 = 0146_z1; // n3 has 1 limb of static storage,
                       // and it contains the value 102
                       // (146 in base 8).

    auto n4 = 0xfe45_z3; // n4 has 3 limbs of static storage,
                         // and it contains the value 65093
                         // (fe45 in base 16).
}

// Test use with initializer lists.
TEST_CASE("integer_literals_init_list")
{
    std::vector<mppp::integer<1>> v = {1_z1, 2_z1, 3_z1};

    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 3);
}
