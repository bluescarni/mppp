// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <atomic>
#include <cmath>
#include <complex>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <gmp.h>

#include <mp++/config.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/integer.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

static const int ntries = 1000;

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp_test;

using int_types = std::tuple<char, signed char, unsigned char, short, unsigned short, int, unsigned, long,
                             unsigned long, long long, unsigned long long, wchar_t
#if defined(MPPP_HAVE_GCC_INT128)
                             ,
                             __uint128_t, __int128_t
#endif
                             >;

using fp_types = std::tuple<float, double
#if defined(MPPP_WITH_MPFR)
                            ,
                            long double
#endif
                            >;

using complex_types = std::tuple<std::complex<float>, std::complex<double>
#if defined(MPPP_WITH_MPFR)
                                 ,
                                 std::complex<long double>
#endif
                                 >;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

// A seed that will be used to init rngs in the multithreaded tests. Each time a batch of N threads
// finishes, this value gets bumped up by N, so that the next time a multithreaded test which uses rng
// is launched it will be inited with a different seed.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static std::mt19937::result_type mt_rng_seed(0u);

struct no_const {
};

struct int_ctor_tester {
    template <typename S>
    struct runner {
        template <typename Int>
        void operator()(const Int &) const
        {
            using integer = integer<S::value>;
            REQUIRE((std::is_constructible<integer, Int>::value));
            REQUIRE(lex_cast(Int(0)) == lex_cast(integer{Int(0)}));
            REQUIRE(lex_cast(Int(42)) == lex_cast(integer{Int(42)}));
            REQUIRE(lex_cast(Int(-42)) == lex_cast(integer{Int(-42)}));
            auto constexpr min = detail::nl_min<Int>(), max = detail::nl_max<Int>();
            REQUIRE(lex_cast(min) == lex_cast(integer{min}));
            REQUIRE(lex_cast(max) == lex_cast(integer{max}));
            std::atomic<bool> fail(false);
            auto f = [&fail](unsigned n) {
                integral_minmax_dist<Int> dist;
                std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
                for (auto i = 0; i < ntries; ++i) {
                    auto tmp = static_cast<Int>(dist(eng));
                    if (lex_cast(tmp) != lex_cast(integer{tmp})) {
                        fail.store(false);
                    }
                }
            };
            std::thread t0(f, 0u), t1(f, 1u), t2(f, 2u), t3(f, 3u);
            t0.join();
            t1.join();
            t2.join();
            t3.join();
            REQUIRE(!fail.load());
            // Update the rng seed so that it does not generate the same sequence
            // for the next integral type.
            mt_rng_seed += 4u;

            // Make sure integer is implicitly ctible
            // from the integral types.
            integer tmp = Int(5);
            REQUIRE(std::is_convertible<Int, integer>::value);

            std::vector<integer> vec = {Int(1), Int(2), Int(3)};
        }
    };
    template <typename S>
    inline void operator()(const S &) const
    {
        tuple_for_each(int_types{}, runner<S>{});
        // Some testing for bool.
        using integer = integer<S::value>;
        REQUIRE((std::is_constructible<integer, bool>::value));
        REQUIRE((lex_cast(integer{false}) == "0"));
        REQUIRE((lex_cast(integer{true}) == "1"));

        integer tmp = true;
        REQUIRE(std::is_convertible<bool, integer>::value);
        std::vector<integer> vec = {true, false};
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 0);

        REQUIRE((!std::is_constructible<integer, no_const>::value));
        std::cout << "n static limbs: " << S::value << ", size: " << sizeof(integer) << '\n';
    }
};

TEST_CASE("integral constructors")
{
    tuple_for_each(sizes{}, int_ctor_tester{});
}

struct int_ass_tester {
    template <typename S>
    struct runner {
        template <typename Int>
        void operator()(const Int &) const
        {
            using integer = integer<S::value>;
            REQUIRE((std::is_assignable<integer &, Int>::value));
            integer n0;
            n0 = Int(0);
            REQUIRE(n0 == 0);
            REQUIRE(n0.is_static());
            auto constexpr min = detail::nl_min<Int>(), max = detail::nl_max<Int>();
            n0 = min;
            REQUIRE(n0 == min);
            n0 = max;
            REQUIRE(n0 == max);
            n0 = Int(42);
            REQUIRE(n0 == Int(42));
            n0 = Int(-42);
            REQUIRE(n0 == Int(-42));
            std::atomic<bool> fail(false);
            auto f = [&fail](unsigned n) {
                integral_minmax_dist<Int> dist;
                std::uniform_int_distribution<int> sdist(0, 1);
                std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
                for (auto i = 0; i < ntries; ++i) {
                    integer n1;
                    if (sdist(eng)) {
                        n1.promote();
                    }
                    auto tmp = static_cast<Int>(dist(eng));
                    n1 = tmp;
                    if (n1 != tmp) {
                        fail.store(false);
                    }
                }
            };
            std::thread t0(f, 0u), t1(f, 1u), t2(f, 2u), t3(f, 3u);
            t0.join();
            t1.join();
            t2.join();
            t3.join();
            REQUIRE(!fail.load());
            // Update the rng seed so that it does not generate the same sequence
            // for the next integral type.
            mt_rng_seed += 4u;
        }
    };
    template <typename S>
    inline void operator()(const S &) const
    {
        tuple_for_each(int_types{}, runner<S>{});
        // Some testing for bool.
        using integer = integer<S::value>;
        REQUIRE((std::is_assignable<integer &, bool>::value));
        integer n0;
        n0 = false;
        REQUIRE(n0.is_zero());
        REQUIRE(n0.is_static());
        n0.promote();
        n0 = true;
        REQUIRE(n0 == 1);
        REQUIRE(n0.is_static());
        REQUIRE((!std::is_assignable<integer &, no_const>::value));
    }
};

