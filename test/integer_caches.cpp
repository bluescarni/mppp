// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <atomic>
#include <cstddef>
#include <random>
#include <thread>
#include <tuple>
#include <type_traits>
#include <vector>

#include <mp++/config.hpp>
#include <mp++/integer.hpp>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

static int ntries = 1000;

using namespace mppp;
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

struct cache_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        std::atomic<bool> flag{true};
        // Run a variety of tests with operands with x number of limbs.
        auto random_xy = [&flag](unsigned x) {
            auto checker = [&flag]() {
#if defined(MPPP_HAVE_THREAD_LOCAL)
                const auto &mpzc = detail::get_thread_local_mpz_cache();
                for (auto s : mpzc.sizes) {
                    if (s) {
                        flag.store(false);
                    }
                }
#endif
            };
            std::mt19937 rng;
            rng.seed(x);
            std::uniform_int_distribution<int> sdist(0, 1);
            detail::mpz_raii tmp;
            std::vector<integer> v_int;
            for (int i = 0; i < ntries; ++i) {
                random_integer(tmp, x, rng);
                v_int.emplace_back(&tmp.m_mpz);
                if (sdist(rng)) {
                    v_int.back().neg();
                }
                if (sdist(rng)) {
                    // Promote sometimes, if possible.
                    v_int.back().promote();
                }
            }
            v_int.resize(0);
            free_integer_caches();
            free_integer_caches();
            free_integer_caches();
            checker();
            for (int i = 0; i < ntries; ++i) {
                random_integer(tmp, x, rng);
                v_int.emplace_back(&tmp.m_mpz);
                if (sdist(rng)) {
                    v_int.back().neg();
                }
                if (sdist(rng)) {
                    // Promote sometimes, if possible.
                    v_int.back().promote();
                }
            }
            v_int.resize(0);
            free_integer_caches();
            free_integer_caches();
            free_integer_caches();
            checker();
            for (int i = 0; i < ntries; ++i) {
                random_integer(tmp, x, rng);
                v_int.emplace_back(&tmp.m_mpz);
                if (sdist(rng)) {
                    v_int.back().neg();
                }
                if (sdist(rng)) {
                    // Promote sometimes, if possible.
                    v_int.back().promote();
                }
            }
            free_integer_caches();
            free_integer_caches();
            free_integer_caches();
        };

        std::thread t0(random_xy, 0);
        std::thread t1(random_xy, 1);
        std::thread t2(random_xy, 2);
        std::thread t3(random_xy, 3);
        std::thread t4(random_xy, 4);
        t0.join();
        t1.join();
        t2.join();
        t3.join();
        t4.join();
    }
};

TEST_CASE("caches")
{
    tuple_for_each(sizes{}, cache_tester{});
}
