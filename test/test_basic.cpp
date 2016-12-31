/* Copyright 2009-2016 Francesco Biscani (bluescarni@gmail.com)

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

#include <mp++.hpp>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// TODO:
// - assignment ops, copy/move ctors.
// - string ctors / conversion.
// - neg.
// - utility funcs.
// - promote() in tests.
// - need comprehensive testing for all the special cases for limbs (1 vs 1, 1 vs 2, 2 vs 1, 2 vs 2),
//   with all the corner cases around upper limits for limbs.

static int ntries = 1000;

using namespace mppp;
using namespace mppp::mppp_impl;
using namespace mppp_test;

using int_types = std::tuple<char, signed char, unsigned char, short, unsigned short, int, unsigned, long,
                             unsigned long, long long, unsigned long long>;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

// A seed that will be used to init rngs in the multithreaded tests. Each time a batch of N threads
// finishes, this value gets bumped up by N, so that the next time a multithreaded test which uses rng
// is launched it will be inited with a different seed.
static std::mt19937::result_type mt_rng_seed(0u);

struct no_const {
};

struct int_ctor_tester {
    template <typename S>
    struct runner {
        template <typename Int>
        void operator()(const Int &) const
        {
            using integer = mp_integer<S::value>;
            REQUIRE((std::is_constructible<integer, Int>::value));
            REQUIRE(lex_cast(Int(0)) == lex_cast(integer{Int(0)}));
            auto constexpr min = std::numeric_limits<Int>::min(), max = std::numeric_limits<Int>::max();
            REQUIRE(lex_cast(min) == lex_cast(integer{min}));
            REQUIRE(lex_cast(max) == lex_cast(integer{max}));
            std::atomic<bool> fail(false);
            auto f = [&fail, min, max](unsigned n) {
                std::uniform_int_distribution<Int> dist(min, max);
                std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
                for (auto i = 0; i < ntries; ++i) {
                    auto tmp = dist(eng);
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
        }
    };
    template <typename S>
    inline void operator()(const S &) const
    {
        tuple_for_each(int_types{}, runner<S>{});
        // Some testing for bool.
        using integer = mp_integer<S::value>;
        REQUIRE((std::is_constructible<integer, bool>::value));
        REQUIRE((lex_cast(integer{false}) == "0"));
        REQUIRE((lex_cast(integer{true}) == "1"));
        REQUIRE((!std::is_constructible<integer, wchar_t>::value));
        REQUIRE((!std::is_constructible<integer, no_const>::value));
        std::cout << "n static limbs: " << S::value << ", size: " << sizeof(integer) << '\n';
    }
};

TEST_CASE("integral constructors")
{
    tuple_for_each(sizes{}, int_ctor_tester{});
}

using fp_types = std::tuple<float, double
#if defined(MPPP_WITH_LONG_DOUBLE)
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
            using integer = mp_integer<S::value>;
            REQUIRE((std::is_constructible<integer, Float>::value));
            if (std::numeric_limits<Float>::is_iec559) {
                REQUIRE_THROWS_PREDICATE(integer{std::numeric_limits<Float>::infinity()}, std::domain_error,
                                         [](const std::domain_error &ex) {
                                             return ex.what()
                                                    == "Cannot init integer from the non-finite floating-point value "
                                                           + std::to_string(std::numeric_limits<Float>::infinity());
                                         });
                REQUIRE_THROWS_PREDICATE(integer{-std::numeric_limits<Float>::infinity()}, std::domain_error,
                                         [](const std::domain_error &ex) {
                                             return ex.what()
                                                    == "Cannot init integer from the non-finite floating-point value "
                                                           + std::to_string(-std::numeric_limits<Float>::infinity());
                                         });
                REQUIRE_THROWS_PREDICATE(integer{std::numeric_limits<Float>::quiet_NaN()}, std::domain_error,
                                         [](const std::domain_error &ex) {
                                             return ex.what()
                                                    == "Cannot init integer from the non-finite floating-point value "
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
        using integer = mp_integer<S::value>;
        REQUIRE_THROWS_PREDICATE(integer{""}, std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '' is not a valid integer in base 10.";
        });
        REQUIRE_THROWS_PREDICATE((integer{"", 2}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '' is not a valid integer in base 2.";
        });
        REQUIRE_THROWS_PREDICATE((integer{"--31"}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '--31' is not a valid integer in base 10.";
        });
        REQUIRE_THROWS_PREDICATE((integer{"-+31"}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '-+31' is not a valid integer in base 10.";
        });
        REQUIRE_THROWS_PREDICATE((integer{"-31a"}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '-31a' is not a valid integer in base 10.";
        });
        REQUIRE_THROWS_PREDICATE((integer{"+a31"}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '+a31' is not a valid integer in base 10.";
        });
        REQUIRE_THROWS_PREDICATE((integer{"+a31"}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '+a31' is not a valid integer in base 10.";
        });
        REQUIRE_THROWS_PREDICATE((integer{"1E45", 12}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '1E45' is not a valid integer in base 12.";
        });
        REQUIRE(lex_cast(integer{"123"}) == "123");
        REQUIRE(lex_cast(integer{"-123"}) == "-123");
        REQUIRE(lex_cast(integer{"123"}) == "123");
        REQUIRE(lex_cast(integer{"0b11", 0}) == "3");
        REQUIRE(lex_cast(integer{"-0b11", 0}) == "-3");
        REQUIRE(lex_cast(integer{"110", 2}) == "6");
        REQUIRE(lex_cast(integer{"-110", 2}) == "-6");
        REQUIRE(lex_cast(integer{"1120211201", 3}) == "31231");
        REQUIRE(lex_cast(integer{"-1120211201", 3}) == "-31231");
    }
};

TEST_CASE("string constructor")
{
    tuple_for_each(sizes{}, string_ctor_tester{});
}

struct mpz_ctor_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = mp_integer<S::value>;
        mpz_raii m;
        REQUIRE(lex_cast(integer{&m.m_mpz}) == "0");
        ::mpz_set_si(&m.m_mpz, 1234);
        REQUIRE(lex_cast(integer{&m.m_mpz}) == "1234");
        ::mpz_set_si(&m.m_mpz, -1234);
        REQUIRE(lex_cast(integer{&m.m_mpz}) == "-1234");
        ::mpz_set_str(&m.m_mpz, "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        REQUIRE(lex_cast(integer{&m.m_mpz})
                == "3218372891372987328917389127389217398271983712987398127398172389712937819237");
        ::mpz_set_str(&m.m_mpz, "-3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        REQUIRE(lex_cast(integer{&m.m_mpz})
                == "-3218372891372987328917389127389217398271983712987398127398172389712937819237");
        // Random testing.
        std::atomic<bool> fail(false);
        auto f = [&fail](unsigned n) {
            std::uniform_int_distribution<long> dist(std::numeric_limits<long>::min(),
                                                     std::numeric_limits<long>::max());
            std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
            for (auto i = 0; i < ntries; ++i) {
                mpz_raii mpz;
                auto tmp = dist(eng);
                ::mpz_set_si(&mpz.m_mpz, tmp);
                if (lex_cast(integer{&mpz.m_mpz}) != lex_cast(tmp)) {
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

TEST_CASE("mpz_t constructor")
{
    tuple_for_each(sizes{}, mpz_ctor_tester{});
}

struct to_string_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = mp_integer<S::value>;
        REQUIRE(integer{}.to_string() == "0");
        REQUIRE(integer{1}.to_string() == "1");
        REQUIRE(integer{-1}.to_string() == "-1");
        REQUIRE(integer{123}.to_string() == "123");
        REQUIRE(integer{-123}.to_string() == "-123");
        REQUIRE(integer{123}.to_string(3) == "11120");
        REQUIRE(integer{-123}.to_string(3) == "-11120");
        REQUIRE_THROWS_PREDICATE((integer{}.to_string(1)), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "Invalid base for string conversion: the base must be between "
                                             "2 and 62, but a value of 1 was provided instead";
        });
        REQUIRE_THROWS_PREDICATE(
            (integer{}.to_string(-12)), std::invalid_argument, [](const std::invalid_argument &ia) {
                return std::string(ia.what()) == "Invalid base for string conversion: the base must be between "
                                                 "2 and 62, but a value of -12 was provided instead";
            });
        REQUIRE_THROWS_PREDICATE((integer{}.to_string(63)), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "Invalid base for string conversion: the base must be between "
                                             "2 and 62, but a value of 63 was provided instead";
        });
    }
};

TEST_CASE("to_string")
{
    tuple_for_each(sizes{}, to_string_tester{});
}

struct stream_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = mp_integer<S::value>;
        {
            std::ostringstream oss;
            oss << integer{};
            REQUIRE(oss.str() == "0");
        }
        {
            std::ostringstream oss;
            oss << integer{123};
            REQUIRE(oss.str() == "123");
        }
        {
            std::ostringstream oss;
            oss << integer{-123};
            REQUIRE(oss.str() == "-123");
        }
    }
};

TEST_CASE("stream")
{
    tuple_for_each(sizes{}, stream_tester{});
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
            using integer = mp_integer<S::value>;
            REQUIRE((is_convertible<integer, Int>::value));
            REQUIRE(roundtrip_conversion<integer>(0));
            auto constexpr min = std::numeric_limits<Int>::min(), max = std::numeric_limits<Int>::max();
            REQUIRE(roundtrip_conversion<integer>(min));
            REQUIRE(roundtrip_conversion<integer>(max));
            REQUIRE(roundtrip_conversion<integer>(min + Int(1)));
            REQUIRE(roundtrip_conversion<integer>(max - Int(1)));
            REQUIRE(roundtrip_conversion<integer>(min + Int(2)));
            REQUIRE(roundtrip_conversion<integer>(max - Int(2)));
            REQUIRE(roundtrip_conversion<integer>(min + Int(3)));
            REQUIRE(roundtrip_conversion<integer>(max - Int(3)));
            REQUIRE(roundtrip_conversion<integer>(min + Int(42)));
            REQUIRE(roundtrip_conversion<integer>(max - Int(42)));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(min) - 1), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(min) - 2), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(min) - 3), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(min) - 123), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(max) + 1), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(max) + 2), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(max) + 3), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(max) + 123), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            // Try with large integers that should trigger a specific error, at least on some platforms.
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(max) * max * max * max * max), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            if (min != Int(0)) {
                REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(min) * min * min * min * min), std::overflow_error,
                                         [](const std::overflow_error &) { return true; });
            }
            std::atomic<bool> fail(false);
            auto f = [&fail, min, max](unsigned n) {
                std::uniform_int_distribution<Int> dist(min, max);
                std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
                for (auto i = 0; i < ntries; ++i) {
                    if (!roundtrip_conversion<integer>(dist(eng))) {
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
        tuple_for_each(int_types{}, runner<S>{});
        // Some testing for bool.
        using integer = mp_integer<S::value>;
        REQUIRE((is_convertible<integer, bool>::value));
        REQUIRE(roundtrip_conversion<integer>(true));
        REQUIRE(roundtrip_conversion<integer>(false));
        // Extra.
        REQUIRE((!is_convertible<integer, wchar_t>::value));
        REQUIRE((!is_convertible<integer, no_conv>::value));
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
            using integer = mp_integer<S::value>;
            REQUIRE((is_convertible<integer, Float>::value));
            REQUIRE(static_cast<Float>(integer{0}) == Float(0));
            REQUIRE(static_cast<Float>(integer{1}) == Float(1));
            REQUIRE(static_cast<Float>(integer{-1}) == Float(-1));
            REQUIRE(static_cast<Float>(integer{12}) == Float(12));
            REQUIRE(static_cast<Float>(integer{-12}) == Float(-12));
            if (std::numeric_limits<Float>::is_iec559) {
                // Try with large numbers.
                REQUIRE(std::abs(static_cast<Float>(integer{"1000000000000000000000000000000"}) - Float(1E30))
                            / Float(1E30)
                        <= std::numeric_limits<Float>::epsilon() * 1000.);
                REQUIRE(std::abs(static_cast<Float>(integer{"-1000000000000000000000000000000"}) + Float(1E30))
                            / Float(1E30)
                        <= std::numeric_limits<Float>::epsilon() * 1000.);
                REQUIRE(static_cast<Float>(integer{std::numeric_limits<Float>::max()})
                        == std::numeric_limits<Float>::max());
                REQUIRE(static_cast<Float>(integer{-std::numeric_limits<Float>::max()})
                        == -std::numeric_limits<Float>::max());
            }
            // Random testing.
            std::atomic<bool> fail(false);
            auto f = [&fail](unsigned n) {
                std::uniform_real_distribution<Float> dist(Float(-100), Float(100));
                std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
                for (auto i = 0; i < ntries; ++i) {
                    const auto tmp = dist(eng);
                    if (static_cast<Float>(integer{tmp}) != std::trunc(tmp)) {
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

TEST_CASE("floating-point conversions")
{
    tuple_for_each(sizes{}, fp_convert_tester{});
}
