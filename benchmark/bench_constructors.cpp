/* Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)

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

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/gmp.hpp>
#include <limits>
#include <random>

#include <mp++.hpp>

#define NONIUS_RUNNER
#include <nonius/main.h++>
#include <nonius/nonius.h++>

using namespace mppp;
namespace bmp = boost::multiprecision;
using bmp::mpz_int;
using bmp::cpp_int;
using integer = mp_integer<1>;

std::mt19937 rng;

NONIUS_BENCHMARK("mp++ long constructor", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long> dist(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    volatile long val = dist(rng);
    meter.measure([&val]() { integer n{val}; });
})

NONIUS_BENCHMARK("mpz_int long constructor", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long> dist(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    volatile long val = dist(rng);
    meter.measure([&val]() { mpz_int retval{val}; });
})

NONIUS_BENCHMARK("cpp_int long constructor", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long> dist(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    volatile long val = dist(rng);
    meter.measure([&val]() { cpp_int retval{val}; });
})

NONIUS_BENCHMARK("mp++ unsigned long constructor", [](nonius::chronometer meter) {
    std::uniform_int_distribution<unsigned long> dist(std::numeric_limits<unsigned long>::min(),
                                                      std::numeric_limits<unsigned long>::max());
    volatile unsigned long val = dist(rng);
    meter.measure([&val]() { integer n{val}; });
})

NONIUS_BENCHMARK("mpz_int unsigned long constructor", [](nonius::chronometer meter) {
    std::uniform_int_distribution<unsigned long> dist(std::numeric_limits<unsigned long>::min(),
                                                      std::numeric_limits<unsigned long>::max());
    volatile unsigned long val = dist(rng);
    meter.measure([&val]() { mpz_int retval{val}; });
})

NONIUS_BENCHMARK("cpp_int unsigned long constructor", [](nonius::chronometer meter) {
    std::uniform_int_distribution<unsigned long> dist(std::numeric_limits<unsigned long>::min(),
                                                      std::numeric_limits<unsigned long>::max());
    volatile unsigned long val = dist(rng);
    meter.measure([&val]() { cpp_int retval{val}; });
})

NONIUS_BENCHMARK("mp++ long long constructor", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long long> dist(std::numeric_limits<long long>::min(),
                                                  std::numeric_limits<long long>::max());
    volatile long long val = dist(rng);
    meter.measure([&val]() { integer n{val}; });
})

NONIUS_BENCHMARK("mpz_int long long constructor", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long long> dist(std::numeric_limits<long long>::min(),
                                                  std::numeric_limits<long long>::max());
    volatile long long val = dist(rng);
    meter.measure([&val]() { mpz_int retval{val}; });
})

NONIUS_BENCHMARK("cpp_int long long constructor", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long long> dist(std::numeric_limits<long long>::min(),
                                                  std::numeric_limits<long long>::max());
    volatile long long val = dist(rng);
    meter.measure([&val]() { cpp_int retval{val}; });
})

NONIUS_BENCHMARK("mp++ unsigned long long constructor", [](nonius::chronometer meter) {
    std::uniform_int_distribution<unsigned long long> dist(std::numeric_limits<unsigned long long>::min(),
                                                           std::numeric_limits<unsigned long long>::max());
    volatile unsigned long long val = dist(rng);
    meter.measure([&val]() { integer n{val}; });
})

NONIUS_BENCHMARK("mpz_int unsigned long long constructor", [](nonius::chronometer meter) {
    std::uniform_int_distribution<unsigned long long> dist(std::numeric_limits<unsigned long long>::min(),
                                                           std::numeric_limits<unsigned long long>::max());
    volatile unsigned long long val = dist(rng);
    meter.measure([&val]() { mpz_int retval{val}; });
})

NONIUS_BENCHMARK("cpp_int unsigned long long constructor", [](nonius::chronometer meter) {
    std::uniform_int_distribution<unsigned long long> dist(std::numeric_limits<unsigned long long>::min(),
                                                           std::numeric_limits<unsigned long long>::max());
    volatile unsigned long long val = dist(rng);
    meter.measure([&val]() { cpp_int retval{val}; });
})

NONIUS_BENCHMARK("mp++ double constructor, small values", [](nonius::chronometer meter) {
    std::uniform_real_distribution<double> dist(-1E6, 1E6);
    volatile double val = dist(rng);
    meter.measure([&val]() { integer retval{val}; });
})

NONIUS_BENCHMARK("mpz_int double constructor, small values", [](nonius::chronometer meter) {
    std::uniform_real_distribution<double> dist(-1E6, 1E6);
    volatile double val = dist(rng);
    meter.measure([&val]() { mpz_int retval{val}; });
})

NONIUS_BENCHMARK("cpp_int double constructor, small values", [](nonius::chronometer meter) {
    std::uniform_real_distribution<double> dist(-1E6, 1E6);
    volatile double val = dist(rng);
    meter.measure([&val]() { cpp_int retval{val}; });
})

NONIUS_BENCHMARK("mp++ double constructor, large values", [](nonius::chronometer meter) {
    std::uniform_real_distribution<double> dist(-1E60, 1E60);
    volatile double val = dist(rng);
    meter.measure([&val]() { integer retval{val}; });
})

NONIUS_BENCHMARK("mpz_int double constructor, large values", [](nonius::chronometer meter) {
    std::uniform_real_distribution<double> dist(-1E60, 1E60);
    volatile double val = dist(rng);
    meter.measure([&val]() { mpz_int retval{val}; });
})

NONIUS_BENCHMARK("cpp_int double constructor, large values", [](nonius::chronometer meter) {
    std::uniform_real_distribution<double> dist(-1E60, 1E60);
    volatile double val = dist(rng);
    meter.measure([&val]() { cpp_int retval{val}; });
})

#if defined(MPPP_WITH_LONG_DOUBLE)

NONIUS_BENCHMARK("mp++ long double constructor, small values", [](nonius::chronometer meter) {
    std::uniform_real_distribution<long double> dist(-1E6, 1E6);
    volatile long double val = dist(rng);
    meter.measure([&val]() { integer retval{val}; });
})

NONIUS_BENCHMARK("mpz_int long double constructor, small values", [](nonius::chronometer meter) {
    std::uniform_real_distribution<long double> dist(-1E6, 1E6);
    volatile long double val = dist(rng);
    meter.measure([&val]() { mpz_int retval{val}; });
})

NONIUS_BENCHMARK("cpp_int long double constructor, small values", [](nonius::chronometer meter) {
    std::uniform_real_distribution<long double> dist(-1E6, 1E6);
    volatile long double val = dist(rng);
    meter.measure([&val]() { cpp_int retval{val}; });
})

NONIUS_BENCHMARK("mp++ long double constructor, large values", [](nonius::chronometer meter) {
    std::uniform_real_distribution<long double> dist(-1E60, 1E60);
    volatile long double val = dist(rng);
    meter.measure([&val]() { integer retval{val}; });
})

NONIUS_BENCHMARK("mpz_int long double constructor, large values", [](nonius::chronometer meter) {
    std::uniform_real_distribution<long double> dist(-1E60, 1E60);
    volatile long double val = dist(rng);
    meter.measure([&val]() { mpz_int retval{val}; });
})

NONIUS_BENCHMARK("cpp_int long double constructor, large values", [](nonius::chronometer meter) {
    std::uniform_real_distribution<long double> dist(-1E60, 1E60);
    volatile long double val = dist(rng);
    meter.measure([&val]() { cpp_int retval{val}; });
})

#endif
