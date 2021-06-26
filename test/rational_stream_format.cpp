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

#include <mp++/rational.hpp>

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
        using rational = rational<S::value>;

        REQUIRE(runner(rational{0}, std::dec) == "0");
        REQUIRE(runner(rational{0}, std::oct) == "0");
        REQUIRE(runner(rational{0}, std::hex) == "0");

        REQUIRE(runner(rational{0}, std::dec, std::showbase) == "0");
        REQUIRE(runner(rational{0}, std::oct, std::showbase) == "0");
        REQUIRE(runner(rational{0}, std::hex, std::showbase) == "0");

        REQUIRE(runner(rational{0}, std::dec, std::showbase, std::uppercase) == "0");
        REQUIRE(runner(rational{0}, std::oct, std::showbase, std::uppercase) == "0");
        REQUIRE(runner(rational{0}, std::hex, std::showbase, std::uppercase) == "0");

        REQUIRE(runner(rational{1}, std::dec) == "1");
        REQUIRE(runner(rational{1}, std::oct) == "1");
        REQUIRE(runner(rational{1}, std::hex) == "1");

        REQUIRE(runner(rational{1}, std::dec, std::showbase) == "1");
        REQUIRE(runner(rational{1}, std::oct, std::showbase) == "01");
        REQUIRE(runner(rational{1}, std::hex, std::showbase) == "0x1");

        REQUIRE(runner(rational{1}, std::dec, std::showbase, std::uppercase) == "1");
        REQUIRE(runner(rational{1}, std::oct, std::showbase, std::uppercase) == "01");
        REQUIRE(runner(rational{1}, std::hex, std::showbase, std::uppercase) == "0X1");

        REQUIRE(runner(rational{-1}, std::dec) == "-1");
        REQUIRE(runner(rational{-1}, std::oct) == "-1");
        REQUIRE(runner(rational{-1}, std::hex) == "-1");

        REQUIRE(runner(rational{-1}, std::dec, std::showbase) == "-1");
        REQUIRE(runner(rational{-1}, std::oct, std::showbase) == "-01");
        REQUIRE(runner(rational{-1}, std::hex, std::showbase) == "-0x1");

        REQUIRE(runner(rational{-1}, std::dec, std::showbase, std::uppercase) == "-1");
        REQUIRE(runner(rational{-1}, std::oct, std::showbase, std::uppercase) == "-01");
        REQUIRE(runner(rational{-1}, std::hex, std::showbase, std::uppercase) == "-0X1");

        REQUIRE(runner(rational{1, 2}, std::dec) == "1/2");
        REQUIRE(runner(rational{1, 2}, std::oct) == "1/2");
        REQUIRE(runner(rational{1, 2}, std::hex) == "1/2");

        REQUIRE(runner(rational{1, 2}, std::dec, std::showbase) == "1/2");
        REQUIRE(runner(rational{1, 2}, std::oct, std::showbase) == "01/02");
        REQUIRE(runner(rational{1, 2}, std::hex, std::showbase) == "0x1/0x2");

        REQUIRE(runner(rational{1, 2}, std::dec, std::showbase, std::uppercase) == "1/2");
        REQUIRE(runner(rational{1, 2}, std::oct, std::showbase, std::uppercase) == "01/02");
        REQUIRE(runner(rational{1, 2}, std::hex, std::showbase, std::uppercase) == "0X1/0X2");

        REQUIRE(runner(rational{-1, 2}, std::dec) == "-1/2");
        REQUIRE(runner(rational{-1, 2}, std::oct) == "-1/2");
        REQUIRE(runner(rational{-1, 2}, std::hex) == "-1/2");

        REQUIRE(runner(rational{-1, 2}, std::dec, std::showbase) == "-1/2");
        REQUIRE(runner(rational{-1, 2}, std::oct, std::showbase) == "-01/02");
        REQUIRE(runner(rational{-1, 2}, std::hex, std::showbase) == "-0x1/0x2");

        REQUIRE(runner(rational{-1, 2}, std::dec, std::showbase, std::uppercase) == "-1/2");
        REQUIRE(runner(rational{-1, 2}, std::oct, std::showbase, std::uppercase) == "-01/02");
        REQUIRE(runner(rational{-1, 2}, std::hex, std::showbase, std::uppercase) == "-0X1/0X2");

        REQUIRE(runner(rational{42}, std::dec) == "42");
        REQUIRE(runner(rational{42}, std::oct) == "52");
        REQUIRE(runner(rational{42}, std::hex) == "2a");

        REQUIRE(runner(rational{42, 13}, std::dec) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::oct) == "52/15");
        REQUIRE(runner(rational{42, 13}, std::hex) == "2a/d");

        REQUIRE(runner(rational{42}, std::setbase(10)) == "42");
        REQUIRE(runner(rational{42}, std::setbase(8)) == "52");
        REQUIRE(runner(rational{42}, std::setbase(16)) == "2a");
        REQUIRE(runner(rational{42}, std::setbase(0)) == "42");
        REQUIRE(runner(rational{42}, std::setbase(-1)) == "42");
        REQUIRE(runner(rational{42}, std::setbase(1)) == "42");
        REQUIRE(runner(rational{42}, std::setbase(3)) == "42");

        REQUIRE(runner(rational{42, 13}, std::setbase(10)) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setbase(8)) == "52/15");
        REQUIRE(runner(rational{42, 13}, std::setbase(16)) == "2a/d");
        REQUIRE(runner(rational{42, 13}, std::setbase(0)) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setbase(-1)) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setbase(1)) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setbase(3)) == "42/13");

        REQUIRE(runner(rational{42}, std::dec, std::showbase) == "42");
        REQUIRE(runner(rational{42}, std::oct, std::showbase) == "052");
        REQUIRE(runner(rational{42}, std::hex, std::showbase) == "0x2a");
        REQUIRE(runner(rational{227191947ll}, std::hex, std::showbase) == "0xd8aac8b");
        REQUIRE(runner(rational{-227191947ll}, std::hex, std::showbase) == "-0xd8aac8b");

        REQUIRE(runner(rational{42, 13}, std::dec, std::showbase) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase) == "052/015");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase) == "0x2a/0xd");
        REQUIRE(runner(rational{227191947ll, 13}, std::hex, std::showbase) == "0xd8aac8b/0xd");
        REQUIRE(runner(rational{-227191947ll, 13}, std::hex, std::showbase) == "-0xd8aac8b/0xd");

        REQUIRE(runner(rational{42}, std::setbase(10), std::showbase) == "42");
        REQUIRE(runner(rational{42}, std::setbase(8), std::showbase) == "052");
        REQUIRE(runner(rational{42}, std::setbase(16), std::showbase) == "0x2a");
        REQUIRE(runner(rational{42}, std::setbase(0), std::showbase) == "42");
        REQUIRE(runner(rational{42}, std::setbase(-1), std::showbase) == "42");
        REQUIRE(runner(rational{42}, std::setbase(1), std::showbase) == "42");
        REQUIRE(runner(rational{42}, std::setbase(3), std::showbase) == "42");

        REQUIRE(runner(rational{42, 13}, std::setbase(10), std::showbase) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setbase(8), std::showbase) == "052/015");
        REQUIRE(runner(rational{42, 13}, std::setbase(16), std::showbase) == "0x2a/0xd");
        REQUIRE(runner(rational{42, 13}, std::setbase(0), std::showbase) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setbase(-1), std::showbase) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setbase(1), std::showbase) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setbase(3), std::showbase) == "42/13");

        REQUIRE(runner(rational{42}, std::dec, std::showbase, std::uppercase) == "42");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase) == "052");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase) == "0X2A");
        REQUIRE(runner(rational{227191947ll}, std::hex, std::showbase, std::uppercase) == "0XD8AAC8B");
        REQUIRE(runner(rational{-227191947ll}, std::hex, std::showbase, std::uppercase) == "-0XD8AAC8B");

        REQUIRE(runner(rational{42, 13}, std::dec, std::showbase, std::uppercase) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase) == "052/015");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase) == "0X2A/0XD");
        REQUIRE(runner(rational{227191947ll, 13}, std::hex, std::showbase, std::uppercase) == "0XD8AAC8B/0XD");
        REQUIRE(runner(rational{-227191947ll, 13}, std::hex, std::showbase, std::uppercase) == "-0XD8AAC8B/0XD");

        REQUIRE(runner(rational{42}, std::dec, std::showpos) == "+42");
        REQUIRE(runner(rational{42}, std::oct, std::showpos) == "+52");
        REQUIRE(runner(rational{42}, std::hex, std::showpos) == "+2a");

        REQUIRE(runner(rational{42, 13}, std::dec, std::showpos) == "+42/13");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showpos) == "+52/15");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showpos) == "+2a/d");

        REQUIRE(runner(rational{42}, std::dec, std::showbase, std::showpos) == "+42");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::showpos) == "+052");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::showpos) == "+0x2a");

        REQUIRE(runner(rational{42, 13}, std::dec, std::showbase, std::showpos) == "+42/13");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::showpos) == "+052/015");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::showpos) == "+0x2a/0xd");

        REQUIRE(runner(rational{42}, std::dec, std::showbase, std::uppercase, std::showpos) == "+42");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos) == "+052");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos) == "+0X2A");

        REQUIRE(runner(rational{42, 13}, std::dec, std::showbase, std::uppercase, std::showpos) == "+42/13");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos) == "+052/015");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos) == "+0X2A/0XD");

        REQUIRE(runner(rational{-42}, std::dec) == "-42");
        REQUIRE(runner(rational{-42}, std::oct) == "-52");
        REQUIRE(runner(rational{-42}, std::hex) == "-2a");

        REQUIRE(runner(rational{-42, 13}, std::dec) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::oct) == "-52/15");
        REQUIRE(runner(rational{-42, 13}, std::hex) == "-2a/d");

        REQUIRE(runner(rational{-42}, std::setbase(10)) == "-42");
        REQUIRE(runner(rational{-42}, std::setbase(8)) == "-52");
        REQUIRE(runner(rational{-42}, std::setbase(16)) == "-2a");
        REQUIRE(runner(rational{-42}, std::setbase(0)) == "-42");
        REQUIRE(runner(rational{-42}, std::setbase(-1)) == "-42");
        REQUIRE(runner(rational{-42}, std::setbase(1)) == "-42");
        REQUIRE(runner(rational{-42}, std::setbase(3)) == "-42");

        REQUIRE(runner(rational{-42, 13}, std::setbase(10)) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setbase(8)) == "-52/15");
        REQUIRE(runner(rational{-42, 13}, std::setbase(16)) == "-2a/d");
        REQUIRE(runner(rational{-42, 13}, std::setbase(0)) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setbase(-1)) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setbase(1)) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setbase(3)) == "-42/13");

        REQUIRE(runner(rational{-42}, std::dec, std::showbase) == "-42");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase) == "-052");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase) == "-0x2a");

        REQUIRE(runner(rational{-42, 13}, std::dec, std::showbase) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase) == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase) == "-0x2a/0xd");

        REQUIRE(runner(rational{-42}, std::dec, std::showbase, std::uppercase) == "-42");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase) == "-052");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase) == "-0X2A");

        REQUIRE(runner(rational{-42, 13}, std::dec, std::showbase, std::uppercase) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase) == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase) == "-0X2A/0XD");

        REQUIRE(runner(rational{-42}, std::setbase(10), std::showbase) == "-42");
        REQUIRE(runner(rational{-42}, std::setbase(8), std::showbase) == "-052");
        REQUIRE(runner(rational{-42}, std::setbase(16), std::showbase) == "-0x2a");
        REQUIRE(runner(rational{-42}, std::setbase(0), std::showbase) == "-42");
        REQUIRE(runner(rational{-42}, std::setbase(-1), std::showbase) == "-42");
        REQUIRE(runner(rational{-42}, std::setbase(1), std::showbase) == "-42");
        REQUIRE(runner(rational{-42}, std::setbase(3), std::showbase) == "-42");

        REQUIRE(runner(rational{-42, 13}, std::setbase(10), std::showbase) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setbase(8), std::showbase) == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::setbase(16), std::showbase) == "-0x2a/0xd");
        REQUIRE(runner(rational{-42, 13}, std::setbase(0), std::showbase) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setbase(-1), std::showbase) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setbase(1), std::showbase) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setbase(3), std::showbase) == "-42/13");

        REQUIRE(runner(rational{-42}, std::dec, std::showpos) == "-42");
        REQUIRE(runner(rational{-42}, std::oct, std::showpos) == "-52");
        REQUIRE(runner(rational{-42}, std::hex, std::showpos) == "-2a");

        REQUIRE(runner(rational{-42, 13}, std::dec, std::showpos) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showpos) == "-52/15");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showpos) == "-2a/d");

        REQUIRE(runner(rational{-42}, std::dec, std::showbase, std::showpos) == "-42");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::showpos) == "-052");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::showpos) == "-0x2a");

        REQUIRE(runner(rational{-42, 13}, std::dec, std::showbase, std::showpos) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::showpos) == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::showpos) == "-0x2a/0xd");

        REQUIRE(runner(rational{-42}, std::dec, std::showbase, std::uppercase, std::showpos) == "-42");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos) == "-052");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos) == "-0X2A");

        REQUIRE(runner(rational{-42, 13}, std::dec, std::showbase, std::uppercase, std::showpos) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos) == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos) == "-0X2A/0XD");

        // Tests with default fill (right).
        REQUIRE(runner(rational{0}, std::setw(0)) == "0");
        REQUIRE(runner(rational{0}, std::setw(-1)) == "0");
        REQUIRE(runner(rational{0}, std::setw(-2)) == "0");
        REQUIRE(runner(rational{0}, std::setw(1)) == "0");
        REQUIRE(runner(rational{0}, std::setw(2)) == " 0");
        REQUIRE(runner(rational{0}, std::setw(10)) == "         0");

        REQUIRE(runner(rational{42}, std::setw(0)) == "42");
        REQUIRE(runner(rational{42}, std::setw(-1)) == "42");
        REQUIRE(runner(rational{42}, std::setw(-2)) == "42");
        REQUIRE(runner(rational{42}, std::setw(1)) == "42");
        REQUIRE(runner(rational{42}, std::setw(2)) == "42");
        REQUIRE(runner(rational{42}, std::setw(10)) == "        42");

        REQUIRE(runner(rational{42, 13}, std::setw(0)) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(-1)) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(-2)) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(1)) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(2)) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(10)) == "     42/13");

        REQUIRE(runner(rational{-42}, std::setw(0)) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(-1)) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(-2)) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(1)) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(2)) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(10)) == "       -42");

        REQUIRE(runner(rational{-42, 13}, std::setw(0)) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(-1)) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(-2)) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(1)) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(2)) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(10)) == "    -42/13");

        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10))
                == "     +0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3)) == "+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4)) == "+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5)) == "+0X2A");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10))
                == "      +052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3)) == "+052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4)) == "+052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5)) == " +052");

        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10))
                == "     -0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3)) == "-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4)) == "-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5)) == "-0X2A");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10))
                == "      -052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3)) == "-052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4)) == "-052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5)) == " -052");

        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10))
                == " +0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3))
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4))
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5))
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10))
                == "  +052/015");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3))
                == "+052/015");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4))
                == "+052/015");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5))
                == "+052/015");

        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10))
                == " -0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3))
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4))
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5))
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10))
                == "  -052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3))
                == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4))
                == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5))
                == "-052/015");

        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'))
                == "*****+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('a'))
                == "aaaaa+0X2A");
        REQUIRE(
            runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::setfill('*'))
            == "+0X2A");
        REQUIRE(
            runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::setfill('*'))
            == "+0X2A");
        REQUIRE(
            runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::setfill('*'))
            == "+0X2A");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'))
                == "******+052");
        REQUIRE(
            runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::setfill('*'))
            == "+052");
        REQUIRE(
            runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::setfill('*'))
            == "+052");
        REQUIRE(
            runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::setfill('*'))
            == "*+052");

        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'))
                == "*+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'))
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'))
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'))
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'))
                == "**+052/015");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'))
                == "+052/015");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'))
                == "+052/015");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'))
                == "+052/015");

        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'))
                == "*****-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('a'))
                == "aaaaa-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'))
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'))
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'))
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'))
                == "******-052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'))
                == "-052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'))
                == "-052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'))
                == "*-052");

        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'))
                == "*-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'))
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'))
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'))
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'))
                == "**-052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'))
                == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'))
                == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'))
                == "-052/015");

        // Tests with right fill.
        REQUIRE(runner(rational{0}, std::setw(0), std::right) == "0");
        REQUIRE(runner(rational{0}, std::setw(-1), std::right) == "0");
        REQUIRE(runner(rational{0}, std::setw(-2), std::right) == "0");
        REQUIRE(runner(rational{0}, std::setw(1), std::right) == "0");
        REQUIRE(runner(rational{0}, std::setw(2), std::right) == " 0");
        REQUIRE(runner(rational{0}, std::setw(10), std::right) == "         0");

        REQUIRE(runner(rational{42}, std::setw(0), std::right) == "42");
        REQUIRE(runner(rational{42}, std::setw(-1), std::right) == "42");
        REQUIRE(runner(rational{42}, std::setw(-2), std::right) == "42");
        REQUIRE(runner(rational{42}, std::setw(1), std::right) == "42");
        REQUIRE(runner(rational{42}, std::setw(2), std::right) == "42");
        REQUIRE(runner(rational{42}, std::setw(10), std::right) == "        42");

        REQUIRE(runner(rational{42, 13}, std::setw(0), std::right) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(-1), std::right) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(-2), std::right) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(1), std::right) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(2), std::right) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(10), std::right) == "     42/13");

        REQUIRE(runner(rational{-42}, std::setw(0), std::right) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(-1), std::right) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(-2), std::right) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(1), std::right) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(2), std::right) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(10), std::right) == "       -42");

        REQUIRE(runner(rational{-42, 13}, std::setw(0), std::right) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(-1), std::right) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(-2), std::right) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(1), std::right) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(2), std::right) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(10), std::right) == "    -42/13");

        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::right)
                == "     +0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::right)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::right)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::right)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::right)
                == "      +052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::right)
                == "+052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::right)
                == "+052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::right)
                == " +052");

        REQUIRE(
            runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::right)
            == " +0X2A/0XD");
        REQUIRE(
            runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::right)
            == "+0X2A/0XD");
        REQUIRE(
            runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::right)
            == "+0X2A/0XD");
        REQUIRE(
            runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::right)
            == "+0X2A/0XD");
        REQUIRE(
            runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::right)
            == "  +052/015");
        REQUIRE(
            runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::right)
            == "+052/015");
        REQUIRE(
            runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::right)
            == "+052/015");
        REQUIRE(
            runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::right)
            == "+052/015");

        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::right)
                == "     -0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::right)
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::right)
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::right)
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::right)
                == "      -052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::right)
                == "-052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::right)
                == "-052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::right)
                == " -052");

        REQUIRE(
            runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::right)
            == " -0X2A/0XD");
        REQUIRE(
            runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::right)
            == "-0X2A/0XD");
        REQUIRE(
            runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::right)
            == "-0X2A/0XD");
        REQUIRE(
            runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::right)
            == "-0X2A/0XD");
        REQUIRE(
            runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::right)
            == "  -052/015");
        REQUIRE(
            runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::right)
            == "-052/015");
        REQUIRE(
            runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::right)
            == "-052/015");
        REQUIRE(
            runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::right)
            == "-052/015");

        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::right)
                == "*****+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('a'), std::right)
                == "aaaaa+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::right)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::right)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::right)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::right)
                == "******+052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::right)
                == "+052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::right)
                == "+052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::right)
                == "*+052");

        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::right)
                == "*+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::right)
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::right)
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::right)
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::right)
                == "**+052/015");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::right)
                == "+052/015");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::right)
                == "+052/015");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::right)
                == "+052/015");

        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::right)
                == "*****-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('a'), std::right)
                == "aaaaa-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::right)
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::right)
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::right)
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::right)
                == "******-052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::right)
                == "-052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::right)
                == "-052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::right)
                == "*-052");

        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::right)
                == "*-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::right)
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::right)
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::right)
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::right)
                == "**-052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::right)
                == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::right)
                == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::right)
                == "-052/015");

        // Left fill.
        REQUIRE(runner(rational{0}, std::setw(0), std::left) == "0");
        REQUIRE(runner(rational{0}, std::setw(-1), std::left) == "0");
        REQUIRE(runner(rational{0}, std::setw(-2), std::left) == "0");
        REQUIRE(runner(rational{0}, std::setw(1), std::left) == "0");
        REQUIRE(runner(rational{0}, std::setw(2), std::left) == "0 ");
        REQUIRE(runner(rational{0}, std::setw(10), std::left) == "0         ");

        REQUIRE(runner(rational{42}, std::setw(0), std::left) == "42");
        REQUIRE(runner(rational{42}, std::setw(-1), std::left) == "42");
        REQUIRE(runner(rational{42}, std::setw(-2), std::left) == "42");
        REQUIRE(runner(rational{42}, std::setw(1), std::left) == "42");
        REQUIRE(runner(rational{42}, std::setw(2), std::left) == "42");
        REQUIRE(runner(rational{42}, std::setw(10), std::left) == "42        ");

        REQUIRE(runner(rational{42, 13}, std::setw(0), std::left) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(-1), std::left) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(-2), std::left) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(1), std::left) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(2), std::left) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(10), std::left) == "42/13     ");

        REQUIRE(runner(rational{-42}, std::setw(0), std::left) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(-1), std::left) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(-2), std::left) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(1), std::left) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(2), std::left) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(10), std::left) == "-42       ");

        REQUIRE(runner(rational{-42, 13}, std::setw(0), std::left) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(-1), std::left) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(-2), std::left) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(1), std::left) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(2), std::left) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(10), std::left) == "-42/13    ");

        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::left)
                == "+0X2A     ");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::left)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::left)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::left)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::left)
                == "+052      ");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::left)
                == "+052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::left)
                == "+052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::left)
                == "+052 ");

        REQUIRE(
            runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::left)
            == "+0X2A/0XD ");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::left)
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::left)
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::left)
                == "+0X2A/0XD");
        REQUIRE(
            runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::left)
            == "+052/015  ");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::left)
                == "+052/015");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::left)
                == "+052/015");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::left)
                == "+052/015");

        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::left)
                == "-0X2A     ");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::left)
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::left)
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::left)
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::left)
                == "-052      ");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::left)
                == "-052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::left)
                == "-052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::left)
                == "-052 ");

        REQUIRE(
            runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::left)
            == "-0X2A/0XD ");
        REQUIRE(
            runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::left)
            == "-0X2A/0XD");
        REQUIRE(
            runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::left)
            == "-0X2A/0XD");
        REQUIRE(
            runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::left)
            == "-0X2A/0XD");
        REQUIRE(
            runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::left)
            == "-052/015  ");
        REQUIRE(
            runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::left)
            == "-052/015");
        REQUIRE(
            runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::left)
            == "-052/015");
        REQUIRE(
            runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::left)
            == "-052/015");

        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::left)
                == "+0X2A*****");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('a'), std::left)
                == "+0X2Aaaaaa");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::left)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::left)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::left)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::left)
                == "+052******");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::left)
                == "+052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::left)
                == "+052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::left)
                == "+052*");

        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::left)
                == "+0X2A/0XD*");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::left)
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::left)
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::left)
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::left)
                == "+052/015**");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::left)
                == "+052/015");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::left)
                == "+052/015");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::left)
                == "+052/015");

        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::left)
                == "-0X2A*****");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('a'), std::left)
                == "-0X2Aaaaaa");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::left)
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::left)
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::left)
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::left)
                == "-052******");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::left)
                == "-052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::left)
                == "-052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::left)
                == "-052*");

        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::left)
                == "-0X2A/0XD*");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::left)
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::left)
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::left)
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::left)
                == "-052/015**");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::left)
                == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::left)
                == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::left)
                == "-052/015");

        // Internal fill.
        REQUIRE(runner(rational{0}, std::setw(0), std::internal) == "0");
        REQUIRE(runner(rational{0}, std::setw(-1), std::internal) == "0");
        REQUIRE(runner(rational{0}, std::setw(-2), std::internal) == "0");
        REQUIRE(runner(rational{0}, std::setw(1), std::internal) == "0");
        REQUIRE(runner(rational{0}, std::setw(2), std::internal) == " 0");
        REQUIRE(runner(rational{0}, std::setw(10), std::internal) == "         0");

        REQUIRE(runner(rational{42}, std::setw(0), std::internal) == "42");
        REQUIRE(runner(rational{42}, std::setw(-1), std::internal) == "42");
        REQUIRE(runner(rational{42}, std::setw(-2), std::internal) == "42");
        REQUIRE(runner(rational{42}, std::setw(1), std::internal) == "42");
        REQUIRE(runner(rational{42}, std::setw(2), std::internal) == "42");
        REQUIRE(runner(rational{42}, std::setw(10), std::internal) == "        42");

        REQUIRE(runner(rational{42, 13}, std::setw(0), std::internal) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(-1), std::internal) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(-2), std::internal) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(1), std::internal) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(2), std::internal) == "42/13");
        REQUIRE(runner(rational{42, 13}, std::setw(10), std::internal) == "     42/13");

        REQUIRE(runner(rational{-42}, std::setw(0), std::internal) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(-1), std::internal) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(-2), std::internal) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(1), std::internal) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(2), std::internal) == "-42");
        REQUIRE(runner(rational{-42}, std::setw(10), std::internal) == "-       42");

        REQUIRE(runner(rational{-42, 13}, std::setw(0), std::internal) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(-1), std::internal) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(-2), std::internal) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(1), std::internal) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(2), std::internal) == "-42/13");
        REQUIRE(runner(rational{-42, 13}, std::setw(10), std::internal) == "-    42/13");

        REQUIRE(
            runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::internal)
            == "+     0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::internal)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::internal)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::internal)
                == "+0X2A");
        REQUIRE(
            runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::internal)
            == "+      052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::internal)
                == "+052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::internal)
                == "+052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::internal)
                == "+ 052");

        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::internal)
                == "+ 0X2A/0XD");
        REQUIRE(
            runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::internal)
            == "+0X2A/0XD");
        REQUIRE(
            runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::internal)
            == "+0X2A/0XD");
        REQUIRE(
            runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::internal)
            == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::internal)
                == "+  052/015");
        REQUIRE(
            runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::internal)
            == "+052/015");
        REQUIRE(
            runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::internal)
            == "+052/015");
        REQUIRE(
            runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::internal)
            == "+052/015");

        REQUIRE(
            runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10), std::internal)
            == "-     0X2A");
        REQUIRE(
            runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3), std::internal)
            == "-0X2A");
        REQUIRE(
            runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4), std::internal)
            == "-0X2A");
        REQUIRE(
            runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5), std::internal)
            == "-0X2A");
        REQUIRE(
            runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10), std::internal)
            == "-      052");
        REQUIRE(
            runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3), std::internal)
            == "-052");
        REQUIRE(
            runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4), std::internal)
            == "-052");
        REQUIRE(
            runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5), std::internal)
            == "- 052");

        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::internal)
                == "- 0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::internal)
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::internal)
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::internal)
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::internal)
                == "-  052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::internal)
                == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::internal)
                == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::internal)
                == "-052/015");

        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::internal)
                == "+*****0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('a'), std::internal)
                == "+aaaaa0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::internal)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::internal)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::internal)
                == "+0X2A");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::internal)
                == "+******052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::internal)
                == "+052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::internal)
                == "+052");
        REQUIRE(runner(rational{42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::internal)
                == "+*052");

        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::internal)
                == "+*0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::internal)
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::internal)
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::internal)
                == "+0X2A/0XD");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::internal)
                == "+**052/015");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::internal)
                == "+052/015");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::internal)
                == "+052/015");
        REQUIRE(runner(rational{42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::internal)
                == "+052/015");

        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::internal)
                == "-*****0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('a'), std::internal)
                == "-aaaaa0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::internal)
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::internal)
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::internal)
                == "-0X2A");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::internal)
                == "-******052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::internal)
                == "-052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::internal)
                == "-052");
        REQUIRE(runner(rational{-42}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::internal)
                == "-*052");

        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::internal)
                == "-*0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::internal)
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::internal)
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::hex, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::internal)
                == "-0X2A/0XD");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(10),
                       std::setfill('*'), std::internal)
                == "-**052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(3),
                       std::setfill('*'), std::internal)
                == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(4),
                       std::setfill('*'), std::internal)
                == "-052/015");
        REQUIRE(runner(rational{-42, 13}, std::oct, std::showbase, std::uppercase, std::showpos, std::setw(5),
                       std::setfill('*'), std::internal)
                == "-052/015");

        // A test to make sure that the stream width is reset to zero properly.
        {
            std::ostringstream oss;
            oss << std::setfill('a') << std::setw(10) << rational{42, 13} << "\n\n\n";
            REQUIRE(oss.str() == "aaaaa42/13\n\n\n");
        }
    }
};

TEST_CASE("out test")
{
    tuple_for_each(sizes{}, out_tester{});
}
