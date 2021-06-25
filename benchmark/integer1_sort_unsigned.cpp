// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

#if defined(MPPP_BENCHMARK_BOOST)

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/gmp.hpp>

#endif

#if defined(MPPP_BENCHMARK_FLINT)

#include <flint/flint.h>
#include <flint/fmpzxx.h>

#endif

#include <fmt/core.h>
#include <fmt/ostream.h>

#include <mp++/config.hpp>
#include <mp++/integer.hpp>

#if defined(MPPP_BENCHMARK_BOOST)

#include <mp++/detail/gmp.hpp>

#endif

#include "utils.hpp"

namespace
{

#if defined(MPPP_BENCHMARK_BOOST)

using cpp_int = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>, boost::multiprecision::et_on>;
using mpz_int = boost::multiprecision::number<boost::multiprecision::gmp_int, boost::multiprecision::et_off>;

#endif

std::mt19937 rng;

constexpr auto size = 30000000ul;

template <typename T>
std::vector<T> get_init_vector()
{
    rng.seed(0);
    std::uniform_int_distribution<unsigned long> dist(0, 600000ul);
    std::vector<T> retval(size);
    std::generate(retval.begin(), retval.end(), [&dist]() { return T(dist(rng)); });
    return retval;
}

const auto benchmark_name = mppp_benchmark_name();

} // namespace

int main()
{
    fmt::print("Benchmark name: {}\n", benchmark_name);

    // Warm up.
    mppp_benchmark::warmup();

    // Prepare the benchmark result data.
    mppp_benchmark::data_t bdata;

    {
        auto v = get_init_vector<mppp::integer<1>>();
        constexpr auto name = "mppp::integer<1>";

        mppp_benchmark::simple_timer st;

        std::sort(v.begin(), v.end());

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, v[0]);
    }

    {
        auto v = get_init_vector<std::uint_least64_t>();
        constexpr auto name = "std::uint64_t";

        mppp_benchmark::simple_timer st;

        std::sort(v.begin(), v.end());

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, v[0]);
    }

#if defined(MPPP_HAVE_GCC_INT128)
    {
        auto v = get_init_vector<__uint128_t>();
        constexpr auto name = "__uint128_t";

        mppp_benchmark::simple_timer st;

        std::sort(v.begin(), v.end());

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, v[0]);
    }
#endif

#if defined(MPPP_BENCHMARK_BOOST)
    {
        auto v = get_init_vector<cpp_int>();
        constexpr auto name = "boost::cpp_int";

        mppp_benchmark::simple_timer st;

        std::sort(v.begin(), v.end());

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, v[0]);
    }

    {
        auto v = get_init_vector<mpz_int>();
        constexpr auto name = "boost::gmp_int";

        mppp_benchmark::simple_timer st;

        std::sort(v.begin(), v.end());

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, v[0]);
    }
#endif

#if defined(MPPP_BENCHMARK_FLINT)
    {
        auto v = get_init_vector<flint::fmpzxx>();
        constexpr auto name = "flint::fmpzxx";

        mppp_benchmark::simple_timer st;

        std::sort(v.begin(), v.end());

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, v[0]);
    }
#endif

    // Write out the .py and .rst files.
    mppp_benchmark::write_out(bdata, benchmark_name);
}
