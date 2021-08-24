// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <atomic>
#include <cmath>
#include <complex>
#include <cstddef>
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

#if defined(MPPP_WITH_BOOST_S11N)

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

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

// A seed that will be used to init rngs in the multithreaded tests. Each time a batch of N threads
// finishes, this value gets bumped up by N, so that the next time a multithreaded test which uses rng
// is launched it will be inited with a different seed.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static std::mt19937::result_type mt_rng_seed(0u);

static const int ntries = 1000;

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
#if defined(MPPP_HAVE_STRING_VIEW)
        q = std::string_view{"1"};
        REQUIRE(q == 1);
        q = std::string_view{"-23"};
        REQUIRE(q == -23);
        q = std::string_view{"-2/-4"};
        REQUIRE((q == rational{1, 2}));
        q = std::string_view{"3/-9"};
        REQUIRE((q == rational{-1, 3}));
        REQUIRE_THROWS_PREDICATE(q = std::string_view{""}, std::invalid_argument, [](const std::invalid_argument &ia) {
            return std::string(ia.what()) == "The string '' is not a valid integer in base 10";
        });
        REQUIRE_THROWS_PREDICATE(q = std::string_view{"-3/0"}, zero_division_error, [](const zero_division_error &ia) {
            return std::string(ia.what())
                   == "A zero denominator was detected in the constructor of a rational from string";
        });
#endif
    }
};

TEST_CASE("string ass")
{
    tuple_for_each(sizes{}, string_ass_tester{});
}

struct mpq_copy_ass_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        REQUIRE((std::is_assignable<rational &, ::mpq_t>::value));
        REQUIRE((!std::is_assignable<const rational &, ::mpq_t>::value));
        rational q;
        detail::mpq_raii m;
        q = &m.m_mpq;
        REQUIRE(lex_cast(q) == "0");
        mpq_set_si(&m.m_mpq, 1234, 1);
        q = &m.m_mpq;
        REQUIRE(lex_cast(q) == "1234");
        mpq_set_si(&m.m_mpq, -1234, 1);
        q = &m.m_mpq;
        REQUIRE(lex_cast(q) == "-1234");
        mpq_set_str(&m.m_mpq, "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        q = &m.m_mpq;
        REQUIRE(lex_cast(q) == "3218372891372987328917389127389217398271983712987398127398172389712937819237");
        mpq_set_str(&m.m_mpq, "-3218372891372987328917389127389217398271983712987398127398172389712937819237/2", 10);
        q = &m.m_mpq;
        REQUIRE(lex_cast(q) == "-3218372891372987328917389127389217398271983712987398127398172389712937819237/2");
    }
};

TEST_CASE("mpq_t copy assignment")
{
    tuple_for_each(sizes{}, mpq_copy_ass_tester{});
}

#if !defined(_MSC_VER)

struct mpq_move_ass_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        REQUIRE((std::is_assignable<rational &, ::mpq_t &&>::value));
        REQUIRE((!std::is_assignable<const rational &, ::mpq_t &&>::value));
        rational q;
        ::mpq_t q0;
        mpq_init(q0);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        q = std::move(q0);
        REQUIRE(lex_cast(q) == "0");
        mpq_init(q0);
        mpq_set_si(q0, 1234, 1);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        q = std::move(q0);
        REQUIRE(lex_cast(q) == "1234");
        mpq_init(q0);
        mpq_set_si(q0, -1234, 1);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        q = std::move(q0);
        REQUIRE(lex_cast(q) == "-1234");
        mpq_init(q0);
        mpq_set_str(q0, "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        q = std::move(q0);
        REQUIRE(lex_cast(q) == "3218372891372987328917389127389217398271983712987398127398172389712937819237");
        mpq_init(q0);
        mpq_set_str(q0, "-3218372891372987328917389127389217398271983712987398127398172389712937819237/2", 10);
        // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
        q = std::move(q0);
        REQUIRE(lex_cast(q) == "-3218372891372987328917389127389217398271983712987398127398172389712937819237/2");
    }
};

TEST_CASE("mpq_t move assignment")
{
    tuple_for_each(sizes{}, mpq_move_ass_tester{});
}

#endif

