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

#include <boost/multiprecision/gmp.hpp>
#include <limits>
#include <random>

#include <mp++.hpp>

#define NONIUS_RUNNER
#include <nonius/main.h++>
#include <nonius/nonius.h++>

namespace bmp = boost::multiprecision;
using namespace mppp;
using mpz_int = bmp::number<bmp::gmp_int, bmp::et_off>;

std::mt19937 rng;

NONIUS_BENCHMARK("ulong conversion", [](nonius::chronometer meter) {
    std::uniform_int_distribution<unsigned long> dist(std::numeric_limits<unsigned long>::min(),
                                                      std::numeric_limits<unsigned long>::max());
    auto val = integer(dist(rng));
    meter.measure([&val]() -> unsigned long {
        auto volatile retval = static_cast<unsigned long>(val);
        return retval;
    });
});

NONIUS_BENCHMARK("bmp ulong conversion", [](nonius::chronometer meter) {
    std::uniform_int_distribution<unsigned long> dist(std::numeric_limits<unsigned long>::min(),
                                                      std::numeric_limits<unsigned long>::max());
    auto val = mpz_int(dist(rng));
    meter.measure([&val]() -> unsigned long {
        auto volatile retval = static_cast<unsigned long>(val);
        return retval;
    });
});

NONIUS_BENCHMARK("ulonglong conversion", [](nonius::chronometer meter) {
    std::uniform_int_distribution<unsigned long long> dist(std::numeric_limits<unsigned long long>::min(),
                                                           std::numeric_limits<unsigned long long>::max());
    auto val = integer(dist(rng));
    meter.measure([&val]() -> unsigned long long {
        auto volatile retval = static_cast<unsigned long long>(val);
        return retval;
    });
});

NONIUS_BENCHMARK("bmp ulonglong conversion", [](nonius::chronometer meter) {
    std::uniform_int_distribution<unsigned long long> dist(std::numeric_limits<unsigned long long>::min(),
                                                           std::numeric_limits<unsigned long long>::max());
    auto val = mpz_int(dist(rng));
    meter.measure([&val]() -> unsigned long long {
        auto volatile retval = static_cast<unsigned long long>(val);
        return retval;
    });
});

NONIUS_BENCHMARK("long conversion", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long> dist(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    auto val = integer(dist(rng));
    meter.measure([&val]() -> long {
        auto volatile retval = static_cast<long>(val);
        return retval;
    });
});

NONIUS_BENCHMARK("bmp long conversion", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long> dist(std::numeric_limits<long>::min(), std::numeric_limits<long>::max());
    auto val = mpz_int(dist(rng));
    meter.measure([&val]() -> long {
        auto volatile retval = static_cast<long>(val);
        return retval;
    });
});

NONIUS_BENCHMARK("long long conversion", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long long> dist(std::numeric_limits<long long>::min(),
                                                  std::numeric_limits<long long>::max());
    auto val = integer(dist(rng));
    meter.measure([&val]() -> long long {
        auto volatile retval = static_cast<long long>(val);
        return retval;
    });
});

NONIUS_BENCHMARK("bmp long long conversion", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long long> dist(std::numeric_limits<long long>::min(),
                                                  std::numeric_limits<long long>::max());
    auto val = mpz_int(dist(rng));
    meter.measure([&val]() -> long long {
        auto volatile retval = static_cast<long long>(val);
        return retval;
    });
});

NONIUS_BENCHMARK("bmp double conversion", [](nonius::chronometer meter) {
    std::uniform_real_distribution<double> dist(-1E20, 1E20);
    auto val = mpz_int(dist(rng));
    meter.measure([&val]() -> double {
        auto volatile retval = static_cast<double>(val);
        return retval;
    });
});

NONIUS_BENCHMARK("double conversion", [](nonius::chronometer meter) {
    std::uniform_real_distribution<double> dist(-1E20, 1E20);
    auto val = integer(dist(rng));
    meter.measure([&val]() -> double {
        auto volatile retval = static_cast<double>(val);
        return retval;
    });
});

#if defined(MPPP_WITH_LONG_DOUBLE)

NONIUS_BENCHMARK("bmp long double conversion", [](nonius::chronometer meter) {
    std::uniform_real_distribution<long double> dist(-1E20l, 1E20l);
    auto val = mpz_int(dist(rng));
    meter.measure([&val]() -> long double {
        auto volatile retval = static_cast<long double>(val);
        return retval;
    });
});

NONIUS_BENCHMARK("long double conversion", [](nonius::chronometer meter) {
    std::uniform_real_distribution<long double> dist(-1E20l, 1E20l);
    auto val = integer(dist(rng));
    meter.measure([&val]() -> long double {
        auto volatile retval = static_cast<long double>(val);
        return retval;
    });
});

#endif

NONIUS_BENCHMARK("string conversion, base 10", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long long> dist(std::numeric_limits<long long>::min(),
                                                  std::numeric_limits<long long>::max());
    auto val = integer(dist(rng));
    meter.measure([&val] { return val.to_string(); });
});

NONIUS_BENCHMARK("string conversion, base 2", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long long> dist(std::numeric_limits<long long>::min(),
                                                  std::numeric_limits<long long>::max());
    auto val = integer(dist(rng));
    meter.measure([&val] { return val.to_string(2); });
});

NONIUS_BENCHMARK("string conversion, base 16", [](nonius::chronometer meter) {
    std::uniform_int_distribution<long long> dist(std::numeric_limits<long long>::min(),
                                                  std::numeric_limits<long long>::max());
    auto val = integer(dist(rng));
    meter.measure([&val] { return val.to_string(16); });
});
