// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <random>
#include <stdexcept>
#include <tuple>
#include <type_traits>

#include <gmp.h>

#include <mp++/config.hpp>
#include <mp++/integer.hpp>

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

// NOLINTNEXTLINE(cert-err58-cpp, cert-msc32-c, cert-msc51-cpp, cppcoreguidelines-avoid-non-const-global-variables)
static std::mt19937 rng;

struct pow_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        // Start with all zeroes.
        detail::mpz_raii m1, m2;
        integer n1, n2;
        mpz_pow_ui(&m1.m_mpz, &m2.m_mpz, 0u);
        REQUIRE(&pow_ui(n1, n2, 0) == &n1);
        REQUIRE((lex_cast(n1) == lex_cast(m1)));
        REQUIRE((lex_cast(pow_ui(n2, 0)) == lex_cast(m1)));
        REQUIRE(n1.is_static());
        detail::mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        std::uniform_int_distribution<unsigned> edist(0, 20);
        // Run a variety of tests with operands with x number of limbs.
        auto random_xy = [&](unsigned x) {
            for (int i = 0; i < ntries; ++i) {
                // NOLINTNEXTLINE(misc-redundant-expression)
                if (sdist(rng) && sdist(rng) && sdist(rng)) {
                    // Reset rop every once in a while.
                    n1 = integer{};
                }
                random_integer(tmp, x, rng);
                mpz_set(&m2.m_mpz, &tmp.m_mpz);
                n2 = integer(detail::mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    mpz_neg(&m2.m_mpz, &m2.m_mpz);
                    n2.neg();
                }
                if (n2.is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2.promote();
                }
                const unsigned ex = edist(rng);
                mpz_pow_ui(&m1.m_mpz, &m2.m_mpz, ex);
                pow_ui(n1, n2, ex);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((lex_cast(pow_ui(n2, ex)) == lex_cast(m1)));
                // Overlap.
                mpz_pow_ui(&m2.m_mpz, &m2.m_mpz, ex);
                pow_ui(n2, n2, ex);
                REQUIRE((lex_cast(n2) == lex_cast(m2)));
            }
        };

        random_xy(0);
        random_xy(1);
        random_xy(2);
        random_xy(3);
        random_xy(4);

        // Tests for the convenience pow() overloads.
        REQUIRE(pow(integer{0}, 0) == 1);
        REQUIRE(pow(integer{0}, false) == 1);
        REQUIRE(pow(integer{3}, true) == 3);
        REQUIRE(pow(0, integer{0}) == 1);
        REQUIRE((std::is_same<integer, decltype(pow(integer{0}, 0))>::value));
        REQUIRE((std::is_same<integer, decltype(pow(0, integer{0}))>::value));
        REQUIRE(pow(integer{4}, 2) == 16);
        REQUIRE(pow(2, integer{4}) == 16);
        REQUIRE(pow(integer{4}, char(0)) == 1);
        REQUIRE(pow(char(4), integer{0}) == 1);
        REQUIRE((std::is_same<integer, decltype(pow(integer{0}, char(0)))>::value));
        REQUIRE((std::is_same<integer, decltype(pow(char(0), integer{0}))>::value));
        REQUIRE(pow(integer{4}, 3ull) == 64);
        REQUIRE(pow(4ull, integer{3}) == 64);
        REQUIRE((std::is_same<integer, decltype(pow(integer{0}, 0ull))>::value));
        REQUIRE((std::is_same<integer, decltype(pow(0ull, integer{0}))>::value));
        REQUIRE(pow(integer{4}, integer{4}) == 256);
        REQUIRE((std::is_same<integer, decltype(pow(integer{0}, integer{0}))>::value));
        REQUIRE(pow(integer{-4}, 2) == 16);
        REQUIRE(pow(-4, integer{2}) == 16);
        REQUIRE(pow(integer{-4}, char(0)) == 1);
        REQUIRE(pow(static_cast<signed char>(-4), integer{0}) == 1);
        REQUIRE(pow(integer{-4}, 3ull) == -64);
        REQUIRE(pow(integer{-4}, integer{4}) == 256);
        if (std::numeric_limits<unsigned long long>::max() > std::numeric_limits<unsigned long>::max()) {
            REQUIRE_THROWS_PREDICATE(pow(integer{-4}, std::numeric_limits<unsigned long long>::max()),
                                     std::overflow_error, [](const std::overflow_error &oe) {
                                         return oe.what()
                                                == "Cannot convert the integral value "
                                                       + std::to_string(std::numeric_limits<unsigned long long>::max())
                                                       + " to unsigned long: the value is too large";
                                     });
            REQUIRE_THROWS_PREDICATE(pow(integer{-4}, integer{std::numeric_limits<unsigned long long>::max()}),
                                     std::overflow_error, [](const std::overflow_error &oe) {
                                         return oe.what()
                                                == "Cannot convert the integral value "
                                                       + std::to_string(std::numeric_limits<unsigned long long>::max())
                                                       + " to unsigned long: the value is too large";
                                     });
            REQUIRE_THROWS_PREDICATE(pow(-4, integer{std::numeric_limits<unsigned long long>::max()}),
                                     std::overflow_error, [](const std::overflow_error &oe) {
                                         return oe.what()
                                                == "Cannot convert the integral value "
                                                       + std::to_string(std::numeric_limits<unsigned long long>::max())
                                                       + " to unsigned long: the value is too large";
                                     });
        }
        REQUIRE_THROWS_PREDICATE(pow(integer{0}, -1), zero_division_error, [](const zero_division_error &zde) {
            return zde.what() == std::string("Cannot raise zero to the negative power -1");
        });
        REQUIRE_THROWS_PREDICATE(pow(integer{0}, -2ll), zero_division_error, [](const zero_division_error &zde) {
            return zde.what() == std::string("Cannot raise zero to the negative power -2");
        });
        REQUIRE_THROWS_PREDICATE(pow(integer{0}, integer{-25}), zero_division_error,
                                 [](const zero_division_error &zde) {
                                     return zde.what() == std::string("Cannot raise zero to the negative power -25");
                                 });
        REQUIRE_THROWS_PREDICATE(pow(0, integer{-1}), zero_division_error, [](const zero_division_error &zde) {
            return zde.what() == std::string("Cannot raise zero to the negative power -1");
        });
        REQUIRE_THROWS_PREDICATE(pow(0ll, integer{-2ll}), zero_division_error, [](const zero_division_error &zde) {
            return zde.what() == std::string("Cannot raise zero to the negative power -2");
        });
        // 1 to negative exp.
        REQUIRE(pow(integer{1}, -1) == 1);
        REQUIRE(pow(1, integer{-1}) == 1);
        REQUIRE(pow(integer{1}, static_cast<signed char>(-2)) == 1);
        REQUIRE(pow(char(1), integer{-2}) == 1);
        REQUIRE(pow(integer{1}, -3ll) == 1);
        REQUIRE(pow(1ll, integer{-3ll}) == 1);
        REQUIRE(pow(integer{1}, integer{-4ll}) == 1);
        // -1 to negative exp.
        REQUIRE(pow(integer{-1}, -1) == -1);
        REQUIRE(pow(integer{-1}, static_cast<signed char>(-2)) == 1);
        REQUIRE(pow(integer{-1}, -3ll) == -1);
        REQUIRE(pow(-1, integer{-1}) == -1);
        REQUIRE(pow(-1, integer{-2}) == 1);
        REQUIRE(pow(-1, integer{-3ll}) == -1);
        REQUIRE(pow(integer{-1}, integer{-4ll}) == 1);
        // n to negative exp.
        REQUIRE(pow(integer{2}, -1) == 0);
        REQUIRE(pow(integer{-3}, static_cast<signed char>(-2)) == 0);
        REQUIRE(pow(integer{4}, -3ll) == 0);
        REQUIRE(pow(2, integer{-1}) == 0);
        REQUIRE(pow(static_cast<signed char>(-3), integer{-2}) == 0);
        REQUIRE(pow(4, integer{-3ll}) == 0);
        REQUIRE(pow(integer{-5}, integer{-4}) == 0);
        // FP testing.
        REQUIRE((std::is_same<float, decltype(pow(integer{}, 0.f))>::value));
        REQUIRE((std::is_same<float, decltype(pow(0.f, integer{}))>::value));
        REQUIRE((std::is_same<double, decltype(pow(integer{}, 0.))>::value));
        REQUIRE((std::is_same<double, decltype(pow(0., integer{}))>::value));
