// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <atomic>
#include <cmath>
#include <cstddef>
#include <gmp.h>
#include <iostream>
#include <limits>
#include <mp++/detail/type_traits.hpp>
#include <mp++/integer.hpp>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#if MPPP_CPLUSPLUS >= 201703L
#include <string_view>
#endif
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

static int ntries = 1000;

using namespace mppp;
using namespace mppp_test;

using int_types = std::tuple<char, signed char, unsigned char, short, unsigned short, int, unsigned, long,
                             unsigned long, long long, unsigned long long
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

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

// A seed that will be used to init rngs in the multithreaded tests. Each time a batch of N threads
// finishes, this value gets bumped up by N, so that the next time a multithreaded test which uses rng
// is launched it will be inited with a different seed.
static std::mt19937::result_type mt_rng_seed(0u);

static std::mt19937 rng;

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
            auto constexpr min = nl_min<Int>(), max = nl_max<Int>();
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
        REQUIRE((!std::is_constructible<integer, wchar_t>::value));
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
            auto constexpr min = nl_min<Int>(), max = nl_max<Int>();
            n0 = min;
            REQUIRE(n0 == min);
            n0 = max;
            REQUIRE(n0 == max);
            std::atomic<bool> fail(false);
            auto f = [&fail](unsigned n) {
                integral_minmax_dist<Int> dist;
                std::uniform_int_distribution<int> sdist(0, 1);
                std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
                for (auto i = 0; i < ntries; ++i) {
                    integer n1;
                    if (sdist(rng)) {
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
        REQUIRE((!std::is_assignable<integer &, wchar_t>::value));
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

struct fp_ass_tester {
    template <typename S>
    struct runner {
        template <typename Float>
        void operator()(const Float &) const
        {
            using integer = integer<S::value>;
            REQUIRE((std::is_assignable<integer &, Float>::value));
            integer n0;
            if (std::numeric_limits<Float>::is_iec559) {
                REQUIRE_THROWS_PREDICATE(
                    n0 = std::numeric_limits<Float>::infinity(), std::domain_error, [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot assign the non-finite floating-point value "
                                      + std::to_string(std::numeric_limits<Float>::infinity()) + " to an integer";
                    });
                REQUIRE_THROWS_PREDICATE(
                    n0 = -std::numeric_limits<Float>::infinity(), std::domain_error, [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot assign the non-finite floating-point value "
                                      + std::to_string(-std::numeric_limits<Float>::infinity()) + " to an integer";
                    });
                REQUIRE_THROWS_PREDICATE(
                    n0 = std::numeric_limits<Float>::quiet_NaN(), std::domain_error, [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot assign the non-finite floating-point value "
                                      + std::to_string(std::numeric_limits<Float>::quiet_NaN()) + " to an integer";
                    });
            }
            n0 = Float(0);
            REQUIRE(n0 == 0);
            REQUIRE(n0.is_static());
            n0.promote();
            n0 = Float(1.5);
            REQUIRE(n0 == 1);
            REQUIRE(n0.is_static());
            n0 = Float(-1.5);
            REQUIRE(n0 == -1);
            n0 = Float(123.9);
            REQUIRE(n0 == 123);
            n0 = Float(-123.9);
            REQUIRE(n0 == -123);
            // Random testing.
            std::atomic<bool> fail(false);
            auto f = [&fail](unsigned n) {
                std::uniform_real_distribution<Float> dist(Float(-100), Float(100));
                std::uniform_int_distribution<int> sdist(0, 1);
                std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
                for (auto i = 0; i < ntries; ++i) {
                    integer n1;
                    if (sdist(rng)) {
                        n1.promote();
                    }
                    auto tmp = dist(eng);
                    n1 = tmp;
                    if (std::trunc(tmp) != n1) {
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

TEST_CASE("floating-point assignment")
{
    tuple_for_each(sizes{}, fp_ass_tester{});
}

struct string_ctor_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        REQUIRE_THROWS_PREDICATE((integer{"", 1}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what())
                   == "In the constructor of integer from string, a base of 1"
                      " was specified, but the only valid values are 0 and any value in the [2,62] range";
        });
        REQUIRE_THROWS_PREDICATE((integer{"", -10}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what())
                   == "In the constructor of integer from string, a base of -10"
                      " was specified, but the only valid values are 0 and any value in the [2,62] range";
        });
        REQUIRE_THROWS_PREDICATE((integer{"", 63}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what())
                   == "In the constructor of integer from string, a base of 63"
                      " was specified, but the only valid values are 0 and any value in the [2,62] range";
        });
        REQUIRE_THROWS_PREDICATE((integer{"00x00abba", 0}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '00x00abba' is not a valid integer in any supported base";
        });
        REQUIRE_THROWS_PREDICATE(integer{""}, std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '' is not a valid integer in base 10";
        });
        REQUIRE_THROWS_PREDICATE((integer{"", 2}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '' is not a valid integer in base 2";
        });
        REQUIRE_THROWS_PREDICATE((integer{"--31"}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '--31' is not a valid integer in base 10";
        });
        REQUIRE_THROWS_PREDICATE((integer{"-+31"}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '-+31' is not a valid integer in base 10";
        });
        REQUIRE_THROWS_PREDICATE((integer{"-31a"}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '-31a' is not a valid integer in base 10";
        });
        REQUIRE_THROWS_PREDICATE((integer{"+a31"}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '+a31' is not a valid integer in base 10";
        });
        REQUIRE_THROWS_PREDICATE((integer{"+a31"}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '+a31' is not a valid integer in base 10";
        });
        REQUIRE_THROWS_PREDICATE((integer{"1E45", 12}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '1E45' is not a valid integer in base 12";
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
        REQUIRE(lex_cast(integer{"0x7b", 0}) == "123");
        REQUIRE(lex_cast(integer{"-0x3039", 0}) == "-12345");
        REQUIRE(lex_cast(integer{"-0225377", 0}) == "-76543");
        REQUIRE(lex_cast(integer{"512", 0}) == "512");
        // Constructor from range of chars.
        std::string s = "-1234";
        REQUIRE((integer{s.data(), s.data() + 5} == -1234));
        REQUIRE((integer{s.data(), s.data() + 4} == -123));
        s = "0x7b";
        REQUIRE((integer{s.data(), s.data() + 4, 0} == 123));
        s = "1E45";
        REQUIRE_THROWS_PREDICATE(
            (integer{s.data(), s.data() + 4, 12}), std::invalid_argument, [](const std::invalid_argument &ia) {
                return std::string(ia.what()) == "The string '1E45' is not a valid integer in base 12";
            });
        // Try with an already terminated string.
        const char *cs = "-1234\0";
        REQUIRE((integer{cs, cs + 5} == -1234));
        REQUIRE((integer{cs, cs + 4} == -123));
#if MPPP_CPLUSPLUS >= 201703L
        std::string_view sv = "-1234";
        REQUIRE((integer{sv} == -1234));
        REQUIRE((integer{std::string_view{sv.data(), 4u}} == -123));
        sv = "0x7b";
        REQUIRE((integer{sv, 0} == 123));
        sv = "1E45";
        REQUIRE_THROWS_PREDICATE((integer{sv, 12}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '1E45' is not a valid integer in base 12";
        });
        REQUIRE((integer{std::string_view{cs, 5}} == -1234));
        REQUIRE((integer{std::string_view{cs, 4}} == -123));
#endif
    }
};

TEST_CASE("string constructor")
{
    tuple_for_each(sizes{}, string_ctor_tester{});
}

struct mpz_copy_ctor_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
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
            std::uniform_int_distribution<long> dist(nl_min<long>(), nl_max<long>());
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

TEST_CASE("mpz_t copy constructor")
{
    tuple_for_each(sizes{}, mpz_copy_ctor_tester{});
}

#if !defined(_MSC_VER)

struct mpz_move_ctor_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        ::mpz_t m0;
        ::mpz_init(m0);
        REQUIRE(lex_cast(integer{std::move(m0)}) == "0");
        ::mpz_init(m0);
        ::mpz_set_si(m0, 1234);
        REQUIRE(lex_cast(integer{std::move(m0)}) == "1234");
        ::mpz_init(m0);
        ::mpz_set_si(m0, -1234);
        REQUIRE(lex_cast(integer{std::move(m0)}) == "-1234");
        ::mpz_init(m0);
        ::mpz_set_str(m0, "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        REQUIRE(lex_cast(integer{std::move(m0)})
                == "3218372891372987328917389127389217398271983712987398127398172389712937819237");
        ::mpz_init(m0);
        ::mpz_set_str(m0, "-3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        REQUIRE(lex_cast(integer{std::move(m0)})
                == "-3218372891372987328917389127389217398271983712987398127398172389712937819237");
        // Random testing.
        std::atomic<bool> fail(false);
        auto f = [&fail](unsigned n) {
            std::uniform_int_distribution<long> dist(nl_min<long>(), nl_max<long>());
            std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
            for (auto i = 0; i < ntries; ++i) {
                ::mpz_t m1;
                ::mpz_init(m1);
                auto tmp = dist(eng);
                ::mpz_set_si(m1, tmp);
                if (lex_cast(integer{std::move(m1)}) != lex_cast(tmp)) {
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

TEST_CASE("mpz_t move constructor")
{
    tuple_for_each(sizes{}, mpz_move_ctor_tester{});
}

#endif

struct limb_array_ctor_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        std::vector<::mp_limb_t> vlimbs;
        vlimbs.emplace_back(1u);
        integer n0{static_cast<::mp_limb_t const *>(nullptr), 0u};
        REQUIRE(n0 == 0);
        REQUIRE(n0.is_static());
        integer n1{vlimbs.data(), 0u};
        REQUIRE(n1 == 0);
        REQUIRE(n1.is_static());
        integer n2{vlimbs.data(), 1};
        REQUIRE(n2 == 1);
        REQUIRE(n2.is_static());
        vlimbs[0] = 42;
        integer n3{vlimbs.data(), 1};
        REQUIRE(n3 == 42);
        REQUIRE(n3.is_static());
        vlimbs.emplace_back(43);
        integer n4{vlimbs.data(), 2};
        REQUIRE(n4 == 42 + (integer{43} << GMP_NUMB_BITS));
        if (S::value >= 2u) {
            REQUIRE(n4.is_static());
        } else {
            REQUIRE(n4.is_dynamic());
        }
        // Test the code snippet in the docs.
        ::mp_limb_t arr[] = {5, 6, 7};
        integer n5{arr, 3};
        REQUIRE(n5 == 5 + (integer{6} << GMP_NUMB_BITS) + (integer{7} << (2 * GMP_NUMB_BITS)));
        // Error handling.
        arr[2] = 0;
        REQUIRE_THROWS_PREDICATE((integer{arr, 3}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what())
                   == "When initialising an integer from an array of limbs, the last element of the "
                      "limbs array must be nonzero";
        });
    }
};

TEST_CASE("limb array constructor")
{
    tuple_for_each(sizes{}, limb_array_ctor_tester{});
}
