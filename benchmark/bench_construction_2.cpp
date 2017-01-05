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

#include <mp++.hpp>
#include <random>

#if defined(MPPP_BENCHMARK_BOOST)
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/gmp.hpp>
#endif

#include "bench_tools.hpp"

using namespace mppp;
using namespace mppp_bench;
using integer = mp_integer<2>;

#if defined(MPPP_BENCHMARK_BOOST)
namespace bmp = boost::multiprecision;
using bmp::mpz_int;
using bmp::cpp_int;
#endif

std::mt19937 rng;

int main()
{
    constexpr auto ntrials = 1000u;
    benchmark_suite(
        "bench_construction_2", "mp++ long", bench_ctor<long, integer>{rng}, ntrials, "mp++ ulong",
        bench_ctor<unsigned long, integer>{rng}, ntrials, "mp++ long long", bench_ctor<long long, integer>{rng},
        ntrials, "mp++ ulong long", bench_ctor<unsigned long long, integer>{rng}, ntrials
#if defined(MPPP_BENCHMARK_BOOST)
        ,
        "cpp_int long", bench_ctor<long, cpp_int>{rng}, ntrials, "mpz_int long", bench_ctor<long, mpz_int>{rng},
        ntrials, "cpp_int ulong", bench_ctor<unsigned long, cpp_int>{rng}, ntrials, "mpz_int ulong",
        bench_ctor<unsigned long, mpz_int>{rng}, ntrials, "cpp_int long long", bench_ctor<long long, cpp_int>{rng},
        ntrials, "mpz_int long long", bench_ctor<long long, mpz_int>{rng}, ntrials, "cpp_int ulong long",
        bench_ctor<unsigned long long, cpp_int>{rng}, ntrials, "mpz_int ulong long",
        bench_ctor<unsigned long long, mpz_int>{rng}, ntrials
#endif
        );
    return 0;
}
