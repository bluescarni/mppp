// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
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
using cpp_int = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>, boost::multiprecision::et_off>;
using mpz_int = boost::multiprecision::number<boost::multiprecision::gmp_int, boost::multiprecision::et_off>;
#endif

#if defined(MPPP_BENCHMARK_FLINT)
using fmpzxx = flint::fmpzxx;
#endif

using integer_t = integer<1>;
static const std::string name = "integer1_sort_signed";

constexpr auto size = 30000000ul;

static std::mt19937 rng;

template <typename T>
static inline std::vector<T> get_init_vector(double &init_time)
{
    rng.seed(0);
    std::uniform_int_distribution<long> dist(-300000l, 300000l);
    simple_timer st;
    std::vector<T> retval(size);
    std::generate(retval.begin(), retval.end(), [&dist]() { return T(dist(rng)); });
    std::cout << initRuntime;
    init_time = st.elapsed();
    return retval;
}

int main()
{
    // Warm up.
    for (auto volatile counter = 0ull; counter < 1000000000ull; ++counter) {
    }
    // Setup of the python output.
    std::string s = pyPrefix;
    {
        std::cout << "\nSort signed 1\n----------------------------------" << std::endl;
        std::cout << bench_mpp;
        simple_timer st1;
        double init_time;
        auto v = get_init_vector<integer_t>(init_time);
        s += "['mp++','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            std::sort(v.begin(), v.end());
            s += "['mp++','sorting'," + std::to_string(st2.elapsed()) + "],";
            std::cout << sortRuntime;
        }
        s += "['mp++','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << totalRuntime;
    }
#if defined(MPPP_BENCHMARK_BOOST)
    {
        std::cout << bench_cpp_int;
        simple_timer st1;
        double init_time;
        auto v = get_init_vector<cpp_int>(init_time);
        s += "['Boost (cpp_int)','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            std::sort(v.begin(), v.end());
            s += "['Boost (cpp_int)','sorting'," + std::to_string(st2.elapsed()) + "],";
            std::cout << sortRuntime;
        }
        s += "['Boost (cpp_int)','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << totalRuntime;
    }
    {
        std::cout << bench_mpz_int;
        simple_timer st1;
        double init_time;
        auto v = get_init_vector<mpz_int>(init_time);
        s += "['Boost (mpz_int)','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            std::sort(v.begin(), v.end());
            s += "['Boost (mpz_int)','sorting'," + std::to_string(st2.elapsed()) + "],";
            std::cout << sortRuntime;
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
        auto v = get_init_vector<fmpzxx>(init_time);
        s += "['FLINT','init'," + std::to_string(init_time) + "],";
        {
            simple_timer st2;
            std::sort(v.begin(), v.end(), [](const fmpzxx &a, const fmpzxx &b) {
                return ::fmpz_cmp(a._data().inner, b._data().inner) < 0;
            });
            s += "['FLINT','sorting'," + std::to_string(st2.elapsed()) + "],";
            std::cout << sortRuntime;
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
