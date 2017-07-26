// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
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
#include <string>
#include <tuple>
#include <vector>

#include "simple_timer.hpp"

#if defined(MPPP_BENCHMARK_BOOST)
#include <boost/multiprecision/mpfr.hpp>
#endif

using namespace mppp;
using namespace mppp_bench;

using real_t = real<1>;
#if defined(MPPP_BENCHMARK_BOOST)
using mpfr_float = boost::multiprecision::mpfr_float;
#endif

static const std::string name = "real_vec_mul";

constexpr auto size = 30000000ul;

template <typename T>
static inline std::tuple<std::vector<T>, std::vector<T>, std::vector<T>> get_init_vectors(double &init_time)
{
    simple_timer st;
    std::vector<T> v1(size, T(1.1)), v2(size, T(1.1)), v3(size, T(0.));
    std::cout << "\nInit runtime: ";
    init_time = st.elapsed();
    return std::make_tuple(std::move(v1), std::move(v2), std::move(v3));
}

template <>
inline std::tuple<std::vector<mpfr_float>, std::vector<mpfr_float>, std::vector<mpfr_float>>
get_init_vectors(double &init_time)
{
    simple_timer st;
    std::vector<mpfr_float> v1(size), v2(size), v3(size);
    mpfr_float v;
    //::mpfr_set_prec(v.backend().data(), 256);
    //::mpfr_set_str(v.backend().data(), "1.1", 10, MPFR_RNDN);
    ::mpfr_set_prec(v.backend().data(), 53);
    ::mpfr_set_d(v.backend().data(), 1.1, MPFR_RNDN);
    std::fill(v1.begin(), v1.end(), v);
    std::fill(v2.begin(), v2.end(), v);
    ::mpfr_set_d(v.backend().data(), 0., MPFR_RNDN);
    std::fill(v3.begin(), v3.end(), v);
    std::cout << "\nInit runtime: ";
    init_time = st.elapsed();
    return std::make_tuple(std::move(v1), std::move(v2), std::move(v3));
}

int main()
{
    // Warm up.
    for (auto volatile counter = 0ull; counter < 1000000000ull; ++counter) {
    }
    // Setup of the python output.
    std::string s = "# -*- coding: utf-8 -*-\n"
                    "def get_data():\n"
                    "    import pandas\n"
                    "    data = [";
    {
        std::cout << "\n\nBenchmarking mp++.";
        std::cout.precision(100);
        simple_timer st1;
        double init_time;
        auto p = get_init_vectors<real_t>(init_time);
        s += "['mp++','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            real_t ret(0.);
            for (auto i = 0ul; i < size; ++i) {
                mul(std::get<2>(p)[i], std::get<0>(p)[i], std::get<1>(p)[i]);
            }
            for (auto i = 0ul; i < size; ++i) {
                add(ret, ret, std::get<2>(p)[i]);
            }
            std::cout << ret << '\n';
            s += "['mp++','arithmetic'," + std::to_string(st2.elapsed()) + "],";
            std::cout << "\nArithmetic runtime: ";
        }
        s += "['mp++','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << "\nTotal runtime: ";
    }
#if defined(MPPP_BENCHMARK_BOOST)
    {
        std::cout << "\n\nBenchmarking mpfr_float.";
        simple_timer st1;
        double init_time;
        auto p = get_init_vectors<mpfr_float>(init_time);
        s += "['Boost (mpfr_float)','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            mpfr_float ret;
            ::mpfr_set_prec(ret.backend().data(), 53);
            ::mpfr_set_d(ret.backend().data(), 0., MPFR_RNDN);
            for (auto i = 0ul; i < size; ++i) {
                ::mpfr_mul(std::get<2>(p)[i].backend().data(), std::get<0>(p)[i].backend().data(),
                           std::get<1>(p)[i].backend().data(), MPFR_RNDN);
            }
            for (auto i = 0ul; i < size; ++i) {
                ::mpfr_add(ret.backend().data(), ret.backend().data(), std::get<2>(p)[i].backend().data(), MPFR_RNDN);
            }
            std::cout << ret << '\n';
            s += "['Boost (mpfr_float)','arithmetic'," + std::to_string(st2.elapsed()) + "],";
            std::cout << "\nArithmetic runtime: ";
        }
        s += "['Boost (mpfr_float)','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << "\nTotal runtime: ";
    }
#endif
}
