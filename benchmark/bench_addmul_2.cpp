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

#include <mp++.hpp>
#include <random>

#include "benchmark_utils.hpp"

using namespace mppp;
using namespace mppp_bench;

using integer = mp_integer<2>;

std::mt19937 rng;

NONIUS_BENCHMARK("mppp half-limb unsigned vector addmul",
                 [](nonius::chronometer meter) { uaddmul_vec_mppp_half<integer>(meter, rng); });
NONIUS_BENCHMARK("mppp half-limb unsigned acc addmul",
                 [](nonius::chronometer meter) { uaddmul_acc_mppp_half<integer>(meter, rng); });
NONIUS_BENCHMARK("mppp 1-1-limb unsigned vector addmul",
                 [](nonius::chronometer meter) { uaddmul_vec_mppp<integer>(meter, rng, 1, 1); });
NONIUS_BENCHMARK("mppp 1-1-limb unsigned acc addmul",
                 [](nonius::chronometer meter) { uaddmul_acc_mppp<integer>(meter, rng, 1, 1); });

NONIUS_BENCHMARK("piranha half-limb unsigned vector addmul",
                 [](nonius::chronometer meter) { uaddmul_vec_piranha_half(meter, rng); });
NONIUS_BENCHMARK("piranha half-limb unsigned acc addmul",
                 [](nonius::chronometer meter) { uaddmul_acc_piranha_half(meter, rng); });
NONIUS_BENCHMARK("piranha 1-1-limb unsigned vector addmul",
                 [](nonius::chronometer meter) { uaddmul_vec_piranha(meter, rng, 1, 1); });
NONIUS_BENCHMARK("piranha 1-1-limb unsigned acc addmul",
                 [](nonius::chronometer meter) { uaddmul_acc_piranha(meter, rng, 1, 1); });
