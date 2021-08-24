// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <complex>
#include <cstddef>
#include <ios>
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

#if defined(MPPP_HAVE_STRING_VIEW)
#include <string_view>
#endif

#if defined(MPPP_WITH_BOOST_S11N)

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#endif

#include <gmp.h>

#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/utils.hpp>
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

struct no_const {
};

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
                    if (sdist(eng)) {
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

struct complex_ass_tester {
    template <typename S>
    struct runner {
        template <typename C>
        void operator()(const C &) const
        {
            using integer = integer<S::value>;
            using Float = typename C::value_type;
            REQUIRE((std::is_assignable<integer &, C>::value));
            integer n0;
            if (std::numeric_limits<Float>::is_iec559) {
                REQUIRE_THROWS_PREDICATE(n0 = C(std::numeric_limits<Float>::infinity(), 0), std::domain_error,
                                         [](const std::domain_error &ex) {
                                             return ex.what()
                                                    == "Cannot assign the non-finite floating-point value "
                                                           + std::to_string(std::numeric_limits<Float>::infinity())
                                                           + " to an integer";
                                         });
                REQUIRE_THROWS_PREDICATE(n0 = C(-std::numeric_limits<Float>::infinity(), 0), std::domain_error,
                                         [](const std::domain_error &ex) {
                                             return ex.what()
                                                    == "Cannot assign the non-finite floating-point value "
                                                           + std::to_string(-std::numeric_limits<Float>::infinity())
                                                           + " to an integer";
                                         });
                REQUIRE_THROWS_PREDICATE(n0 = C(std::numeric_limits<Float>::quiet_NaN(), 0), std::domain_error,
                                         [](const std::domain_error &ex) {
                                             return ex.what()
                                                    == "Cannot assign the non-finite floating-point value "
                                                           + std::to_string(std::numeric_limits<Float>::quiet_NaN())
                                                           + " to an integer";
                                         });
                REQUIRE_THROWS_PREDICATE(
                    n0 = C(0, std::numeric_limits<Float>::quiet_NaN()), std::domain_error,
                    [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                                      + detail::to_string(std::numeric_limits<Float>::quiet_NaN()) + " to an integer";
                    });
                REQUIRE_THROWS_PREDICATE(
                    n0 = C(0, std::numeric_limits<Float>::infinity()), std::domain_error,
                    [](const std::domain_error &ex) {
                        return ex.what()
                               == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                                      + detail::to_string(std::numeric_limits<Float>::infinity()) + " to an integer";
                    });
            }
            REQUIRE_THROWS_PREDICATE(n0 = C(0, 1), std::domain_error, [](const std::domain_error &ex) {
                return ex.what()
                       == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(Float(1)) + " to an integer";
            });
            REQUIRE_THROWS_PREDICATE(n0 = C(-1, 1), std::domain_error, [](const std::domain_error &ex) {
                return ex.what()
                       == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(Float(1)) + " to an integer";
            });
            REQUIRE_THROWS_PREDICATE(n0 = C(1, 1), std::domain_error, [](const std::domain_error &ex) {
                return ex.what()
                       == "Cannot assign a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(Float(1)) + " to an integer";
            });
            REQUIRE((n0 = C(0, 0)) == 0);
            REQUIRE((n0 = C(12, 0)) == 12);
            REQUIRE((n0 = C(-42, 0)) == -42);
        }
    };
    template <typename S>
    inline void operator()(const S &) const
    {
        tuple_for_each(complex_types{}, runner<S>{});
    }
};

TEST_CASE("complex assignment")
{
    tuple_for_each(sizes{}, complex_ass_tester{});
}

struct mppp_ass_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        using rational = rational<S::value>;

        integer n{};
        n = rational{5};
        REQUIRE(n == 5);
        n = rational{-5, 6};
        REQUIRE(n == 0);
        n = rational{6, 7};
        REQUIRE(n == 0);
        n = rational{8, 7};
        REQUIRE(n == 1);
        n = rational{8, -7};
        REQUIRE(n == -1);
        n = rational{16, 7};
        REQUIRE(n == 2);
        REQUIRE(std::is_same<decltype(n = rational{16, 7}), integer &>::value);

#if defined(MPPP_WITH_QUADMATH)
        n = real128{42};
        REQUIRE(n == 42);
        n = real128{"-45.6"};
        REQUIRE(n == -45);
        REQUIRE(std::is_same<decltype(n = real128{"-45.6"}), integer &>::value);
#endif

#if defined(MPPP_WITH_MPFR)
        n = real{-42};
        REQUIRE(n == -42);
        n = real{"45.1", 100};
        REQUIRE(n == 45);
        REQUIRE(std::is_same<decltype(n = real{"45.1", 100}), integer &>::value);
#endif
    }
};

