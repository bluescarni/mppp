/* Copyright 2009-2016 Francesco Biscani (bluescarni@gmail.com)

This file is part of the mp++ library.

The mp++ library is free software; you can redistribute it and/or modify
it under the terms of either:

  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.

or

  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.

or both in parallel, as here.

The mp++ library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the mp++ library.  If not,
see https://www.gnu.org/licenses/. */

#include <limits>
#include <random>

#include <mp++.hpp>

#define NONIUS_RUNNER
#include <nonius/main.h++>
#include <nonius/nonius.h++>

using namespace mppp;

std::mt19937 rng;

NONIUS_BENCHMARK("long constructor", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long> dist(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    auto val = dist(rng);
    meter.measure([val] { return integer{val}; });
})

NONIUS_BENCHMARK("long long constructor", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long long> dist(std::numeric_limits<long long>::min(),
                                                  std::numeric_limits<long long>::max());
    auto val = dist(rng);
    meter.measure([val] { return integer{val}; });
})

NONIUS_BENCHMARK("unsigned long constructor", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long> dist(std::numeric_limits<unsigned long>::min(),
                                             std::numeric_limits<unsigned long>::max());
    auto val = dist(rng);
    meter.measure([val] { return integer{val}; });
})

NONIUS_BENCHMARK("unsigned long long constructor", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long long> dist(std::numeric_limits<unsigned long long>::min(),
                                                  std::numeric_limits<unsigned long long>::max());
    auto val = dist(rng);
    meter.measure([val] { return integer{val}; });
})

NONIUS_BENCHMARK("double constructor, small values", [](nonius::chronometer meter) {
    std::uniform_real_distribution<double> dist(-1E6, 1E6);
    auto val = dist(rng);
    meter.measure([val] { return integer{val}; });
})

NONIUS_BENCHMARK("double constructor, large values", [](nonius::chronometer meter) {
    std::uniform_real_distribution<double> dist(-1E128, 1E128);
    auto val = dist(rng);
    meter.measure([val] { return integer{val}; });
})

#if defined(MPPP_WITH_LONG_DOUBLE)

NONIUS_BENCHMARK("long double constructor, small values", [](nonius::chronometer meter) {
    std::uniform_real_distribution<long double> dist(-1E6, 1E6);
    auto val = dist(rng);
    meter.measure([val] { return integer{val}; });
})

NONIUS_BENCHMARK("long double constructor, large values", [](nonius::chronometer meter) {
    std::uniform_real_distribution<long double> dist(-1E128, 1E128);
    auto val = dist(rng);
    meter.measure([val] { return integer{val}; });
})

#endif
