// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <cstdint>
#include <numeric>
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
    std::uniform_int_distribution<unsigned> dist(0u, 10000u);
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
        auto v = get_init_vector<mppp::integer<2>>();
        constexpr auto name = "mppp::integer<2>";

        std::vector<unsigned> c_out(size);

        mppp_benchmark::simple_timer st;

        std::transform(v.begin(), v.end(), c_out.begin(),
                       [](const mppp::integer<2> &n) { return static_cast<unsigned>(n); });

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, std::accumulate(c_out.begin(), c_out.end(), 0ul));
    }

#if defined(MPPP_BENCHMARK_BOOST)
    {
        auto v = get_init_vector<cpp_int>();
        constexpr auto name = "boost::cpp_int";

        std::vector<unsigned> c_out(size);

        mppp_benchmark::simple_timer st;

        std::transform(v.begin(), v.end(), c_out.begin(), [](const cpp_int &n) { return static_cast<unsigned>(n); });

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, std::accumulate(c_out.begin(), c_out.end(), 0ul));
    }

    {
        auto v = get_init_vector<mpz_int>();
        constexpr auto name = "boost::gmp_int";

        std::vector<unsigned> c_out(size);

        mppp_benchmark::simple_timer st;

        std::transform(v.begin(), v.end(), c_out.begin(),
                       [](const mpz_int &n) { return static_cast<unsigned>(mpz_get_ui(n.backend().data())); });

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, std::accumulate(c_out.begin(), c_out.end(), 0ul));
    }
#endif

#if defined(MPPP_BENCHMARK_FLINT)
    {
        auto v = get_init_vector<flint::fmpzxx>();
        constexpr auto name = "flint::fmpzxx";

        std::vector<unsigned> c_out(size);

        mppp_benchmark::simple_timer st;

        std::transform(v.begin(), v.end(), c_out.begin(),
                       [](const flint::fmpzxx &n) { return static_cast<unsigned>(::fmpz_get_ui(n._data().inner)); });

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, std::accumulate(c_out.begin(), c_out.end(), 0ul));
    }
#endif

    // Write out the .py and .rst files.
    mppp_benchmark::write_out(bdata, benchmark_name);
}
