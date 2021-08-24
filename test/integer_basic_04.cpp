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
#include <limits>
#include <random>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>

#include <gmp.h>

#include <mp++/config.hpp>
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

// NOLINTNEXTLINE(cert-err58-cpp, cert-msc32-c, cert-msc51-cpp, cppcoreguidelines-avoid-non-const-global-variables)
static std::mt19937 rng;

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
    T rop1, rop2;
    bool retval = (static_cast<T>(tmp) == x) && (lex_cast(x) == lex_cast(tmp));
    const bool res1 = tmp.get(rop1);
    const bool res2 = get(rop2, tmp);
    retval = retval && res1 && res2;
    retval = retval && (lex_cast(rop1) == lex_cast(tmp)) && (lex_cast(rop2) == lex_cast(tmp));
    return retval;
}

struct no_conv {
};

struct int_convert_tester {
    template <typename S>
    struct runner {
        template <typename Int>
        void operator()(const Int &) const
        {
            using integer = integer<S::value>;
            REQUIRE((is_convertible<integer, Int>::value));
            REQUIRE(roundtrip_conversion<integer>(0));
            auto constexpr min = detail::nl_min<Int>(), max = detail::nl_max<Int>();
            REQUIRE(roundtrip_conversion<integer>(min));
            REQUIRE(roundtrip_conversion<integer>(max));
            REQUIRE(roundtrip_conversion<integer>(Int(42)));
            REQUIRE(roundtrip_conversion<integer>(Int(-42)));
            REQUIRE(roundtrip_conversion<integer>(min + Int(1)));
            REQUIRE(roundtrip_conversion<integer>(max - Int(1)));
            REQUIRE(roundtrip_conversion<integer>(min + Int(2)));
            REQUIRE(roundtrip_conversion<integer>(max - Int(2)));
            REQUIRE(roundtrip_conversion<integer>(min + Int(3)));
            REQUIRE(roundtrip_conversion<integer>(max - Int(3)));
            REQUIRE(roundtrip_conversion<integer>(min + Int(42)));
            REQUIRE(roundtrip_conversion<integer>(max - Int(42)));
            Int rop(1);
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(min) - 1), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE(!(integer(min) - 1).get(rop));
            REQUIRE(!get(rop, integer(min) - 1));
            REQUIRE(rop == Int(1));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(min) - 2), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE(!(integer(min) - 2).get(rop));
            REQUIRE(!get(rop, integer(min) - 2));
            REQUIRE(rop == Int(1));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(min) - 3), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE(!(integer(min) - 3).get(rop));
            REQUIRE(!get(rop, integer(min) - 3));
            REQUIRE(rop == Int(1));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(min) - 123), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE(!(integer(min) - 123).get(rop));
            REQUIRE(!get(rop, integer(min) - 123));
            REQUIRE(rop == Int(1));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(max) + 1), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE(!(integer(max) + 1).get(rop));
            REQUIRE(!get(rop, integer(max) + 1));
            REQUIRE(rop == Int(1));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(max) + 2), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE(!(integer(max) + 2).get(rop));
            REQUIRE(!get(rop, integer(max) + 2));
            REQUIRE(rop == Int(1));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(max) + 3), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE(!(integer(max) + 3).get(rop));
            REQUIRE(!get(rop, integer(max) + 3));
            REQUIRE(rop == Int(1));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(max) + 123), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE(!(integer(max) + 123).get(rop));
            REQUIRE(!get(rop, integer(max) + 123));
            REQUIRE(rop == Int(1));
            // Try with large integers that should trigger a specific error, at least on some platforms.
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(max) * max * max * max * max), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE(!(integer(max) * max * max * max * max).get(rop));
            REQUIRE(!get(rop, integer(max) * max * max * max * max));
            REQUIRE(rop == Int(1));
            if (min != Int(0)) {
                REQUIRE_THROWS_PREDICATE(static_cast<Int>(integer(min) * min * min * min * min), std::overflow_error,
                                         [](const std::overflow_error &) { return true; });
                REQUIRE(!(integer(min) * min * min * min * min).get(rop));
                REQUIRE(!get(rop, integer(min) * min * min * min * min));
                REQUIRE(rop == Int(1));
            }
            std::atomic<bool> fail(false);
            auto f = [&fail](unsigned n) {
                integral_minmax_dist<Int> dist;
                std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
                for (auto i = 0; i < ntries; ++i) {
                    if (!roundtrip_conversion<integer>(static_cast<Int>(dist(eng)))) {
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

            // Check that integer cannot be implicitly converted
            // to C++ ints.
            REQUIRE(!std::is_convertible<integer, Int>::value);
        }
    };
    template <typename S>
    inline void operator()(const S &) const
    {
        tuple_for_each(int_types{}, runner<S>{});
        // Some testing for bool.
        using integer = integer<S::value>;
        REQUIRE((is_convertible<integer, bool>::value));
        REQUIRE(roundtrip_conversion<integer>(true));
        REQUIRE(roundtrip_conversion<integer>(false));
        // Extra.
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
            using integer = integer<S::value>;
            REQUIRE((is_convertible<integer, Float>::value));
            Float rop(1);
            REQUIRE(static_cast<Float>(integer{0}) == Float(0));
            REQUIRE(integer{0}.get(rop));
            REQUIRE(get(rop, integer{0}));
            REQUIRE(rop == Float(0));
            REQUIRE(static_cast<Float>(integer{1}) == Float(1));
            REQUIRE(integer{1}.get(rop));
            REQUIRE(get(rop, integer{1}));
            REQUIRE(rop == Float(1));
            REQUIRE(static_cast<Float>(integer{-1}) == Float(-1));
            REQUIRE(integer{-1}.get(rop));
            REQUIRE(get(rop, integer{-1}));
            REQUIRE(rop == Float(-1));
            REQUIRE(static_cast<Float>(integer{12}) == Float(12));
            REQUIRE(integer{12}.get(rop));
            REQUIRE(get(rop, integer{12}));
            REQUIRE(rop == Float(12));
            REQUIRE(static_cast<Float>(integer{-12}) == Float(-12));
            REQUIRE(integer{-12}.get(rop));
            REQUIRE(get(rop, integer{-12}));
            REQUIRE(rop == Float(-12));
            if (std::numeric_limits<Float>::is_iec559) {
                // Try with large numbers.
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
                    Float rop1;
                    const auto tmp = dist(eng);
                    if (static_cast<Float>(integer{tmp}) != std::trunc(tmp)) {
                        fail.store(false);
                    }
                    if (!integer{tmp}.get(rop1)) {
                        fail.store(false);
                    }
                    if (!get(rop1, integer{tmp})) {
                        fail.store(false);
                    }
                    if (rop1 != std::trunc(tmp)) {
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

// A few simple tests, as the conversions
// are based on the fp ones.
struct complex_convert_tester {
    template <typename S>
    struct runner {
        template <typename C>
        void operator()(const C &) const
        {
            using integer = integer<S::value>;

            // Type traits.
            REQUIRE((is_convertible<integer, C>::value));
            REQUIRE((is_convertible<C, integer>::value));

            // Casts to C.
            REQUIRE(static_cast<C>(integer{0}) == C{0, 0});
            REQUIRE(static_cast<C>(integer{1}) == C{1, 0});
            REQUIRE(static_cast<C>(integer{-42}) == -C{42, 0});
            C rop{4, 5};

            // get() functions.
            REQUIRE(integer{1}.get(rop));
            REQUIRE(rop == C{1, 0});
            REQUIRE(integer{0}.get(rop));
            REQUIRE(rop == C{0, 0});
            REQUIRE(get(rop, integer{-5}));
            REQUIRE(rop == C{-5, 0});
            REQUIRE(get(rop, integer{0}));
            REQUIRE(rop == C{0, 0});

            // Functional cast form from integer to C.
            REQUIRE(C(integer{}) == C{0, 0});
            REQUIRE(C(integer{-37}) == C{-37, 0});
            REQUIRE(C(integer{42}) == C{42, 0});
        }
    };
    template <typename S>
    inline void operator()(const S &) const
    {
        tuple_for_each(complex_types{}, runner<S>{});
    }
};

TEST_CASE("complex conversions")
{
    tuple_for_each(sizes{}, complex_convert_tester{});
}

struct sizes_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        integer n;
        REQUIRE(n.nbits() == 0u);
        REQUIRE(n.size() == 0u);
        n = 1;
        REQUIRE(n.nbits() == 1u);
        REQUIRE(n.size() == 1u);
        n = -1;
        REQUIRE(n.nbits() == 1u);
        REQUIRE(n.size() == 1u);
        n = 3;
        REQUIRE(n.nbits() == 2u);
        REQUIRE(n.size() == 1u);
        n = -3;
        REQUIRE(n.nbits() == 2u);
        REQUIRE(n.size() == 1u);
        n = 1;
        n <<= GMP_NUMB_BITS;
        REQUIRE(n.nbits() == GMP_NUMB_BITS + 1);
        REQUIRE(n.size() == 2u);
        n = -1;
        n <<= GMP_NUMB_BITS;
        REQUIRE(n.nbits() == GMP_NUMB_BITS + 1);
        REQUIRE(n.size() == 2u);
        // Static data member.
        REQUIRE(integer::ssize == S::value);
        // Random testing.
        detail::mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        auto random_x = [&](unsigned x) {
            for (int i = 0; i < ntries; ++i) {
                random_integer(tmp, x, rng);
                n = &tmp.m_mpz;
                if (n.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n.promote();
                }
                const auto res1 = n.nbits();
                const auto res2 = n.sgn() ? mpz_sizeinbase(&tmp.m_mpz, 2) : 0u;
                REQUIRE(res1 == res2);
            }
        };
        random_x(0);
        random_x(1);
        random_x(2);
        random_x(3);
        random_x(4);
    }
};

TEST_CASE("sizes")
{
    tuple_for_each(sizes{}, sizes_tester{});
}
