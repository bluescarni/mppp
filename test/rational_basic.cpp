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
#include <gmp.h>
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
            REQUIRE((std::is_constructible<rational, Int &&>::value));
            REQUIRE((std::is_constructible<rational, const Int &>::value));
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
        using rational = rational<S::value>;
        // Def ctor.
        REQUIRE((lex_cast(rational{}) == "0"));
        // Some testing for bool.
        REQUIRE((std::is_constructible<rational, bool>::value));
        REQUIRE((std::is_constructible<rational, bool &>::value));
        REQUIRE((std::is_constructible<rational, const bool &>::value));
        REQUIRE((std::is_constructible<rational, bool &&>::value));
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
        char c0 = 0;
        int m5 = -5;
        q = rational{c0, m5};
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
            REQUIRE((std::is_constructible<rational, Float &>::value));
            REQUIRE((std::is_constructible<rational, Float &&>::value));
            REQUIRE((std::is_constructible<rational, const Float &>::value));
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
        REQUIRE((std::is_constructible<rational, std::string &&>::value));
        REQUIRE((std::is_constructible<rational, const std::string &>::value));
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

struct mpq_ctor_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        mpq_raii m;
        REQUIRE((std::is_constructible<rational, const ::mpq_t>::value));
        REQUIRE(lex_cast(rational{&m.m_mpq}) == "0");
        ::mpz_set_si(mpq_numref(&m.m_mpq), 1234);
        REQUIRE(lex_cast(rational{&m.m_mpq}) == "1234");
        ::mpz_set_si(mpq_numref(&m.m_mpq), -1234);
        REQUIRE(lex_cast(rational{&m.m_mpq}) == "-1234");
        ::mpz_set_si(mpq_numref(&m.m_mpq), 4);
        ::mpz_set_si(mpq_denref(&m.m_mpq), -3);
        REQUIRE(lex_cast(rational{&m.m_mpq}) == "4/-3");
        ::mpz_set_str(mpq_numref(&m.m_mpq),
                      "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        REQUIRE(lex_cast(rational{&m.m_mpq})
                == "3218372891372987328917389127389217398271983712987398127398172389712937819237/-3");
        ::mpz_set_str(mpq_denref(&m.m_mpq),
                      "-3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        REQUIRE(lex_cast(rational{&m.m_mpq})
                == "3218372891372987328917389127389217398271983712987398127398172389712937819237/"
                   "-3218372891372987328917389127389217398271983712987398127398172389712937819237");
    }
};

TEST_CASE("mpq_t constructor")
{
    tuple_for_each(sizes{}, mpq_ctor_tester{});
}

struct mpz_ctor_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        using integer = integer<S::value>;
        mpz_raii m;
        REQUIRE((std::is_constructible<rational, const ::mpz_t>::value));
        REQUIRE(rational{&m.m_mpz}.is_zero());
        REQUIRE(rational{&m.m_mpz}.get_num().is_static());
        REQUIRE(rational{&m.m_mpz}.get_den().is_one());
        REQUIRE(rational{&m.m_mpz}.get_den().is_static());
        ::mpz_set_si(&m.m_mpz, 1234);
        REQUIRE(rational{&m.m_mpz}.get_num() == 1234);
        REQUIRE(rational{&m.m_mpz}.get_num().is_static());
        REQUIRE(rational{&m.m_mpz}.get_den().is_one());
        REQUIRE(rational{&m.m_mpz}.get_den().is_static());
        ::mpz_set_si(&m.m_mpz, -1234);
        REQUIRE(rational{&m.m_mpz}.get_num() == -1234);
        REQUIRE(rational{&m.m_mpz}.get_num().is_static());
        REQUIRE(rational{&m.m_mpz}.get_den().is_one());
        REQUIRE(rational{&m.m_mpz}.get_den().is_static());
        ::mpz_set_str(&m.m_mpz, "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        REQUIRE(rational{&m.m_mpz}.get_num()
                == integer{"3218372891372987328917389127389217398271983712987398127398172389712937819237"});
        REQUIRE(rational{&m.m_mpz}.get_den().is_one());
        REQUIRE(rational{&m.m_mpz}.get_den().is_static());
        ::mpz_set_str(&m.m_mpz, "-3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        REQUIRE(rational{&m.m_mpz}.get_num()
                == -integer{"3218372891372987328917389127389217398271983712987398127398172389712937819237"});
        REQUIRE(rational{&m.m_mpz}.get_den().is_one());
        REQUIRE(rational{&m.m_mpz}.get_den().is_static());
    }
};

