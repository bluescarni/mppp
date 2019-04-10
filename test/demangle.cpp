// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>
#include <mp++/detail/demangle.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>

#if MPPP_CPLUSPLUS >= 201703L
#include <string_view>
#endif
#include <string>
#include <thread>
#include <typeinfo>
#include <vector>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

TEST_CASE("demangle")
{
    std::cout << detail::demangle<std::string>() << '\n';
    std::cout << detail::demangle<int>() << '\n';
    std::cout << detail::demangle<const int>() << '\n';
    std::cout << detail::demangle<const volatile int>() << '\n';
    std::cout << detail::demangle<volatile int>() << '\n';
    std::cout << detail::demangle<long double>() << '\n';
    std::cout << detail::demangle<std::vector<std::vector<float>>>() << '\n';
    std::cout << detail::demangle<integer<1>>() << '\n';
    std::cout << detail::demangle<rational<2>>() << '\n';
    std::cout << detail::demangle<rational<2> &>() << '\n';
    std::cout << detail::demangle<rational<2> const>() << '\n';
    std::cout << detail::demangle<rational<2> const &>() << '\n';
    std::cout << detail::demangle<rational<2> *>() << '\n';
    std::cout << detail::demangle<const rational<2> *>() << '\n';
    std::cout << detail::demangle<const rational<2> *const>() << '\n';
    std::cout << detail::demangle<const rational<2> *const &>() << '\n';
    std::cout << detail::demangle<void>() << '\n';
    std::cout << detail::demangle<void const>() << '\n';
    std::cout << detail::demangle<void volatile>() << '\n';
    std::cout << detail::demangle<void volatile const>() << '\n';
#if defined(MPPP_HAVE_GCC_INT128)
    std::cout << detail::demangle<__int128_t>() << '\n';
    std::cout << detail::demangle<__int128_t *>() << '\n';
    std::cout << detail::demangle<__int128_t const *>() << '\n';
    std::cout << detail::demangle<__int128_t const *const>() << '\n';
    std::cout << detail::demangle<__uint128_t>() << '\n';
    std::cout << detail::demangle<__uint128_t *>() << '\n';
    std::cout << detail::demangle<__uint128_t const *>() << '\n';
    std::cout << detail::demangle<__uint128_t const *const>() << '\n';
    std::cout << detail::demangle<const __int128_t>() << '\n';
    std::cout << detail::demangle<const __uint128_t>() << '\n';
    std::cout << detail::demangle<__int128_t &>() << '\n';
    std::cout << detail::demangle<__uint128_t &>() << '\n';
    std::cout << detail::demangle<__int128_t &&>() << '\n';
    std::cout << detail::demangle<__uint128_t &&>() << '\n';
    std::cout << detail::demangle<const __int128_t &>() << '\n';
    std::cout << detail::demangle<const __uint128_t &>() << '\n';
    std::cout << detail::demangle<std::vector<__int128_t>>() << '\n';
#endif

    // Couple of multithreaded tests.
    auto t_func = []() -> std::string {
        std::string tmp;
        for (auto i = 0; i < 100; ++i) {
            tmp += detail::demangle<std::vector<std::vector<float>>>();
        }
        return tmp;
    };

    std::thread t1(t_func);
    std::thread t2(t_func);
    std::thread t3(t_func);
    std::thread t4(t_func);
    std::thread t5(t_func);
    std::thread t6(t_func);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();

#if MPPP_CPLUSPLUS >= 201703L
    // Test the string view overload.
    const auto tname = typeid(int).name();
    const std::string_view sv(tname);
    REQUIRE(detail::demangle(sv) == detail::demangle(tname));
#endif
}
