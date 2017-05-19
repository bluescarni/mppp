// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/mp++.hpp>

#include <atomic>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

static int ntries = 1000;

using namespace mppp;
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

using int_types = std::tuple<char, signed char, unsigned char, short, unsigned short, int, unsigned, long,
                             unsigned long, long long, unsigned long long>;

struct no_const {
};

// A seed that will be used to init rngs in the multithreaded tests. Each time a batch of N threads
// finishes, this value gets bumped up by N, so that the next time a multithreaded test which uses rng
// is launched it will be inited with a different seed.
static std::mt19937::result_type mt_rng_seed(0u);

// NOTE: char types are not supported in uniform_int_distribution by the standard.
// Use a small wrapper to get an int distribution instead, with the min max limits
// from the char type. We will be casting back when using the distribution.
template <typename T,
          typename std::enable_if<!(std::is_same<char, T>::value || std::is_same<signed char, T>::value
                                    || std::is_same<unsigned char, T>::value),
                                  int>::type
          = 0>
static inline std::uniform_int_distribution<T> get_int_dist(T min, T max)
{
    return std::uniform_int_distribution<T>(min, max);
}

template <typename T,
          typename std::enable_if<std::is_same<char, T>::value || std::is_same<signed char, T>::value
                                      || std::is_same<unsigned char, T>::value,
                                  int>::type
          = 0>
static inline std::uniform_int_distribution<typename std::conditional<std::is_signed<T>::value, int, unsigned>::type>
get_int_dist(T min, T max)
{
    return std::uniform_int_distribution<typename std::conditional<std::is_signed<T>::value, int, unsigned>::type>(min,
                                                                                                                   max);
}