TEST_CASE("mpz_t constructor")
{
    tuple_for_each(sizes{}, mpz_ctor_tester{});
}

struct copy_move_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        using integer = typename rational::int_t;
        REQUIRE((!std::is_assignable<rational &, const wchar_t &>::value));
        REQUIRE((!std::is_assignable<rational &, const std::vector<int> &>::value));
        REQUIRE((!std::is_assignable<const rational &, int>::value));
        rational q;
        q = 123;
        REQUIRE(lex_cast(q) == "123");
        q = -123ll;
        REQUIRE(lex_cast(q) == "-123");
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        rational q2{q};
        REQUIRE(lex_cast(q2) == "-123");
        REQUIRE(q2.get_num().is_static());
        REQUIRE(q2.get_den().is_static());
        q2._get_den().promote();
        rational q3{q2};
        REQUIRE(lex_cast(q3) == "-123");
        REQUIRE(q3.get_num().is_static());
        REQUIRE(q3.get_den().is_dynamic());
        q3 = q;
        REQUIRE(lex_cast(q3) == "-123");
        REQUIRE(q3.get_num().is_static());
        REQUIRE(q3.get_den().is_static());
        rational q4{std::move(q2)};
        REQUIRE(q2.get_num().is_zero());
        REQUIRE(q2.get_den().is_one());
        REQUIRE(q2.get_num().is_static());
        REQUIRE(q2.get_den().is_static());
        REQUIRE(lex_cast(q4) == "-123");
        REQUIRE(q4.get_num().is_static());
        REQUIRE(q4.get_den().is_dynamic());
        // Revive q2.
        q2 = q;
        REQUIRE(lex_cast(q2) == "-123");
        REQUIRE(q2.get_num().is_static());
        REQUIRE(q2.get_den().is_static());
        q2 = std::move(q4);
        REQUIRE(q4.get_num().is_zero());
        REQUIRE(q4.get_den().is_one());
        REQUIRE(q4.get_num().is_static());
        REQUIRE(q4.get_den().is_static());
        REQUIRE(lex_cast(q2) == "-123");
        REQUIRE(q2.get_num().is_static());
        REQUIRE(q2.get_den().is_dynamic());
        // Self assignments.
        q2 = q2;
        REQUIRE(lex_cast(q2) == "-123");
        REQUIRE(q2.get_num().is_static());
        REQUIRE(q2.get_den().is_dynamic());
        q2 = std::move(q2);
        REQUIRE(lex_cast(q2) == "-123");
        REQUIRE(q2.get_num().is_static());
        REQUIRE(q2.get_den().is_dynamic());
        q = 1.23;
        REQUIRE(lex_cast(q.get_num()) == lex_cast(rational(1.23).get_num()));
        REQUIRE(lex_cast(q.get_den()) == lex_cast(rational(1.23).get_den()));
        q = integer{-12};
        REQUIRE(lex_cast(q) == "-12");
        q = rational{3, -12};
        REQUIRE(lex_cast(q) == "-1/4");
        // Check that move operations reset to zero the right operand.
        q = "4/5";
        auto qa(std::move(q));
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q._get_num().promote();
        auto qb(std::move(q));
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q._get_den().promote();
        auto qc(std::move(q));
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q._get_num().promote();
        q._get_den().promote();
        auto qd(std::move(q));
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q2 = std::move(q);
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q._get_num().promote();
        q2 = std::move(q);
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q._get_den().promote();
        q2 = std::move(q);
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q._get_num().promote();
        q._get_den().promote();
        q2 = std::move(q);
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q2._get_num().promote();
        q2 = std::move(q);
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q2._get_den().promote();
        q2 = std::move(q);
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q2._get_num().promote();
        q2._get_den().promote();
        q2 = std::move(q);
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q._get_num().promote();
        q._get_den().promote();
        q2._get_num().promote();
        q2._get_den().promote();
        q2 = std::move(q);
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q._get_den().promote();
        q2._get_num().promote();
        q2._get_den().promote();
        q2 = std::move(q);
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q._get_num().promote();
        q2._get_num().promote();
        q2._get_den().promote();
        q2 = std::move(q);
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q._get_num().promote();
        q._get_den().promote();
        q2._get_den().promote();
        q2 = std::move(q);
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q._get_num().promote();
        q._get_den().promote();
        q2._get_num().promote();
        q2 = std::move(q);
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q._get_num().promote();
        q2._get_num().promote();
        q2 = std::move(q);
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q._get_den().promote();
        q2._get_num().promote();
        q2 = std::move(q);
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q._get_num().promote();
        q2._get_den().promote();
        q2 = std::move(q);
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q._get_den().promote();
        q2._get_den().promote();
        q2 = std::move(q);
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
    }
};

