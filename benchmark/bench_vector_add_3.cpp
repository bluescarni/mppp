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

using integer = mp_integer<3>;

#if defined(MPPP_BENCHMARK_BOOST)
namespace bmp = boost::multiprecision;
using bmp::cpp_int;
#endif

std::mt19937 rng;

int main()
{
    constexpr auto ntrials = 1000u;
    benchmark_suite("bench_vector_add_3", "mp++ 1-1", uadd_vec_mppp<integer>{rng, 1, 1}, ntrials, "gmp 1-1",
                    uadd_vec_gmp{rng, 1, 1}, ntrials, "mp++ 1-2", uadd_vec_mppp<integer>{rng, 1, 2}, ntrials, "gmp 1-2",
                    uadd_vec_gmp{rng, 1, 2}, ntrials, "mp++ 2-2", uadd_vec_mppp<integer>{rng, 2, 2}, ntrials, "gmp 2-2",
                    uadd_vec_gmp{rng, 2, 2}, ntrials
#if defined(MPPP_BENCHMARK_FLINT)
                    ,
                    "flint 1-1", uadd_vec_fmpz{rng, 1, 1}, ntrials, "flint 1-2", uadd_vec_fmpz{rng, 1, 2}, ntrials,
                    "flint 2-2", uadd_vec_fmpz{rng, 2, 2}, ntrials
#endif
#if defined(MPPP_BENCHMARK_BOOST)
                    ,
                    "cpp_int 1-1", uadd_vec_boost<cpp_int>{rng, 1, 1}, ntrials, "cpp_int 1-2",
                    uadd_vec_boost<cpp_int>{rng, 1, 2}, ntrials, "cpp_int 2-2", uadd_vec_boost<cpp_int>{rng, 2, 2},
                    ntrials
#endif
                    );
    return 0;
}