struct mpz_ass_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;
        using integer = integer<S::value>;
        REQUIRE((std::is_assignable<rational &, ::mpz_t>::value));
        REQUIRE((!std::is_assignable<const rational &, ::mpz_t>::value));
        rational q{6, 5};
        detail::mpz_raii m;
        mpz_set_si(&m.m_mpz, 1234);
        q = &m.m_mpz;
        REQUIRE(q.get_num() == 1234);
        REQUIRE(q.get_num().is_static());
        REQUIRE(q.get_den() == 1);
        REQUIRE(q.get_den().is_static());
        q = "-7/3";
        mpz_set_si(&m.m_mpz, -1234);
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
        mpz_set_str(&m.m_mpz, "3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
        q = &m.m_mpz;
        REQUIRE(q.get_num() == integer{"3218372891372987328917389127389217398271983712987398127398172389712937819237"});
        REQUIRE(q.get_den() == 1);
        REQUIRE(q.get_den().is_static());
        mpz_set_str(&m.m_mpz, "-3218372891372987328917389127389217398271983712987398127398172389712937819237", 10);
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
        q = static_cast<signed char>(-11);
        REQUIRE(lex_cast(q) == "-11");
        q = integer{"-2323232312312311"};
        REQUIRE(lex_cast(q) == "-2323232312312311");
        integer tmp_int{"-4323232312312311"};
        q = tmp_int;
        REQUIRE(lex_cast(q) == "-4323232312312311");
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
        q = std::complex<float>{-42, 0};
        REQUIRE(q == -42);
        REQUIRE_THROWS_PREDICATE(q = std::complex<float>(0, 1), std::domain_error, [](const std::domain_error &ex) {
            return ex.what()
                   == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                          + detail::to_string(1.f);
        });

        q = std::complex<double>{-43, 0};
        REQUIRE(q == -43);
        REQUIRE_THROWS_PREDICATE(q = std::complex<double>(0, 1), std::domain_error, [](const std::domain_error &ex) {
            return ex.what()
                   == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                          + detail::to_string(1.);
        });

#if defined(MPPP_WITH_MPFR)
        q = std::complex<long double>{-44, 0};
        REQUIRE(q == -44);
        REQUIRE_THROWS_PREDICATE(
            q = std::complex<long double>(0, 1), std::domain_error, [](const std::domain_error &ex) {
                return ex.what()
                       == "Cannot construct a rational from a complex C++ value with a non-zero imaginary part of "
                              + detail::to_string(1.l);
            });
#endif

#if defined(MPPP_HAVE_GCC_INT128)
        q = __int128{-42};
        REQUIRE(q == -42);
        q = __uint128_t{84};
        REQUIRE(q == 84);
#endif

#if defined(MPPP_WITH_QUADMATH)
        q = real128{123};
        REQUIRE(q == 123);
        q = real128{"-1.5"};
        REQUIRE(q == rational{3, -2});
        REQUIRE(std::is_same<decltype(q = real128{"-1.5"}), rational &>::value);
#endif

#if defined(MPPP_WITH_MPFR)
        q = real{42};
        REQUIRE(q == 42);
        q = real{"-457.5", 100};
        REQUIRE(q == rational{-915, 2});
        REQUIRE(std::is_same<decltype(q = real{"-457.5", 100}), rational &>::value);
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
            using rational = rational<S::value>;
            using integer = typename rational::int_t;
            REQUIRE((is_convertible<rational, Int>::value));
            REQUIRE(roundtrip_conversion<rational>(0));
            auto constexpr min = detail::nl_min<Int>(), max = detail::nl_max<Int>();
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
            Int rop(1);
            if (min != Int(0)) {
                REQUIRE(static_cast<Int>(rational{integer(min) * 3, integer(min) * -2}) == Int(-1));
                REQUIRE((rational{integer(min) * 3, integer(min) * -2}.get(rop)));
                REQUIRE((get(rop, rational{integer(min) * 3, integer(min) * -2})));
                REQUIRE(rop == Int(-1));
            }
            REQUIRE(static_cast<Int>(rational{integer(max) * 5, integer(max) * 2}) == Int(2));
            REQUIRE((rational{integer(max) * 5, integer(max) * 2}.get(rop)));
            REQUIRE((get(rop, rational{integer(max) * 5, integer(max) * 2})));
            REQUIRE(rop == Int(2));
            rop = Int(1);
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(integer(min) * 2, 2) - 1), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE((!(rational(integer(min) * 2, 2) - 1).get(rop)));
            REQUIRE((!get(rop, (rational(integer(min) * 2, 2) - 1))));
            REQUIRE(rop == Int(1));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(min) - 1), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE((!(rational(min) - 1).get(rop)));
            REQUIRE((!get(rop, (rational(min) - 1))));
            REQUIRE(rop == Int(1));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(min) - 2), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE((!(rational(min) - 2).get(rop)));
            REQUIRE((!get(rop, (rational(min) - 2))));
            REQUIRE(rop == Int(1));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(min) - 3), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE((!(rational(min) - 3).get(rop)));
            REQUIRE((!get(rop, (rational(min) - 3))));
            REQUIRE(rop == Int(1));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(min) - 123), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE((!(rational(min) - 123).get(rop)));
            REQUIRE((!get(rop, (rational(min) - 123))));
            REQUIRE(rop == Int(1));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(max) + 1), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE((!(rational(max) + 1).get(rop)));
            REQUIRE((!get(rop, (rational(max) + 1))));
            REQUIRE(rop == Int(1));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(max) + 2), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE((!(rational(max) + 2).get(rop)));
            REQUIRE((!get(rop, (rational(max) + 2))));
            REQUIRE(rop == Int(1));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(max) + 3), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE((!(rational(max) + 3).get(rop)));
            REQUIRE((!get(rop, (rational(max) + 3))));
            REQUIRE(rop == Int(1));
            REQUIRE_THROWS_PREDICATE(static_cast<Int>(rational(max) + 123), std::overflow_error,
                                     [](const std::overflow_error &) { return true; });
            REQUIRE((!(rational(max) + 123).get(rop)));
            REQUIRE((!get(rop, (rational(max) + 123))));
            REQUIRE(rop == Int(1));
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
        REQUIRE((!is_convertible<rational, no_conv>::value));
        // Conversion to int_t.
        integer rop;
        REQUIRE((is_convertible<rational, integer>::value));
        REQUIRE(roundtrip_conversion<rational>(integer{42}));
        REQUIRE(roundtrip_conversion<rational>(integer{-42}));
        REQUIRE(static_cast<integer>(rational{1, 2}) == 0);
        REQUIRE((rational{1, 2}.get(rop)));
        REQUIRE((get(rop, rational{1, 2})));
        REQUIRE(rop == 0);
        REQUIRE(static_cast<integer>(rational{3, 2}) == 1);
        REQUIRE((rational{3, 2}.get(rop)));
        REQUIRE((get(rop, rational{3, 2})));
        REQUIRE(rop == 1);
        REQUIRE(static_cast<integer>(rational{3, -2}) == -1);
        REQUIRE((rational{3, -2}.get(rop)));
        REQUIRE((get(rop, rational{3, -2})));
        REQUIRE(rop == -1);
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
            Float rop(1);
            REQUIRE((is_convertible<rational, Float>::value));
            REQUIRE(static_cast<Float>(rational{0}) == Float(0));
            REQUIRE(rational{0}.get(rop));
            REQUIRE(get(rop, rational{0}));
            REQUIRE(rop == Float(0));
            REQUIRE(static_cast<Float>(rational{1}) == Float(1));
            REQUIRE(rational{1}.get(rop));
            REQUIRE(get(rop, rational{1}));
            REQUIRE(rop == Float(1));
            REQUIRE(static_cast<Float>(rational{-1}) == Float(-1));
            REQUIRE(rational{-1}.get(rop));
            REQUIRE(get(rop, rational{-1}));
            REQUIRE(rop == Float(-1));
            REQUIRE(static_cast<Float>(rational{12}) == Float(12));
            REQUIRE(rational{12}.get(rop));
            REQUIRE(get(rop, rational{12}));
            REQUIRE(rop == Float(12));
            REQUIRE(static_cast<Float>(rational{-12}) == Float(-12));
            REQUIRE(rational{-12}.get(rop));
            REQUIRE(get(rop, rational{-12}));
            REQUIRE(rop == Float(-12));
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
                        Float rop1;
                        const auto tmp = dist(eng);
                        if (!roundtrip_conversion<rational>(tmp)) {
                            fail.store(false);
                        }
                        if (!rational{tmp}.get(rop1)) {
                            fail.store(false);
                        }
                        if (!get(rop1, rational{tmp})) {
                            fail.store(false);
                        }
                        if (rop1 != std::trunc(tmp)) {
                            fail.store(false);
                        }
                    }
                }
                {
                    std::uniform_real_distribution<Float> dist(Float(-1E-9), Float(1E-9));
                    std::mt19937 eng(static_cast<std::mt19937::result_type>(n + mt_rng_seed));
                    for (auto i = 0; i < ntries; ++i) {
                        Float rop1;
                        const auto tmp = dist(eng);
                        if (!roundtrip_conversion<rational>(tmp)) {
                            fail.store(false);
                        }
                        if (!rational{tmp}.get(rop1)) {
                            fail.store(false);
                        }
                        if (!get(rop1, rational{tmp})) {
                            fail.store(false);
                        }
                        if (rop1 != std::trunc(tmp)) {
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

struct complex_convert_tester {
    template <typename S>
    struct runner {
        template <typename C>
        void operator()(const C &) const
        {
            using rational = rational<S::value>;
            using Float = typename C::value_type;

            // Type traits.
            REQUIRE((is_convertible<rational, C>::value));
            REQUIRE((is_convertible<C, rational>::value));

            C rop{1, 2};

            REQUIRE(static_cast<C>(rational{0}) == C{});
            REQUIRE(static_cast<C>(rational{123}) == C{Float(123)});
            REQUIRE(static_cast<C>(rational{-45}) == C{Float(-45)});

            REQUIRE(rational{0}.get(rop));
            REQUIRE(rop == C{});
            REQUIRE(rational{123}.get(rop));
            REQUIRE(rop == C{Float(123)});
            REQUIRE(rational{-45}.get(rop));
            REQUIRE(rop == C{Float(-45)});

            REQUIRE(get(rop, rational{0}));
            REQUIRE(rop == C{});
            REQUIRE(get(rop, rational{123}));
            REQUIRE(rop == C{Float(123)});
            REQUIRE(get(rop, rational{-45}));
            REQUIRE(rop == C{Float(-45)});

            // Functional cast form from rational to C.
            REQUIRE(C(rational{}) == C{0, 0});
            REQUIRE(C(rational{-37}) == C{-37, 0});
            REQUIRE(C(rational{42}) == C{42, 0});
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
        REQUIRE(&canonicalise(q) == &q);
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
    }
};

TEST_CASE("stream")
{
    tuple_for_each(sizes{}, stream_tester{});
}

#if MPPP_CPLUSPLUS >= 201703L

TEST_CASE("rational nts")
{
    REQUIRE(std::is_nothrow_swappable_v<rational<1>>);
    REQUIRE(std::is_nothrow_swappable_v<rational<2>>);
    REQUIRE(std::is_nothrow_swappable_v<rational<6>>);
    REQUIRE(std::is_nothrow_swappable_v<rational<10>>);
    REQUIRE(std::is_nothrow_swappable_v<rational<15>>);
}

#endif

#if defined(MPPP_WITH_BOOST_S11N)

template <typename T, typename OA, typename IA>
void test_s11n()
{
    std::stringstream ss;

    auto x = T{-42} / 13;
    {
        OA oa(ss);
        oa << x;
    }

    x = 0;
    {
        IA ia(ss);
        ia >> x;
    }

    REQUIRE(x == -42 / T{13});
}

struct boost_s11n_tester {
    template <typename S>
    void operator()(const S &) const
    {
        using rational = rational<S::value>;

        test_s11n<rational, boost::archive::text_oarchive, boost::archive::text_iarchive>();
        test_s11n<rational, boost::archive::binary_oarchive, boost::archive::binary_iarchive>();
    }
};

TEST_CASE("boost_s11n")
{
    tuple_for_each(sizes{}, boost_s11n_tester{});
}

#endif