TEST_CASE("copy and move")
{
    tuple_for_each(sizes{}, copy_move_tester{});
}

struct string_ass_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        rational q;
        REQUIRE((std::is_assignable<rational &, std::string>::value));
        REQUIRE((std::is_assignable<rational &, std::string &&>::value));
        REQUIRE((std::is_assignable<rational &, const std::string &>::value));
        REQUIRE((!std::is_assignable<const rational &, const std::string &>::value));
        q = "1";
        REQUIRE(lex_cast(q) == "1");
        q = "-23";
        REQUIRE(lex_cast(q) == "-23");
        q = std::string("-2/-4");
        REQUIRE(lex_cast(q) == "1/2");
        q = "3/-9";
        REQUIRE(lex_cast(q) == "-1/3");
        REQUIRE_THROWS_PREDICATE(q = "", std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '' is not a valid integer in base 10";
        });
        REQUIRE_THROWS_PREDICATE(q = std::string("-3/0"), zero_division_error, [](const zero_division_error &ia) {
            return std::string(ia.what())
                   == "A zero denominator was detected in the constructor of a rational from string";
        });
    }
};

TEST_CASE("string ass")
{
    tuple_for_each(sizes{}, string_ass_tester{});
}

struct mpq_ass_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        REQUIRE((std::is_assignable<rational &, ::mpq_t>::value));
        REQUIRE((!std::is_assignable<const rational &, ::mpq_t>::value));
        rational q;
        mpq_raii m;
        REQUIRE(lex_cast(rational{&m.m_mpq}) == "0");
        ::mpq_set_si(&m.m_mpq, 1234, 1);
        q = &m.m_mpq;
        REQUIRE(lex_cast(q) == "1234");
        ::mpq_set_si(&m.m_mpq, -1234, 1);
        q = &m.m_mpq;
        REQUIRE(lex_cast(q) == "-1234");
        ::mpq_set_str(&m.m_mpq, "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        q = &m.m_mpq;
        REQUIRE(lex_cast(q) == "3218372891372987328917389127389217398271983712987398127398172389712937819237");
        ::mpq_set_str(&m.m_mpq, "-3218372891372987328917389127389217398271983712987398127398172389712937819237/2", 10);
        q = &m.m_mpq;
        REQUIRE(lex_cast(q) == "-3218372891372987328917389127389217398271983712987398127398172389712937819237/2");
    }
};

TEST_CASE("mpq_t assignment")
{
    tuple_for_each(sizes{}, mpq_ass_tester{});
}

struct mpz_ass_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        using integer = integer<S::value>;
        REQUIRE((std::is_assignable<rational &, ::mpz_t>::value));
        REQUIRE((!std::is_assignable<const rational &, ::mpz_t>::value));
        rational q{6, 5};
        mpz_raii m;
        ::mpz_set_si(&m.m_mpz, 1234);
        q = &m.m_mpz;
        REQUIRE(q.get_num() == 1234);
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den() == 1);
        REQUIRE(q.get_den().is_static());
        q = "-7/3";
        ::mpz_set_si(&m.m_mpz, -1234);
        q = &m.m_mpz;
        REQUIRE(q.get_num() == -1234);
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den() == 1);
        REQUIRE(q.get_den().is_static());
        q = "3218372891372987328917389127389217398271983712987398127398172389712937819237/"
            "1232137219837921379128378921738971982713918723";
        q = &m.m_mpz;
        REQUIRE(q.get_num() == -1234);
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den() == 1);
        REQUIRE(q.get_den().is_static());
        q = "-3218372891372987328917389127389217398271983712987398127398172389712937819237/"
            "1232137219837921379128378921738971982713918723";
        q = &m.m_mpz;
        REQUIRE(q.get_num() == -1234);
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den() == 1);
        REQUIRE(q.get_den().is_static());
        ::mpz_set_str(&m.m_mpz, "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        q = &m.m_mpz;
        REQUIRE(q.get_num() == integer{"3218372891372987328917389127389217398271983712987398127398172389712937819237"});
        REQUIRE(q.get_den() == 1);
        REQUIRE(q.get_den().is_static());
        ::mpz_set_str(&m.m_mpz, "-3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        q = &m.m_mpz;
        REQUIRE(q.get_num()
                == -integer{"3218372891372987328917389127389217398271983712987398127398172389712937819237"});
        REQUIRE(q.get_den() == 1);
        REQUIRE(q.get_den().is_static());
    }
};

TEST_CASE("mpz_t assignment")
{
    tuple_for_each(sizes{}, mpz_ass_tester{});
}

