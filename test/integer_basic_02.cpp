// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <atomic>
#include <cstddef>
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

#if defined(MPPP_HAVE_STRING_VIEW)
#include <string_view>
#endif

#include <gmp.h>

#include <mp++/detail/type_traits.hpp>
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

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

// A seed that will be used to init rngs in the multithreaded tests. Each time a batch of N threads
// finishes, this value gets bumped up by N, so that the next time a multithreaded test which uses rng
// is launched it will be inited with a different seed.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static std::mt19937::result_type mt_rng_seed(0u);

// NOLINTNEXTLINE(cert-err58-cpp, cert-msc32-c, cert-msc51-cpp, cppcoreguidelines-avoid-non-const-global-variables)
static std::mt19937 rng;

struct no_const {
};

struct nbits_ctor_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        REQUIRE((integer{integer_bitcnt_t(0)}.is_static()));
        REQUIRE((integer{integer_bitcnt_t(0)}.is_zero()));
        REQUIRE((integer{integer_bitcnt_t(1)}.is_static()));
        REQUIRE((integer{integer_bitcnt_t(1)}.is_zero()));
        REQUIRE((integer{integer_bitcnt_t(2)}.is_static()));
        REQUIRE((integer{integer_bitcnt_t(2)}.is_zero()));
        REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS)}.is_static()));
        REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS)}.is_zero()));
        if (S::value == 1) {
            REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS + 1)}.is_dynamic()));
            REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS + 1)}.is_zero()));
            REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS + 1)}.get_mpz_t()->_mp_alloc == 2));
            REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS + 2)}.is_dynamic()));
            REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS + 2)}.is_zero()));
            REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS + 2)}.get_mpz_t()->_mp_alloc == 2));
            REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS * 2)}.is_dynamic()));
            REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS * 2)}.is_zero()));
            REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS * 2)}.get_mpz_t()->_mp_alloc == 2));
            REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS * 2 + 1)}.is_dynamic()));
            REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS * 2 + 1)}.is_zero()));
            REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS * 2 + 1)}.get_mpz_t()->_mp_alloc == 3));
        }
        REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS * S::value)}.is_static()));
        REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS * S::value)}.is_zero()));
        REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS * S::value + 1)}.is_dynamic()));
        REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS * S::value + 1)}.is_zero()));
        REQUIRE((integer{integer_bitcnt_t(GMP_NUMB_BITS * S::value + 1)}.get_mpz_t()->_mp_alloc == S::value + 1));
    }
};

TEST_CASE("nbits constructor")
{
    tuple_for_each(sizes{}, nbits_ctor_tester{});
}

struct copy_move_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
#if MPPP_CPLUSPLUS >= 201402L
        REQUIRE(std::is_nothrow_move_constructible<integer>::value);
#endif
        integer n;
        REQUIRE(n.is_static());
        n = 123;
        REQUIRE(n.is_static());
        integer m{n};
        REQUIRE(n.is_static());
        REQUIRE(m.is_static());
        REQUIRE(n == 123);
        REQUIRE(m == 123);
        m.promote();
        REQUIRE(m.is_dynamic());
        integer m2{std::move(m)};
        REQUIRE(m2.is_dynamic());
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(m.is_static());
        REQUIRE(m == 0);
        m = 123;
        integer m3{std::move(m)};
        REQUIRE(m3 == 123);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(m.is_static());
        REQUIRE(m3.is_static());
        m3.promote();
        integer m4{m3};
        REQUIRE(m3 == 123);
        REQUIRE(m4 == 123);
        REQUIRE(m3.is_dynamic());
        REQUIRE(m4.is_dynamic());
        m4 = *&m4;
        REQUIRE(m4.is_dynamic());
        REQUIRE(m4 == 123);
        m4 = std::move(m4);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(m4.is_dynamic());
        REQUIRE(m4 == 123);
        integer m5{12}, m6{-10};
        m5 = m6;
        REQUIRE(m5.is_static());
        REQUIRE(m5 == -10);
        m5 = m4;
        REQUIRE(m5.is_dynamic());
        REQUIRE(m5 == 123);
        m4 = m6;
        REQUIRE(m4.is_static());
        REQUIRE(m4 == -10);
        m4.promote();
        m5 = m4;
        REQUIRE(m5.is_dynamic());
        REQUIRE(m5 == -10);
        m4 = std::move(m5);
        REQUIRE(m4.is_dynamic());
        REQUIRE(m4 == -10);
        m4 = integer{-1};
        REQUIRE(m4.is_static());
        REQUIRE(m4 == -1);
        m4.promote();
        m5 = 10;
        m5.promote();
        m4 = std::move(m5);
        REQUIRE(m4.is_dynamic());
        REQUIRE(m4 == 10);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        m5 = -1;
        m5 = std::move(m4);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(m4.is_static());
        REQUIRE(m4 == 0);
        REQUIRE(m5.is_dynamic());
        REQUIRE(m5 == 10);
    }
};

