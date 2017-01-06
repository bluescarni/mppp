/* Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)

This file is part of the mp++ library.

The mp++ library is free software; you can redistribute it and/or modify
it under the terms of either:

  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.

or

  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.

or both in parallel, as here.

The mp++ library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the mp++ library.  If not,
see https://www.gnu.org/licenses/. */

#include <algorithm>
#include <iostream>
#include <mp++.hpp>
#include <tuple>
#include <vector>

#include "simple_timer.hpp"

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
using cpp_int = boost::multiprecision::cpp_int;
using mpz_int = boost::multiprecision::mpz_int;
#endif

#if defined(MPPP_BENCHMARK_FLINT)
using fmpzxx = flint::fmpzxx;
#endif

using integer = mp_integer<2>;

constexpr auto size = 30000000ul;

template <typename T>
static inline std::tuple<std::vector<T>, std::vector<T>, std::vector<T>> get_init_vectors()
{
    simple_timer st;
    std::vector<T> v1(size), v2(size), v3(size);
    std::fill(v1.begin(), v1.end(), T(2));
    std::fill(v2.begin(), v2.end(), T(2));
    std::cout << "\nInit runtime: ";
    return std::make_tuple(std::move(v1), std::move(v2), std::move(v3));
}

int main()
{
    {
        std::cout << "\n\nBenchmarking mp++.";
        simple_timer st1;
        auto p = get_init_vectors<integer>();
        {
            simple_timer st2;
            integer ret(0);
            for (auto i = 0ul; i < size; ++i) {
                mul(std::get<2>(p)[i], std::get<0>(p)[i], std::get<1>(p)[i]);
            }
            for (auto i = 0ul; i < size; ++i) {
                add(ret, ret, std::get<2>(p)[i]);
            }
            std::cout << ret << '\n';
            std::cout << "\nArithmetic runtime: ";
        }
        std::cout << "\nTotal runtime: ";
    }
#if defined(MPPP_BENCHMARK_BOOST)
    {
        std::cout << "\n\nBenchmarking cpp_int.";
        simple_timer st1;
        auto p = get_init_vectors<cpp_int>();
        {
            simple_timer st2;
            cpp_int ret(0);
            for (auto i = 0ul; i < size; ++i) {
                std::get<2>(p)[i] = std::get<0>(p)[i] * std::get<1>(p)[i];
            }
            for (auto i = 0ul; i < size; ++i) {
                ret += std::get<2>(p)[i];
            }
            std::cout << ret << '\n';
            std::cout << "\nArithmetic runtime: ";
        }
        std::cout << "\nTotal runtime: ";
    }
    {
        std::cout << "\n\nBenchmarking mpz_int.";
        simple_timer st1;
        auto p = get_init_vectors<mpz_int>();
        {
            simple_timer st2;
            mpz_int ret(0);
            for (auto i = 0ul; i < size; ++i) {
                ::mpz_mul(std::get<2>(p)[i].backend().data(), std::get<0>(p)[i].backend().data(),
                          std::get<1>(p)[i].backend().data());
            }
            for (auto i = 0ul; i < size; ++i) {
                ::mpz_add(ret.backend().data(), ret.backend().data(), std::get<2>(p)[i].backend().data());
            }
            std::cout << ret << '\n';
            std::cout << "\nArithmetic runtime: ";
        }
        std::cout << "\nTotal runtime: ";
    }
#endif
#if defined(MPPP_BENCHMARK_FLINT)
    {
        std::cout << "\n\nBenchmarking fmpzxx.";
        simple_timer st1;
        auto p = get_init_vectors<fmpzxx>();
        {
            simple_timer st2;
            fmpzxx ret(0);
            for (auto i = 0ul; i < size; ++i) {
                ::fmpz_mul(std::get<2>(p)[i]._data().inner, std::get<0>(p)[i]._data().inner,
                           std::get<1>(p)[i]._data().inner);
            }
            for (auto i = 0ul; i < size; ++i) {
                ::fmpz_add(ret._data().inner, ret._data().inner, std::get<2>(p)[i]._data().inner);
            }
            std::cout << ret << '\n';
            std::cout << "\nArithmetic runtime: ";
        }
        std::cout << "\nTotal runtime: ";
    }
#endif
}