struct gen_ass_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        using integer = typename rational::int_t;
        rational q;
        q = 12;
        REQUIRE(lex_cast(q) == "12");
        q = (signed char)-11;
        REQUIRE(lex_cast(q) == "-11");
        q = integer{"-2323232312312311"};
        REQUIRE(lex_cast(q) == "-2323232312312311");
        if (std::numeric_limits<double>::radix == 2) {
            q = -1.5;
            REQUIRE(lex_cast(q) == "-3/2");
        }
#if defined(MPPP_WITH_MPFR)
        if (std::numeric_limits<long double>::radix == 2) {
            q = -4.5l;
            REQUIRE(lex_cast(q) == "-9/2");
        }
#endif
    }
};

TEST_CASE("generic assignment")
{
    tuple_for_each(sizes{}, gen_ass_tester{});
}

struct yes {
};

struct no {
};

template <typename From, typename To>
static inline auto test_static_cast(int) -> decltype(void(static_cast<To>(std::declval<const From &>())), yes{});

template <typename From, typename To>
static inline no test_static_cast(...);

template <typename From, typename To>
using is_convertible = std::integral_constant<bool, std::is_same<decltype(test_static_cast<From, To>(0)), yes>::value>;

template <typename Integer, typename T>
static inline bool roundtrip_conversion(const T &x)
{
    Integer tmp{x};
    return (static_cast<T>(tmp) == x) && (lex_cast(x) == lex_cast(tmp));
}

struct no_conv {
};

struct int_convert_tester {
    template <typename S>
    struct runner {
        template <typename Int>
        void operator()(const Int &) const
        {
            using rational = rational<S::value>;
            using integer = typename rational::int_t;
            REQUIRE((is_convertible<rational, Int>::value));
            REQUIRE(roundtrip_conversion<rational>(0));
            auto constexpr min = std::numeric_limits<Int>::min(), max = std::numeric_limits<Int>::max();
            REQUIRE(roundtrip_conversion<rational>(min));
            REQUIRE(roundtrip_conversion<rational>(max));
            REQUIRE(roundtrip_conversion<rational>(min + Int(1)));
            REQUIRE(roundtrip_conversion<rational>(max - Int(1)));
            REQUIRE(roundtrip_conversion<rational>(min + Int(2)));
            REQUIRE(roundtrip_conversion<rational>(max - Int(2)));
            REQUIRE(roundtrip_conversion<rational>(min + Int(3)));
            REQUIRE(roundtrip_conversion<rational>(max - Int(3)));
            REQUIRE(roundtrip_conversion<rational>(min + Int(42)));
            REQUIRE(roundtrip_conversion<rational>(max - Int(42)));
            if (min != Int(0)) {
                REQUIRE(static_cast<Int>(rational{integer(min) * 3, integer(min) * -2}) == Int(-1));
            }
            REQUIRE(static_cast<Int>(rational{integer(max) * 5, integer(max) * 2}) == Int(2));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(integer(min) * 2, 2) - 1), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(min) - 1), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(min) - 2), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(min) - 3), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(min) - 123), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(max) + 1), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(max) + 2), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(max) + 3), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(max) + 123), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
        }
    };
    template <typename S>
    inline void operator()(const S &) const
    {
        tuple_for_each(int_types{}, runner<S>{});
        // Some testing for bool.
        using rational = rational<S::value>;
        using integer = typename rational::int_t;
        REQUIRE((is_convertible<rational, bool>::value));
        REQUIRE(roundtrip_conversion<rational>(true));
        REQUIRE(roundtrip_conversion<rational>(false));
        // Extra.
        REQUIRE((!is_convertible<rational, wchar_t>::value));
        REQUIRE((!is_convertible<rational, no_conv>::value));
        // Conversion to int_t.
        REQUIRE((is_convertible<rational, integer>::value));
        REQUIRE(roundtrip_conversion<rational>(integer{42}));
        REQUIRE(roundtrip_conversion<rational>(integer{-42}));
        REQUIRE(static_cast<integer>(rational{1, 2}) == 0);
        REQUIRE(static_cast<integer>(rational{3, 2}) == 1);
        REQUIRE(static_cast<integer>(rational{3, -2}) == -1);
    }
};

TEST_CASE("integral conversions")
{
    tuple_for_each(sizes{}, int_convert_tester{});
}