TEST_CASE("mp++ assignments")
{
    tuple_for_each(sizes{}, mppp_ass_tester{});
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
#if defined(MPPP_HAVE_STRING_VIEW)
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
        detail::mpz_raii m;
        REQUIRE(lex_cast(integer{&m.m_mpz}) == "0");
        mpz_set_si(&m.m_mpz, 1234);
        REQUIRE(lex_cast(integer{&m.m_mpz}) == "1234");
        mpz_set_si(&m.m_mpz, -1234);
        REQUIRE(lex_cast(integer{&m.m_mpz}) == "-1234");
        mpz_set_str(&m.m_mpz, "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        REQUIRE(lex_cast(integer{&m.m_mpz})
                == "3218372891372987328917389127389217398271983712987398127398172389712937819237");
        mpz_set_str(&m.m_mpz, "-3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        REQUIRE(lex_cast(integer{&m.m_mpz})
                == "-3218372891372987328917389127389217398271983712987398127398172389712937819237");
        // Random testing.
        std::atomic<bool> fail(false);
        auto f = [&fail](unsigned n) {
            std::uniform_int_distribution<long> dist(detail::nl_min<long>(), detail::nl_max<long>());
            std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
            for (auto i = 0; i < ntries; ++i) {
                detail::mpz_raii mpz;
                auto tmp = dist(eng);
                mpz_set_si(&mpz.m_mpz, tmp);
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
        mpz_init(m0);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        REQUIRE(lex_cast(integer{std::move(m0)}) == "0");
        mpz_init(m0);
        mpz_set_si(m0, 1234);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        REQUIRE(lex_cast(integer{std::move(m0)}) == "1234");
        mpz_init(m0);
        mpz_set_si(m0, -1234);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        REQUIRE(lex_cast(integer{std::move(m0)}) == "-1234");
        mpz_init(m0);
        mpz_set_str(m0, "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        REQUIRE(lex_cast(integer{std::move(m0)})
                == "3218372891372987328917389127389217398271983712987398127398172389712937819237");
        mpz_init(m0);
        mpz_set_str(m0, "-3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        REQUIRE(lex_cast(integer{std::move(m0)})
                == "-3218372891372987328917389127389217398271983712987398127398172389712937819237");
        // Random testing.
        std::atomic<bool> fail(false);
        auto f = [&fail](unsigned n) {
            std::uniform_int_distribution<long> dist(detail::nl_min<long>(), detail::nl_max<long>());
            std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
            for (auto i = 0; i < ntries; ++i) {
                ::mpz_t m1;
                mpz_init(m1);
                auto tmp = dist(eng);
                mpz_set_si(m1, tmp);
                // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
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

template <typename I, typename Dest>
using binary_save_t = decltype(binary_save(std::declval<const I &>(), std::declval<Dest>()));

template <typename I, typename Dest>
using binary_load_t = decltype(binary_load(std::declval<I &>(), std::declval<Dest>()));

struct binary_s11n_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // MSVC (apart from clang-cl) has troubles with the detection idiom,
        // let's hope they fix SFINAE eventually.
#if !defined(_MSC_VER) || (defined(_MSC_VER) && defined(__clang__))
        REQUIRE(!detail::is_detected<binary_save_t, integer, std::vector<char>>::value);
        REQUIRE(!detail::is_detected<binary_save_t, integer, std::vector<char> &&>::value);
        REQUIRE(detail::is_detected<binary_save_t, integer, std::vector<char> &>::value);
        REQUIRE(!detail::is_detected<binary_save_t, integer, const std::vector<char>>::value);
        REQUIRE(!detail::is_detected<binary_save_t, integer, const std::vector<char> &>::value);
        REQUIRE(detail::is_detected<binary_save_t, integer, char *>::value);
        REQUIRE(detail::is_detected<binary_save_t, integer, char *&&>::value);
        REQUIRE(detail::is_detected<binary_save_t, integer, char *&>::value);
        REQUIRE(detail::is_detected<binary_save_t, integer, char *const &>::value);
        REQUIRE(!detail::is_detected<binary_save_t, integer, const char *>::value);
        REQUIRE(detail::is_detected<binary_load_t, integer, std::vector<char>>::value);
        REQUIRE(detail::is_detected<binary_load_t, integer, std::vector<char> &&>::value);
        REQUIRE(detail::is_detected<binary_load_t, integer, const std::vector<char>>::value);
        REQUIRE(detail::is_detected<binary_load_t, integer, const std::vector<char> &>::value);
        REQUIRE(!detail::is_detected<binary_load_t, const integer, std::vector<char>>::value);
#endif
        std::vector<char> buffer;
        std::stringstream ss;
        auto clear_ss = [&ss]() {
            ss.clear();
            ss.str("");
        };
        // A few tests with zero value.
        integer n1, n2, n3, n4, n5;
        REQUIRE(n1.binary_size() == sizeof(detail::mpz_size_t));
        buffer.resize(n1.binary_size());
        REQUIRE(n1.binary_save(buffer.data()) == n1.binary_size());
        REQUIRE(n1.binary_save(ss) == n1.binary_size());
        REQUIRE(ss.good());
        detail::mpz_size_t size = -1;
        std::copy(buffer.data(), buffer.data() + sizeof(detail::mpz_size_t),
                  detail::make_uai(reinterpret_cast<char *>(&size)));
        REQUIRE(size == 0);
        REQUIRE(n2.binary_load(buffer.data()) == n1.binary_size());
        REQUIRE(n2 == 0);
        REQUIRE(n2.is_static());
        n5 = 1;
        REQUIRE(n5.binary_load(ss) == n1.binary_size());
        REQUIRE(ss.good());
        REQUIRE(n5 == 0);
        REQUIRE(n5.is_static());
        clear_ss();
        n1.promote();
        REQUIRE(n1.binary_save(buffer.data()) == n1.binary_size());
        REQUIRE(n1.binary_save(ss) == n1.binary_size());
        REQUIRE(ss.good());
        size = -1;
        std::copy(buffer.data(), buffer.data() + sizeof(detail::mpz_size_t),
                  detail::make_uai(reinterpret_cast<char *>(&size)));
        REQUIRE(size == 0);
        n2.promote();
        REQUIRE(n2.binary_load(buffer.data()) == n1.binary_size());
        REQUIRE(n2 == 0);
        REQUIRE(n2.is_static());
        n5 = 1;
        n5.promote();
        REQUIRE(n5.binary_load(ss) == n1.binary_size());
        REQUIRE(ss.good());
        REQUIRE(n5 == 0);
        REQUIRE(n5.is_static());
        clear_ss();
        detail::mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Buffers based on std::vector and std::array.
        std::vector<char> vbuffer;
        // Make space for plenty of limbs.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
        std::array<char, sizeof(detail::mpz_size_t) + sizeof(::mp_limb_t) * 100u> sb;
        // Run a variety of tests with operands with x and y number of limbs.
        auto random_xy = [&](unsigned x, unsigned y) {
            for (int i = 0; i < ntries; ++i) {
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset every once in a while.
                    n1 = integer{};
                }
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset every once in a while.
                    n2 = integer{};
                }
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset every once in a while.
                    vbuffer = std::vector<char>{};
                }
                // Create two random integers with x and y limbs.
                random_integer(tmp, x, rng);
                n1 = &tmp.m_mpz;
                random_integer(tmp, y, rng);
                n2 = &tmp.m_mpz;
                // Negate and/or promote randomly.
                if (sdist(rng)) {
                    n1.neg();
                }
                if (sdist(rng)) {
                    n2.neg();
                }
                if (n1.is_static() && sdist(rng)) {
                    n1.promote();
                }
                if (n2.is_static() && sdist(rng)) {
                    n2.promote();
                }
                // Copy over n2 to n3, n4 and n5.
                n3 = n2;
                n4 = n2;
                n5 = n2;
                REQUIRE(n1.binary_size() == binary_size(n1));
                // Save n1 into the buffer, load the buffer into n2.
                // Then check we get the same data back.
                buffer.resize(n1.binary_size());
                REQUIRE(binary_save(n1, buffer.data()) == n1.binary_size());
                REQUIRE(binary_load(n2, buffer.data()) == n1.binary_size());
                REQUIRE(n1 == n2);
                REQUIRE(n2.is_static() == (n1.size() <= S::value));
                // Test the std vector interface as well.
                REQUIRE(binary_save(n1, vbuffer) == n1.binary_size());
                REQUIRE(binary_load(n3, vbuffer) == n1.binary_size());
                REQUIRE(n1 == n3);
                REQUIRE(n3.is_static() == (n1.size() <= S::value));
                // std::array interface.
                REQUIRE(binary_save(n1, sb) == n1.binary_size());
                REQUIRE(binary_load(n4, sb) == n1.binary_size());
                REQUIRE(n1 == n4);
                REQUIRE(n4.is_static() == (n1.size() <= S::value));
                // Stream interface.
                REQUIRE(binary_save(n1, ss) == n1.binary_size());
                REQUIRE(binary_load(n5, ss) == n1.binary_size());
                REQUIRE(n1 == n5);
                REQUIRE(n5.is_static() == (n1.size() <= S::value));
                clear_ss();
            }
        };

        random_xy(1, 0);
        random_xy(0, 1);
        random_xy(1, 1);

        random_xy(0, 2);
        random_xy(1, 2);
        random_xy(2, 0);
        random_xy(2, 1);
        random_xy(2, 2);

        random_xy(0, 3);
        random_xy(1, 3);
        random_xy(2, 3);
        random_xy(3, 0);
        random_xy(3, 1);
        random_xy(3, 2);
        random_xy(3, 3);

        random_xy(0, 4);
        random_xy(1, 4);
        random_xy(2, 4);
        random_xy(3, 4);
        random_xy(4, 0);
        random_xy(4, 1);
        random_xy(4, 2);
        random_xy(4, 3);
        random_xy(4, 4);

        // Error checking.
        // Load into a static int a static value with topmost limb 0.
        n1 = integer{-1};
        size = 1;
        buffer.resize(sizeof(size) + sizeof(::mp_limb_t) * unsigned(size));
        ::mp_limb_t l = 0;
        std::copy(reinterpret_cast<char *>(&size), reinterpret_cast<char *>(&size) + sizeof(size),
                  detail::make_uai(buffer.data()));
        std::copy(reinterpret_cast<char *>(&l), reinterpret_cast<char *>(&l) + sizeof(l),
                  detail::make_uai(buffer.data() + sizeof(size)));
        REQUIRE_THROWS_PREDICATE(
            n1.binary_load(buffer.data()), std::invalid_argument, [](const std::invalid_argument &ex) {
                return std::string(ex.what())
                       == "Invalid data detected in the binary deserialisation of an integer: the most "
                          "significant limb of the value cannot be zero";
            });
        REQUIRE(n1 == 0);

        // Load into a static int a dynamic value with topmost limb 0.
        n1 = integer{-1};
        size = static_cast<detail::mpz_size_t>(S::value + 1u);
        buffer.resize(sizeof(size) + sizeof(::mp_limb_t) * unsigned(size));
        std::array<::mp_limb_t, S::value + 1u> sbuffer{{}};
        std::copy(reinterpret_cast<char *>(&size), reinterpret_cast<char *>(&size) + sizeof(size),
                  detail::make_uai(buffer.data()));
        std::copy(reinterpret_cast<char *>(sbuffer.data()),
                  reinterpret_cast<char *>(sbuffer.data()) + sizeof(::mp_limb_t) * unsigned(size),
                  detail::make_uai(buffer.data() + sizeof(size)));
        REQUIRE_THROWS_PREDICATE(
            n1.binary_load(buffer.data()), std::invalid_argument, [](const std::invalid_argument &ex) {
                return std::string(ex.what())
                       == "Invalid data detected in the binary deserialisation of an integer: the most "
                          "significant limb of the value cannot be zero";
            });
        REQUIRE(n1 == 0);

        // Load into a dynamic int a static value with topmost limb 0.
        n1 = integer{-1};
        n1.promote();
        size = 1;
        buffer.resize(sizeof(size) + sizeof(::mp_limb_t) * unsigned(size));
        l = 0;
        std::copy(reinterpret_cast<char *>(&size), reinterpret_cast<char *>(&size) + sizeof(size),
                  detail::make_uai(buffer.data()));
        std::copy(reinterpret_cast<char *>(&l), reinterpret_cast<char *>(&l) + sizeof(l),
                  detail::make_uai(buffer.data() + sizeof(size)));
        REQUIRE_THROWS_PREDICATE(
            n1.binary_load(buffer.data()), std::invalid_argument, [](const std::invalid_argument &ex) {
                return std::string(ex.what())
                       == "Invalid data detected in the binary deserialisation of an integer: the most "
                          "significant limb of the value cannot be zero";
            });
        REQUIRE(n1 == 0);

        // Load into a dynamic int a dynamic value with topmost limb 0.
        n1 = integer{-1};
        n1.promote();
        size = static_cast<detail::mpz_size_t>(S::value + 1u);
        buffer.resize(sizeof(size) + sizeof(::mp_limb_t) * unsigned(size));
        sbuffer = std::array<::mp_limb_t, S::value + 1u>{{}};
        std::copy(reinterpret_cast<char *>(&size), reinterpret_cast<char *>(&size) + sizeof(size),
                  detail::make_uai(buffer.data()));
        std::copy(reinterpret_cast<char *>(sbuffer.data()),
                  reinterpret_cast<char *>(sbuffer.data()) + sizeof(::mp_limb_t) * unsigned(size),
                  detail::make_uai(buffer.data() + sizeof(size)));
        REQUIRE_THROWS_PREDICATE(
            n1.binary_load(buffer.data()), std::invalid_argument, [](const std::invalid_argument &ex) {
                return std::string(ex.what())
                       == "Invalid data detected in the binary deserialisation of an integer: the most "
                          "significant limb of the value cannot be zero";
            });
        REQUIRE(n1 == 0);

        // Test errors in the vector and array interfaces.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
        std::array<char, 0> zero_arr;
        REQUIRE(binary_save(n1, zero_arr) == 0u);
        std::vector<char> zero_vec;
        REQUIRE_THROWS_PREDICATE(binary_load(n1, zero_arr), std::invalid_argument, [](const std::invalid_argument &ex) {
            return std::string(ex.what())
                   == "Invalid vector size in the deserialisation of an integer via a std::array: the std::array "
                      "size must be at least "
                          + std::to_string(sizeof(detail::mpz_size_t)) + " bytes, but it is only 0 bytes";
        });
        REQUIRE_THROWS_PREDICATE(binary_load(n1, zero_vec), std::invalid_argument, [](const std::invalid_argument &ex) {
            return std::string(ex.what())
                   == "Invalid vector size in the deserialisation of an integer via a std::vector: the std::vector "
                      "size must be at least "
                          + std::to_string(sizeof(detail::mpz_size_t)) + " bytes, but it is only 0 bytes";
        });
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
        std::array<char, sizeof(detail::mpz_size_t) + sizeof(::mp_limb_t) * 2u> small_arr;
        const detail::mpz_size_t tmp_size = 3;
        std::copy(reinterpret_cast<const char *>(&tmp_size),
                  reinterpret_cast<const char *>(&tmp_size) + sizeof(detail::mpz_size_t),
                  detail::make_uai(small_arr.data()));
        REQUIRE_THROWS_PREDICATE(binary_load(n1, small_arr), std::invalid_argument,
                                 [](const std::invalid_argument &ex) {
                                     return std::string(ex.what())
                                            == "Invalid vector size in the deserialisation of an integer via a "
                                               "std::array: the number of limbs stored in the std::array (2) is less "
                                               "than the integer size in limbs stored in the header of the vector (3)";
                                 });
        std::vector<char> small_vec;
        small_vec.resize(sizeof(detail::mpz_size_t) + sizeof(::mp_limb_t) * 2u);
        std::copy(reinterpret_cast<const char *>(&tmp_size),
                  reinterpret_cast<const char *>(&tmp_size) + sizeof(detail::mpz_size_t),
                  detail::make_uai(small_vec.data()));
        REQUIRE_THROWS_PREDICATE(binary_load(n1, small_vec), std::invalid_argument,
                                 [](const std::invalid_argument &ex) {
                                     return std::string(ex.what())
                                            == "Invalid vector size in the deserialisation of an integer via a "
                                               "std::vector: the number of limbs stored in the std::vector (2) is less "
                                               "than the integer size in limbs stored in the header of the vector (3)";
                                 });

        // Test errors in the stream interface.
        clear_ss();
        n1 = 4;
        REQUIRE(binary_load(n1, ss) == 0u);
        REQUIRE(n1 == 4);
        clear_ss();
        size = 1;
        ss.write(reinterpret_cast<const char *>(&size), sizeof(detail::mpz_size_t));
        REQUIRE(binary_load(n1, ss) == 0u);
        REQUIRE(n1 == 4);
        clear_ss();
        ss.setstate(std::ios_base::failbit);
        REQUIRE(binary_save(n1, ss) == 0u);
    }
};

