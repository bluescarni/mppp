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
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

#if defined(MPPP_BENCHMARK_BOOST)
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/gmp.hpp>
#include <gmp.h>
#endif

#include "simple_timer.hpp"
#include "constStrings.hpp"

#include "boost/format.hpp"


using namespace mppp;
using namespace mppp_bench;

#if defined(MPPP_BENCHMARK_BOOST)
// Make sure we use the cpp_int version that does the overflow check, in order to
// match mp++'s behaviour.
using cpp_int
    = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<
                                        0, 0, boost::multiprecision::signed_magnitude, boost::multiprecision::checked>,
                                    boost::multiprecision::et_off>;
using mpz_int = boost::multiprecision::number<boost::multiprecision::gmp_int, boost::multiprecision::et_off>;
#endif

using integer_t = integer<2>;
static const std::string name = "integer2_int_conversion";

constexpr auto size = 30000000ul;

static std::mt19937 rng;

template <typename T>
static inline std::vector<T> get_init_vector(double &init_time)
{
    rng.seed(0);
    std::uniform_int_distribution<int> dist(-10000, 10000);
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
        std::cout << "\nInteger Conversion 2\n----------------------------------" << std::endl;
        std::cout << bench_mpp;
        simple_timer st1;
        double init_time;
        auto v = get_init_vector<integer_t>(init_time);
        s += "['mp++','init'," + std::to_string(init_time) + "],";
        std::vector<int> c_out(size);
        {
            simple_timer st2;
            std::transform(v.begin(), v.end(), c_out.begin(), [](const integer_t &n) { return static_cast<int>(n); });
            s += "['mp++','convert'," + std::to_string(st2.elapsed()) + "],";
            std::cout << convRuntime;
        }
        s += "['mp++','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << " / " << std::accumulate(c_out.begin(), c_out.end(), 0l);
        std::cout << totalRuntime;
    }
#if defined(MPPP_BENCHMARK_BOOST)
    {
        std::cout << bench_cpp_int;
        simple_timer st1;
        double init_time;
        auto v = get_init_vector<cpp_int>(init_time);
        s += "['Boost (cpp_int)','init'," + std::to_string(init_time) + "],";
        std::vector<int> c_out(size);
        {
            simple_timer st2;
            std::transform(v.begin(), v.end(), c_out.begin(), [](const cpp_int &n) { return static_cast<int>(n); });
            s += "['Boost (cpp_int)','convert'," + std::to_string(st2.elapsed()) + "],";
            std::cout << convRuntime;
        }
        s += "['Boost (cpp_int)','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << " / " << std::accumulate(c_out.begin(), c_out.end(), 0l);
        std::cout << totalRuntime;
    }
    {
        std::cout << bench_mpz_int;
        simple_timer st1;
        double init_time;
        auto v = get_init_vector<mpz_int>(init_time);
        s += "['Boost (mpz_int)','init'," + std::to_string(init_time) + "],";
        std::vector<int> c_out(size);
        {
            simple_timer st2;
            std::transform(v.begin(), v.end(), c_out.begin(),
                           [](const mpz_int &n) { return static_cast<int>(::mpz_get_si(n.backend().data())); });
            s += "['Boost (mpz_int)','convert'," + std::to_string(st2.elapsed()) + "],";
            std::cout << convRuntime;
        }
        s += "['Boost (mpz_int)','total'," + std::to_string(st1.elapsed()) + "],";
        std::cout << " / " << std::accumulate(c_out.begin(), c_out.end(), 0l);
        std::cout << totalRuntime;
    }
#endif
    s += boost::str(boost::format(pySuffix) % name);
    std::ofstream of(name + ".py", std::ios_base::trunc);
    of << s;
    of.close();
    std::cout << "\n\n" << std::flush;
}