TEST_CASE("integral assignment")
{
    tuple_for_each(sizes{}, int_ass_tester{});
}

struct fp_ctor_tester {
    template <typename S>
    struct runner {
        template <typename Float>
        void operator()(const Float &) const
        {
            using integer = integer<S::value>;
            REQUIRE((std::is_constructible<integer, Float>::value));
            if (std::numeric_limits<Float>::is_iec559) {
                REQUIRE_THROWS_PREDICATE(
                    integer{std::numeric_limits<Float>::infinity()}, std::domain_error,
                    [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot construct an integer from the non-finite floating-point value "
                                      + std::to_string(std::numeric_limits<Float>::infinity());
                    });
                REQUIRE_THROWS_PREDICATE(
                    integer{-std::numeric_limits<Float>::infinity()}, std::domain_error,
                    [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot construct an integer from the non-finite floating-point value "
                                      + std::to_string(-std::numeric_limits<Float>::infinity());
                    });
                REQUIRE_THROWS_PREDICATE(
                    integer{std::numeric_limits<Float>::quiet_NaN()}, std::domain_error,
                    [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot construct an integer from the non-finite floating-point value "
                                      + std::to_string(std::numeric_limits<Float>::quiet_NaN());
                    });
            }
            REQUIRE(lex_cast(integer{Float(0)}) == "0");
            REQUIRE(lex_cast(integer{Float(1.5)}) == "1");
            REQUIRE(lex_cast(integer{Float(-1.5)}) == "-1");
            REQUIRE(lex_cast(integer{Float(123.9)}) == "123");
            REQUIRE(lex_cast(integer{Float(-123.9)}) == "-123");
            // Random testing.
            std::atomic<bool> fail(false);
            auto f = [&fail](unsigned n) {
                std::uniform_real_distribution<Float> dist(Float(-100), Float(100));
                std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
                for (auto i = 0; i < ntries; ++i) {
                    auto tmp = dist(eng);
                    if (lex_cast(integer{std::trunc(tmp)}) != lex_cast(integer{tmp})) {
                        fail.store(false);
                    }
                }
            };
            std::thread t0(f, 0u), t1(f, 1u), t2(f, 2u), t3(f, 3u);
            t0.join();
            t1.join();
            t2.join();
            t3.join();
            REQUIRE(!fail.load());
            mt_rng_seed += 4u;

            // Make sure integer is *not* implicitly ctible
            // from the fp types.
            REQUIRE(!std::is_convertible<Float, integer>::value);
        }
    };
    template <typename S>
    inline void operator()(const S &) const
    {
        tuple_for_each(fp_types{}, runner<S>{});
    }
};

TEST_CASE("floating-point constructors")
{
    tuple_for_each(sizes{}, fp_ctor_tester{});
}

struct complex_ctor_tester {
    template <typename S>
    struct runner {
        template <typename C>
        void operator()(const C &) const
        {
            using integer = integer<S::value>;
            using Float = typename C::value_type;

            REQUIRE((std::is_constructible<integer, C>::value));
            REQUIRE(integer{C(0, 0)} == integer{0});
            REQUIRE(integer{C(123, 0)} == integer{123});
            REQUIRE(integer{C(-456, 0)} == integer{-456});
            if (std::numeric_limits<Float>::is_iec559) {
                REQUIRE_THROWS_PREDICATE(
                    integer{(C{std::numeric_limits<Float>::infinity(), 0})}, std::domain_error,
                    [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot construct an integer from the non-finite floating-point value "
                                      + std::to_string(std::numeric_limits<Float>::infinity());
                    });
                REQUIRE_THROWS_PREDICATE(
                    integer{(C{-std::numeric_limits<Float>::infinity(), 0})}, std::domain_error,
                    [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot construct an integer from the non-finite floating-point value "
                                      + std::to_string(-std::numeric_limits<Float>::infinity());
                    });
                REQUIRE_THROWS_PREDICATE(
                    integer{(C{std::numeric_limits<Float>::quiet_NaN(), 0})}, std::domain_error,
                    [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot construct an integer from the non-finite floating-point value "
                                      + std::to_string(std::numeric_limits<Float>::quiet_NaN());
                    });
                REQUIRE_THROWS_PREDICATE(integer{(C{0, std::numeric_limits<Float>::quiet_NaN()})}, std::domain_error,
                                         [](const std::domain_error &ex) {
                                             return ex.what()
                                                    == "Cannot construct an integer from a complex C++ "
                                                       "value with a non-zero imaginary part of "
                                                           + detail::to_string(std::numeric_limits<Float>::quiet_NaN());
                                         });
                REQUIRE_THROWS_PREDICATE(integer{(C{0, std::numeric_limits<Float>::infinity()})}, std::domain_error,
                                         [](const std::domain_error &ex) {
                                             return ex.what()
                                                    == "Cannot construct an integer from a complex C++ "
                                                       "value with a non-zero imaginary part of "
                                                           + detail::to_string(std::numeric_limits<Float>::infinity());
                                         });
            }

            REQUIRE_THROWS_PREDICATE(integer{(C{0, 1})}, std::domain_error, [](const std::domain_error &ex) {
                return ex.what()
                       == "Cannot construct an integer from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(Float(1));
            });

            REQUIRE_THROWS_PREDICATE(integer{(C{-1, 1})}, std::domain_error, [](const std::domain_error &ex) {
                return ex.what()
                       == "Cannot construct an integer from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(Float(1));
            });
        }
    };
    template <typename S>
    inline void operator()(const S &) const
    {
        tuple_for_each(complex_types{}, runner<S>{});
    }
};

TEST_CASE("complex constructors")
{
    tuple_for_each(sizes{}, complex_ctor_tester{});
}
