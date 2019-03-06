// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/real128.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iomanip>
#include <ios>
#include <sstream>
#include <string>
#include <utility>

using namespace mppp;

static inline void runner_impl(std::ostringstream &) {}

template <typename F, typename... Flags>
static inline void runner_impl(std::ostringstream &oss, F &&f, Flags &&... flags)
{
    oss << std::forward<F>(f);
    runner_impl(oss, std::forward<Flags>(flags)...);
}

template <typename Int, typename... Flags>
static inline std::string runner(const Int &n, Flags &&... flags)
{
    std::ostringstream oss;
    runner_impl(oss, std::forward<Flags>(flags)...);
    oss << n;
    return oss.str();
}

TEST_CASE("real128 stream format")
{
    // Simple examples.
    REQUIRE(runner(real128{0}) == "0");
    REQUIRE(runner(real128{1}) == "1");
    REQUIRE(runner(real128{-1}) == "-1");
    REQUIRE(runner(real128{42}) == "42");
    REQUIRE(runner(real128{-42}) == "-42");
    REQUIRE(runner(real128{21} / 2) == "10.5");
    REQUIRE(runner(real128{-21} / 2) == "-10.5");
    REQUIRE(runner(real128{"-inf"}) == "-inf");
    REQUIRE(runner(real128{"inf"}) == "inf");
    REQUIRE(runner(real128{"nan"}) == "nan");

    // Showpos.
    REQUIRE(runner(real128{0}, std::showpos) == "+0");
    REQUIRE(runner(real128{1}, std::showpos) == "+1");
    REQUIRE(runner(real128{-1}, std::showpos) == "-1");
    REQUIRE(runner(real128{42}, std::showpos) == "+42");
    REQUIRE(runner(real128{-42}, std::showpos) == "-42");
    REQUIRE(runner(real128{21} / 2, std::showpos) == "+10.5");
    REQUIRE(runner(real128{-21} / 2, std::showpos) == "-10.5");
    REQUIRE(runner(real128{"-inf"}, std::showpos) == "-inf");
    REQUIRE(runner(real128{"inf"}, std::showpos) == "+inf");
    REQUIRE(runner(real128{"nan"}, std::showpos) == "nan");

    // Scientific notation.
    REQUIRE(runner(real128{0}, std::showpos, std::scientific) == runner(0., std::showpos, std::scientific));
    REQUIRE(runner(real128{1}, std::showpos, std::scientific) == runner(1., std::showpos, std::scientific));
    REQUIRE(runner(real128{-1}, std::showpos, std::scientific) == runner(-1., std::showpos, std::scientific));
    REQUIRE(runner(real128{42}, std::showpos, std::scientific) == runner(42., std::showpos, std::scientific));
    REQUIRE(runner(real128{-42}, std::showpos, std::scientific) == runner(-42., std::showpos, std::scientific));
    REQUIRE(runner(real128{21} / 2, std::showpos, std::scientific) == "+1.050000e+01");
    REQUIRE(runner(real128{-21} / 2, std::showpos, std::scientific) == "-1.050000e+01");
    REQUIRE(runner(real128{"-inf"}, std::showpos, std::scientific) == "-inf");
    REQUIRE(runner(real128{"inf"}, std::showpos, std::scientific) == "+inf");
    REQUIRE(runner(real128{"nan"}, std::showpos, std::scientific) == "nan");

    // Fixed format.
    REQUIRE(runner(real128{0}, std::showpos, std::fixed) == runner(0., std::showpos, std::fixed));
    REQUIRE(runner(real128{1}, std::showpos, std::fixed) == runner(1., std::showpos, std::fixed));
    REQUIRE(runner(real128{-1}, std::showpos, std::fixed) == runner(-1., std::showpos, std::fixed));
    REQUIRE(runner(real128{42}, std::showpos, std::fixed) == runner(42., std::showpos, std::fixed));
    REQUIRE(runner(real128{-42}, std::showpos, std::fixed) == runner(-42., std::showpos, std::fixed));
    REQUIRE(runner(real128{21} / 2, std::showpos, std::fixed) == "+10.500000");
    REQUIRE(runner(real128{-21} / 2, std::showpos, std::fixed) == "-10.500000");
    REQUIRE(runner(real128{"-inf"}, std::showpos, std::fixed) == "-inf");
    REQUIRE(runner(real128{"inf"}, std::showpos, std::fixed) == "+inf");
    REQUIRE(runner(real128{"nan"}, std::showpos, std::fixed) == "nan");

    // Hexfloat.
    REQUIRE(runner(real128{0}, std::showpos, std::hexfloat) == "+0x0p+0");
    REQUIRE(runner(real128{0}, std::hexfloat) == "0x0p+0");
    REQUIRE(runner(real128{0}, std::showpos, std::hexfloat, std::setprecision(100)) == "+0x0p+0");
    REQUIRE(runner(real128{0}, std::hexfloat, std::setprecision(100)) == "0x0p+0");
}