#if defined(MPPP_WITH_MPFR)
        REQUIRE((std::is_same<long double, decltype(pow(integer{}, 0.l))>::value));
        REQUIRE((std::is_same<long double, decltype(pow(0.l, integer{}))>::value));
#endif
        REQUIRE(pow(integer{2}, 4.5f) == std::pow(2.f, 4.5f));
        REQUIRE(pow(4.5f, integer{-2}) == std::pow(4.5f, -2.f));
        REQUIRE(pow(integer{2}, 4.5) == std::pow(2., 4.5));
        REQUIRE(pow(4.5, integer{-2}) == std::pow(4.5, -2.));
        // NOTE: in FreeBSD, the powl() function currently computes
        // the result in double precision, rather than long double precision.
        // However, in Release builds, std::pow(2.l, 4.5l) is actually
        // computed in effectively long double precision at compile time
        // by the compiler (i.e., the runtime powl() is bypassed and not actually
        // invoked). This leads to inconsistencies in the two tests below,
        // so we just disable them in FreeBSD. See also:
        // https://github.com/bluescarni/mppp/issues/132
        // NOTE: the same problem seems to happen on aarch64.
#if defined(MPPP_WITH_MPFR) && !defined(__FreeBSD__) && !defined(__aarch64__)
        REQUIRE(pow(integer{2}, 4.5l) == std::pow(2.l, 4.5l));
        REQUIRE(pow(4.5l, integer{-2}) == std::pow(4.5l, -2.l));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE(pow(integer{2}, __int128_t{4}) == 16);
        REQUIRE(pow(__int128_t{4}, integer{2}) == 16);
        REQUIRE(pow(integer{2}, __uint128_t{4}) == 16);
        REQUIRE(pow(__uint128_t{4}, integer{2}) == 16);
