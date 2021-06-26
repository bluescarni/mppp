// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include <mp++/integer.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

static inline void runner_impl(std::ostringstream &) {}

template <typename F, typename... Flags>
static inline void runner_impl(std::ostringstream &oss, F &&f, Flags &&...flags)
{
    oss << std::forward<F>(f);
    runner_impl(oss, std::forward<Flags>(flags)...);
}

template <typename Int, typename... Flags>
static inline std::string runner(const Int &n, Flags &&...flags)
{
    std::ostringstream oss;
    runner_impl(oss, std::forward<Flags>(flags)...);
    oss << n;
    return oss.str();
}

struct out_tester {
    template <typename S>
    // NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;

        REQUIRE(runner(integer{0}, std::dec) == "0");
        REQUIRE(runner(integer{0}, std::oct) == "0");
        REQUIRE(runner(integer{0}, std::hex) == "0");

        REQUIRE(runner(integer{0}, std::dec, std::showbase) == "0");
        REQUIRE(runner(integer{0}, std::oct, std::showbase) == "0");
        REQUIRE(runner(integer{0}, std::hex, std::showbase) == "0");

        REQUIRE(runner(integer{0}, std::dec, std::showbase, std::uppercase) == "0");
        REQUIRE(runner(integer{0}, std::oct, std::showbase, std::uppercase) == "0");
        REQUIRE(runner(integer{0}, std::hex, std::showbase, std::uppercase) == "0");

        REQUIRE(runner(integer{1}, std::dec) == "1");
        REQUIRE(runner(integer{1}, std::oct) == "1");
        REQUIRE(runner(integer{1}, std::hex) == "1");

        REQUIRE(runner(integer{1}, std::dec, std::showbase) == "1");
        REQUIRE(runner(integer{1}, std::oct, std::showbase) == "01");
        REQUIRE(runner(integer{1}, std::hex, std::showbase) == "0x1");

        REQUIRE(runner(integer{1}, std::dec, std::showbase, std::uppercase) == "1");
        REQUIRE(runner(integer{1}, std::oct, std::showbase, std::uppercase) == "01");
        REQUIRE(runner(integer{1}, std::hex, std::showbase, std::uppercase) == "0X1");

        REQUIRE(runner(integer{-1}, std::dec) == "-1");
        REQUIRE(runner(integer{-1}, std::oct) == "-1");
        REQUIRE(runner(integer{-1}, std::hex) == "-1");

        REQUIRE(runner(integer{-1}, std::dec, std::showbase) == "-1");
        REQUIRE(runner(integer{-1}, std::oct, std::showbase) == "-01");
        REQUIRE(runner(integer{-1}, std::hex, std::showbase) == "-0x1");

        REQUIRE(runner(integer{-1}, std::dec, std::showbase, std::uppercase) == "-1");
        REQUIRE(runner(integer{-1}, std::oct, std::showbase, std::uppercase) == "-01");
        REQUIRE(runner(integer{-1}, std::hex, std::showbase, std::uppercase) == "-0X1");

        REQUIRE(runner(integer{42}, std::dec) == "42");
        REQUIRE(runner(integer{42}, std::oct) == "52");
        REQUIRE(runner(integer{42}, std::hex) == "2a");

        REQUIRE(runner(integer{42}, std::setbase(10)) == "42");
        REQUIRE(runner(integer{42}, std::setbase(8)) == "52");
        REQUIRE(runner(integer{42}, std::setbase(16)) == "2a");
        REQUIRE(runner(integer{42}, std::setbase(0)) == "42");
        REQUIRE(runner(integer{42}, std::setbase(-1)) == "42");
        REQUIRE(runner(integer{42}, std::setbase(1)) == "42");
        REQUIRE(runner(integer{42}, std::setbase(3)) == "42");

        REQUIRE(runner(integer{42}, std::dec, std::showbase) == "42");
        REQUIRE(runner(integer{42}, std::oct, std::showbase) == "052");
        REQUIRE(runner(integer{42}, std::hex, std::showbase) == "0x2a");
        REQUIRE(runner(integer{227191947ll}, std::hex, std::showbase) == "0xd8aac8b");
        REQUIRE(runner(integer{-227191947ll}, std::hex, std::showbase) == "-0xd8aac8b");

