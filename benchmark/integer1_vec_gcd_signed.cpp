// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <fstream>
#include <iostream>
#include <mp++/mp++.hpp>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#include "simple_timer.hpp"
#include "constStrings.hpp"

#include "boost/format.hpp"


#if defined(MPPP_BENCHMARK_BOOST)
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/gmp.hpp>
#include <gmp.h>
#endif

#if defined(MPPP_BENCHMARK_FLINT)
#include <flint/flint.h>
#include <flint/fmpzxx.h>
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
static const std::string name = "integer1_vec_gcd_signed";

constexpr auto size = 30000000ul;

template <typename T>
static inline std::tuple<std::vector<T>, std::vector<T>, std::vector<T>> get_init_vectors(double &init_time)
{
    rng.seed(1);
    std::uniform_int_distribution<int> dist(1, 10), sign(0, 1);
    simple_timer st;
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
    std::cout << initRuntime;
    init_time = st.elapsed();
    return std::make_tuple(std::move(v1), std::move(v2), std::move(v3));
}

int main()
{
    // Warm up.
    for (auto volatile counter = 0ull; counter < 1000000000ull; ++counter) {
    }
    // Setup of the python output.
    std::string s = pyPrefix;
    {
        std::cout << "\nVector GCD signed 1\n----------------------------------" << std::endl;
        std::cout << bench_mpp;
        simple_timer st1;
        double init_time;
        auto p = get_init_vectors<integer_t>(init_time);
        s += "['mp++','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            integer_t ret(0);
            for (auto i = 0ul; i < size; ++i) {
                gcd(std::get<2>(p)[i], std::get<0>(p)[i], std::get<1>(p)[i]);
                ret += std::get<2>(p)[i];
            }
            std::cout << " / " << ret;
            s += "['mp++','operation'," + std::to_string(st2.elapsed()) + "],";
            std::cout << operRuntime;
        }
        s += "['mp++','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << totalRuntime;
    }
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
                std::get<2>(p)[i] = gcd(std::get<0>(p)[i], std::get<1>(p)[i]);
                ret += std::get<2>(p)[i];
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
                ::mpz_gcd(std::get<2>(p)[i].backend().data(), std::get<0>(p)[i].backend().data(),
                          std::get<1>(p)[i].backend().data());
                ::mpz_add(ret.backend().data(), ret.backend().data(), std::get<2>(p)[i].backend().data());
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
                ::fmpz_gcd(std::get<2>(p)[i]._data().inner, std::get<0>(p)[i]._data().inner,
                           std::get<1>(p)[i]._data().inner);
                ::fmpz_add(ret._data().inner, ret._data().inner, std::get<2>(p)[i]._data().inner);
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