#endif
        // Complex testing.
        REQUIRE(std::is_same<std::complex<float>, decltype(mppp::pow(integer{2}, std::complex<float>{2}))>::value);
        REQUIRE(std::is_same<std::complex<float>, decltype(mppp::pow(std::complex<float>{2}, integer{2}))>::value);
        REQUIRE(std::is_same<std::complex<double>, decltype(mppp::pow(integer{2}, std::complex<double>{2}))>::value);
        REQUIRE(std::is_same<std::complex<double>, decltype(mppp::pow(std::complex<double>{2}, integer{2}))>::value);
#if defined(MPPP_WITH_MPFR)
        REQUIRE(std::is_same<std::complex<long double>,
                             decltype(mppp::pow(integer{2}, std::complex<long double>{2}))>::value);
        REQUIRE(std::is_same<std::complex<long double>,
                             decltype(mppp::pow(std::complex<long double>{2}, integer{2}))>::value);
#endif

        // NOTE: fully qualify the pow() call because on MSVC there are
        // template implementations of std::pow() for complex that, through
        // ADL, are preferred over mp++'s ones. Not sure if what MSVC
        // is doing is 100% compliant.
        REQUIRE(mppp::pow(integer{2}, std::complex<float>{2}) == std::complex<float>{4, 0});
        REQUIRE(mppp::pow(std::complex<float>{2}, integer{2}) == std::complex<float>{4, 0});
        REQUIRE(mppp::pow(integer{2}, std::complex<double>{2}) == std::complex<double>{4, 0});
        REQUIRE(mppp::pow(std::complex<double>{2}, integer{2}) == std::complex<double>{4, 0});
#if defined(MPPP_WITH_MPFR) && !defined(__FreeBSD__) && !defined(_ARCH_PPC)
        REQUIRE(mppp::pow(integer{2}, std::complex<long double>{2}) == std::complex<long double>{4, 0});
        REQUIRE(mppp::pow(std::complex<long double>{2}, integer{2}) == std::complex<long double>{4, 0});
#endif
    }
};

TEST_CASE("pow")
{
    tuple_for_each(sizes{}, pow_tester{});
}