TEST_CASE("copy and move")
{
    tuple_for_each(sizes{}, copy_move_tester{});
}

struct mpz_copy_ass_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n;
        detail::mpz_raii m;
        n = &m.m_mpz;
        REQUIRE(lex_cast(n) == "0");
        mpz_set_si(&m.m_mpz, 1234);
        n = &m.m_mpz;
        REQUIRE(n == 1234);
        mpz_set_si(&m.m_mpz, -1234);
        n = &m.m_mpz;
        REQUIRE(n == -1234);
        mpz_set_str(&m.m_mpz, "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        n = &m.m_mpz;
        REQUIRE(n == integer("3218372891372987328917389127389217398271983712987398127398172389712937819237"));
        mpz_set_str(&m.m_mpz, "-3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        n = &m.m_mpz;
        REQUIRE(n == integer("-3218372891372987328917389127389217398271983712987398127398172389712937819237"));
        // Random testing.
        std::atomic<bool> fail(false);
        auto f = [&fail](unsigned u) {
            std::uniform_int_distribution<long> dist(detail::nl_min<long>(), detail::nl_max<long>());
            std::uniform_int_distribution<int> sdist(0, 1);
            std::mt19937 eng(static_cast<std::mt19937::result_type>(u + mt_rng_seed));
            for (auto i = 0; i < ntries; ++i) {
                detail::mpz_raii mpz;
                auto tmp = dist(eng);
                mpz_set_si(&mpz.m_mpz, tmp);
                integer z;
                if (sdist(eng)) {
                    z.promote();
                }
                z = &mpz.m_mpz;
                if (z != tmp) {
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

TEST_CASE("mpz_t copy assignment")
{
    tuple_for_each(sizes{}, mpz_copy_ass_tester{});
}

#if !defined(_MSC_VER)

struct mpz_move_ass_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n;
        ::mpz_t m0;
        mpz_init(m0);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        n = std::move(m0);
        REQUIRE(lex_cast(n) == "0");
        mpz_init(m0);
        mpz_set_si(m0, 1234);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        n = std::move(m0);
        REQUIRE(n == 1234);
        mpz_init(m0);
        mpz_set_si(m0, -1234);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        n = std::move(m0);
        REQUIRE(n == -1234);
        mpz_init(m0);
        mpz_set_str(m0, "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        n = std::move(m0);
        REQUIRE(n == integer("3218372891372987328917389127389217398271983712987398127398172389712937819237"));
        mpz_init(m0);
        mpz_set_str(m0, "-3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        n = std::move(m0);
        REQUIRE(n == integer("-3218372891372987328917389127389217398271983712987398127398172389712937819237"));
        // Random testing.
        std::atomic<bool> fail(false);
        auto f = [&fail](unsigned u) {
            std::uniform_int_distribution<long> dist(detail::nl_min<long>(), detail::nl_max<long>());
            std::uniform_int_distribution<int> sdist(0, 1);
            std::mt19937 eng(static_cast<std::mt19937::result_type>(u + mt_rng_seed));
            for (auto i = 0; i < ntries; ++i) {
                ::mpz_t m1;
                mpz_init(m1);
                auto tmp = dist(eng);
                mpz_set_si(m1, tmp);
                integer z;
                if (sdist(eng)) {
                    z.promote();
                }
                // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
                z = std::move(m1);
                if (z != tmp) {
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

TEST_CASE("mpz_t move assignment")
{
    tuple_for_each(sizes{}, mpz_move_ass_tester{});
}

#endif

struct string_ass_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n;
        n = "123";
        REQUIRE(n == 123);
        n = " -456 ";
        REQUIRE(n == -456);
        n = std::string("123");
        REQUIRE(n == 123);
        n = std::string(" -456 ");
        REQUIRE(n == -456);
        REQUIRE_THROWS_PREDICATE(n = "", std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '' is not a valid integer in base 10";
        });
#if defined(MPPP_HAVE_STRING_VIEW)
        n = std::string_view(" -123 ");
        REQUIRE(n == -123);
        n = std::string_view("4563 ");
        REQUIRE(n == 4563);
        REQUIRE_THROWS_PREDICATE(n = std::string_view(""), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '' is not a valid integer in base 10";
        });
#endif
    }
};

TEST_CASE("string assignment")
{
    tuple_for_each(sizes{}, string_ass_tester{});
}

struct promdem_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n;
        REQUIRE(n.promote());
        REQUIRE(n.sgn() == 0);
        REQUIRE(n.is_dynamic());
        REQUIRE(!n.promote());
        REQUIRE(n.demote());
        REQUIRE(n.sgn() == 0);
        REQUIRE(n.is_static());
        REQUIRE(!n.demote());
        n = -5;
        REQUIRE(n.promote());
        REQUIRE(n == -5);
        REQUIRE(n.is_dynamic());
        REQUIRE(!n.promote());
        REQUIRE(n.demote());
        REQUIRE(n == -5);
        REQUIRE(n.is_static());
        REQUIRE(!n.demote());
        n = integer{"312321983721983791287392817328917398217398712938719273981273"};
        if (n.size() > S::value) {
            REQUIRE(n.is_dynamic());
            REQUIRE(!n.demote());
            REQUIRE(n.is_dynamic());
        }
    }
};

TEST_CASE("promote and demote")
{
    tuple_for_each(sizes{}, promdem_tester{});
}

struct sign_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n;
        REQUIRE(n.sgn() == 0);
        REQUIRE(sgn(n) == 0);
        n.promote();
        REQUIRE(n.sgn() == 0);
        REQUIRE(sgn(n) == 0);
        n = 12;
        REQUIRE(n.sgn() == 1);
        REQUIRE(sgn(n) == 1);
        n.promote();
        REQUIRE(n.sgn() == 1);
        REQUIRE(sgn(n) == 1);
        n = -34;
        REQUIRE(n.sgn() == -1);
        REQUIRE(sgn(n) == -1);
        n.promote();
        REQUIRE(n.sgn() == -1);
        REQUIRE(sgn(n) == -1);
    }
};

TEST_CASE("sign")
{
    tuple_for_each(sizes{}, sign_tester{});
}

struct to_string_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        REQUIRE(integer{}.to_string() == "0");
        REQUIRE(integer{1}.to_string() == "1");
        REQUIRE(integer{-1}.to_string() == "-1");
        REQUIRE(integer{123}.to_string() == "123");
        REQUIRE(integer{-123}.to_string() == "-123");
        REQUIRE(integer{123}.to_string(3) == "11120");
        REQUIRE(integer{-123}.to_string(3) == "-11120");
        REQUIRE_THROWS_PREDICATE((integer{}.to_string(1)), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what())
                   == "Invalid base for string conversion: the base must be between "
                      "2 and 62, but a value of 1 was provided instead";
        });
        REQUIRE_THROWS_PREDICATE((integer{}.to_string(-12)), std::invalid_argument,
                                 [](const std::invalid_argument &ia) {
                                     return std::string(ia.what())
                                            == "Invalid base for string conversion: the base must be between "
                                               "2 and 62, but a value of -12 was provided instead";
                                 });
        REQUIRE_THROWS_PREDICATE((integer{}.to_string(63)), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what())
                   == "Invalid base for string conversion: the base must be between "
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
        using integer = integer<S::value>;
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
