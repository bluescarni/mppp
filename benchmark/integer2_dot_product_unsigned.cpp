// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <random>
#include <utility>
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
#include <mp++/detail/gmp.hpp>
#include <mp++/integer.hpp>

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
std::pair<std::vector<T>, std::vector<T>> get_init_vectors()
{
    rng.seed(0);
    std::uniform_int_distribution<unsigned> dist(1u, 7u);
    std::vector<T> v1(size), v2(size);
    std::generate(v1.begin(), v1.end(),
                  [&dist]() { return static_cast<T>((T(dist(rng)) << (GMP_NUMB_BITS / 2)) + dist(rng)); });
    std::generate(v2.begin(), v2.end(),
                  [&dist]() { return static_cast<T>((T(dist(rng)) << (GMP_NUMB_BITS / 2) + dist(rng))); });
    return std::make_pair(std::move(v1), std::move(v2));
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
        auto p = get_init_vectors<mppp::integer<2>>();
        constexpr auto name = "mppp::integer<2>";

        mppp::integer<2> ret(0);

        mppp_benchmark::simple_timer st;

        for (auto i = 0ul; i < size; ++i) {
            addmul(ret, p.first[i], p.second[i]);
        }

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, ret);
    }

#if defined(MPPP_HAVE_GCC_INT128)
    {
        auto p = get_init_vectors<__uint128_t>();
        constexpr auto name = "__uint128_t";

        __uint128_t ret(0);

        mppp_benchmark::simple_timer st;

        for (auto i = 0ul; i < size; ++i) {
            ret += p.first[i] * p.second[i];
        }

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, ret);
    }
#endif

#if defined(MPPP_BENCHMARK_BOOST)
    {
        auto p = get_init_vectors<cpp_int>();
        constexpr auto name = "boost::cpp_int";

        cpp_int ret(0);

        mppp_benchmark::simple_timer st;

        for (auto i = 0ul; i < size; ++i) {
            ret += p.first[i] * p.second[i];
        }

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, ret);
    }

    {
        auto p = get_init_vectors<mpz_int>();
        constexpr auto name = "boost::gmp_int";

        mpz_int ret(0);

        mppp_benchmark::simple_timer st;

        for (auto i = 0ul; i < size; ++i) {
            mpz_addmul(ret.backend().data(), p.first[i].backend().data(), p.second[i].backend().data());
        }

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, ret);
    }
#endif

#if defined(MPPP_BENCHMARK_FLINT)
    {
        auto p = get_init_vectors<flint::fmpzxx>();
        constexpr auto name = "flint::fmpzxx";

        flint::fmpzxx ret(0);

        mppp_benchmark::simple_timer st;

        for (auto i = 0ul; i < size; ++i) {
            ::fmpz_addmul(ret._data().inner, p.first[i]._data().inner, p.second[i]._data().inner);
        }

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, ret);
    }
#endif

    // Write out the .py and .rst files.
    mppp_benchmark::write_out(bdata, benchmark_name);
}