TEST_CASE("binary s11n")
{
    tuple_for_each(sizes{}, binary_s11n_tester{});
}

#if MPPP_CPLUSPLUS >= 201703L

TEST_CASE("integer nts")
{
    REQUIRE(std::is_nothrow_swappable_v<integer<1>>);
    REQUIRE(std::is_nothrow_swappable_v<integer<2>>);
    REQUIRE(std::is_nothrow_swappable_v<integer<6>>);
    REQUIRE(std::is_nothrow_swappable_v<integer<10>>);
    REQUIRE(std::is_nothrow_swappable_v<integer<15>>);
}

#endif

#if defined(MPPP_WITH_BOOST_S11N)

template <typename T, typename OA, typename IA>
void test_s11n()
{
    std::stringstream ss;

    auto x = T{-42};
    {
        OA oa(ss);
        oa << x;
    }

    x = 0;
    {
        IA ia(ss);
        ia >> x;
    }

    REQUIRE(x == -42);
}

struct boost_s11n_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using integer = integer<S::value>;

        test_s11n<integer, boost::archive::text_oarchive, boost::archive::text_iarchive>();
        test_s11n<integer, boost::archive::binary_oarchive, boost::archive::binary_iarchive>();
    }
};

TEST_CASE("boost_s11n")
{
    tuple_for_each(sizes{}, boost_s11n_tester{});
}

#endif
