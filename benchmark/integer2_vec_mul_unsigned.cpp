// Copyright 2016-2023 Francesco Biscani (bluescarni@gmail.com)
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
std::tuple<std::vector<T>, std::vector<T>, std::vector<T>, std::vector<T>> get_init_vectors()
{
    rng.seed(0);
    std::uniform_int_distribution<unsigned> dist(1u, 7u);
    std::vector<T> v1(size), v2(size), v3(size), v4(size);
    std::generate(v1.begin(), v1.end(), [&dist]() { return static_cast<T>(T(dist(rng)) << (GMP_NUMB_BITS / 2)); });
    std::generate(v2.begin(), v2.end(), [&dist]() { return static_cast<T>(T(dist(rng)) << (GMP_NUMB_BITS / 2)); });
    std::generate(v3.begin(), v3.end(), [&dist]() { return static_cast<T>(T(dist(rng)) << (GMP_NUMB_BITS / 2)); });
    return std::make_tuple(std::move(v1), std::move(v2), std::move(v3), std::move(v4));
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

        mppp_benchmark::simple_timer st;

        for (auto i = 0ul; i < size; ++i) {
            mul(std::get<3>(p)[i], std::get<0>(p)[i], std::get<1>(p)[i]);
        }
        for (auto i = 0ul; i < size; ++i) {
            add(std::get<3>(p)[i], std::get<2>(p)[i], std::get<3>(p)[i]);
        }

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, std::get<3>(p)[size - 1u]);
    }

#if defined(MPPP_HAVE_GCC_INT128)
    {
        auto p = get_init_vectors<__uint128_t>();
        constexpr auto name = "__uint128_t";

        mppp_benchmark::simple_timer st;

        for (auto i = 0ul; i < size; ++i) {
            std::get<3>(p)[i] = std::get<0>(p)[i] * std::get<1>(p)[i];
        }
        for (auto i = 0ul; i < size; ++i) {
            std::get<3>(p)[i] = std::get<2>(p)[i] + std::get<3>(p)[i];
        }

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, std::get<3>(p)[size - 1u]);
    }
#endif

#if defined(MPPP_BENCHMARK_BOOST)
    {
        auto p = get_init_vectors<cpp_int>();
        constexpr auto name = "boost::cpp_int";

        mppp_benchmark::simple_timer st;

        for (auto i = 0ul; i < size; ++i) {
            std::get<3>(p)[i] = std::get<0>(p)[i] * std::get<1>(p)[i];
        }
        for (auto i = 0ul; i < size; ++i) {
            std::get<3>(p)[i] += std::get<2>(p)[i];
        }

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, std::get<3>(p)[size - 1u]);
    }

    {
        auto p = get_init_vectors<mpz_int>();
        constexpr auto name = "boost::gmp_int";

        mppp_benchmark::simple_timer st;

        for (auto i = 0ul; i < size; ++i) {
            mpz_mul(std::get<3>(p)[i].backend().data(), std::get<0>(p)[i].backend().data(),
                    std::get<1>(p)[i].backend().data());
        }
        for (auto i = 0ul; i < size; ++i) {
            mpz_add(std::get<3>(p)[i].backend().data(), std::get<2>(p)[i].backend().data(),
                    std::get<3>(p)[i].backend().data());
        }

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, std::get<3>(p)[size - 1u]);
    }
#endif

#if defined(MPPP_BENCHMARK_FLINT)
    {
        auto p = get_init_vectors<flint::fmpzxx>();
        constexpr auto name = "flint::fmpzxx";

        mppp_benchmark::simple_timer st;

        for (auto i = 0ul; i < size; ++i) {
            ::fmpz_mul(std::get<3>(p)[i]._data().inner, std::get<0>(p)[i]._data().inner,
                       std::get<1>(p)[i]._data().inner);
        }
        for (auto i = 0ul; i < size; ++i) {
            ::fmpz_add(std::get<3>(p)[i]._data().inner, std::get<2>(p)[i]._data().inner,
                       std::get<3>(p)[i]._data().inner);
        }

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, std::get<3>(p)[size - 1u]);
    }
#endif

    // Write out the .py and .rst files.
    mppp_benchmark::write_out(bdata, benchmark_name);
}
