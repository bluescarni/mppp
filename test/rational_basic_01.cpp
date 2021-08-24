// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <atomic>
#include <complex>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(MPPP_HAVE_STRING_VIEW)
#include <string_view>
#endif

#include <mp++/detail/gmp.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/exceptions.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>

#if defined(MPPP_WITH_QUADMATH)

#include <mp++/real128.hpp>

#endif

#if defined(MPPP_WITH_MPFR)

#include <mp++/real.hpp>

#endif

#include "catch.hpp"
#include "test_utils.hpp"

static const int ntries = 1000;

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

using int_types = std::tuple<char, signed char, unsigned char, short, unsigned short, int, unsigned, long,
                             unsigned long, long long, unsigned long long, wchar_t
#if defined(MPPP_HAVE_GCC_INT128)
                             ,
                             __int128_t, __uint128_t
#endif
                             >;

struct no_const {
};

// A seed that will be used to init rngs in the multithreaded tests. Each time a batch of N threads
// finishes, this value gets bumped up by N, so that the next time a multithreaded test which uses rng
// is launched it will be inited with a different seed.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static std::mt19937::result_type mt_rng_seed(0u);

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
            auto constexpr min = detail::nl_min<Int>(), max = detail::nl_max<Int>();
            REQUIRE(lex_cast(min) == lex_cast(rational{min}));
            REQUIRE(lex_cast(max) == lex_cast(rational{max}));
            std::atomic<bool> fail(false);
            auto f = [&fail](unsigned n) {
                auto dist = integral_minmax_dist<Int>{};
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

            // Make sure rational is implicitly ctible
            // from the integral types.
            rational tmp = Int(5);
            REQUIRE(std::is_convertible<Int, rational>::value);

            std::vector<rational> vec = {Int(1), Int(2), Int(3)};
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
        REQUIRE((!std::is_constructible<rational, no_const>::value));

        rational tmp = true;
        REQUIRE(std::is_convertible<bool, rational>::value);
        std::vector<rational> vec = {true, false};
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 0);

        std::cout << "n static limbs: " << S::value << ", size: " << sizeof(rational) << '\n';
        // Testing for the ctor from int_t.
        using integer = typename rational::int_t;
        REQUIRE((std::is_constructible<rational, integer>::value));
        REQUIRE((lex_cast(rational{integer{0}}) == "0"));
        REQUIRE((lex_cast(rational{integer{1}}) == "1"));
        REQUIRE((lex_cast(rational{integer{-12}}) == "-12"));
        REQUIRE((lex_cast(rational{integer{123}}) == "123"));
        REQUIRE((lex_cast(rational{integer{-123}}) == "-123"));
        integer tmp_int{-123};
        REQUIRE((lex_cast(rational{tmp_int}) == "-123"));

        REQUIRE(std::is_convertible<integer, rational>::value);
        vec = std::vector<rational>{integer(0), integer(1)};

        // Testing for the ctor from num/den.
        REQUIRE((std::is_constructible<rational, integer, integer>::value));
        REQUIRE((std::is_constructible<rational, integer, int>::value));
        REQUIRE((std::is_constructible<rational, short, integer>::value));
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
        // Examples with make_canonical = false.
        q = rational{-9, 17, false};
        REQUIRE((lex_cast(q) == "-9/17"));
        REQUIRE((lex_cast(q.get_num()) == "-9"));
        REQUIRE((lex_cast(q.get_den()) == "17"));
        q = rational{-9, -17, false};
        REQUIRE(q.get_num() == -9);
        REQUIRE(q.get_den() == -17);
        q = rational{0, -17, false};
        REQUIRE(q.get_num() == 0);
        REQUIRE(q.get_den() == -17);
        q = rational{2, -4, false};
        REQUIRE(q.get_num() == 2);
        REQUIRE(q.get_den() == -4);
        q.canonicalise();
        REQUIRE(q.get_num() == -1);
        REQUIRE(q.get_den() == 2);
        q = rational{0, -17, false};
        q.canonicalise();
        REQUIRE(q.get_num() == 0);
        REQUIRE(q.get_den() == 1);
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

            // Make sure rational is *not* implicitly ctible
            // from the fp types.
            REQUIRE(!std::is_convertible<Float, rational>::value);
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