struct fp_convert_tester {
    template <typename S>
    struct runner {
        template <typename Float>
        void operator()(const Float &) const
        {
            using rational = rational<S::value>;
            REQUIRE((is_convertible<rational, Float>::value));
            REQUIRE(static_cast<Float>(rational{0}) == Float(0));
            REQUIRE(static_cast<Float>(rational{1}) == Float(1));
            REQUIRE(static_cast<Float>(rational{-1}) == Float(-1));
            REQUIRE(static_cast<Float>(rational{12}) == Float(12));
            REQUIRE(static_cast<Float>(rational{-12}) == Float(-12));
            if (std::numeric_limits<Float>::is_iec559) {
                REQUIRE(static_cast<Float>(rational{1, 2}) == Float(.5));
                REQUIRE(static_cast<Float>(rational{3, -2}) == Float(-1.5));
                REQUIRE(static_cast<Float>(rational{7, 2}) == Float(3.5));
            }
            // Random testing.
            std::atomic<bool> fail(false);
            auto f = [&fail](unsigned n) {
                {
                    std::uniform_real_distribution<Float> dist(Float(-1E9), Float(1E9));
                    std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
                    for (auto i = 0; i < ntries; ++i) {
                        const auto tmp = dist(eng);
                        if (!roundtrip_conversion<rational>(tmp)) {
                            fail.store(false);
                        }
                    }
                }
                {
                    std::uniform_real_distribution<Float> dist(Float(-1E-9), Float(1E-9));
                    std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
                    for (auto i = 0; i < ntries; ++i) {
                        const auto tmp = dist(eng);
                        if (!roundtrip_conversion<rational>(tmp)) {
                            fail.store(false);
                        }
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

TEST_CASE("floating-point conversions")
{
    tuple_for_each(sizes{}, fp_convert_tester{});
}

struct is_canonical_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using rational = rational<S::value>;
        rational q;
        REQUIRE(q.is_canonical());
        q._get_den() = -1;
        REQUIRE(!q.is_canonical());
        q = "5/10";
        REQUIRE(q.is_canonical());
        q._get_den() = -10;
        REQUIRE(!q.is_canonical());
        q = 5;
        REQUIRE(q.is_canonical());
        q._get_den() = 0;
        REQUIRE(!q.is_canonical());
    }
};

TEST_CASE("is_canonical")
{
    tuple_for_each(sizes{}, is_canonical_tester{});
}

struct canonicalise_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using rational = rational<S::value>;
        rational q;
        q.canonicalise().canonicalise();
        REQUIRE(q.get_num() == 0);
        REQUIRE(q.get_den() == 1);
        q._get_num() = 3;
        q._get_den() = -6;
        canonicalise(q);
        REQUIRE(q.get_num() == -1);
        REQUIRE(q.get_den() == 2);
        q._get_num() = 0;
        q._get_den() = -6;
        canonicalise(q);
        REQUIRE(q.get_num() == 0);
        REQUIRE(q.get_den() == 1);
        q._get_num() = 3;
        q._get_den() = -7;
        canonicalise(q);
        REQUIRE(q.get_num() == -3);
        REQUIRE(q.get_den() == 7);
    }
};

TEST_CASE("canonicalise")
{
    tuple_for_each(sizes{}, canonicalise_tester{});
}

struct stream_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        {
            std::ostringstream oss;
            oss << rational{};
            REQUIRE(oss.str() == "0");
        }
        {
            std::ostringstream oss;
            oss << rational{123};
            REQUIRE(oss.str() == "123");
        }
        {
            std::ostringstream oss;
            oss << rational{-123};
            REQUIRE(oss.str() == "-123");
        }
        {
            std::ostringstream oss;
            oss << rational{6, -12};
            REQUIRE(oss.str() == "-1/2");
        }
        {
            std::ostringstream oss;
            oss << rational{12, 6};
            REQUIRE(oss.str() == "2");
        }
        {
            std::stringstream ss;
            ss << rational{};
            rational q(12);
            ss >> q;
            REQUIRE(lex_cast(q) == "0");
        }
        {
            std::stringstream ss;
            ss << rational{-123};
            rational q;
            ss >> q;
            REQUIRE(lex_cast(q) == "-123");
        }
        {
            std::stringstream ss;
            ss << rational{-12, 6};
            rational q;
            ss >> q;
            REQUIRE(lex_cast(q) == "-2");
        }
        {
            std::stringstream ss;
            ss << rational{6, 12};
            rational q;
            ss >> q;
            REQUIRE(lex_cast(q) == "1/2");
        }
        {
            std::stringstream ss;
            ss.str("-42");
            rational q;
            ss >> q;
            REQUIRE(lex_cast(q) == "-42");
        }
    }
};

TEST_CASE("stream")
{
    tuple_for_each(sizes{}, stream_tester{});
}