        REQUIRE(runner(integer{42}, std::setbase(10), std::showbase) == "42");
        REQUIRE(runner(integer{42}, std::setbase(8), std::showbase) == "052");
        REQUIRE(runner(integer{42}, std::setbase(16), std::showbase) == "0x2a");
        REQUIRE(runner(integer{42}, std::setbase(0), std::showbase) == "42");
        REQUIRE(runner(integer{42}, std::setbase(-1), std::showbase) == "42");
        REQUIRE(runner(integer{42}, std::setbase(1), std::showbase) == "42");
        REQUIRE(runner(integer{42}, std::setbase(3), std::showbase) == "42");

        REQUIRE(runner(integer{42}, std::dec, std::showbase, std::uppercase) == "42");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase) == "052");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase) == "0X2A");
        REQUIRE(runner(integer{227191947ll}, std::hex, std::showbase, std::uppercase) == "0XD8AAC8B");
        REQUIRE(runner(integer{-227191947ll}, std::hex, std::showbase, std::uppercase) == "-0XD8AAC8B");

        REQUIRE(runner(integer{42}, std::dec, std::showpos) == "+42");
        REQUIRE(runner(integer{42}, std::oct, std::showpos) == "+52");
        REQUIRE(runner(integer{42}, std::hex, std::showpos) == "+2a");

        REQUIRE(runner(integer{42}, std::dec, std::showbase, std::showpos) == "+42");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::showpos) == "+052");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::showpos) == "+0x2a");

        REQUIRE(runner(integer{42}, std::dec, std::showbase, std::uppercase, std::showpos) == "+42");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos) == "+052");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos) == "+0X2A");

        REQUIRE(runner(integer{-42}, std::dec) == "-42");
        REQUIRE(runner(integer{-42}, std::oct) == "-52");
        REQUIRE(runner(integer{-42}, std::hex) == "-2a");

        REQUIRE(runner(integer{-42}, std::setbase(10)) == "-42");
        REQUIRE(runner(integer{-42}, std::setbase(8)) == "-52");
        REQUIRE(runner(integer{-42}, std::setbase(16)) == "-2a");
        REQUIRE(runner(integer{-42}, std::setbase(0)) == "-42");
        REQUIRE(runner(integer{-42}, std::setbase(-1)) == "-42");
        REQUIRE(runner(integer{-42}, std::setbase(1)) == "-42");
        REQUIRE(runner(integer{-42}, std::setbase(3)) == "-42");

        REQUIRE(runner(integer{-42}, std::dec, std::showbase) == "-42");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase) == "-052");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase) == "-0x2a");

        REQUIRE(runner(integer{-42}, std::dec, std::showbase, std::uppercase) == "-42");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase) == "-052");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase) == "-0X2A");

        REQUIRE(runner(integer{-42}, std::setbase(10), std::showbase) == "-42");
        REQUIRE(runner(integer{-42}, std::setbase(8), std::showbase) == "-052");
        REQUIRE(runner(integer{-42}, std::setbase(16), std::showbase) == "-0x2a");
        REQUIRE(runner(integer{-42}, std::setbase(0), std::showbase) == "-42");
        REQUIRE(runner(integer{-42}, std::setbase(-1), std::showbase) == "-42");
        REQUIRE(runner(integer{-42}, std::setbase(1), std::showbase) == "-42");
        REQUIRE(runner(integer{-42}, std::setbase(3), std::showbase) == "-42");

        REQUIRE(runner(integer{-42}, std::dec, std::showpos) == "-42");
        REQUIRE(runner(integer{-42}, std::oct, std::showpos) == "-52");
        REQUIRE(runner(integer{-42}, std::hex, std::showpos) == "-2a");

        REQUIRE(runner(integer{-42}, std::dec, std::showbase, std::showpos) == "-42");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::showpos) == "-052");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::showpos) == "-0x2a");

        REQUIRE(runner(integer{-42}, std::dec, std::showbase, std::uppercase, std::showpos) == "-42");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos) == "-052");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos) == "-0X2A");

        // Tests with default fill (right).
        REQUIRE(runner(integer{0}, std::setw(0)) == "0");
        REQUIRE(runner(integer{0}, std::setw(-1)) == "0");
        REQUIRE(runner(integer{0}, std::setw(-2)) == "0");
        REQUIRE(runner(integer{0}, std::setw(1)) == "0");
        REQUIRE(runner(integer{0}, std::setw(2)) == " 0");
        REQUIRE(runner(integer{0}, std::setw(10)) == "         0");

        REQUIRE(runner(integer{42}, std::setw(0)) == "42");
        REQUIRE(runner(integer{42}, std::setw(-1)) == "42");
        REQUIRE(runner(integer{42}, std::setw(-2)) == "42");
        REQUIRE(runner(integer{42}, std::setw(1)) == "42");
        REQUIRE(runner(integer{42}, std::setw(2)) == "42");
        REQUIRE(runner(integer{42}, std::setw(10)) == "        42");

        REQUIRE(runner(integer{-42}, std::setw(0)) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(-1)) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(-2)) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(1)) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(2)) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(10)) == "       -42");

        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10))
                == "     +0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3)) == "+0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4)) == "+0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5)) == "+0X2A");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10))
                == "      +052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3)) == "+052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4)) == "+052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5)) == " +052");

        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10))
                == "     -0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3)) == "-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4)) == "-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5)) == "-0X2A");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10))
                == "      -052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3)) == "-052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4)) == "-052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5)) == " -052");

        REQUIRE(
            runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::setfill('*'))
            == "*****+0X2A");
        REQUIRE(
            runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::setfill('a'))
            == "aaaaa+0X2A");
        REQUIRE(
            runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::setfill('*'))
            == "+0X2A");
        REQUIRE(
            runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::setfill('*'))
            == "+0X2A");
        REQUIRE(
            runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::setfill('*'))
            == "+0X2A");
        REQUIRE(
            runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::setfill('*'))
            == "******+052");
        REQUIRE(
            runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::setfill('*'))
            == "+052");
        REQUIRE(
            runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::setfill('*'))
            == "+052");
        REQUIRE(
            runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::setfill('*'))
            == "*+052");

        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'))
                == "*****-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('a'))
                == "aaaaa-0X2A");
        REQUIRE(
            runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::setfill('*'))
            == "-0X2A");
        REQUIRE(
            runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::setfill('*'))
            == "-0X2A");
        REQUIRE(
            runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::setfill('*'))
            == "-0X2A");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'))
                == "******-052");
        REQUIRE(
            runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::setfill('*'))
            == "-052");
        REQUIRE(
            runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::setfill('*'))
            == "-052");
        REQUIRE(
            runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::setfill('*'))
            == "*-052");

        // Tests with right fill.
        REQUIRE(runner(integer{0}, std::setw(0), std::right) == "0");
        REQUIRE(runner(integer{0}, std::setw(-1), std::right) == "0");
        REQUIRE(runner(integer{0}, std::setw(-2), std::right) == "0");
        REQUIRE(runner(integer{0}, std::setw(1), std::right) == "0");
        REQUIRE(runner(integer{0}, std::setw(2), std::right) == " 0");
        REQUIRE(runner(integer{0}, std::setw(10), std::right) == "         0");

        REQUIRE(runner(integer{42}, std::setw(0), std::right) == "42");
        REQUIRE(runner(integer{42}, std::setw(-1), std::right) == "42");
        REQUIRE(runner(integer{42}, std::setw(-2), std::right) == "42");
        REQUIRE(runner(integer{42}, std::setw(1), std::right) == "42");
        REQUIRE(runner(integer{42}, std::setw(2), std::right) == "42");
        REQUIRE(runner(integer{42}, std::setw(10), std::right) == "        42");

        REQUIRE(runner(integer{-42}, std::setw(0), std::right) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(-1), std::right) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(-2), std::right) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(1), std::right) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(2), std::right) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(10), std::right) == "       -42");

        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::right)
                == "     +0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::right)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::right)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::right)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::right)
                == "      +052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::right)
                == "+052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::right)
                == "+052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::right)
                == " +052");

        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::right)
                == "     -0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::right)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::right)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::right)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::right)
                == "      -052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::right)
                == "-052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::right)
                == "-052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::right)
                == " -052");

        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::right)
                == "*****+0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('a'), std::right)
                == "aaaaa+0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::right)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::right)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::right)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::right)
                == "******+052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::right)
                == "+052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::right)
                == "+052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::right)
                == "*+052");

        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::right)
                == "*****-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('a'), std::right)
                == "aaaaa-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::right)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::right)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::right)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::right)
                == "******-052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::right)
                == "-052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::right)
                == "-052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::right)
                == "*-052");

        // Left fill.
        REQUIRE(runner(integer{0}, std::setw(0), std::left) == "0");
        REQUIRE(runner(integer{0}, std::setw(-1), std::left) == "0");
        REQUIRE(runner(integer{0}, std::setw(-2), std::left) == "0");
        REQUIRE(runner(integer{0}, std::setw(1), std::left) == "0");
        REQUIRE(runner(integer{0}, std::setw(2), std::left) == "0 ");
        REQUIRE(runner(integer{0}, std::setw(10), std::left) == "0         ");

        REQUIRE(runner(integer{42}, std::setw(0), std::left) == "42");
        REQUIRE(runner(integer{42}, std::setw(-1), std::left) == "42");
        REQUIRE(runner(integer{42}, std::setw(-2), std::left) == "42");
        REQUIRE(runner(integer{42}, std::setw(1), std::left) == "42");
        REQUIRE(runner(integer{42}, std::setw(2), std::left) == "42");
        REQUIRE(runner(integer{42}, std::setw(10), std::left) == "42        ");

        REQUIRE(runner(integer{-42}, std::setw(0), std::left) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(-1), std::left) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(-2), std::left) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(1), std::left) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(2), std::left) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(10), std::left) == "-42       ");

        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::left)
                == "+0X2A     ");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::left)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::left)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::left)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::left)
                == "+052      ");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::left)
                == "+052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::left)
                == "+052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::left)
                == "+052 ");

        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::left)
                == "-0X2A     ");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::left)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::left)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::left)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::left)
                == "-052      ");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::left)
                == "-052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::left)
                == "-052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::left)
                == "-052 ");

        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::left)
                == "+0X2A*****");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('a'), std::left)
                == "+0X2Aaaaaa");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::left)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::left)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::left)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::left)
                == "+052******");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::left)
                == "+052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::left)
                == "+052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::left)
                == "+052*");

        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::left)
                == "-0X2A*****");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('a'), std::left)
                == "-0X2Aaaaaa");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::left)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::left)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::left)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::left)
                == "-052******");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::left)
                == "-052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::left)
                == "-052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::left)
                == "-052*");

        // Internal fill.
        REQUIRE(runner(integer{0}, std::setw(0), std::internal) == "0");
        REQUIRE(runner(integer{0}, std::setw(-1), std::internal) == "0");
        REQUIRE(runner(integer{0}, std::setw(-2), std::internal) == "0");
        REQUIRE(runner(integer{0}, std::setw(1), std::internal) == "0");
        REQUIRE(runner(integer{0}, std::setw(2), std::internal) == " 0");
        REQUIRE(runner(integer{0}, std::setw(10), std::internal) == "         0");

        REQUIRE(runner(integer{42}, std::setw(0), std::internal) == "42");
        REQUIRE(runner(integer{42}, std::setw(-1), std::internal) == "42");
        REQUIRE(runner(integer{42}, std::setw(-2), std::internal) == "42");
        REQUIRE(runner(integer{42}, std::setw(1), std::internal) == "42");
        REQUIRE(runner(integer{42}, std::setw(2), std::internal) == "42");
        REQUIRE(runner(integer{42}, std::setw(10), std::internal) == "        42");

        REQUIRE(runner(integer{-42}, std::setw(0), std::internal) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(-1), std::internal) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(-2), std::internal) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(1), std::internal) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(2), std::internal) == "-42");
        REQUIRE(runner(integer{-42}, std::setw(10), std::internal) == "-       42");

        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::internal)
                == "+0X     2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::internal)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::internal)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::internal)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::internal)
                == "+0      52");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::internal)
                == "+052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::internal)
                == "+052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::internal)
                == "+0 52");

        REQUIRE(
            runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::internal)
            == "-0X     2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::internal)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::internal)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::internal)
                == "-0X2A");
        REQUIRE(
            runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::internal)
            == "-0      52");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::internal)
                == "-052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::internal)
                == "-052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::internal)
                == "-0 52");

        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::internal)
                == "+0X*****2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('a'), std::internal)
                == "+0Xaaaaa2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::internal)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::internal)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::internal)
                == "+0X2A");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::internal)
                == "+0******52");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::internal)
                == "+052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::internal)
                == "+052");
        REQUIRE(runner(integer{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::internal)
                == "+0*52");

        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::internal)
                == "-0X*****2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('a'), std::internal)
                == "-0Xaaaaa2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::internal)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::internal)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::internal)
                == "-0X2A");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::internal)
                == "-0******52");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::internal)
                == "-052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::internal)
                == "-052");
        REQUIRE(runner(integer{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::internal)
                == "-0*52");

        // A test to make sure that the stream width is reset to zero properly.
        {
            std::ostringstream oss;
            oss << std::setfill('a') << std::setw(10) << integer{42} << "\n\n\n";
            REQUIRE(oss.str() == "aaaaaaaa42\n\n\n");
        }
    }
};

TEST_CASE("out test")
{
    tuple_for_each(sizes{}, out_tester{});
}