using complex_types = std::tuple<std::complex<float>, std::complex<double>
#if defined(MPPP_WITH_MPFR)
                                 ,
                                 std::complex<long double>
#endif
                                 >;

struct complex_ctor_tester {
    template <typename S>
    struct runner {
        template <typename C>
        void operator()(const C &) const
        {
            using rational = rational<S::value>;
            using Float = typename C::value_type;

            REQUIRE((std::is_constructible<rational, C>::value));
            REQUIRE((std::is_constructible<rational, C &>::value));
            REQUIRE((std::is_constructible<rational, C &&>::value));
            REQUIRE((std::is_constructible<rational, const C &>::value));

            // A few simple tests.
            REQUIRE(rational{C{0, 0}} == 0);
            REQUIRE(rational{C{1, 0}} == 1);
            REQUIRE(rational{C{-42, 0}} == -42);

            if (std::numeric_limits<Float>::is_iec559) {
                REQUIRE_THROWS_PREDICATE(
                    rational{C(std::numeric_limits<Float>::infinity(), Float(0))}, std::domain_error,
                    [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot construct a rational from the non-finite floating-point value "
                                      + std::to_string(std::numeric_limits<Float>::infinity());
                    });
                REQUIRE_THROWS_PREDICATE(
                    rational{C(-std::numeric_limits<Float>::infinity(), Float(0))}, std::domain_error,
                    [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot construct a rational from the non-finite floating-point value "
                                      + std::to_string(-std::numeric_limits<Float>::infinity());
                    });
                REQUIRE_THROWS_PREDICATE(
                    rational{C(std::numeric_limits<Float>::quiet_NaN(), Float(0))}, std::domain_error,
                    [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot construct a rational from the non-finite floating-point value "
                                      + std::to_string(std::numeric_limits<Float>::quiet_NaN());
                    });
                REQUIRE_THROWS_PREDICATE(rational{(C{0, std::numeric_limits<Float>::quiet_NaN()})}, std::domain_error,
                                         [](const std::domain_error &ex) {
                                             return ex.what()
                                                    == "Cannot construct a rational from a complex C++ "
                                                       "value with a non-zero imaginary part of "
                                                           + detail::to_string(std::numeric_limits<Float>::quiet_NaN());
                                         });
                REQUIRE_THROWS_PREDICATE(rational{(C{0, std::numeric_limits<Float>::infinity()})}, std::domain_error,
                                         [](const std::domain_error &ex) {
                                             return ex.what()
                                                    == "Cannot construct a rational from a complex C++ "
                                                       "value with a non-zero imaginary part of "
                                                           + detail::to_string(std::numeric_limits<Float>::infinity());
                                         });
            }

            REQUIRE_THROWS_PREDICATE(rational{(C{0, 1})}, std::domain_error, [](const std::domain_error &ex) {
                return ex.what()
                       == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(Float(1));
            });

            REQUIRE_THROWS_PREDICATE(rational{(C{-1, 1})}, std::domain_error, [](const std::domain_error &ex) {
                return ex.what()
                       == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(Float(1));
            });

            // Make sure rational is *not* implicitly ctible
            // from complex.
            REQUIRE(!std::is_convertible<C, rational>::value);
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

struct string_ctor_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        REQUIRE(detail::is_rational<rational>::value);
        REQUIRE(!detail::is_rational<int>::value);
        REQUIRE((detail::is_same_ssize_rational<rational, rational>::value));
        REQUIRE((!detail::is_same_ssize_rational<rational, mppp::rational<S::value + 1u>>::value));
        REQUIRE((!detail::is_same_ssize_rational<mppp::rational<S::value + 1u>, rational>::value));
        REQUIRE((!detail::is_same_ssize_rational<void, int>::value));
        REQUIRE((!detail::is_same_ssize_rational<rational, std::string>::value));
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
        // Constructor from range of chars.
        std::string s = "-1234";
        REQUIRE((rational{s.data(), s.data() + 5} == -1234));
        REQUIRE((rational{s.data(), s.data() + 4} == -123));
        s = "-1234/345";
        REQUIRE((rational{s.data(), s.data() + 9} == rational{-1234, 345}));
        REQUIRE((rational{s.data(), s.data() + 8} == rational{-617, 17}));
        s = "0x7b";
        REQUIRE((rational{s.data(), s.data() + 4, 0} == 123));
        s = "1E45";
        REQUIRE_THROWS_PREDICATE(
            (rational{s.data(), s.data() + 4, 12}), std::invalid_argument, [](const std::invalid_argument &ia) {
                return std::string(ia.what()) == "The string '1E45' is not a valid integer in base 12";
            });
        // Try with an already terminated string.
        const char *cs = "-1234/345\0";
        REQUIRE((rational{cs, cs + 9} == rational{-1234, 345}));
        REQUIRE((rational{cs, cs + 8} == rational{-617, 17}));
#if defined(MPPP_HAVE_STRING_VIEW)
        std::string_view sv = "-1234/345";
        REQUIRE((rational{sv} == rational{-1234, 345}));
        REQUIRE((rational{std::string_view{sv.data(), 8u}} == rational{-617, 17}));
        sv = "0x7b";
        REQUIRE((rational{sv, 0} == 123));
        sv = "1E45";
        REQUIRE_THROWS_PREDICATE((rational{sv, 12}), std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '1E45' is not a valid integer in base 12";
        });
        REQUIRE((rational{std::string_view{cs, 9}} == rational{-1234, 345}));
        REQUIRE((rational{std::string_view{cs, 8}} == rational{-617, 17}));
#endif
    }
};