struct int_ctor_tester {
    template <typename S>
    struct runner {
        template <typename Int>
        void operator()(const Int &) const
        {
            using rational = rational<S::value>;
            REQUIRE((std::is_constructible<rational, Int>::value));
            REQUIRE(lex_cast(Int(0)) == lex_cast(rational{Int(0)}));
            auto constexpr min = std::numeric_limits<Int>::min(), max = std::numeric_limits<Int>::max();
            REQUIRE(lex_cast(min) == lex_cast(rational{min}));
            REQUIRE(lex_cast(max) == lex_cast(rational{max}));
            std::atomic<bool> fail(false);
            auto f = [&fail, min, max](unsigned n) {
                auto dist = get_int_dist(min, max);
                std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
                for (auto i = 0; i < ntries; ++i) {
                    auto tmp = static_cast<Int>(dist(eng));
                    if (lex_cast(tmp) != lex_cast(rational{tmp})) {
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
        using rational = rational<S::value>;
        REQUIRE((std::is_constructible<rational, bool>::value));
        REQUIRE((lex_cast(rational{false}) == "0"));
        REQUIRE((lex_cast(rational{true}) == "1"));
        REQUIRE((!std::is_constructible<rational, wchar_t>::value));
        REQUIRE((!std::is_constructible<rational, no_const>::value));
        std::cout << "n static limbs: " << S::value << ", size: " << sizeof(rational) << '\n';
        // Testing for the ctor from int_t.
        using integer = typename rational::int_t;
        REQUIRE((std::is_constructible<rational, integer>::value));
        REQUIRE((lex_cast(rational{integer{0}}) == "0"));
        REQUIRE((lex_cast(rational{integer{1}}) == "1"));
        REQUIRE((lex_cast(rational{integer{-12}}) == "-12"));
        REQUIRE((lex_cast(rational{integer{123}}) == "123"));
        REQUIRE((lex_cast(rational{integer{-123}}) == "-123"));
        // Testing for the ctor from num/den.
        REQUIRE((std::is_constructible<rational, integer, integer>::value));
        REQUIRE((std::is_constructible<rational, integer, int>::value));
        REQUIRE((std::is_constructible<rational, short, integer>::value));
        REQUIRE((!std::is_constructible<rational, short, wchar_t>::value));
        REQUIRE((!std::is_constructible<rational, std::string, integer>::value));
        REQUIRE((!std::is_constructible<rational, float, integer>::value));
        REQUIRE((!std::is_constructible<rational, integer, float>::value));
        auto q = rational{integer{0}, integer{5}};
        REQUIRE((lex_cast(q.get_num()) == "0"));
        REQUIRE((lex_cast(q.get_den()) == "1"));
        q = rational{char(0), -5};
        REQUIRE((lex_cast(q.get_num()) == "0"));
        REQUIRE((lex_cast(q.get_den()) == "1"));
        REQUIRE_THROWS_PREDICATE((rational{1, 0}), zero_division_error, [](const zero_division_error &ex) {
            return ex.what() == std::string("Cannot construct a rational with zero as denominator");
        });
        REQUIRE_THROWS_PREDICATE((rational{0, char(0)}), zero_division_error, [](const zero_division_error &ex) {
            return ex.what() == std::string("Cannot construct a rational with zero as denominator");
        });
        q = rational{-5, integer{25}};
        REQUIRE((lex_cast(q) == "-1/5"));
        q = rational{5ull, -25};
        REQUIRE((lex_cast(q) == "-1/5"));
        REQUIRE((lex_cast(q.get_num()) == "-1"));
        REQUIRE((lex_cast(q.get_den()) == "5"));
        // A couple of examples with GCD 1.
        q = rational{3, -7};
        REQUIRE((lex_cast(q) == "-3/7"));
        REQUIRE((lex_cast(q.get_num()) == "-3"));
        REQUIRE((lex_cast(q.get_den()) == "7"));
        q = rational{-9, 17};
        REQUIRE((lex_cast(q) == "-9/17"));
        REQUIRE((lex_cast(q.get_num()) == "-9"));
        REQUIRE((lex_cast(q.get_den()) == "17"));
    }
};

TEST_CASE("integral constructors")
{
    tuple_for_each(sizes{}, int_ctor_tester{});
}

using fp_types = std::tuple<float, double
#if defined(MPPP_WITH_MPFR)
                            ,
                            long double
#endif
                            >;

struct fp_ctor_tester {
    template <typename S>
    struct runner {
        template <typename Float>
        void operator()(const Float &) const
        {
            using rational = rational<S::value>;
            REQUIRE((std::is_constructible<rational, Float>::value));
            if (std::numeric_limits<Float>::is_iec559) {
                REQUIRE_THROWS_PREDICATE(
                    rational{std::numeric_limits<Float>::infinity()}, std::domain_error,
                    [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot construct a rational from the non-finite floating-point value "
                                      + std::to_string(std::numeric_limits<Float>::infinity());
                    });
                REQUIRE_THROWS_PREDICATE(
                    rational{-std::numeric_limits<Float>::infinity()}, std::domain_error,
                    [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot construct a rational from the non-finite floating-point value "
                                      + std::to_string(-std::numeric_limits<Float>::infinity());
                    });
                REQUIRE_THROWS_PREDICATE(
                    rational{std::numeric_limits<Float>::quiet_NaN()}, std::domain_error,
                    [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot construct a rational from the non-finite floating-point value "
                                      + std::to_string(std::numeric_limits<Float>::quiet_NaN());
                    });
            }
            REQUIRE(lex_cast(rational{Float(0)}) == "0");
            REQUIRE(static_cast<Float>(rational{Float(1.5)}) == Float(1.5));
            REQUIRE(static_cast<Float>(rational{Float(-1.5)}) == Float(-1.5));
            REQUIRE(static_cast<Float>(rational{Float(123.9)}) == Float(123.9));
            REQUIRE(static_cast<Float>(rational{Float(-123.9)}) == Float(-123.9));
            // Random testing.
            std::atomic<bool> fail(false);
            auto f = [&fail](unsigned n) {
                std::uniform_real_distribution<Float> dist(Float(-100), Float(100));
                std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
                for (auto i = 0; i < ntries; ++i) {
                    auto tmp = dist(eng);
                    if (static_cast<Float>(rational{tmp}) != tmp) {
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

struct string_ctor_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        REQUIRE((std::is_constructible<rational, const char *>::value));
        REQUIRE((std::is_constructible<rational, std::string>::value));
        REQUIRE((std::is_constructible<rational, char *>::value));
        auto q = rational{"0"};
        REQUIRE((lex_cast(q) == "0"));
        q = rational{std::string{"0"}};
        REQUIRE((lex_cast(q) == "0"));
        q = rational{std::string{"-123"}};
        REQUIRE((lex_cast(q) == "-123"));
        q = rational{std::string{"123"}, 16};
        REQUIRE((lex_cast(q) == "291"));
        q = rational{std::string{"-4/5"}};
        REQUIRE((lex_cast(q) == "-4/5"));
        q = rational{std::string{"4/-5"}};
        REQUIRE((lex_cast(q) == "-4/5"));
        q = rational{std::string{"4/-20"}};
        REQUIRE((lex_cast(q) == "-1/5"));
        q = rational{std::string{" 3 /  9 "}};
        REQUIRE((lex_cast(q) == "1/3"));
        // Try a different base.
        q = rational{std::string{" 10 /  -110 "}, 2};
        REQUIRE((lex_cast(q) == "-1/3"));
        q = rational{std::string{" -10 /  110 "}, 2};
        REQUIRE((lex_cast(q) == "-1/3"));
        REQUIRE_THROWS_PREDICATE(
            (q = rational{std::string{" -10 /  110 "}, 1}), std::invalid_argument, [](const std::invalid_argument &ia) {
                return std::string(ia.what())
                       == "In the constructor of integer from string, a base of 1"
                          " was specified, but the only valid values are 0 and any value in the [2,62] range";
            });
        REQUIRE_THROWS_PREDICATE(
            (q = rational{std::string{" -1 /0 "}}), zero_division_error, [](const zero_division_error &ia) {
                return std::string(ia.what())
                       == "A zero denominator was detected in the constructor of a rational from string";
            });
        REQUIRE_THROWS_PREDICATE(
            (q = rational{std::string{" -1 / "}, 0}), std::invalid_argument, [](const std::invalid_argument &ia) {
                return std::string(ia.what()) == "The string ' ' is not a valid integer in any supported base";
            });
        REQUIRE_THROWS_PREDICATE(
            (q = rational{std::string{" -1 /"}, 0}), std::invalid_argument, [](const std::invalid_argument &ia) {
                return std::string(ia.what()) == "The string '' is not a valid integer in any supported base";
            });
        REQUIRE_THROWS_PREDICATE((q = rational{std::string{" -1 /"}, 10}), std::invalid_argument,
                                 [](const std::invalid_argument &ia) {
                                     return std::string(ia.what()) == "The string '' is not a valid integer in base 10";
                                 });
        REQUIRE_THROWS_PREDICATE((q = rational{std::string{""}}), std::invalid_argument,
                                 [](const std::invalid_argument &ia) {
                                     return std::string(ia.what()) == "The string '' is not a valid integer in base 10";
                                 });
    }
};

TEST_CASE("string constructor")
{
    tuple_for_each(sizes{}, string_ctor_tester{});
}
