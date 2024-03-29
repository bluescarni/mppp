// Copyright 2016-2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <algorithm>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

#if MPPP_CPLUSPLUS >= 201703L

#include <numeric>

#endif

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
std::tuple<std::vector<T>, std::vector<T>, std::vector<T>> get_init_vectors()
{
    rng.seed(1);
    std::uniform_int_distribution<int> dist(1, 10), sign(0, 1);
    std::vector<T> v1(size), v2(size), v3(size);
    auto mult_rng = [&dist, &sign](unsigned n) -> T {
        T retval(dist(rng));
        for (auto i = 1u; i < n; ++i) {
            retval *= dist(rng);
        }
        return static_cast<T>(retval * (sign(rng) ? 1 : -1));
    };
    std::generate(v1.begin(), v1.end(), [&mult_rng]() { return mult_rng(14); });
    std::generate(v2.begin(), v2.end(), [&mult_rng]() { return mult_rng(14); });
    return std::make_tuple(std::move(v1), std::move(v2), std::move(v3));
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
        auto p = get_init_vectors<mppp::integer<1>>();
        constexpr auto name = "mppp::integer<1>";

        mppp::integer<1> ret(0);

        mppp_benchmark::simple_timer st;

        for (auto i = 0ul; i < size; ++i) {
            gcd(std::get<2>(p)[i], std::get<0>(p)[i], std::get<1>(p)[i]);
            ret += std::get<2>(p)[i];
        }

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, ret);
    }

#if MPPP_CPLUSPLUS >= 201703L
    {
        auto p = get_init_vectors<std::int_least64_t>();
        constexpr auto name = "std::int64_t";

        std::int_least64_t ret = 0;

        mppp_benchmark::simple_timer st;

        for (auto i = 0ul; i < size; ++i) {
            std::get<2>(p)[i] = std::gcd(std::get<0>(p)[i], std::get<1>(p)[i]);
            ret += std::get<2>(p)[i];
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

        cpp_int ret = 0;

        mppp_benchmark::simple_timer st;

        for (auto i = 0ul; i < size; ++i) {
            std::get<2>(p)[i] = gcd(std::get<0>(p)[i], std::get<1>(p)[i]);
            ret += std::get<2>(p)[i];
        }

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, ret);
    }

    {
        auto p = get_init_vectors<mpz_int>();
        constexpr auto name = "boost::gmp_int";

        mpz_int ret = 0;

        mppp_benchmark::simple_timer st;

        for (auto i = 0ul; i < size; ++i) {
            mpz_gcd(std::get<2>(p)[i].backend().data(), std::get<0>(p)[i].backend().data(),
                    std::get<1>(p)[i].backend().data());
            mpz_add(ret.backend().data(), ret.backend().data(), std::get<2>(p)[i].backend().data());
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
            ::fmpz_gcd(std::get<2>(p)[i]._data().inner, std::get<0>(p)[i]._data().inner,
                       std::get<1>(p)[i]._data().inner);
            ::fmpz_add(ret._data().inner, ret._data().inner, std::get<2>(p)[i]._data().inner);
        }

        const auto runtime = st.elapsed();
        bdata.emplace_back(name, runtime);
        fmt::print(mppp_benchmark::res_print_format, name, runtime, ret);
    }
#endif

    // Write out the .py and .rst files.
    mppp_benchmark::write_out(bdata, benchmark_name);
}