TEST_CASE("string constructor")
{
    tuple_for_each(sizes{}, string_ctor_tester{});
}

struct mpq_copy_ctor_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        detail::mpq_raii m;
        REQUIRE((std::is_constructible<rational, const ::mpq_t>::value));
        REQUIRE(lex_cast(rational{&m.m_mpq}) == "0");
        mpz_set_si(mpq_numref(&m.m_mpq), 1234);
        REQUIRE(lex_cast(rational{&m.m_mpq}) == "1234");
        mpz_set_si(mpq_numref(&m.m_mpq), -1234);
        REQUIRE(lex_cast(rational{&m.m_mpq}) == "-1234");
        mpz_set_si(mpq_numref(&m.m_mpq), 4);
        mpz_set_si(mpq_denref(&m.m_mpq), -3);
        REQUIRE(lex_cast(rational{&m.m_mpq}) == "4/-3");
        mpz_set_str(mpq_numref(&m.m_mpq),
                    "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        REQUIRE(lex_cast(rational{&m.m_mpq})
                == "3218372891372987328917389127389217398271983712987398127398172389712937819237/-3");
        mpz_set_str(mpq_denref(&m.m_mpq),
                    "-3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        REQUIRE(lex_cast(rational{&m.m_mpq})
                == "3218372891372987328917389127389217398271983712987398127398172389712937819237/"
                   "-3218372891372987328917389127389217398271983712987398127398172389712937819237");
    }
};

TEST_CASE("mpq_t copy constructor")
{
    tuple_for_each(sizes{}, mpq_copy_ctor_tester{});
}

#if !defined(_MSC_VER)

struct mpq_move_ctor_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        ::mpq_t q0;
        mpq_init(q0);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        REQUIRE(rational{std::move(q0)} == 0);
        mpq_init(q0);
        mpz_set_si(mpq_numref(q0), 1234);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        REQUIRE(rational{std::move(q0)} == 1234);
        mpq_init(q0);
        mpz_set_si(mpq_numref(q0), -1234);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        REQUIRE(rational{std::move(q0)} == -1234);
        mpq_init(q0);
        mpz_set_si(mpq_numref(q0), 4);
        mpz_set_si(mpq_denref(q0), -3);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        REQUIRE(lex_cast(rational{std::move(q0)}) == "4/-3");
        mpq_init(q0);
        mpz_set_str(mpq_numref(q0), "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        mpz_set_si(mpq_denref(q0), -3);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        REQUIRE(lex_cast(rational{std::move(q0)})
                == "3218372891372987328917389127389217398271983712987398127398172389712937819237/-3");
        mpq_init(q0);
        mpz_set_str(mpq_numref(q0), "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        mpz_set_str(mpq_denref(q0), "-3218372891372987328917389127389217398271983712987398127398172389712937819237",
                    10);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        REQUIRE(lex_cast(rational{std::move(q0)})
                == "3218372891372987328917389127389217398271983712987398127398172389712937819237/"
                   "-3218372891372987328917389127389217398271983712987398127398172389712937819237");
    }
};

