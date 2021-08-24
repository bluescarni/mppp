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
#include <utility>

#include <gmp.h>

#include <mp++/config.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>

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

template <typename T, typename U>
using pow_t = decltype(pow(std::declval<const T &>(), std::declval<const U &>()));

template <typename T, typename U>
using has_pow = detail::is_detected<pow_t, T, U>;

static inline void mpq_pow(::mpq_t rop, const ::mpq_t base, long exp)
{
    if (exp >= 0) {
        mpz_pow_ui(mpq_numref(rop), mpq_numref(base), static_cast<unsigned long>(exp));
        mpz_pow_ui(mpq_denref(rop), mpq_denref(base), static_cast<unsigned long>(exp));
    } else {
        mpz_pow_ui(mpq_numref(rop), mpq_denref(base), static_cast<unsigned long>(-exp));
        mpz_pow_ui(mpq_denref(rop), mpq_numref(base), static_cast<unsigned long>(-exp));
        if (mpz_sgn(mpq_denref(rop)) < 0) {
            mpz_neg(mpq_numref(rop), mpq_numref(rop));
            mpz_neg(mpq_denref(rop), mpq_denref(rop));
        }
    }
}

struct pow_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using rational = rational<S::value>;
        using integer = typename rational::int_t;
        REQUIRE((has_pow<rational, rational>::value));
        REQUIRE((has_pow<rational, integer>::value));
        REQUIRE((has_pow<rational, int>::value));
        REQUIRE((has_pow<rational, char>::value));
        REQUIRE((has_pow<rational, unsigned>::value));
        REQUIRE((has_pow<rational, double>::value));
        REQUIRE((has_pow<integer, rational>::value));
        REQUIRE((has_pow<int, rational>::value));
        REQUIRE((has_pow<char, rational>::value));
        REQUIRE((has_pow<unsigned, rational>::value));
        REQUIRE((has_pow<float, rational>::value));
        REQUIRE((!has_pow<std::string, rational>::value));
        REQUIRE((!has_pow<rational, std::string>::value));
        // Start with all zeroes.
        detail::mpq_raii m1, m2;
        rational n1, n2, ret;
        REQUIRE((std::is_same<rational, decltype(pow(n1, n2))>::value));
        ret = pow(n1, n2);
        ::mpq_pow(&m1.m_mpq, &m2.m_mpq, 0);
        REQUIRE((lex_cast(ret) == lex_cast(m1)));
        REQUIRE(ret.get_num().is_static());
        REQUIRE(ret.get_den().is_static());
        detail::mpq_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        std::uniform_int_distribution<long> edist(-20, 20);
        // Run a variety of tests with operands with x number of limbs.
        auto random_xy = [&](unsigned x) {
            for (int i = 0; i < ntries; ++i) {
                random_rational(tmp, x, rng);
                mpq_set(&m2.m_mpq, &tmp.m_mpq);
                n2 = rational(&tmp.m_mpq);
                if (sdist(rng)) {
                    mpq_neg(&m2.m_mpq, &m2.m_mpq);
                    n2.neg();
                }
                if (n2.get_num().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2._get_num().promote();
                }
                if (n2.get_den().is_static() && sdist(rng)) {
                    // Promote sometimes, if possible.
                    n2._get_den().promote();
                }
                const auto ex = edist(rng);
                if (mpq_sgn(&m2.m_mpq) == 0 && ex < 0) {
                    REQUIRE_THROWS_PREDICATE(pow(n2, ex), zero_division_error, [&ex](const zero_division_error &zde) {
                        return zde.what()
                               == "Cannot raise rational zero to the negative exponent " + std::to_string(ex);
                    });
                    continue;
                }
                ::mpq_pow(&m1.m_mpq, &m2.m_mpq, ex);
                n1 = pow(n2, ex);
                REQUIRE((lex_cast(n1) == lex_cast(m1)));
                REQUIRE((std::is_same<rational, decltype(pow(n1, ex))>::value));
                REQUIRE((std::is_same<rational, decltype(pow(n1, integer{ex}))>::value));
                REQUIRE((std::is_same<rational, decltype(pow(n1, rational{ex}))>::value));
                REQUIRE(n1 == pow(n2, integer{ex}));
                REQUIRE(n1 == pow(n2, rational{ex}));
                if (ex != 0 && ex != 1 && ex != -1 && !is_zero(n2) && !is_one(n2)) {
                    rational tmp_exp{integer{ex} + 1, ex};
                    REQUIRE_THROWS_PREDICATE(pow(n2, tmp_exp), std::domain_error, ([&](const std::domain_error &de) {
                                                 return de.what()
                                                        == "Cannot raise the rational base " + n2.to_string()
                                                               + " to the non-integral exponent " + tmp_exp.to_string();
                                             }));
                }
            }
        };

        random_xy(0);
        random_xy(1);
        random_xy(2);
        random_xy(3);
        random_xy(4);

        // Integral base, rational power.
        REQUIRE(pow(2, rational{2}) == rational{4});
        REQUIRE(pow(2, rational{-2}) == (rational{1, 4}));
        REQUIRE_THROWS_PREDICATE(
            pow(2, rational(1, 2)), std::domain_error, ([&](const std::domain_error &de) {
                return de.what() == std::string("Cannot raise the rational base 2 to the non-integral exponent 1/2");
            }));
        REQUIRE_THROWS_PREDICATE(
            pow(2ull, rational(-1, 2)), std::domain_error, ([&](const std::domain_error &de) {
                return de.what() == std::string("Cannot raise the rational base 2 to the non-integral exponent -1/2");
            }));
        REQUIRE((std::is_same<rational, decltype(pow(2, rational(1, 2)))>::value));
        REQUIRE((std::is_same<rational, decltype(pow(2ull, rational(1, 2)))>::value));

        // Some floating point tests.
        REQUIRE((std::is_same<float, decltype(pow(2.f, rational(1, 2)))>::value));
        REQUIRE((std::is_same<double, decltype(pow(rational(1, 2), 2.))>::value));
