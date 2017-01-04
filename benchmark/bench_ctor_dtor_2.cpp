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
#include <random>

#include <mp++.hpp>

#define NONIUS_RUNNER
#include <nonius/main.h++>
#include <nonius/nonius.h++>

#include "benchmark_utils.hpp"

using namespace mppp;
using namespace mppp_bench;
namespace bmp = boost::multiprecision;
using bmp::mpz_int;
using bmp::cpp_int;
using integer = mp_integer<2>;

std::mt19937 rng;

NONIUS_BENCHMARK("mp++ long constructor", [](nonius::chronometer meter) { bench_ctor<long, integer>(meter, rng); })
NONIUS_BENCHMARK("mpz_int long constructor", [](nonius::chronometer meter) { bench_ctor<long, mpz_int>(meter, rng); })
NONIUS_BENCHMARK("cpp_int long constructor", [](nonius::chronometer meter) { bench_ctor<long, cpp_int>(meter, rng); })

NONIUS_BENCHMARK("mp++ unsigned long constructor",
                 [](nonius::chronometer meter) { bench_ctor<unsigned long, integer>(meter, rng); })
NONIUS_BENCHMARK("mpz_int unsigned long constructor",
                 [](nonius::chronometer meter) { bench_ctor<unsigned long, mpz_int>(meter, rng); })
NONIUS_BENCHMARK("cpp_int unsigned long constructor",
                 [](nonius::chronometer meter) { bench_ctor<unsigned long, cpp_int>(meter, rng); })

NONIUS_BENCHMARK("mp++ long long constructor",
                 [](nonius::chronometer meter) { bench_ctor<long long, integer>(meter, rng); })
NONIUS_BENCHMARK("mpz_int long long constructor",
                 [](nonius::chronometer meter) { bench_ctor<long long, mpz_int>(meter, rng); })
NONIUS_BENCHMARK("cpp_int long long constructor",
                 [](nonius::chronometer meter) { bench_ctor<long long, cpp_int>(meter, rng); })

NONIUS_BENCHMARK("mp++ unsigned long long constructor",
                 [](nonius::chronometer meter) { bench_ctor<unsigned long long, integer>(meter, rng); })
NONIUS_BENCHMARK("mpz_int unsigned long long constructor",
                 [](nonius::chronometer meter) { bench_ctor<unsigned long long, mpz_int>(meter, rng); })
NONIUS_BENCHMARK("cpp_int unsigned long long constructor",
                 [](nonius::chronometer meter) { bench_ctor<unsigned long long, cpp_int>(meter, rng); })

NONIUS_BENCHMARK("mp++ double constructor, small values",
                 [](nonius::chronometer meter) { bench_ctor<double, integer>(meter, rng, -1E6, 1E6); })
NONIUS_BENCHMARK("mpz_int double constructor, small values",
                 [](nonius::chronometer meter) { bench_ctor<double, mpz_int>(meter, rng, -1E6, 1E6); })
NONIUS_BENCHMARK("cpp_int double constructor, small values",
                 [](nonius::chronometer meter) { bench_ctor<double, cpp_int>(meter, rng, -1E6, 1E6); })

NONIUS_BENCHMARK("mp++ double constructor, large values",
                 [](nonius::chronometer meter) { bench_ctor<double, integer>(meter, rng, -1E60, 1E60); })
NONIUS_BENCHMARK("mpz_int double constructor, large values",
                 [](nonius::chronometer meter) { bench_ctor<double, mpz_int>(meter, rng, -1E60, 1E60); })
NONIUS_BENCHMARK("cpp_int double constructor, large values",
                 [](nonius::chronometer meter) { bench_ctor<double, cpp_int>(meter, rng, -1E60, 1E60); })

#if defined(MPPP_WITH_LONG_DOUBLE)

NONIUS_BENCHMARK("mp++ long double constructor, small values",
                 [](nonius::chronometer meter) { bench_ctor<long double, integer>(meter, rng, -1E6, 1E6); })
NONIUS_BENCHMARK("mpz_int long double constructor, small values",
                 [](nonius::chronometer meter) { bench_ctor<long double, mpz_int>(meter, rng, -1E6, 1E6); })
NONIUS_BENCHMARK("cpp_int long double constructor, small values",
                 [](nonius::chronometer meter) { bench_ctor<long double, cpp_int>(meter, rng, -1E6, 1E6); })

NONIUS_BENCHMARK("mp++ long double constructor, large values",
                 [](nonius::chronometer meter) { bench_ctor<long double, integer>(meter, rng, -1E60, 1E60); })
NONIUS_BENCHMARK("mpz_int long double constructor, large values",
                 [](nonius::chronometer meter) { bench_ctor<long double, mpz_int>(meter, rng, -1E60, 1E60); })
NONIUS_BENCHMARK("cpp_int long double constructor, large values",
                 [](nonius::chronometer meter) { bench_ctor<long double, cpp_int>(meter, rng, -1E60, 1E60); })

#endif