TEST_CASE("mpq_t move constructor")
{
    tuple_for_each(sizes{}, mpq_move_ctor_tester{});
}

#endif

struct mpz_ctor_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        using integer = integer<S::value>;
        detail::mpz_raii m;
        REQUIRE((std::is_constructible<rational, const ::mpz_t>::value));
        REQUIRE(rational{&m.m_mpz}.is_zero());
        REQUIRE(rational{&m.m_mpz}.get_num().is_static());
        REQUIRE(rational{&m.m_mpz}.get_den().is_one());
        REQUIRE(rational{&m.m_mpz}.get_den().is_static());
        mpz_set_si(&m.m_mpz, 1234);
        REQUIRE(rational{&m.m_mpz}.get_num() == 1234);
        REQUIRE(rational{&m.m_mpz}.get_num().is_static());
        REQUIRE(rational{&m.m_mpz}.get_den().is_one());
        REQUIRE(rational{&m.m_mpz}.get_den().is_static());
        mpz_set_si(&m.m_mpz, -1234);
        REQUIRE(rational{&m.m_mpz}.get_num() == -1234);
        REQUIRE(rational{&m.m_mpz}.get_num().is_static());
        REQUIRE(rational{&m.m_mpz}.get_den().is_one());
        REQUIRE(rational{&m.m_mpz}.get_den().is_static());
        mpz_set_str(&m.m_mpz, "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        REQUIRE(rational{&m.m_mpz}.get_num()
                == integer{"3218372891372987328917389127389217398271983712987398127398172389712937819237"});
        REQUIRE(rational{&m.m_mpz}.get_den().is_one());
        REQUIRE(rational{&m.m_mpz}.get_den().is_static());
        mpz_set_str(&m.m_mpz, "-3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
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
    // NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        using integer = typename rational::int_t;
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
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
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
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(q4.get_num().is_zero());
        REQUIRE(q4.get_den().is_one());
        REQUIRE(q4.get_num().is_static());
        REQUIRE(q4.get_den().is_static());
        REQUIRE(lex_cast(q2) == "-123");
        REQUIRE(q2.get_num().is_static());
        REQUIRE(q2.get_den().is_dynamic());
        // Self assignments.
        q2 = *&q2;
        REQUIRE(lex_cast(q2) == "-123");
        REQUIRE(q2.get_num().is_static());
        REQUIRE(q2.get_den().is_dynamic());
        q2 = std::move(q2);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
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
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q._get_num().promote();
        auto qb(std::move(q));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q._get_den().promote();
        auto qc(std::move(q));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q._get_num().promote();
        q._get_den().promote();
        auto qd(std::move(q));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q2 = std::move(q);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q._get_num().promote();
        q2 = std::move(q);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q._get_den().promote();
        q2 = std::move(q);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q._get_num().promote();
        q._get_den().promote();
        q2 = std::move(q);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q2._get_num().promote();
        q2 = std::move(q);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q2._get_den().promote();
        q2 = std::move(q);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());
        q = "4/5";
        q2 = "3/4";
        q2._get_num().promote();
        q2._get_den().promote();
        q2 = std::move(q);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
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
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
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
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
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
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
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
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
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
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
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
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
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
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
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
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
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
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(q.get_num().is_zero());
        REQUIRE(q.get_den().is_one());
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den().is_static());

        // Do some minimal testing for swapping as well.
        q = 0;
        q2 = 1;
        swap(q2, q2);
        REQUIRE(q2 == 1);
        swap(q, q2);
        REQUIRE(q == 1);
        REQUIRE(q2 == 0);
        q = "4/5";
        q2 = "-3/7";
        swap(q, q2);
        REQUIRE(q == rational{-3, 7});
        REQUIRE(q2 == rational{4, 5});
        REQUIRE(noexcept(swap(q, q2)));
    }
};

TEST_CASE("copy and move")
{
    tuple_for_each(sizes{}, copy_move_tester{});
}
