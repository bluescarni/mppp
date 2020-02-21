// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <mp++/mp++.hpp>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "constStrings.hpp"
#include "simple_timer.hpp"

#include <boost/format.hpp>

#if defined(MPPP_BENCHMARK_BOOST)
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/gmp.hpp>
#include <gmp.h>
#endif

#if defined(MPPP_BENCHMARK_FLINT)
#include <flint.h>
#include <fmpzxx.h>
#endif

using namespace mppp;
using namespace mppp_bench;

#if defined(MPPP_BENCHMARK_BOOST)
using cpp_int = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>, boost::multiprecision::et_on>;
using mpz_int = boost::multiprecision::number<boost::multiprecision::gmp_int, boost::multiprecision::et_off>;
#endif

#if defined(MPPP_BENCHMARK_FLINT)
using fmpzxx = flint::fmpzxx;
#endif

static std::mt19937 rng;

using integer_t = integer<1>;
static const std::string name = "integer1_dot_product_unsigned";

constexpr auto size = 30000000ul;

template <typename T>
static inline std::pair<std::vector<T>, std::vector<T>> get_init_vectors(double &init_time)
{
    rng.seed(0);
    std::uniform_int_distribution<unsigned> dist(1u, 7u);
    simple_timer st;
    std::vector<T> v1(size), v2(size);
    std::generate(v1.begin(), v1.end(), [&dist]() { return T(dist(rng)); });
    std::generate(v2.begin(), v2.end(), [&dist]() { return T(dist(rng)); });
    std::cout << initRuntime;
    init_time = st.elapsed();
    return std::make_pair(std::move(v1), std::move(v2));
}

int main()
{
    // Warm up.
    for (auto volatile counter = 0ull; counter < 1000000000ull; ++counter) {
    }
    // Setup of the python output.
    std::string s = pyPrefix;
    {
        std::cout << "\nDot Product unsigned 1\n----------------------------------" << std::endl;
        std::cout << bench_mpp;
        simple_timer st1;
        double init_time;
        auto p = get_init_vectors<integer_t>(init_time);
        s += "['mp++','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            integer_t ret(0);
            for (auto i = 0ul; i < size; ++i) {
                addmul(ret, p.first[i], p.second[i]);
            }
            std::cout << " / " << ret;
            s += "['mp++','operation'," + std::to_string(st2.elapsed()) + "],";
            std::cout << operRuntime;
        }
        s += "['mp++','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << totalRuntime;
    }
    {
        std::cout << "\n\nBenchmarking uint64.";
        simple_timer st1;
        double init_time;
        auto p = get_init_vectors<std::uint_least64_t>(init_time);
        s += "['uint64','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            std::uint_least64_t ret(0);
            for (auto i = 0ul; i < size; ++i) {
                ret += p.first[i] * p.second[i];
            }
            std::cout << " / " << ret;
            s += "['uint64','operation'," + std::to_string(st2.elapsed()) + "],";
            std::cout << operRuntime;
        }
        s += "['uint64','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << totalRuntime;
    }
#if defined(MPPP_HAVE_GCC_INT128)
    {
        std::cout << "\n\nBenchmarking uint128.";
        simple_timer st1;
        double init_time;
        auto p = get_init_vectors<__uint128_t>(init_time);
        s += "['uint128','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            __uint128_t ret(0);
            for (auto i = 0ul; i < size; ++i) {
                ret += p.first[i] * p.second[i];
            }
            std::cout << " / " << static_cast<std::uint_least64_t>(ret);
            s += "['uint128','operation'," + std::to_string(st2.elapsed()) + "],";
            std::cout << operRuntime;
        }
        s += "['uint128','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << totalRuntime;
    }
#endif
#if defined(MPPP_BENCHMARK_BOOST)
    {
        std::cout << bench_cpp_int;
        simple_timer st1;
        double init_time;
        auto p = get_init_vectors<cpp_int>(init_time);
        s += "['Boost (cpp_int)','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            cpp_int ret(0);
            for (auto i = 0ul; i < size; ++i) {
                ret += p.first[i] * p.second[i];
            }
            std::cout << " / " << ret;
            s += "['Boost (cpp_int)','operation'," + std::to_string(st2.elapsed()) + "],";
            std::cout << operRuntime;
        }
        s += "['Boost (cpp_int)','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << totalRuntime;
    }
    {
        std::cout << bench_mpz_int;
        simple_timer st1;
        double init_time;
        auto p = get_init_vectors<mpz_int>(init_time);
        s += "['Boost (mpz_int)','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            mpz_int ret(0);
            for (auto i = 0ul; i < size; ++i) {
                ::mpz_addmul(ret.backend().data(), p.first[i].backend().data(), p.second[i].backend().data());
            }
            std::cout << " / " << ret;
            s += "['Boost (mpz_int)','operation'," + std::to_string(st2.elapsed()) + "],";
            std::cout << operRuntime;
        }
        s += "['Boost (mpz_int)','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << totalRuntime;
    }
#endif
#if defined(MPPP_BENCHMARK_FLINT)
    {
        std::cout << bench_fmpzxx;
        simple_timer st1;
        double init_time;
        auto p = get_init_vectors<fmpzxx>(init_time);
        s += "['FLINT','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            fmpzxx ret(0);
            for (auto i = 0ul; i < size; ++i) {
                ::fmpz_addmul(ret._data().inner, p.first[i]._data().inner, p.second[i]._data().inner);
            }
            std::cout << " / " << ret;
            s += "['FLINT','operation'," + std::to_string(st2.elapsed()) + "],";
            std::cout << operRuntime;
        }
        s += "['FLINT','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << totalRuntime;
    }
#endif
    s += boost::str(boost::format(pySuffix) % name);
    std::ofstream of(name + ".py", std::ios_base::trunc);
    of << s;
    of.close();
    std::cout << "\n\n" << std::flush;
}