#if defined(MPPP_WITH_MPFR)
        REQUIRE((std::is_same<long double, decltype(pow(rational(1, 2), 2.l))>::value));
#endif
        REQUIRE(std::abs(pow(2.f, rational{1, 2}) - std::sqrt(2.f)) < 1E-8);
        REQUIRE(std::abs(pow(rational{2}, .5f) - std::sqrt(2.f)) < 1E-8);
        REQUIRE(std::abs(pow(2., rational{1, 2}) - std::sqrt(2.)) < 1E-8);
        REQUIRE(std::abs(pow(rational{2}, .5) - std::sqrt(2.)) < 1E-8);
#if defined(MPPP_WITH_MPFR)
        REQUIRE(std::abs(pow(2.l, rational{1, 2}) - std::sqrt(2.l)) < 1E-8);
        REQUIRE(std::abs(pow(rational{2}, .5l) - std::sqrt(2.l)) < 1E-8);
#endif

        // Some special casing with base 1.
        REQUIRE((pow(rational{1}, rational{1, 2}) == rational{1}));
        REQUIRE((pow(rational{1}, integer{-2}) == rational{1}));
        REQUIRE((pow(rational{1}, 2ull) == rational{1}));
        REQUIRE((pow(rational{1}, static_cast<signed char>(-1)) == rational{1}));
        REQUIRE((pow(1, rational{3, 4}) == rational{1}));
        REQUIRE((pow(1, rational{-3, 4}) == rational{1}));

#if defined(MPPP_HAVE_GCC_INT128)
        REQUIRE(pow(rational{2, 3}, __int128_t{2}) == rational{4, 9});
        REQUIRE(pow(rational{2, 3}, __uint128_t{2}) == rational{4, 9});
        REQUIRE(pow(__int128_t{2}, rational{3}) == 8);
        REQUIRE(pow(__uint128_t{2}, rational{3}) == 8);
#endif

        // Complex testing.
        REQUIRE(std::is_same<std::complex<float>, decltype(mppp::pow(rational{2}, std::complex<float>{2}))>::value);
        REQUIRE(std::is_same<std::complex<float>, decltype(mppp::pow(std::complex<float>{2}, rational{2}))>::value);
        REQUIRE(std::is_same<std::complex<double>, decltype(mppp::pow(rational{2}, std::complex<double>{2}))>::value);
        REQUIRE(std::is_same<std::complex<double>, decltype(mppp::pow(std::complex<double>{2}, rational{2}))>::value);
#if defined(MPPP_WITH_MPFR)
        REQUIRE(std::is_same<std::complex<long double>,
                             decltype(mppp::pow(rational{2}, std::complex<long double>{2}))>::value);
        REQUIRE(std::is_same<std::complex<long double>,
                             decltype(mppp::pow(std::complex<long double>{2}, rational{2}))>::value);
#endif

        // NOTE: fully qualify the pow() call because on MSVC there are
        // template implementations of std::pow() for complex that, through
        // ADL, are preferred over mp++'s ones. Not sure if what MSVC
        // is doing is 100% compliant.
        REQUIRE(mppp::pow(rational{2}, std::complex<float>{2}) == std::complex<float>{4, 0});
        REQUIRE(mppp::pow(std::complex<float>{2}, rational{2}) == std::complex<float>{4, 0});
        REQUIRE(mppp::pow(rational{2}, std::complex<double>{2}) == std::complex<double>{4, 0});
        REQUIRE(mppp::pow(std::complex<double>{2}, rational{2}) == std::complex<double>{4, 0});
#if defined(MPPP_WITH_MPFR) && !defined(__FreeBSD__) && !defined(_ARCH_PPC)
        REQUIRE(mppp::pow(rational{2}, std::complex<long double>{2}) == std::complex<long double>{4, 0});
        REQUIRE(mppp::pow(std::complex<long double>{2}, rational{2}) == std::complex<long double>{4, 0});
#endif
    }
};

TEST_CASE("pow")
{
    tuple_for_each(sizes{}, pow_tester{});
}
