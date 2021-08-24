// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>
#include <stdexcept>
#include <utility>

#include <mp++/config.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real.hpp>

#if defined(MPPP_WITH_QUADMATH)
#include <mp++/real128.hpp>
#endif

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

using int_t = integer<1>;
using rat_t = rational<1>;

TEST_CASE("real identity")
{
    real r0{};
    REQUIRE((+r0).zero_p());
    REQUIRE(!(+r0).signbit());
    REQUIRE((+real{}).zero_p());
    REQUIRE(!(+real{}).signbit());
    REQUIRE((+r0).get_prec() == real_prec_min());
    REQUIRE((+real{}).get_prec() == real_prec_min());
    r0 = 123;
    REQUIRE(mpfr_cmp_ui((+r0).get_mpfr_t(), 123ul) == 0);
    REQUIRE((+r0).get_prec() == detail::nl_digits<int>() + 1);
    REQUIRE(mpfr_cmp_ui((+std::move(r0)).get_mpfr_t(), 123ul) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("real binary add")
{
    real r0, r1;
    REQUIRE(real{} + real{} == real{});
    REQUIRE((real{} + real{}).get_prec() == real_prec_min());
    r0 = 23;
    r1 = -1;
    REQUIRE(r0 + r1 == real{22});
    REQUIRE(std::move(r0) + r1 == real{22});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
    r0 = real{23};
    REQUIRE(r0 + std::move(r1) == real{22});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r1.get_mpfr_t()->_mpfr_d);
    r1 = real{-1};
    REQUIRE(std::move(r0) + std::move(r1) == real{22});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_mpfr_t()->_mpfr_d);
    r0 = real{23};
    REQUIRE((real{1, 10} + real{2, 20} == real{3}));
    REQUIRE((real{1, 10} + real{2, 20}).get_prec() == 20);
    REQUIRE((real{1, 20} + real{2, 10} == real{3}));
    REQUIRE((real{1, 20} + real{2, 10}).get_prec() == 20);
    // Integrals.
    REQUIRE((real{1, 10} + 10 == real{11}));
    REQUIRE((real{1, 10} + 10).get_prec() == detail::nl_digits<int>() + 1);
    REQUIRE((real{1, 10} + wchar_t{10} == real{11}));
    REQUIRE((10 + real{1, 10} == real{11}));
    REQUIRE((10 + real{1, 10}).get_prec() == detail::nl_digits<int>() + 1);
    REQUIRE((wchar_t{10} + real{1, 10} == real{11}));
    REQUIRE((real{1, 100} + 10 == real{11}));
    REQUIRE((real{1, 100} + 10).get_prec() == std::max(100, detail::nl_digits<int>() + 1));
    REQUIRE((10 + real{1, 100} == real{11}));
    REQUIRE((10 + real{1, 100}).get_prec() == std::max(100, detail::nl_digits<int>() + 1));
    REQUIRE((real{1, 10} + true == real{2}));
    REQUIRE((real{1, 10} + true).get_prec() == 10);
    REQUIRE((false + real{1, 10} == real{1}));
    REQUIRE((false + real{1, 10}).get_prec() == 10);
    REQUIRE((real{1, 10} + 10u == real{11}));
    REQUIRE((real{1, 10} + 10u).get_prec() == detail::nl_digits<unsigned>());
    REQUIRE((10u + real{1, 10} == real{11}));
    REQUIRE((10u + real{1, 10}).get_prec() == detail::nl_digits<unsigned>());
    REQUIRE((real{1, 100} + 10u == real{11}));
    REQUIRE((real{1, 100} + 10u).get_prec() == std::max(100, detail::nl_digits<unsigned>()));
    REQUIRE((10u + real{1, 100} == real{11}));
    REQUIRE((10u + real{1, 100}).get_prec() == std::max(100, detail::nl_digits<unsigned>()));
    REQUIRE((real{1, 10} + 10ll == real{11}));
    REQUIRE((real{1, 10} + 10ll).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE((real{0, 10} + detail::nl_max<long long>() == real{detail::nl_max<long long>()}));
    REQUIRE((real{0, 10} + detail::nl_max<long long>()).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE((real{0, 10} + detail::nl_min<long long>() == real{detail::nl_min<long long>()}));
    REQUIRE((real{0, 10} + detail::nl_min<long long>()).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE((10ll + real{1, 10} == real{11}));
    REQUIRE((10ll + real{1, 10}).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE((real{1, 100} + 10ll == real{11}));
    REQUIRE((real{1, 100} + 10ll).get_prec() == std::max(100, detail::nl_digits<long long>() + 1));
    REQUIRE((10ll + real{1, 100} == real{11}));
    REQUIRE((10ll + real{1, 100}).get_prec() == std::max(100, detail::nl_digits<long long>() + 1));
    REQUIRE((real{1, 10} + 10ull == real{11}));
    REQUIRE((real{1, 10} + 10ull).get_prec() == detail::nl_digits<unsigned long long>());
    REQUIRE((10ull + real{1, 10} == real{11}));
    REQUIRE((10ull + real{1, 10}).get_prec() == detail::nl_digits<unsigned long long>());
    REQUIRE((real{0, 10} + detail::nl_max<unsigned long long>() == real{detail::nl_max<unsigned long long>()}));
    REQUIRE((real{0, 10} + detail::nl_max<unsigned long long>()).get_prec() == detail::nl_digits<unsigned long long>());
    REQUIRE((real{1, 100} + 10ull == real{11}));
    REQUIRE((real{1, 100} + 10ull).get_prec() == std::max(100, detail::nl_digits<unsigned long long>()));
    REQUIRE((10ull + real{1, 100} == real{11}));
    REQUIRE((10ull + real{1, 100}).get_prec() == std::max(100, detail::nl_digits<unsigned long long>()));
    // Floating-point.
    REQUIRE((real{1, 10} + 10.f == real{11}));
    REQUIRE((real{1, 10} + 10.f).get_prec() == detail::dig2mpfr_prec<float>());
    REQUIRE((10.f + real{1, 10} == real{11}));
    REQUIRE((10.f + real{1, 10}).get_prec() == detail::dig2mpfr_prec<float>());
    REQUIRE((real{1, 100} + 10.f == real{11}));
    REQUIRE((real{1, 100} + 10.f).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<float>()));
    REQUIRE((10.f + real{1, 100} == real{11}));
    REQUIRE((10.f + real{1, 100}).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<float>()));
    REQUIRE((real{1, 10} + 10. == real{11}));
    REQUIRE((real{1, 10} + 10.).get_prec() == detail::dig2mpfr_prec<double>());
    REQUIRE((10. + real{1, 10} == real{11}));
    REQUIRE((10. + real{1, 10}).get_prec() == detail::dig2mpfr_prec<double>());
    REQUIRE((real{1, 100} + 10. == real{11}));
    REQUIRE((real{1, 100} + 10.).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<double>()));
    REQUIRE((10. + real{1, 100} == real{11}));
    REQUIRE((10. + real{1, 100}).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<double>()));
    REQUIRE((real{1, 10} + 10.l == real{11}));
    REQUIRE((real{1, 10} + 10.l).get_prec() == detail::dig2mpfr_prec<long double>());
    REQUIRE((10.l + real{1, 10} == real{11}));
    REQUIRE((10.l + real{1, 10}).get_prec() == detail::dig2mpfr_prec<long double>());
    REQUIRE((real{1, 100} + 10.l == real{11}));
    REQUIRE((real{1, 100} + 10.l).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<long double>()));
    REQUIRE((10.l + real{1, 100} == real{11}));
    REQUIRE((10.l + real{1, 100}).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<long double>()));
    // Integer.
    REQUIRE((real{1, 10} + int_t{10} == real{11}));
    REQUIRE((real{1, 10} + int_t{10}).get_prec() == GMP_NUMB_BITS);
    REQUIRE((int_t{10} + real{1, 10} == real{11}));
    REQUIRE((int_t{10} + real{1, 10}).get_prec() == GMP_NUMB_BITS);
    REQUIRE((real{1, 100} + int_t{10} == real{11}));
    REQUIRE((real{1, 100} + int_t{10}).get_prec() == std::max(100, GMP_NUMB_BITS));
    REQUIRE((int_t{10} + real{1, 100} == real{11}));
    REQUIRE((int_t{10} + real{1, 100}).get_prec() == std::max(100, GMP_NUMB_BITS));
    // Rational.
    REQUIRE((real{1, 10} + rat_t{10} == real{11}));
    REQUIRE((real{1, 10} + rat_t{10}).get_prec() == GMP_NUMB_BITS * 2);
    REQUIRE((rat_t{10} + real{1, 10} == real{11}));
    REQUIRE((rat_t{10} + real{1, 10}).get_prec() == GMP_NUMB_BITS * 2);
    REQUIRE((real{1, 100} + rat_t{10} == real{11}));
    REQUIRE((real{1, 100} + rat_t{10}).get_prec() == std::max(100, GMP_NUMB_BITS * 2));
    REQUIRE((rat_t{10} + real{1, 100} == real{11}));
    REQUIRE((rat_t{10} + real{1, 100}).get_prec() == std::max(100, GMP_NUMB_BITS * 2));
#if defined(MPPP_WITH_QUADMATH)
    REQUIRE((real{1, 10} + real128{10} == real{11}));
    REQUIRE((real{1, 10} + real128{10}).get_prec() == 113);
    REQUIRE((real128{10} + real{1, 10} == real{11}));
    REQUIRE((real128{10} + real{1, 10}).get_prec() == 113);
    REQUIRE((real{1, 200} + real128{10} == real{11}));
    REQUIRE((real{1, 200} + real128{10}).get_prec() == 200);
    REQUIRE((real128{10} + real{1, 200} == real{11}));
    REQUIRE((real128{10} + real{1, 200}).get_prec() == 200);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE((real{1, 10} + __int128_t{10} == real{11}));
    REQUIRE((real{1, 10} + __int128_t{10}).get_prec() == 128);
    REQUIRE((__int128_t{10} + real{1, 10} == real{11}));
    REQUIRE((__int128_t{10} + real{1, 10}).get_prec() == 128);
    REQUIRE((real{1, 10} + __uint128_t{10} == real{11}));
    REQUIRE((real{1, 10} + __uint128_t{10}).get_prec() == 128);
    REQUIRE((__uint128_t{10} + real{1, 10} == real{11}));
    REQUIRE((__uint128_t{10} + real{1, 10}).get_prec() == 128);
    REQUIRE((real{1, 200} + __int128_t{10} == real{11}));
    REQUIRE((real{1, 200} + __int128_t{10}).get_prec() == 200);
    REQUIRE((__int128_t{10} + real{1, 200} == real{11}));
    REQUIRE((__int128_t{10} + real{1, 200}).get_prec() == 200);
    REQUIRE((real{1, 200} + __uint128_t{10} == real{11}));
    REQUIRE((real{1, 200} + __uint128_t{10}).get_prec() == 200);
    REQUIRE((__uint128_t{10} + real{1, 200} == real{11}));
    REQUIRE((__uint128_t{10} + real{1, 200}).get_prec() == 200);

    // Try also large values.
    REQUIRE(1.23_r512 + (__int128_t{1} << 65) == 1.23_r512 + pow(2_r128, 65));
    REQUIRE((__int128_t{1} << 65) + 1.23_r512 == 1.23_r512 + pow(2_r128, 65));

    REQUIRE(1.23_r512 + (__uint128_t{1} << 65) == 1.23_r512 + pow(2_r128, 65));
    REQUIRE((__uint128_t{1} << 65) + 1.23_r512 == 1.23_r512 + pow(2_r128, 65));
#endif

    // Ensure correct precision handling if one operand (i.e., rational)
    // cannot be converted exactly to real.
    REQUIRE(abs((1_r512 + 1 / 10_q1) - 1.1_r512) < pow(2_r512, -510));
    REQUIRE(abs((1 / 10_q1 + 1_r512) - 1.1_r512) < pow(2_r512, -510));
}

TEST_CASE("real left in-place add")
{
    real r0, r1;
    const real r1_const;
    r0 += r1;
    REQUIRE(r0.zero_p());
    REQUIRE(!r0.signbit());
    r0 += r1_const;
    REQUIRE(r0.zero_p());
    REQUIRE(!r0.signbit());
    r0 = 5;
    r1 = 6;
    r0 += r1;
    REQUIRE(r0 == real{11});
    r0 = real{};
    r0 += real{12345678ll};
    REQUIRE(r0 == real{12345678ll});
    REQUIRE(r0.get_prec() == detail::nl_digits<long long>() + 1);
    // Integrals.
    r0 = real{};
    r0 += 123;
    REQUIRE(r0 == real{123});
    REQUIRE(r0.get_prec() == detail::nl_digits<int>() + 1);
    r0 = real{};
    r0 += 123u;
    REQUIRE(r0 == real{123u});
    REQUIRE(r0.get_prec() == detail::nl_digits<unsigned>());
    r0 = real{};
    r0 += true;
    REQUIRE(r0 == real{1});
    REQUIRE(r0.get_prec() == std::max<::mpfr_prec_t>(detail::nl_digits<bool>(), real_prec_min()));
    r0 = real{};
    r0 += 123ll;
    REQUIRE(r0 == real{123ll});
    REQUIRE(r0.get_prec() == detail::nl_digits<long long>() + 1);
    r0 = real{};
    r0 += detail::nl_max<long long>();
    REQUIRE(r0 == real{detail::nl_max<long long>()});
    REQUIRE(r0.get_prec() == detail::nl_digits<long long>() + 1);
    r0 = real{};
    r0 += detail::nl_min<long long>();
    REQUIRE(r0 == real{detail::nl_min<long long>()});
    REQUIRE(r0.get_prec() == detail::nl_digits<long long>() + 1);
    r0 = real{};
    r0 += 123ull;
    REQUIRE(r0 == real{123ull});
    REQUIRE(r0.get_prec() == detail::nl_digits<unsigned long long>());
    r0 = real{};
    r0 += detail::nl_max<unsigned long long>();
    REQUIRE(r0 == real{detail::nl_max<unsigned long long>()});
    REQUIRE(r0.get_prec() == detail::nl_digits<unsigned long long>());
    // Floating-point.
    r0 = real{};
    r0 += 123.f;
    REQUIRE(r0 == real{123.f});
    REQUIRE(r0.get_prec() == detail::dig2mpfr_prec<float>());
    r0 = real{};
    r0 += 123.;
    REQUIRE(r0 == real{123.});
    REQUIRE(r0.get_prec() == detail::dig2mpfr_prec<double>());
    r0 = real{};
    r0 += 123.l;
    REQUIRE(r0 == real{123.l});
    REQUIRE(r0.get_prec() == detail::dig2mpfr_prec<long double>());
    // Integer.
    r0 = real{};
    r0 += int_t{123};
    REQUIRE(r0 == real{int_t{123}});
    REQUIRE(r0.get_prec() == GMP_NUMB_BITS);
    // Rational.
    r0 = real{};
    r0 += rat_t{123};
    REQUIRE(r0 == real{rat_t{123}});
    REQUIRE(r0.get_prec() == GMP_NUMB_BITS * 2);

    r0 = 1_r512;
    r0 += 1 / 10_q1;
    REQUIRE(abs(r0 - 1.1_r512) <= pow(2_r512, -500));
#if defined(MPPP_WITH_QUADMATH)
    r0 = real{};
    r0 += real128{123};
    REQUIRE(r0 == real{real128{123}});
    REQUIRE(r0.get_prec() == 113);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    r0 = real{};
    REQUIRE((r0 += __int128_t{10}) == real{10});
    REQUIRE(r0.get_prec() == 128);
    r0 = real{};
    REQUIRE((r0 += __uint128_t{10}) == real{10});
    REQUIRE(r0.get_prec() == 128);

    // Try also large values.
    r0 = real{};
    REQUIRE((r0 += (__int128_t{1}) << 65) == pow(2_r128, 65));
    REQUIRE(r0.get_prec() == 128);

    r0 = real{};
    REQUIRE((r0 += (__uint128_t{1}) << 65) == pow(2_r128, 65));
    REQUIRE(r0.get_prec() == 128);
#endif

    // Check stealing move semantics.
    r0 = real{42};
    r1 = real{1, detail::real_deduce_precision(0) * 10};
    r0 += std::move(r1);
    REQUIRE(r0 == 43);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0) * 10);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1 == 42);
}

TEST_CASE("real right in-place add")
{
    // Integrals.
    {
        int n = 3;
        n += real{2};
        REQUIRE(n == 5);
        n = 1;
        REQUIRE_THROWS_AS(n += real{detail::nl_max<int>()}, std::overflow_error);
        REQUIRE_THROWS_AS((n += real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1);
        n = -1;
        REQUIRE_THROWS_AS(n += real{detail::nl_min<int>()}, std::overflow_error);
        REQUIRE(n == -1);

        // Check stealing.
        real r{123, detail::real_deduce_precision(5) + 1};
        n += std::move(r);
        REQUIRE(n == 122);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());
    }
    {
        unsigned n = 3;
        n += real{2};
        REQUIRE(n == 5);
        n = 1;
        REQUIRE_THROWS_AS(n += real{detail::nl_max<unsigned>()}, std::overflow_error);
        REQUIRE_THROWS_AS((n += real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1u);
    }
    {
        bool n = true;
        n += real{2};
        REQUIRE(n);
    }
    {
        long long n = 3;
        n += real{2};
        REQUIRE(n == 5);
        n = 1;
        REQUIRE_THROWS_AS(n += real{detail::nl_max<long long>()}, std::overflow_error);
        REQUIRE_THROWS_AS((n += real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1);
        n = -1;
        REQUIRE_THROWS_AS(n += real{detail::nl_min<long long>()}, std::overflow_error);
        REQUIRE(n == -1);
    }
    {
        unsigned long long n = 3;
        n += real{2};
        REQUIRE(n == 5);
        n = 1;
        REQUIRE_THROWS_AS(n += real{detail::nl_max<unsigned long long>()}, std::overflow_error);
        REQUIRE_THROWS_AS((n += real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1u);
    }
    // Floating-point.
    {
        float x = 3;
        x += real{2};
        REQUIRE(x == 5.f);
        if (std::numeric_limits<float>::is_iec559) {
            x = detail::nl_max<float>();
            x += real{detail::nl_max<float>()};
            REQUIRE(std::isinf(x));
        }
    }
    {
        double x = 3;
        x += real{2};
        REQUIRE(x == 5.);
        if (std::numeric_limits<double>::is_iec559) {
            x = detail::nl_max<double>();
            x += real{detail::nl_max<double>()};
            REQUIRE(std::isinf(x));
        }
    }
    {
        long double x = 3;
        x += real{2};
        REQUIRE(x == 5.l);
        if (std::numeric_limits<long double>::is_iec559) {
            x = detail::nl_max<long double>();
            x += real{detail::nl_max<long double>()};
            REQUIRE(std::isinf(x));
        }
    }
    // Integer.
    {
        int_t n{3};
        n += real{2};
        REQUIRE(n == 5);
        n = 1;
        REQUIRE_THROWS_AS((n += real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1);
    }
    // Rational.
    {
        rat_t n{3};
        n += real{2};
        REQUIRE(n == 5);
        n = 1;
        REQUIRE_THROWS_AS((n += real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1);

        n = 1 / 10_q1;
        n += 1_r512;
        REQUIRE(abs(n - 1.1_r512) <= pow(2_r512, -500));
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        real128 x{3};
        x += real{2};
        REQUIRE(x == 5);
        x = real128_max();
        x += real{real128_max()};
        REQUIRE(isinf(x));
    }
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    {
        __int128_t n128 = 5;
        n128 += real{2};
        REQUIRE(n128 == 7);
        __int128_t un128 = 5;
        un128 += real{2};
        REQUIRE(un128 == 7);
    }
#endif
}

TEST_CASE("real neg copy")
{
    real r0{};
    REQUIRE((-r0).zero_p());
    REQUIRE((-r0).signbit());
    REQUIRE((-real{}).zero_p());
    REQUIRE((-real{}).signbit());
    REQUIRE((-r0).get_prec() == real_prec_min());
    REQUIRE((-real{}).get_prec() == real_prec_min());
    r0 = 123;
    REQUIRE(mpfr_cmp_si((-r0).get_mpfr_t(), -123l) == 0);
    REQUIRE((-r0).get_prec() == detail::nl_digits<int>() + 1);
    REQUIRE(mpfr_cmp_si((-std::move(r0)).get_mpfr_t(), -123l) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("real binary sub")
{
    real r0, r1;
    REQUIRE(real{} - real{} == real{});
    REQUIRE((real{} - real{}).get_prec() == real_prec_min());
    r0 = 23;
    r1 = -1;
    REQUIRE(r0 - r1 == real{24});
    REQUIRE(std::move(r0) - r1 == real{24});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
    r0 = real{23};
    REQUIRE(r0 - std::move(r1) == real{24});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r1.get_mpfr_t()->_mpfr_d);
    r1 = real{-1};
    REQUIRE(std::move(r0) - std::move(r1) == real{24});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_mpfr_t()->_mpfr_d);
    r0 = real{23};
    REQUIRE((real{1, 10} - real{2, 20} == real{-1}));
    REQUIRE((real{1, 10} - real{2, 20}).get_prec() == 20);
    REQUIRE((real{1, 20} - real{2, 10} == real{-1}));
    REQUIRE((real{1, 20} - real{2, 10}).get_prec() == 20);
    // Integrals.
    REQUIRE((real{1, 10} - 10 == real{-9}));
    REQUIRE((real{1, 10} - wchar_t{10} == real{-9}));
    REQUIRE((real{1, 10} - 10).get_prec() == detail::nl_digits<int>() + 1);
    REQUIRE((10 - real{1, 10} == real{9}));
    REQUIRE((wchar_t{10} - real{1, 10} == real{9}));
    REQUIRE((10 - real{1, 10}).get_prec() == detail::nl_digits<int>() + 1);
    REQUIRE((real{1, 100} - 10 == real{-9}));
    REQUIRE((real{1, 100} - 10).get_prec() == std::max(100, detail::nl_digits<int>() + 1));
    REQUIRE((10 - real{1, 100} == real{9}));
    REQUIRE((10 - real{1, 100}).get_prec() == std::max(100, detail::nl_digits<int>() + 1));
    REQUIRE((real{1, 10} - true == real{0}));
    REQUIRE((real{1, 10} - true).get_prec() == 10);
    REQUIRE((false - real{1, 10} == real{-1}));
    REQUIRE((false - real{1, 10}).get_prec() == 10);
    REQUIRE((real{1, 10} - 10u == real{-9}));
    REQUIRE((real{1, 10} - 10u).get_prec() == detail::nl_digits<unsigned>());
    REQUIRE((10u - real{1, 10} == real{9}));
    REQUIRE((10u - real{1, 10}).get_prec() == detail::nl_digits<unsigned>());
    REQUIRE((real{1, 100} - 10u == real{-9}));
    REQUIRE((real{1, 100} - 10u).get_prec() == std::max(100, detail::nl_digits<unsigned>()));
    REQUIRE((10u - real{1, 100} == real{9}));
    REQUIRE((10u - real{1, 100}).get_prec() == std::max(100, detail::nl_digits<unsigned>()));
    REQUIRE((real{1, 10} - 10ll == real{-9}));
    REQUIRE((real{1, 10} - 10ll).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE((real{0, 10} - detail::nl_max<long long>() == -real{detail::nl_max<long long>()}));
    REQUIRE((real{0, 10} - detail::nl_max<long long>()).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE((real{0, 10} - detail::nl_min<long long>() == -real{detail::nl_min<long long>()}));
    REQUIRE((real{0, 10} - detail::nl_min<long long>()).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE((10ll - real{1, 10} == real{9}));
    REQUIRE((10ll - real{1, 10}).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE((real{1, 100} - 10ll == real{-9}));
    REQUIRE((real{1, 100} - 10ll).get_prec() == std::max(100, detail::nl_digits<long long>() + 1));
    REQUIRE((10ll - real{1, 100} == real{9}));
    REQUIRE((10ll - real{1, 100}).get_prec() == std::max(100, detail::nl_digits<long long>() + 1));
    REQUIRE((real{1, 10} - 10ull == real{-9}));
    REQUIRE((real{1, 10} - 10ull).get_prec() == detail::nl_digits<unsigned long long>());
    REQUIRE((10ull - real{1, 10} == real{9}));
    REQUIRE((10ull - real{1, 10}).get_prec() == detail::nl_digits<unsigned long long>());
    REQUIRE((real{0, 10} - detail::nl_max<unsigned long long>() == -real{detail::nl_max<unsigned long long>()}));
    REQUIRE((real{0, 10} - detail::nl_max<unsigned long long>()).get_prec() == detail::nl_digits<unsigned long long>());
    REQUIRE((real{1, 100} - 10ull == real{-9}));
    REQUIRE((real{1, 100} - 10ull).get_prec() == std::max(100, detail::nl_digits<unsigned long long>()));
    REQUIRE((10ull - real{1, 100} == real{9}));
    REQUIRE((10ull - real{1, 100}).get_prec() == std::max(100, detail::nl_digits<unsigned long long>()));
    // Floating-point.
    REQUIRE((real{1, 10} - 10.f == real{-9}));
    REQUIRE((real{1, 10} - 10.f).get_prec() == detail::dig2mpfr_prec<float>());
    REQUIRE((10.f - real{1, 10} == real{9}));
    REQUIRE((10.f - real{1, 10}).get_prec() == detail::dig2mpfr_prec<float>());
    REQUIRE((real{1, 100} - 10.f == real{-9}));
    REQUIRE((real{1, 100} - 10.f).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<float>()));
    REQUIRE((10.f - real{1, 100} == real{9}));
    REQUIRE((10.f - real{1, 100}).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<float>()));
    REQUIRE((real{1, 10} - 10. == real{-9}));
    REQUIRE((real{1, 10} - 10.).get_prec() == detail::dig2mpfr_prec<double>());
    REQUIRE((10. - real{1, 10} == real{9}));
    REQUIRE((10. - real{1, 10}).get_prec() == detail::dig2mpfr_prec<double>());
    REQUIRE((real{1, 100} - 10. == real{-9}));
    REQUIRE((real{1, 100} - 10.).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<double>()));
    REQUIRE((10. - real{1, 100} == real{9}));
    REQUIRE((10. - real{1, 100}).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<double>()));
    REQUIRE((real{1, 10} - 10.l == real{-9}));
    REQUIRE((real{1, 10} - 10.l).get_prec() == detail::dig2mpfr_prec<long double>());
    REQUIRE((10.l - real{1, 10} == real{9}));
    REQUIRE((10.l - real{1, 10}).get_prec() == detail::dig2mpfr_prec<long double>());
    REQUIRE((real{1, 100} - 10.l == real{-9}));
    REQUIRE((real{1, 100} - 10.l).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<long double>()));
    REQUIRE((10.l - real{1, 100} == real{9}));
    REQUIRE((10.l - real{1, 100}).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<long double>()));
    // Integer.
    REQUIRE((real{1, 10} - int_t{10} == real{-9}));
    REQUIRE((real{1, 10} - int_t{10}).get_prec() == GMP_NUMB_BITS);
    REQUIRE((int_t{10} - real{1, 10} == real{9}));
    REQUIRE((int_t{10} - real{1, 10}).get_prec() == GMP_NUMB_BITS);
    REQUIRE((real{1, 100} - int_t{10} == real{-9}));
    REQUIRE((real{1, 100} - int_t{10}).get_prec() == std::max(100, GMP_NUMB_BITS));
    REQUIRE((int_t{10} - real{1, 100} == real{9}));
    REQUIRE((int_t{10} - real{1, 100}).get_prec() == std::max(100, GMP_NUMB_BITS));
    // Rational.
    REQUIRE((real{1, 10} - rat_t{10} == real{-9}));
    REQUIRE((real{1, 10} - rat_t{10}).get_prec() == GMP_NUMB_BITS * 2);
    REQUIRE((rat_t{10} - real{1, 10} == real{9}));
    REQUIRE((rat_t{10} - real{1, 10}).get_prec() == GMP_NUMB_BITS * 2);
    REQUIRE((real{1, 100} - rat_t{10} == real{-9}));
    REQUIRE((real{1, 100} - rat_t{10}).get_prec() == std::max(100, GMP_NUMB_BITS * 2));
    REQUIRE((rat_t{10} - real{1, 100} == real{9}));
    REQUIRE((rat_t{10} - real{1, 100}).get_prec() == std::max(100, GMP_NUMB_BITS * 2));
#if defined(MPPP_WITH_QUADMATH)
    REQUIRE((real{1, 10} - real128{10} == real{-9}));
    REQUIRE((real{1, 10} - real128{10}).get_prec() == 113);
    REQUIRE((real128{10} - real{1, 10} == real{9}));
    REQUIRE((real128{10} - real{1, 10}).get_prec() == 113);
    REQUIRE((real{1, 200} - real128{10} == real{-9}));
    REQUIRE((real{1, 200} - real128{10}).get_prec() == 200);
    REQUIRE((real128{10} - real{1, 200} == real{9}));
    REQUIRE((real128{10} - real{1, 200}).get_prec() == 200);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE((real{1, 10} - __int128_t{10} == real{-9}));
    REQUIRE((real{1, 10} - __int128_t{10}).get_prec() == 128);
    REQUIRE((__int128_t{10} - real{1, 10} == real{9}));
    REQUIRE((__int128_t{10} - real{1, 10}).get_prec() == 128);
    REQUIRE((real{1, 10} - __uint128_t{10} == real{-9}));
    REQUIRE((real{1, 10} - __uint128_t{10}).get_prec() == 128);
    REQUIRE((__uint128_t{10} - real{1, 10} == real{9}));
    REQUIRE((__uint128_t{10} - real{1, 10}).get_prec() == 128);
    REQUIRE((real{1, 200} - __int128_t{10} == real{-9}));
    REQUIRE((real{1, 200} - __int128_t{10}).get_prec() == 200);
    REQUIRE((__int128_t{10} - real{1, 200} == real{9}));
    REQUIRE((__int128_t{10} - real{1, 200}).get_prec() == 200);
    REQUIRE((real{1, 200} - __uint128_t{10} == real{-9}));
    REQUIRE((real{1, 200} - __uint128_t{10}).get_prec() == 200);
    REQUIRE((__uint128_t{10} - real{1, 200} == real{9}));
    REQUIRE((__uint128_t{10} - real{1, 200}).get_prec() == 200);

    // Try also large values.
    REQUIRE(1.23_r512 - (__int128_t{1} << 65) == 1.23_r512 - pow(2_r128, 65));
    REQUIRE((__int128_t{1} << 65) - 1.23_r512 == -1.23_r512 + pow(2_r128, 65));

    REQUIRE(1.23_r512 - (__uint128_t{1} << 65) == 1.23_r512 - pow(2_r128, 65));
    REQUIRE((__uint128_t{1} << 65) - 1.23_r512 == -1.23_r512 + pow(2_r128, 65));
#endif

    // Ensure correct precision handling if one operand (i.e., rational)
    // cannot be converted exactly to real.
    REQUIRE(abs((-1_r512 - 1 / 10_q1) + 1.1_r512) < pow(2_r512, -510));
    REQUIRE(abs((-1 / 10_q1 - 1_r512) + 1.1_r512) < pow(2_r512, -510));
}

TEST_CASE("real left in-place sub")
{
    real r0, r1;
    const real r1_const;
    r0 -= r1;
    REQUIRE(r0.zero_p());
    REQUIRE(!r0.signbit());
    r0 -= r1_const;
    REQUIRE(r0.zero_p());
    REQUIRE(!r0.signbit());
    r0 = 5;
    r1 = 6;
    r0 -= r1;
    REQUIRE(r0 == real{-1});
    r0 = real{};
    r0 -= real{12345678ll};
    REQUIRE(r0 == real{-12345678ll});
    REQUIRE(r0.get_prec() == detail::nl_digits<long long>() + 1);
    // Integrals.
    r0 = real{};
    r0 -= 123;
    REQUIRE(r0 == real{-123});
    REQUIRE(r0.get_prec() == detail::nl_digits<int>() + 1);
    r0 = real{};
    r0 -= 123u;
    REQUIRE(r0 == -real{123u});
    REQUIRE(r0.get_prec() == detail::nl_digits<unsigned>());
    r0 = real{};
    r0 -= true;
    REQUIRE(r0 == real{-1});
    REQUIRE(r0.get_prec() == std::max<::mpfr_prec_t>(detail::nl_digits<bool>(), real_prec_min()));
    r0 = real{};
    r0 -= 123ll;
    REQUIRE(r0 == real{-123ll});
    REQUIRE(r0.get_prec() == detail::nl_digits<long long>() + 1);
    r0 = real{};
    r0 -= detail::nl_max<long long>();
    REQUIRE(r0 == -real{detail::nl_max<long long>()});
    REQUIRE(r0.get_prec() == detail::nl_digits<long long>() + 1);
    r0 = real{};
    r0 -= detail::nl_min<long long>();
    REQUIRE(r0 == -real{detail::nl_min<long long>()});
    REQUIRE(r0.get_prec() == detail::nl_digits<long long>() + 1);
    r0 = real{};
    r0 -= 123ull;
    REQUIRE(r0 == -real{123ull});
    REQUIRE(r0.get_prec() == detail::nl_digits<unsigned long long>());
    r0 = real{};
    r0 -= detail::nl_max<unsigned long long>();
    REQUIRE(r0 == -real{detail::nl_max<unsigned long long>()});
    REQUIRE(r0.get_prec() == detail::nl_digits<unsigned long long>());
    // Floating-point.
    r0 = real{};
    r0 -= 123.f;
    REQUIRE(r0 == real{-123.f});
    REQUIRE(r0.get_prec() == detail::dig2mpfr_prec<float>());
    r0 = real{};
    r0 -= 123.;
    REQUIRE(r0 == real{-123.});
    REQUIRE(r0.get_prec() == detail::dig2mpfr_prec<double>());
    r0 = real{};
    r0 -= 123.l;
    REQUIRE(r0 == real{-123.l});
    REQUIRE(r0.get_prec() == detail::dig2mpfr_prec<long double>());
    // Integer.
    r0 = real{};
    r0 -= int_t{123};
    REQUIRE(r0 == real{-int_t{123}});
    REQUIRE(r0.get_prec() == GMP_NUMB_BITS);
    // Rational.
    r0 = real{};
    r0 -= rat_t{123};
    REQUIRE(r0 == real{-rat_t{123}});
    REQUIRE(r0.get_prec() == GMP_NUMB_BITS * 2);

    r0 = -1_r512;
    r0 -= 1 / 10_q1;
    REQUIRE(abs(r0 + 1.1_r512) <= pow(2_r512, -500));
#if defined(MPPP_WITH_QUADMATH)
    r0 = real{};
    r0 -= real128{123};
    REQUIRE(r0 == real{-real128{123}});
    REQUIRE(r0.get_prec() == 113);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    r0 = real{};
    REQUIRE((r0 -= __int128_t{10}) == real{-10});
    REQUIRE(r0.get_prec() == 128);
    r0 = real{};
    REQUIRE((r0 -= __uint128_t{10}) == real{-10});
    REQUIRE(r0.get_prec() == 128);

    // Try also large values.
    r0 = real{};
    REQUIRE((r0 -= (__int128_t{1}) << 65) == -pow(2_r128, 65));
    REQUIRE(r0.get_prec() == 128);

    r0 = real{};
    REQUIRE((r0 -= (__uint128_t{1}) << 65) == -pow(2_r128, 65));
    REQUIRE(r0.get_prec() == 128);
#endif

    // Check stealing move semantics.
    r0 = real{42};
    r1 = real{1, detail::real_deduce_precision(0) * 10};
    r0 -= std::move(r1);
    REQUIRE(r0 == 41);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0) * 10);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1 == 42);
}

TEST_CASE("real right in-place sub")
{
    // Integrals.
    {
        int n = 3;
        n -= real{2};
        REQUIRE(n == 1);
        n = 1;
        REQUIRE_THROWS_AS(n -= -real{detail::nl_max<int>()}, std::overflow_error);
        REQUIRE_THROWS_AS((n -= real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1);
        n = -1;
        REQUIRE_THROWS_AS(n -= -real{detail::nl_min<int>()}, std::overflow_error);
        REQUIRE(n == -1);

        // Check stealing.
        real r{123, detail::real_deduce_precision(5) + 1};
        n -= std::move(r);
        REQUIRE(n == -124);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());
    }
    {
        unsigned n = 3;
        n -= real{2};
        REQUIRE(n == 1);
        n = 1;
        REQUIRE_THROWS_AS(n -= -real{detail::nl_max<unsigned>()}, std::overflow_error);
        REQUIRE_THROWS_AS((n -= real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1u);
    }
    {
        bool n = true;
        n -= real{2};
        REQUIRE(n);
    }
    {
        long long n = 3;
        n -= real{2};
        REQUIRE(n == 1);
        n = 1;
        REQUIRE_THROWS_AS(n -= -real{detail::nl_max<long long>()}, std::overflow_error);
        REQUIRE_THROWS_AS((n -= real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1);
        n = -1;
        REQUIRE_THROWS_AS(n -= -real{detail::nl_min<long long>()}, std::overflow_error);
        REQUIRE(n == -1);
    }
    {
        unsigned long long n = 3;
        n -= real{2};
        REQUIRE(n == 1);
        n = 1;
        REQUIRE_THROWS_AS(n -= -real{detail::nl_max<unsigned long long>()}, std::overflow_error);
        REQUIRE_THROWS_AS((n -= real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1u);
    }
    // Floating-point.
    {
        float x = 3;
        x -= real{2};
        REQUIRE(x == 1.f);
        if (std::numeric_limits<float>::is_iec559) {
            x = -detail::nl_max<float>();
            x -= real{detail::nl_max<float>()};
            REQUIRE(std::isinf(x));
        }
    }
    {
        double x = 3;
        x -= real{2};
        REQUIRE(x == 1.);
        if (std::numeric_limits<double>::is_iec559) {
            x = -detail::nl_max<double>();
            x -= real{detail::nl_max<double>()};
            REQUIRE(std::isinf(x));
        }
    }
    {
        long double x = 3;
        x -= real{2};
        REQUIRE(x == 1.l);
        if (std::numeric_limits<long double>::is_iec559) {
            x = -detail::nl_max<long double>();
            x -= real{detail::nl_max<long double>()};
            REQUIRE(std::isinf(x));
        }
    }
    // Integer.
    {
        int_t n{3};
        n -= real{2};
        REQUIRE(n == 1);
        n = 1;
        REQUIRE_THROWS_AS((n -= real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1);
    }
    // Rational.
    {
        rat_t n{3};
        n -= real{2};
        REQUIRE(n == 1);
        n = 1;
        REQUIRE_THROWS_AS((n -= real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1);

        n = -1 / 10_q1;
        n -= 1_r512;
        REQUIRE(abs(n + 1.1_r512) <= pow(2_r512, -500));
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        real128 x{3};
        x -= real{2};
        REQUIRE(x == 1);
        x = -real128_max();
        x -= real{real128_max()};
        REQUIRE(isinf(x));
    }
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    {
        __int128_t n128 = 5;
        n128 -= real{2};
        REQUIRE(n128 == 3);
        __int128_t un128 = 5;
        un128 -= real{2};
        REQUIRE(un128 == 3);
    }
#endif
}

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("real binary mul")
{
    real r0, r1;
    REQUIRE(real{} * real{} == real{});
    REQUIRE((real{} * real{}).get_prec() == real_prec_min());
    r0 = 23;
    r1 = -1;
    REQUIRE(r0 * r1 == real{-23});
    REQUIRE(std::move(r0) * r1 == real{-23});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
    r0 = real{23};
    REQUIRE(r0 * std::move(r1) == real{-23});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r1.get_mpfr_t()->_mpfr_d);
    r1 = real{-1};
    REQUIRE(std::move(r0) * std::move(r1) == real{-23});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_mpfr_t()->_mpfr_d);
    r0 = real{23};
    REQUIRE((real{1, 10} * real{2, 20} == real{2}));
    REQUIRE((real{1, 10} * real{2, 20}).get_prec() == 20);
    REQUIRE((real{1, 20} * real{2, 10} == real{2}));
    REQUIRE((real{1, 20} * real{2, 10}).get_prec() == 20);
    // Integrals.
    REQUIRE((real{1, 10} * 10 == real{10}));
    REQUIRE((real{1, 10} * wchar_t{10} == real{10}));
    REQUIRE((real{1, 10} * 10).get_prec() == detail::nl_digits<int>() + 1);
    REQUIRE((10 * real{1, 10} == real{10}));
    REQUIRE((wchar_t{10} * real{1, 10} == real{10}));
    REQUIRE((10 * real{1, 10}).get_prec() == detail::nl_digits<int>() + 1);
    REQUIRE((real{1, 100} * 10 == real{10}));
    REQUIRE((real{1, 100} * 10).get_prec() == std::max(100, detail::nl_digits<int>() + 1));
    REQUIRE((10 * real{1, 100} == real{10}));
    REQUIRE((10 * real{1, 100}).get_prec() == std::max(100, detail::nl_digits<int>() + 1));
    REQUIRE((real{1, 10} * true == real{1}));
    REQUIRE((real{1, 10} * true).get_prec() == 10);
    REQUIRE((false * real{1, 10} == real{0}));
    REQUIRE((false * real{1, 10}).get_prec() == 10);
    REQUIRE((real{1, 10} * 10u == real{10}));
    REQUIRE((real{1, 10} * 10u).get_prec() == detail::nl_digits<unsigned>());
    REQUIRE((10u * real{1, 10} == real{10}));
    REQUIRE((10u * real{1, 10}).get_prec() == detail::nl_digits<unsigned>());
    REQUIRE((real{1, 100} * 10u == real{10}));
    REQUIRE((real{1, 100} * 10u).get_prec() == std::max(100, detail::nl_digits<unsigned>()));
    REQUIRE((10u * real{1, 100} == real{10}));
    REQUIRE((10u * real{1, 100}).get_prec() == std::max(100, detail::nl_digits<unsigned>()));
    REQUIRE((real{1, 10} * 10ll == real{10}));
    REQUIRE((real{1, 10} * 10ll).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE((real{1, 10} * detail::nl_max<long long>() == real{detail::nl_max<long long>()}));
    REQUIRE((real{1, 10} * detail::nl_max<long long>()).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE((real{1, 10} * detail::nl_min<long long>() == real{detail::nl_min<long long>()}));
    REQUIRE((real{1, 10} * detail::nl_min<long long>()).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE((10ll * real{1, 10} == real{10}));
    REQUIRE((10ll * real{1, 10}).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE((real{1, 100} * 10ll == real{10}));
    REQUIRE((real{1, 100} * 10ll).get_prec() == std::max(100, detail::nl_digits<long long>() + 1));
    REQUIRE((10ll * real{1, 100} == real{10}));
    REQUIRE((10ll * real{1, 100}).get_prec() == std::max(100, detail::nl_digits<long long>() + 1));
    REQUIRE((real{1, 10} * 10ull == real{10}));
    REQUIRE((real{1, 10} * 10ull).get_prec() == detail::nl_digits<unsigned long long>());
    REQUIRE((10ull * real{1, 10} == real{10}));
    REQUIRE((10ull * real{1, 10}).get_prec() == detail::nl_digits<unsigned long long>());
    REQUIRE((real{1, 10} * detail::nl_max<unsigned long long>() == real{detail::nl_max<unsigned long long>()}));
    REQUIRE((real{1, 10} * detail::nl_max<unsigned long long>()).get_prec() == detail::nl_digits<unsigned long long>());
    REQUIRE((real{1, 100} * 10ull == real{10}));
    REQUIRE((real{1, 100} * 10ull).get_prec() == std::max(100, detail::nl_digits<unsigned long long>()));
    REQUIRE((10ull * real{1, 100} == real{10}));
    REQUIRE((10ull * real{1, 100}).get_prec() == std::max(100, detail::nl_digits<unsigned long long>()));
    // Floating-point.
    REQUIRE((real{1, 10} * 10.f == real{10}));
    REQUIRE((real{1, 10} * 10.f).get_prec() == detail::dig2mpfr_prec<float>());
    REQUIRE((10.f * real{1, 10} == real{10}));
    REQUIRE((10.f * real{1, 10}).get_prec() == detail::dig2mpfr_prec<float>());
    REQUIRE((real{1, 100} * 10.f == real{10}));
    REQUIRE((real{1, 100} * 10.f).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<float>()));
    REQUIRE((10.f * real{1, 100} == real{10}));
    REQUIRE((10.f * real{1, 100}).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<float>()));
    REQUIRE((real{1, 10} * 10. == real{10}));
    REQUIRE((real{1, 10} * 10.).get_prec() == detail::dig2mpfr_prec<double>());
    REQUIRE((10. * real{1, 10} == real{10}));
    REQUIRE((10. * real{1, 10}).get_prec() == detail::dig2mpfr_prec<double>());
    REQUIRE((real{1, 100} * 10. == real{10}));
    REQUIRE((real{1, 100} * 10.).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<double>()));
    REQUIRE((10. * real{1, 100} == real{10}));
    REQUIRE((10. * real{1, 100}).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<double>()));
    REQUIRE((real{1, 10} * 10.l == real{10}));
    REQUIRE((real{1, 10} * 10.l).get_prec() == detail::dig2mpfr_prec<long double>());
    REQUIRE((10.l * real{1, 10} == real{10}));
    REQUIRE((10.l * real{1, 10}).get_prec() == detail::dig2mpfr_prec<long double>());
    REQUIRE((real{1, 100} * 10.l == real{10}));
    REQUIRE((real{1, 100} * 10.l).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<long double>()));
    REQUIRE((10.l * real{1, 100} == real{10}));
    REQUIRE((10.l * real{1, 100}).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<long double>()));
    // Integer.
    REQUIRE((real{1, 10} * int_t{10} == real{10}));
    REQUIRE((real{1, 10} * int_t{10}).get_prec() == GMP_NUMB_BITS);
    REQUIRE((int_t{10} * real{1, 10} == real{10}));
    REQUIRE((int_t{10} * real{1, 10}).get_prec() == GMP_NUMB_BITS);
    REQUIRE((real{1, 100} * int_t{10} == real{10}));
    REQUIRE((real{1, 100} * int_t{10}).get_prec() == std::max(100, GMP_NUMB_BITS));
    REQUIRE((int_t{10} * real{1, 100} == real{10}));
    REQUIRE((int_t{10} * real{1, 100}).get_prec() == std::max(100, GMP_NUMB_BITS));
    // Rational.
    REQUIRE((real{1, 10} * rat_t{10} == real{10}));
    REQUIRE((real{1, 10} * rat_t{10}).get_prec() == GMP_NUMB_BITS * 2);
    REQUIRE((rat_t{10} * real{1, 10} == real{10}));
    REQUIRE((rat_t{10} * real{1, 10}).get_prec() == GMP_NUMB_BITS * 2);
    REQUIRE((real{1, 100} * rat_t{10} == real{10}));
    REQUIRE((real{1, 100} * rat_t{10}).get_prec() == std::max(100, GMP_NUMB_BITS * 2));
    REQUIRE((rat_t{10} * real{1, 100} == real{10}));
    REQUIRE((rat_t{10} * real{1, 100}).get_prec() == std::max(100, GMP_NUMB_BITS * 2));
#if defined(MPPP_WITH_QUADMATH)
    REQUIRE((real{1, 10} * real128{10} == real{10}));
    REQUIRE((real{1, 10} * real128{10}).get_prec() == 113);
    REQUIRE((real128{10} * real{1, 10} == real{10}));
    REQUIRE((real128{10} * real{1, 10}).get_prec() == 113);
    REQUIRE((real{1, 200} * real128{10} == real{10}));
    REQUIRE((real{1, 200} * real128{10}).get_prec() == 200);
    REQUIRE((real128{10} * real{1, 200} == real{10}));
    REQUIRE((real128{10} * real{1, 200}).get_prec() == 200);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE((real{1, 10} * __int128_t{10} == real{10}));
    REQUIRE((real{1, 10} * __int128_t{10}).get_prec() == 128);
    REQUIRE((__int128_t{10} * real{1, 10} == real{10}));
    REQUIRE((__int128_t{10} * real{1, 10}).get_prec() == 128);
    REQUIRE((real{1, 10} * __uint128_t{10} == real{10}));
    REQUIRE((real{1, 10} * __uint128_t{10}).get_prec() == 128);
    REQUIRE((__uint128_t{10} * real{1, 10} == real{10}));
    REQUIRE((__uint128_t{10} * real{1, 10}).get_prec() == 128);
    REQUIRE((real{1, 200} * __int128_t{10} == real{10}));
    REQUIRE((real{1, 200} * __int128_t{10}).get_prec() == 200);
    REQUIRE((__int128_t{10} * real{1, 200} == real{10}));
    REQUIRE((__int128_t{10} * real{1, 200}).get_prec() == 200);
    REQUIRE((real{1, 200} * __uint128_t{10} == real{10}));
    REQUIRE((real{1, 200} * __uint128_t{10}).get_prec() == 200);
    REQUIRE((__uint128_t{10} * real{1, 200} == real{10}));
    REQUIRE((__uint128_t{10} * real{1, 200}).get_prec() == 200);

    // Try also large values.
    REQUIRE(1.23_r512 * (__int128_t{1} << 65) == 1.23_r512 * pow(2_r128, 65));
    REQUIRE((__int128_t{1} << 65) * 1.23_r512 == 1.23_r512 * pow(2_r128, 65));

    REQUIRE(1.23_r512 * (__uint128_t{1} << 65) == 1.23_r512 * pow(2_r128, 65));
    REQUIRE((__uint128_t{1} << 65) * 1.23_r512 == 1.23_r512 * pow(2_r128, 65));
#endif

    // Ensure correct precision handling if one operand (i.e., rational)
    // cannot be converted exactly to real.
    REQUIRE(abs((1_r512 * (1 / 10_q1)) - 0.1_r512) < pow(2_r512, -510));
    REQUIRE(abs(((1 / 10_q1) * 1_r512) - 0.1_r512) < pow(2_r512, -510));
}

TEST_CASE("real left in-place mul")
{
    real r0, r1;
    const real r1_const;
    r0 *= r1;
    REQUIRE(r0.zero_p());
    REQUIRE(!r0.signbit());
    r0 *= r1_const;
    REQUIRE(r0.zero_p());
    REQUIRE(!r0.signbit());
    r0 = 5;
    r1 = 6;
    r0 *= r1;
    REQUIRE(r0 == real{30});
    r0 = real{1, real_prec_min()};
    r0 *= real{12345678ll};
    REQUIRE(r0 == real{12345678ll});
    REQUIRE(r0.get_prec() == detail::nl_digits<long long>() + 1);
    // Integrals.
    r0 = real{1, real_prec_min()};
    r0 *= 123;
    REQUIRE(r0 == real{123});
    REQUIRE(r0.get_prec() == detail::nl_digits<int>() + 1);
    r0 = real{1, real_prec_min()};
    r0 *= 123u;
    REQUIRE(r0 == real{123u});
    REQUIRE(r0.get_prec() == detail::nl_digits<unsigned>());
    r0 = real{1, real_prec_min()};
    r0 *= true;
    REQUIRE(r0 == real{1});
    REQUIRE(r0.get_prec() == std::max<::mpfr_prec_t>(detail::nl_digits<bool>(), real_prec_min()));
    r0 = real{1, real_prec_min()};
    r0 *= 123ll;
    REQUIRE(r0 == real{123ll});
    REQUIRE(r0.get_prec() == detail::nl_digits<long long>() + 1);
    r0 = real{1, real_prec_min()};
    r0 *= detail::nl_max<long long>();
    REQUIRE(r0 == real{detail::nl_max<long long>()});
    REQUIRE(r0.get_prec() == detail::nl_digits<long long>() + 1);
    r0 = real{1, real_prec_min()};
    r0 *= detail::nl_min<long long>();
    REQUIRE(r0 == real{detail::nl_min<long long>()});
    REQUIRE(r0.get_prec() == detail::nl_digits<long long>() + 1);
    r0 = real{1, real_prec_min()};
    r0 *= 123ull;
    REQUIRE(r0 == real{123ull});
    REQUIRE(r0.get_prec() == detail::nl_digits<unsigned long long>());
    r0 = real{1, real_prec_min()};
    r0 *= detail::nl_max<unsigned long long>();
    REQUIRE(r0 == real{detail::nl_max<unsigned long long>()});
    REQUIRE(r0.get_prec() == detail::nl_digits<unsigned long long>());
    // Floating-point.
    r0 = real{1, real_prec_min()};
    r0 *= 123.f;
    REQUIRE(r0 == real{123.f});
    REQUIRE(r0.get_prec() == detail::dig2mpfr_prec<float>());
    r0 = real{1, real_prec_min()};
    r0 *= 123.;
    REQUIRE(r0 == real{123.});
    REQUIRE(r0.get_prec() == detail::dig2mpfr_prec<double>());
    r0 = real{1, real_prec_min()};
    r0 *= 123.l;
    REQUIRE(r0 == real{123.l});
    REQUIRE(r0.get_prec() == detail::dig2mpfr_prec<long double>());
    // Integer.
    r0 = real{1, real_prec_min()};
    r0 *= int_t{123};
    REQUIRE(r0 == real{int_t{123}});
    REQUIRE(r0.get_prec() == GMP_NUMB_BITS);
    // Rational.
    r0 = real{1, real_prec_min()};
    r0 *= rat_t{123};
    REQUIRE(r0 == real{rat_t{123}});
    REQUIRE(r0.get_prec() == GMP_NUMB_BITS * 2);

    r0 = 1_r512;
    r0 *= 1 / 10_q1;
    REQUIRE(abs(r0 - .1_r512) <= pow(2_r512, -500));
#if defined(MPPP_WITH_QUADMATH)
    r0 = real{1, real_prec_min()};
    r0 *= real128{123};
    REQUIRE(r0 == real{real128{123}});
    REQUIRE(r0.get_prec() == 113);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    r0 = real{-1};
    REQUIRE((r0 *= __int128_t{10}) == real{-10});
    REQUIRE(r0.get_prec() == 128);
    r0 = real{-1};
    REQUIRE((r0 *= __uint128_t{10}) == real{-10});
    REQUIRE(r0.get_prec() == 128);

    // Try also large values.
    r0 = real{1, 32};
    REQUIRE((r0 *= (__int128_t{1}) << 65) == pow(2_r128, 65));
    REQUIRE(r0.get_prec() == 128);

    r0 = real{1, 32};
    REQUIRE((r0 *= (__uint128_t{1}) << 65) == pow(2_r128, 65));
    REQUIRE(r0.get_prec() == 128);
#endif

    // Check stealing move semantics.
    r0 = real{42};
    r1 = real{2, detail::real_deduce_precision(0) * 10};
    r0 *= std::move(r1);
    REQUIRE(r0 == 84);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0) * 10);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1 == 42);
}

TEST_CASE("real right in-place mul")
{
    // Integrals.
    {
        int n = 3;
        n *= real{2};
        REQUIRE(n == 6);
        n = 2;
        REQUIRE_THROWS_AS(n *= real{detail::nl_max<int>()}, std::overflow_error);
        REQUIRE_THROWS_AS((n *= real{"inf", 5}), std::domain_error);
        REQUIRE(n == 2);
        n = -2;
        REQUIRE_THROWS_AS(n *= real{detail::nl_min<int>()}, std::overflow_error);
        REQUIRE(n == -2);

        // Check stealing.
        real r{2, detail::real_deduce_precision(5) + 1};
        n *= std::move(r);
        REQUIRE(n == -4);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());
    }
    {
        unsigned n = 3;
        n *= real{2};
        REQUIRE(n == 6);
        n = 2;
        REQUIRE_THROWS_AS(n *= real{detail::nl_max<unsigned>()}, std::overflow_error);
        REQUIRE_THROWS_AS((n *= real{"inf", 5}), std::domain_error);
        REQUIRE(n == 2u);
    }
    {
        bool n = true;
        n *= real{2};
        REQUIRE(n);
    }
    {
        long long n = 3;
        n *= real{2};
        REQUIRE(n == 6);
        n = 2;
        REQUIRE_THROWS_AS(n *= real{detail::nl_max<long long>()}, std::overflow_error);
        REQUIRE_THROWS_AS((n *= real{"inf", 5}), std::domain_error);
        REQUIRE(n == 2);
        n = -2;
        REQUIRE_THROWS_AS(n *= real{detail::nl_min<long long>()}, std::overflow_error);
        REQUIRE(n == -2);
    }
    {
        unsigned long long n = 3;
        n *= real{2};
        REQUIRE(n == 6);
        n = 2;
        REQUIRE_THROWS_AS(n *= real{detail::nl_max<unsigned long long>()}, std::overflow_error);
        REQUIRE_THROWS_AS((n *= real{"inf", 5}), std::domain_error);
        REQUIRE(n == 2u);
    }
    // Floating-point.
    {
        float x = 3;
        x *= real{2};
        REQUIRE(x == 6.f);
        if (std::numeric_limits<float>::is_iec559) {
            x = detail::nl_max<float>();
            x *= real{detail::nl_max<float>()};
            REQUIRE(std::isinf(x));
        }
    }
    {
        double x = 3;
        x *= real{2};
        REQUIRE(x == 6.);
        if (std::numeric_limits<double>::is_iec559) {
            x = detail::nl_max<double>();
            x *= real{detail::nl_max<double>()};
            REQUIRE(std::isinf(x));
        }
    }
    {
        long double x = 3;
        x *= real{2};
        REQUIRE(x == 6.l);
        if (std::numeric_limits<long double>::is_iec559) {
            x = detail::nl_max<long double>();
            x *= real{detail::nl_max<long double>()};
            REQUIRE(std::isinf(x));
        }
    }
    // Integer.
    {
        int_t n{3};
        n *= real{2};
        REQUIRE(n == 6);
        n = 1;
        REQUIRE_THROWS_AS((n *= real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1);
    }
    // Rational.
    {
        rat_t n{3};
        n *= real{2};
        REQUIRE(n == 6);
        n = 1;
        REQUIRE_THROWS_AS((n *= real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1);

        n = 1 / 10_q1;
        n *= 1_r512;
        REQUIRE(abs(n - .1_r512) <= pow(2_r512, -500));
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        real128 x{3};
        x *= real{2};
        REQUIRE(x == 6);
        x = real128_max();
        x *= real{real128_max()};
        REQUIRE(isinf(x));
    }
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    {
        __int128_t n128 = 5;
        n128 *= real{2};
        REQUIRE(n128 == 10);
        __int128_t un128 = 5;
        un128 *= real{2};
        REQUIRE(un128 == 10);
    }
#endif
}

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("real binary div")
{
    real r0, r1;
    REQUIRE((real{} / real{}).nan_p());
    REQUIRE((real{} / real{}).get_prec() == real_prec_min());
    r0 = 23;
    r1 = -1;
    REQUIRE(r0 / r1 == real{-23});
    REQUIRE(std::move(r0) / r1 == real{-23});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
    r0 = real{23};
    REQUIRE(r0 / std::move(r1) == real{-23});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r1.get_mpfr_t()->_mpfr_d);
    r1 = real{-1};
    REQUIRE(std::move(r0) / std::move(r1) == real{-23});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_mpfr_t()->_mpfr_d);
    r0 = real{23};
    REQUIRE((real{1, 10} / real{2, 20} == real{".5", 10}));
    REQUIRE((real{1, 10} / real{2, 20}).get_prec() == 20);
    REQUIRE((real{1, 20} / real{2, 10} == real{".5", 10}));
    REQUIRE((real{1, 20} / real{2, 10}).get_prec() == 20);
    // Integrals.
    REQUIRE((real{5, 10} / 10 == real{".5", 10}));
    REQUIRE((real{5, 10} / wchar_t{10} == real{".5", 10}));
    REQUIRE((real{5, 10} / 10).get_prec() == detail::nl_digits<int>() + 1);
    REQUIRE((10 / real{1, 10} == real{10}));
    REQUIRE((wchar_t{10} / real{1, 10} == real{10}));
    REQUIRE((10 / real{1, 10}).get_prec() == detail::nl_digits<int>() + 1);
    REQUIRE((real{5, 100} / 10 == real{".5", 10}));
    REQUIRE((real{5, 100} / 10).get_prec() == std::max(100, detail::nl_digits<int>() + 1));
    REQUIRE((10 / real{1, 100} == real{10}));
    REQUIRE((10 / real{1, 100}).get_prec() == std::max(100, detail::nl_digits<int>() + 1));
    REQUIRE((real{1, 10} / true == real{1}));
    REQUIRE((real{1, 10} / true).get_prec() == 10);
    REQUIRE((false / real{1, 10} == real{0}));
    REQUIRE((false / real{1, 10}).get_prec() == 10);
    REQUIRE((real{5, 10} / 10u == real{".5", 10}));
    REQUIRE((real{5, 10} / 10u).get_prec() == detail::nl_digits<unsigned>());
    REQUIRE((10u / real{1, 10} == real{10}));
    REQUIRE((10u / real{1, 10}).get_prec() == detail::nl_digits<unsigned>());
    REQUIRE((real{5, 100} / 10u == real{".5", 10}));
    REQUIRE((real{5, 100} / 10u).get_prec() == std::max(100, detail::nl_digits<unsigned>()));
    REQUIRE((10u / real{1, 100} == real{10}));
    REQUIRE((10u / real{1, 100}).get_prec() == std::max(100, detail::nl_digits<unsigned>()));
    REQUIRE((real{5, 10} / 10ll == real{".5", 10}));
    REQUIRE((real{5, 10} / 10ll).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE((real{1, 10} / detail::nl_max<long long>() == 1 / real{detail::nl_max<long long>()}));
    REQUIRE((real{1, 10} / detail::nl_max<long long>()).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE((real{1, 10} / detail::nl_min<long long>() == 1 / real{detail::nl_min<long long>()}));
    REQUIRE((real{1, 10} / detail::nl_min<long long>()).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE((10ll / real{1, 10} == real{10}));
    REQUIRE((10ll / real{1, 10}).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE((real{5, 100} / 10ll == real{".5", 10}));
    REQUIRE((real{5, 100} / 10ll).get_prec() == std::max(100, detail::nl_digits<long long>() + 1));
    REQUIRE((10ll / real{1, 100} == real{10}));
    REQUIRE((10ll / real{1, 100}).get_prec() == std::max(100, detail::nl_digits<long long>() + 1));
    REQUIRE((real{5, 10} / 10ull == real{".5", 10}));
    REQUIRE((real{5, 10} / 10ull).get_prec() == detail::nl_digits<unsigned long long>());
    REQUIRE((10ull / real{1, 10} == real{10}));
    REQUIRE((10ull / real{1, 10}).get_prec() == detail::nl_digits<unsigned long long>());
    REQUIRE((real{1, 10} / detail::nl_max<unsigned long long>() == 1 / real{detail::nl_max<unsigned long long>()}));
    REQUIRE((real{1, 10} / detail::nl_max<unsigned long long>()).get_prec() == detail::nl_digits<unsigned long long>());
    REQUIRE((real{5, 100} / 10ull == real{".5", 10}));
    REQUIRE((real{5, 100} / 10ull).get_prec() == std::max(100, detail::nl_digits<unsigned long long>()));
    REQUIRE((10ull / real{1, 100} == real{10}));
    REQUIRE((10ull / real{1, 100}).get_prec() == std::max(100, detail::nl_digits<unsigned long long>()));
    // Floating-point.
    REQUIRE((real{5, 10} / 10.f == real{".5", 10}));
    REQUIRE((real{5, 10} / 10.f).get_prec() == detail::dig2mpfr_prec<float>());
    REQUIRE((10.f / real{1, 10} == real{10}));
    REQUIRE((10.f / real{1, 10}).get_prec() == detail::dig2mpfr_prec<float>());
    REQUIRE((real{5, 100} / 10.f == real{".5", 10}));
    REQUIRE((real{5, 100} / 10.f).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<float>()));
    REQUIRE((10.f / real{1, 100} == real{10}));
    REQUIRE((10.f / real{1, 100}).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<float>()));
    REQUIRE((real{5, 10} / 10. == real{".5", 10}));
    REQUIRE((real{5, 10} / 10.).get_prec() == detail::dig2mpfr_prec<double>());
    REQUIRE((10. / real{1, 10} == real{10}));
    REQUIRE((10. / real{1, 10}).get_prec() == detail::dig2mpfr_prec<double>());
    REQUIRE((real{5, 100} / 10. == real{".5", 10}));
    REQUIRE((real{5, 100} / 10.).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<double>()));
    REQUIRE((10. / real{1, 100} == real{10}));
    REQUIRE((10. / real{1, 100}).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<double>()));
    REQUIRE((real{5, 10} / 10.l == real{".5", 10}));
    REQUIRE((real{5, 10} / 10.l).get_prec() == detail::dig2mpfr_prec<long double>());
    REQUIRE((10.l / real{1, 10} == real{10}));
    REQUIRE((10.l / real{1, 10}).get_prec() == detail::dig2mpfr_prec<long double>());
    REQUIRE((real{5, 100} / 10.l == real{".5", 10}));
    REQUIRE((real{5, 100} / 10.l).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<long double>()));
    REQUIRE((10.l / real{1, 100} == real{10}));
    REQUIRE((10.l / real{1, 100}).get_prec() == std::max<::mpfr_prec_t>(100, detail::dig2mpfr_prec<long double>()));
    // Integer.
    REQUIRE((real{5, 10} / int_t{10} == real{".5", 10}));
    REQUIRE((real{5, 10} / int_t{10}).get_prec() == GMP_NUMB_BITS);
    REQUIRE((int_t{10} / real{1, 10} == real{10}));
    REQUIRE((int_t{10} / real{1, 10}).get_prec() == GMP_NUMB_BITS);
    REQUIRE((real{5, 100} / int_t{10} == real{".5", 10}));
    REQUIRE((real{5, 100} / int_t{10}).get_prec() == std::max(100, GMP_NUMB_BITS));
    REQUIRE((int_t{10} / real{1, 100} == real{10}));
    REQUIRE((int_t{10} / real{1, 100}).get_prec() == std::max(100, GMP_NUMB_BITS));
    // Rational.
    REQUIRE((real{5, 10} / rat_t{10} == real{".5", 10}));
    REQUIRE((real{5, 10} / rat_t{10}).get_prec() == GMP_NUMB_BITS * 2);
    REQUIRE((rat_t{10} / real{1, 10} == real{10}));
    REQUIRE((rat_t{10} / real{1, 10}).get_prec() == GMP_NUMB_BITS * 2);
    REQUIRE((real{5, 100} / rat_t{10} == real{".5", 10}));
    REQUIRE((real{5, 100} / rat_t{10}).get_prec() == std::max(100, GMP_NUMB_BITS * 2));
    REQUIRE((rat_t{10} / real{1, 100} == real{10}));
    REQUIRE((rat_t{10} / real{1, 100}).get_prec() == std::max(100, GMP_NUMB_BITS * 2));
#if defined(MPPP_WITH_QUADMATH)
    REQUIRE((real{5, 10} / real128{10} == real{".5", 10}));
    REQUIRE((real{5, 10} / real128{10}).get_prec() == 113);
    REQUIRE((real128{10} / real{1, 10} == real{10}));
    REQUIRE((real128{10} / real{1, 10}).get_prec() == 113);
    REQUIRE((real{5, 200} / real128{10} == real{".5", 10}));
    REQUIRE((real{5, 200} / real128{10}).get_prec() == 200);
    REQUIRE((real128{10} / real{1, 200} == real{10}));
    REQUIRE((real128{10} / real{1, 200}).get_prec() == 200);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE((real{5, 10} / __int128_t{10} == real{1} / 2));
    REQUIRE((real{1, 10} / __int128_t{10}).get_prec() == 128);
    REQUIRE((__int128_t{10} / real{2, 10} == real{5}));
    REQUIRE((__int128_t{10} / real{2, 10}).get_prec() == 128);
    REQUIRE((real{5, 10} / __uint128_t{10} == real{1} / 2));
    REQUIRE((real{1, 10} / __uint128_t{10}).get_prec() == 128);
    REQUIRE((__uint128_t{10} / real{2, 10} == real{5}));
    REQUIRE((__uint128_t{10} / real{1, 10}).get_prec() == 128);
    REQUIRE((real{5, 200} / __int128_t{10} == real{1} / 2));
    REQUIRE((real{1, 200} / __int128_t{10}).get_prec() == 200);
    REQUIRE((__int128_t{10} / real{5, 200} == real{2}));
    REQUIRE((__int128_t{10} / real{1, 200}).get_prec() == 200);
    REQUIRE((real{5, 200} / __uint128_t{10} == real{1} / 2));
    REQUIRE((real{1, 200} / __uint128_t{10}).get_prec() == 200);
    REQUIRE((__uint128_t{10} / real{2, 200} == real{5}));
    REQUIRE((__uint128_t{10} / real{1, 200}).get_prec() == 200);

    // Try also large values.
    REQUIRE(1.23_r512 / (__int128_t{1} << 65) == 1.23_r512 / pow(2_r128, 65));
    REQUIRE((__int128_t{1} << 65) / 1.23_r512 == pow(2_r128, 65) / 1.23_r512);

    REQUIRE(1.23_r512 / (__uint128_t{1} << 65) == 1.23_r512 / pow(2_r128, 65));
    REQUIRE((__uint128_t{1} << 65) / 1.23_r512 == pow(2_r128, 65) / 1.23_r512);
#endif

    // Ensure correct precision handling if one operand (i.e., rational)
    // cannot be converted exactly to real.
    REQUIRE(abs((1_r512 / (1 / 10_q1)) - 10_r512) < pow(2_r512, -510));
    REQUIRE(abs(((1 / 10_q1) / 1_r512) - .1_r512) < pow(2_r512, -510));
}

TEST_CASE("real left in-place div")
{
    real r0, r1;
    const real r1_const;
    r0 /= r1;
    REQUIRE(r0.nan_p());
    r0 /= r1_const;
    REQUIRE(r0.nan_p());
    r0 = 5;
    r1 = 2;
    r0 /= r1;
    REQUIRE((r0 == real{"2.5", 10}));
    r0 = real{1, real_prec_min()};
    r0 /= real{12345678ll};
    REQUIRE(r0 == 1 / real{12345678ll});
    REQUIRE(r0.get_prec() == detail::nl_digits<long long>() + 1);
    // Integrals.
    r0 = real{1, real_prec_min()};
    r0 /= 123;
    REQUIRE(r0 == 1 / real{123});
    REQUIRE(r0.get_prec() == detail::nl_digits<int>() + 1);
    r0 = real{1, real_prec_min()};
    r0 /= 123u;
    REQUIRE(r0 == 1 / real{123u});
    REQUIRE(r0.get_prec() == detail::nl_digits<unsigned>());
    r0 = real{1, real_prec_min()};
    r0 /= true;
    REQUIRE(r0 == real{1});
    REQUIRE(r0.get_prec() == std::max<::mpfr_prec_t>(detail::nl_digits<bool>(), real_prec_min()));
    r0 = real{1, real_prec_min()};
    r0 /= 123ll;
    REQUIRE(r0 == 1 / real{123ll});
    REQUIRE(r0.get_prec() == detail::nl_digits<long long>() + 1);
    r0 = real{1, real_prec_min()};
    r0 /= detail::nl_max<long long>();
    REQUIRE(r0 == 1 / real{detail::nl_max<long long>()});
    REQUIRE(r0.get_prec() == detail::nl_digits<long long>() + 1);
    r0 = real{1, real_prec_min()};
    r0 /= detail::nl_min<long long>();
    REQUIRE(r0 == 1 / real{detail::nl_min<long long>()});
    REQUIRE(r0.get_prec() == detail::nl_digits<long long>() + 1);
    r0 = real{1, real_prec_min()};
    r0 /= 123ull;
    REQUIRE(r0 == 1 / real{123ull});
    REQUIRE(r0.get_prec() == detail::nl_digits<unsigned long long>());
    r0 = real{1, real_prec_min()};
    r0 /= detail::nl_max<unsigned long long>();
    REQUIRE(r0 == 1 / real{detail::nl_max<unsigned long long>()});
    REQUIRE(r0.get_prec() == detail::nl_digits<unsigned long long>());
    // Floating-point.
    r0 = real{1, real_prec_min()};
    r0 /= 123.f;
    REQUIRE(r0 == char(1) / real{123.f});
    REQUIRE(r0.get_prec() == detail::dig2mpfr_prec<float>());
    r0 = real{1, real_prec_min()};
    r0 /= 123.;
    REQUIRE(r0 == 1 / real{123.});
    REQUIRE(r0.get_prec() == detail::dig2mpfr_prec<double>());
    r0 = real{1, real_prec_min()};
    r0 /= 123.l;
    REQUIRE(r0 == 1 / real{123.l});
    REQUIRE(r0.get_prec() == detail::dig2mpfr_prec<long double>());
    // Integer.
    r0 = real{1, real_prec_min()};
    r0 /= int_t{123};
    REQUIRE(r0 == 1 / real{int_t{123}});
    REQUIRE(r0.get_prec() == GMP_NUMB_BITS);
    // Rational.
    r0 = real{1, real_prec_min()};
    r0 /= rat_t{123};
    REQUIRE(r0 == 1 / real{rat_t{123}});
    REQUIRE(r0.get_prec() == GMP_NUMB_BITS * 2);

    r0 = -1_r512;
    r0 /= 1 / 10_q1;
    REQUIRE(abs(r0 + 10_r512) <= pow(2_r512, -500));
#if defined(MPPP_WITH_QUADMATH)
    r0 = real{1, real_prec_min()};
    r0 /= real128{123};
    REQUIRE(r0 == 1 / real{real128{123}});
    REQUIRE(r0.get_prec() == 113);
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    r0 = real{-1};
    REQUIRE((r0 /= __int128_t{2}) == real{1} / real{-2});
    REQUIRE(r0.get_prec() == 128);
    r0 = real{-1};
    REQUIRE((r0 /= __uint128_t{2}) == 1 / real{-2});
    REQUIRE(r0.get_prec() == 128);

    // Try also large values.
    r0 = real{1};
    REQUIRE((r0 /= (__int128_t{1}) << 65) == pow(2_r128, -65));
    REQUIRE(r0.get_prec() == 128);

    r0 = real{1};
    REQUIRE((r0 /= (__uint128_t{1}) << 65) == pow(2_r128, -65));
    REQUIRE(r0.get_prec() == 128);
#endif

    // Check stealing move semantics.
    r0 = real{42};
    r1 = real{2, detail::real_deduce_precision(0) * 10};
    r0 /= std::move(r1);
    REQUIRE(r0 == 21);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0) * 10);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1 == 42);
}

TEST_CASE("real right in-place div")
{
    // Integrals.
    {
        int n = 3;
        n /= real{2};
        REQUIRE(n == 1);
        n = 2;
        REQUIRE_THROWS_AS((n /= real{0, 5}), std::domain_error);
        REQUIRE(n == 2);

        // Check stealing.
        real r{2, detail::real_deduce_precision(5) + 1};
        n /= std::move(r);
        REQUIRE(n == 1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());
    }
    {
        unsigned n = 3;
        n /= real{2};
        REQUIRE(n == 1);
        n = 2;
        REQUIRE_THROWS_AS((n /= real{0, 5}), std::domain_error);
        REQUIRE(n == 2u);
    }
    {
        bool n = true;
        n /= real{2};
        REQUIRE(n);
    }
    {
        long long n = 3;
        n /= real{2};
        REQUIRE(n == 1);
        n = 2;
        REQUIRE_THROWS_AS((n /= real{0, 5}), std::domain_error);
        REQUIRE(n == 2);
    }
    {
        unsigned long long n = 3;
        n /= real{2};
        REQUIRE(n == 1);
        n = 2;
        REQUIRE_THROWS_AS((n /= real{0, 5}), std::domain_error);
        REQUIRE(n == 2u);
    }
    // Floating-point.
    {
        float x = 4;
        x /= real{2};
        REQUIRE(x == 2.f);
        if (std::numeric_limits<float>::is_iec559) {
            x = detail::nl_max<float>();
            x /= real{detail::nl_max<float>()};
            REQUIRE(x == 1);
        }
    }
    {
        double x = 4;
        x /= real{2};
        REQUIRE(x == 2.);
        if (std::numeric_limits<double>::is_iec559) {
            x = detail::nl_max<double>();
            x /= real{detail::nl_max<double>()};
            REQUIRE(x == 1);
        }
    }
    {
        long double x = 4;
        x /= real{2};
        REQUIRE(x == 2.l);
        if (std::numeric_limits<long double>::is_iec559) {
            x = detail::nl_max<long double>();
            x /= real{detail::nl_max<long double>()};
            REQUIRE(x == 1);
        }
    }
    // Integer.
    {
        int_t n{3};
        n /= real{2};
        REQUIRE(n == 1);
        REQUIRE_THROWS_AS((n /= real{0, 5}), std::domain_error);
        REQUIRE(n == 1);
    }
    // Rational.
    {
        rat_t n{3};
        n /= real{2};
        REQUIRE((n == rat_t{3, 2}));
        n = 1;
        REQUIRE_THROWS_AS((n /= real{0, 5}), std::domain_error);
        REQUIRE(n == 1);

        n = -1 / 10_q1;
        n /= 10_r512;
        REQUIRE(abs(n + 0.01_r512) <= pow(2_r512, -500));
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        real128 x{3};
        x /= real{2};
        REQUIRE(x == real128{"1.5"});
        x = real128_max();
        x /= real{real128_max()};
        REQUIRE(x == 1);
    }
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    {
        __int128_t n128 = 5;
        n128 /= real{2};
        REQUIRE(n128 == 2);
        __int128_t un128 = 6;
        un128 /= real{2};
        REQUIRE(un128 == 3);
    }
#endif
}

TEST_CASE("real eqineq")
{
    REQUIRE(real{} == real{});
    REQUIRE(!(real{} != real{}));
    REQUIRE(real{1} == real{1});
    REQUIRE(!(real{1} != real{1}));
    REQUIRE(!(real{2} == real{1}));
    REQUIRE(real{2} != real{1});
    REQUIRE(!(real{"inf", 64} == real{45}));
    REQUIRE((real{"inf", 64} != real{45}));
    REQUIRE((-real{"inf", 64} == -real{"inf", 4}));
    REQUIRE(!(-real{"inf", 64} != -real{"inf", 4}));
    REQUIRE((real{"inf", 64} == real{"inf", 4}));
    REQUIRE(!(real{"inf", 64} != real{"inf", 4}));
    REQUIRE(!(real{"nan", 5} == real{1}));
    REQUIRE(!(real{1} == real{"nan", 5}));
    REQUIRE(!(real{"nan", 6} == real{"nan", 5}));
    REQUIRE((real{"nan", 5} != real{1}));
    REQUIRE((real{1} != real{"nan", 5}));
    REQUIRE((real{"nan", 6} != real{"nan", 5}));
    // Integrals.
    REQUIRE(!(real{} != 0));
    REQUIRE(!(real{} != wchar_t{0}));
    REQUIRE(1u == real{1});
    REQUIRE(wchar_t{1} == real{1});
    REQUIRE(!(1ll != real{1}));
    REQUIRE(!(wchar_t{1} != real{1}));
    REQUIRE(!(real{2} == 1ull));
    REQUIRE(2 != real{1});
    REQUIRE(wchar_t{2} != real{1});
    REQUIRE(!(real{"inf", 64} == 45));
    REQUIRE((real{"inf", 64} != 45l));
    REQUIRE(!(real{"nan", 5} == 1));
    REQUIRE(!(1 == real{"nan", 5}));
    REQUIRE((real{"nan", 5} != 1ul));
    REQUIRE((1l != real{"nan", 5}));
    // The bool specialisation.
    REQUIRE(real{1} == true);
    REQUIRE(real{0} == false);
    REQUIRE(true == real{1});
    REQUIRE(false == real{0});
    REQUIRE(real{1} != false);
    REQUIRE(real{0} != true);
    REQUIRE(false != real{1});
    REQUIRE(true != real{0});
    REQUIRE(!(real{"nan", 45} == true));
    REQUIRE(!(false == real{"nan", 45}));
    REQUIRE(real{"nan", 45} != true);
    REQUIRE(false != real{"nan", 45});
    // FP.
    REQUIRE(!(real{} != 0.f));
    REQUIRE(1. == real{1});
    REQUIRE(!(1.l != real{1}));
    REQUIRE(!(real{2} == 1.));
    REQUIRE(2.f != real{1});
    REQUIRE(!(real{"inf", 64} == 45.));
    REQUIRE((real{"inf", 64} != 4.l));
    REQUIRE(!(real{"nan", 5} == 1.));
    REQUIRE(!(1.l == real{"nan", 5}));
    REQUIRE((real{"nan", 5} != 1.));
    REQUIRE((1.f != real{"nan", 5}));
    if (std::numeric_limits<double>::has_quiet_NaN) {
        REQUIRE(std::numeric_limits<double>::quiet_NaN() != real{"nan", 5});
        REQUIRE(real{"nan", 5} != std::numeric_limits<double>::quiet_NaN());
        REQUIRE(!(std::numeric_limits<double>::quiet_NaN() == real{"nan", 5}));
        REQUIRE(!(real{"nan", 5} == std::numeric_limits<double>::quiet_NaN()));
    }
    // int/rat.
    REQUIRE(!(real{} != int_t{0}));
    REQUIRE(rat_t{1u} == real{1});
    REQUIRE(!(int_t{1ll} != real{1}));
    REQUIRE(!(real{2} == rat_t{1ull}));
    REQUIRE(rat_t{2} != real{1});
    REQUIRE(!(real{"inf", 64} == int_t{45}));
    REQUIRE((real{"inf", 64} != rat_t{45l}));
    REQUIRE(!(real{"nan", 5} == int_t{1}));
    REQUIRE(!(rat_t{1} == real{"nan", 5}));
    REQUIRE((real{"nan", 5} != int_t{1ul}));
    REQUIRE((rat_t{1l} != real{"nan", 5}));
#if defined(MPPP_WITH_QUADMATH)
    REQUIRE(!(real{} != real128{0}));
    REQUIRE(real128{1u} == real{1});
    REQUIRE(!(real128{1ll} != real{1}));
    REQUIRE(!(real{2} == real128{1ull}));
    REQUIRE(real128{2} != real{1});
    REQUIRE(!(real{"inf", 64} == real128{45}));
    REQUIRE((real{"inf", 64} != real128{45l}));
    REQUIRE(!(real{"nan", 5} == real128{1}));
    REQUIRE(!(real128{1} == real{"nan", 5}));
    REQUIRE((real{"nan", 5} != real128{1ul}));
    REQUIRE((real128{1l} != real{"nan", 5}));
    REQUIRE(real{"nan", 5} != real128{"nan"});
    REQUIRE(real128{"nan"} != real{"nan", 5});
    REQUIRE(!(real{"nan", 5} == real128{"nan"}));
    REQUIRE(!(real128{"nan"} == real{"nan", 5}));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE(real{-1} == __int128_t{-1});
    REQUIRE(__int128_t{-1} == real{-1});
    REQUIRE(real{2} == __uint128_t{2});
    REQUIRE(__uint128_t{2} == real{2});
    REQUIRE(real{-1} != __int128_t{-2});
    REQUIRE(__int128_t{-1} != real{-2});
    REQUIRE(real{2} != __uint128_t{3});
    REQUIRE(__uint128_t{2} != real{3});
    REQUIRE(pow(2_r128, 65) == (__uint128_t{1} << 65));
    REQUIRE((__uint128_t{1} << 65) == pow(2_r128, 65));
    REQUIRE(pow(2_r128, 65) == (__int128_t{1} << 65));
    REQUIRE((__int128_t{1} << 65) == pow(2_r128, 65));
    REQUIRE(pow(2_r128, 65) != (__uint128_t{1} << 65) + 1u);
    REQUIRE((__uint128_t{1} << 65) + 1u != pow(2_r128, 65));
    REQUIRE(pow(2_r128, 65) != (__int128_t{1} << 65) + 1);
    REQUIRE((__int128_t{1} << 65) + 1 != pow(2_r128, 65));
#endif

    // With std::complex.
    REQUIRE(real{42} == std::complex<double>{42, 0});
    REQUIRE(std::complex<double>{42, 0} == real{42});
    REQUIRE(!(real{42} != std::complex<double>{42, 0}));
    REQUIRE(!(std::complex<double>{42, 0} != real{42}));

    REQUIRE(real{42} != std::complex<double>{43, 0});
    REQUIRE(std::complex<double>{43, 0} != real{42});
    REQUIRE(!(real{42} == std::complex<double>{43, 0}));
    REQUIRE(!(std::complex<double>{43, 0} == real{42}));

    REQUIRE(real{42} != std::complex<double>{43, 1});
    REQUIRE(std::complex<double>{43, -1} != real{42});
    REQUIRE(!(real{42} == std::complex<double>{43, 2}));
    REQUIRE(!(std::complex<double>{43, -3} == real{42}));
}

TEST_CASE("real lt")
{
    REQUIRE(!(real{} < real{}));
    REQUIRE(!(real{1} < real{1}));
    REQUIRE((real{1} < real{2}));
    REQUIRE(!(real{"inf", 64} < real{45}));
    REQUIRE(!(-real{"inf", 64} < -real{"inf", 4}));
    REQUIRE(!(real{"inf", 64} < real{"inf", 4}));
    REQUIRE(!(real{"nan", 5} < real{1}));
    REQUIRE(!(real{1} < real{"nan", 5}));
    REQUIRE(!(real{"nan", 6} < real{"nan", 5}));
    // Integrals.
    REQUIRE((1u < real{2}));
    REQUIRE((wchar_t{1} < real{2}));
    REQUIRE(!(real{2} < 1ull));
    REQUIRE(!(real{2} < wchar_t{1}));
    REQUIRE(!(real{"inf", 64} < 45));
    REQUIRE(!(real{"nan", 5} < 1));
    REQUIRE(!(1 < real{"nan", 5}));
    REQUIRE(!(real{"nan", 5} < 1u));
    REQUIRE(!(1u < real{"nan", 5}));
    // The bool specialisations.
    REQUIRE(real{0} < true);
    REQUIRE(!(real{1} < true));
    REQUIRE(real{-1} < false);
    REQUIRE(!(real{"nan", 5} < false));
    REQUIRE(!(true < real{1}));
    REQUIRE(true < real{2});
    REQUIRE(!(false < real{-1}));
    REQUIRE(!(false < real{"nan", 5}));
    // FP.
    REQUIRE(!(1. < real{1}));
    REQUIRE(!(1.f < real{1}));
    REQUIRE(!(1.l < real{1}));
    REQUIRE((real{0.1} < 1.));
    REQUIRE((real{0.1} < 1.f));
    REQUIRE((real{0.1} < 1.l));
    REQUIRE(!(real{"inf", 64} < 45.));
    REQUIRE(!(real{"nan", 5} < 1.));
    REQUIRE(!(real{"nan", 5} < 1.f));
    REQUIRE(!(real{"nan", 5} < 1.l));
    REQUIRE(!(1. < real{"nan", 5}));
    REQUIRE(!(1.f < real{"nan", 5}));
    REQUIRE(!(1.l < real{"nan", 5}));
    if (std::numeric_limits<double>::has_quiet_NaN) {
        REQUIRE(!(std::numeric_limits<double>::quiet_NaN() < real{"nan", 5}));
        REQUIRE(!(real{"nan", 5} < std::numeric_limits<double>::quiet_NaN()));
    }
    // int/rat.
    REQUIRE((rat_t{0u} < real{1}));
    REQUIRE(!(real{2} < rat_t{1ull}));
    REQUIRE(!(real{"inf", 64} < int_t{45}));
    REQUIRE(!(real{"nan", 5} < int_t{1}));
    REQUIRE(!(int_t{1} < real{"nan", 5}));
    REQUIRE(!(rat_t{1} < real{"nan", 5}));
    REQUIRE(!(real{"nan", 5} < rat_t{1}));
#if defined(MPPP_WITH_QUADMATH)
    REQUIRE((real128{} < real{1}));
    REQUIRE(!(real{2} < real128{1ull}));
    REQUIRE(!(real{"inf", 64} < real128{45}));
    REQUIRE(!(real{"nan", 5} < real128{1}));
    REQUIRE(!(real128{1} < real{"nan", 5}));
    REQUIRE(!(real{"nan", 5} < real128{"nan"}));
    REQUIRE(!(real128{"nan"} < real{"nan", 5}));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE(real{-2} < __int128_t{-1});
    REQUIRE(__int128_t{-2} < real{-1});
    REQUIRE(real{2} < __uint128_t{3});
    REQUIRE(__uint128_t{2} < real{3});
    REQUIRE(pow(2_r128, 65) < (__uint128_t{1} << 65) + 1u);
    REQUIRE((__uint128_t{1} << 65) < pow(2_r128, 65) + 1);
    REQUIRE(pow(2_r128, 65) < (__int128_t{1} << 65) + 1);
    REQUIRE((__int128_t{1} << 65) < pow(2_r128, 65) + 1);
#endif
}

TEST_CASE("real lte")
{
    REQUIRE((real{} <= real{}));
    REQUIRE((real{1} <= real{1}));
    REQUIRE((real{1} <= real{2}));
    REQUIRE(!(real{"inf", 64} <= real{45}));
    REQUIRE((-real{"inf", 64} <= -real{"inf", 4}));
    REQUIRE((real{"inf", 64} <= real{"inf", 4}));
    REQUIRE(!(real{"nan", 5} <= real{1}));
    REQUIRE(!(real{1} <= real{"nan", 5}));
    REQUIRE(!(real{"nan", 6} <= real{"nan", 5}));
    // Integrals.
    REQUIRE((1u <= real{2}));
    REQUIRE((wchar_t{1} <= real{2}));
    REQUIRE(!(real{2} <= 1ull));
    REQUIRE(!(real{2} <= wchar_t{1}));
    REQUIRE(!(real{"inf", 64} <= 45));
    REQUIRE(!(real{"nan", 5} <= 1));
    REQUIRE(!(1 <= real{"nan", 5}));
    REQUIRE(!(real{"nan", 5} <= 1u));
    REQUIRE(!(1u <= real{"nan", 5}));
    // The bool specialisations.
    REQUIRE(real{0} <= true);
    REQUIRE(real{1} <= true);
    REQUIRE(real{-1} <= false);
    REQUIRE(!(real{"nan", 5} <= false));
    REQUIRE(true <= real{1});
    REQUIRE(true <= real{2});
    REQUIRE(!(false <= real{-1}));
    REQUIRE(!(false <= real{"nan", 5}));
    // FP.
    REQUIRE((1. <= real{1}));
    REQUIRE((1.f <= real{1}));
    REQUIRE((1.l <= real{1}));
    REQUIRE((real{0.1} <= 1.));
    REQUIRE((real{0.1} <= 1.f));
    REQUIRE((real{0.1} <= 1.l));
    REQUIRE(!(real{"inf", 64} <= 45.));
    REQUIRE(!(real{"nan", 5} <= 1.));
    REQUIRE(!(real{"nan", 5} <= 1.f));
    REQUIRE(!(real{"nan", 5} <= 1.l));
    REQUIRE(!(1. <= real{"nan", 5}));
    REQUIRE(!(1.f <= real{"nan", 5}));
    REQUIRE(!(1.l <= real{"nan", 5}));
    if (std::numeric_limits<double>::has_quiet_NaN) {
        REQUIRE(!(std::numeric_limits<double>::quiet_NaN() <= real{"nan", 5}));
        REQUIRE(!(real{"nan", 5} <= std::numeric_limits<double>::quiet_NaN()));
    }
    // int/rat.
    REQUIRE((rat_t{0u} <= real{1}));
    REQUIRE(!(real{2} <= rat_t{1ull}));
    REQUIRE(!(real{"inf", 64} <= int_t{45}));
    REQUIRE(!(real{"nan", 5} <= int_t{1}));
    REQUIRE(!(int_t{1} <= real{"nan", 5}));
    REQUIRE(!(rat_t{1} <= real{"nan", 5}));
    REQUIRE(!(real{"nan", 5} <= rat_t{1}));
#if defined(MPPP_WITH_QUADMATH)
    REQUIRE((real128{} <= real{1}));
    REQUIRE(!(real{2} <= real128{1ull}));
    REQUIRE(!(real{"inf", 64} <= real128{45}));
    REQUIRE(!(real{"nan", 5} <= real128{1}));
    REQUIRE(!(real128{1} <= real{"nan", 5}));
    REQUIRE(!(real{"nan", 5} <= real128{"nan"}));
    REQUIRE(!(real128{"nan"} <= real{"nan", 5}));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE(real{-2} <= __int128_t{-1});
    REQUIRE(__int128_t{-2} <= real{-2});
    REQUIRE(real{2} <= __uint128_t{3});
    REQUIRE(__uint128_t{3} <= real{3});
    REQUIRE(pow(2_r128, 65) <= (__uint128_t{1} << 65) + 1u);
    REQUIRE((__uint128_t{1} << 65) <= pow(2_r128, 65) + 1);
    REQUIRE(pow(2_r128, 65) <= (__int128_t{1} << 65) + 1);
    REQUIRE((__int128_t{1} << 65) <= pow(2_r128, 65) + 1);
#endif
}

TEST_CASE("real gt")
{
    REQUIRE(!(real{} > real{}));
    REQUIRE(!(real{1} > real{1}));
    REQUIRE(!(real{1} > real{2}));
    REQUIRE((real{"inf", 64} > real{45}));
    REQUIRE(!(-real{"inf", 64} > -real{"inf", 4}));
    REQUIRE(!(real{"inf", 64} > real{"inf", 4}));
    REQUIRE(!(real{"nan", 5} > real{1}));
    REQUIRE(!(real{1} > real{"nan", 5}));
    REQUIRE(!(real{"nan", 6} > real{"nan", 5}));
    // Integrals.
    REQUIRE(!(1u > real{2}));
    REQUIRE(!(wchar_t{1} > real{2}));
    REQUIRE((real{2} > 1ull));
    REQUIRE((real{2} > wchar_t{1}));
    REQUIRE((real{"inf", 64} > 45));
    REQUIRE(!(real{"nan", 5} > 1));
    REQUIRE(!(1 > real{"nan", 5}));
    REQUIRE(!(real{"nan", 5} > 1u));
    REQUIRE(!(1u > real{"nan", 5}));
    // The bool specialisations.
    REQUIRE(real{2} > true);
    REQUIRE(!(real{1} > true));
    REQUIRE(!(real{-1} > false));
    REQUIRE(!(real{"nan", 5} > false));
    REQUIRE(true > real{0});
    REQUIRE(!(true > real{1}));
    REQUIRE(false > real{-1});
    REQUIRE(!(false > real{"nan", 5}));
    // FP.
    REQUIRE(!(1. > real{1}));
    REQUIRE(!(real{0.1} > 1.));
    REQUIRE(!(1.f > real{1}));
    REQUIRE(!(real{0.1} > 1.f));
    REQUIRE(!(1.l > real{1}));
    REQUIRE(!(real{0.1} > 1.l));
    REQUIRE((real{"inf", 64} > 45.));
    REQUIRE(!(real{"nan", 5} > 1.));
    REQUIRE(!(real{"nan", 5} > 1.f));
    REQUIRE(!(real{"nan", 5} > 1.l));
    REQUIRE(!(1. > real{"nan", 10}));
    REQUIRE(!(1.f > real{"nan", 10}));
    REQUIRE(!(1.l > real{"nan", 5}));
    if (std::numeric_limits<double>::has_quiet_NaN) {
        REQUIRE(!(std::numeric_limits<double>::quiet_NaN() > real{"nan", 5}));
        REQUIRE(!(real{"nan", 5} > std::numeric_limits<double>::quiet_NaN()));
    }
    // int/rat.
    REQUIRE(!(rat_t{0u} > real{1}));
    REQUIRE((real{2} > rat_t{1ull}));
    REQUIRE((real{"inf", 64} > int_t{45}));
    REQUIRE(!(real{"nan", 5} > int_t{1}));
    REQUIRE(!(int_t{1} > real{"nan", 5}));
    REQUIRE(!(rat_t{1} > real{"nan", 5}));
    REQUIRE(!(real{"nan", 5} > rat_t{1}));
#if defined(MPPP_WITH_QUADMATH)
    REQUIRE(!(real128{} > real{1}));
    REQUIRE((real{2} > real128{1ull}));
    REQUIRE((real{"inf", 64} > real128{45}));
    REQUIRE(!(real{"nan", 5} > real128{1}));
    REQUIRE(!(real128{1} > real{"nan", 5}));
    REQUIRE(!(real{"nan", 5} > real128{"nan"}));
    REQUIRE(!(real128{"nan"} > real{"nan", 5}));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE(real{2} > __int128_t{-1});
    REQUIRE(__int128_t{2} > real{-1});
    REQUIRE(real{5} > __uint128_t{3});
    REQUIRE(__uint128_t{5} > real{2});
    REQUIRE(pow(2_r128, 65) + 1 > (__uint128_t{1} << 65));
    REQUIRE((__uint128_t{1} << 65) + 1 > pow(2_r128, 65));
    REQUIRE(pow(2_r128, 65) + 1 > (__int128_t{1} << 65));
    REQUIRE((__int128_t{1} << 65) + 1 > pow(2_r128, 65));
#endif
}

TEST_CASE("real gte")
{
    REQUIRE((real{} >= real{}));
    REQUIRE((real{1} >= real{1}));
    REQUIRE(!(real{1} >= real{2}));
    REQUIRE((real{"inf", 64} >= real{45}));
    REQUIRE((-real{"inf", 64} >= -real{"inf", 4}));
    REQUIRE((real{"inf", 64} >= real{"inf", 4}));
    REQUIRE(!(real{"nan", 5} >= real{1}));
    REQUIRE(!(real{1} >= real{"nan", 5}));
    REQUIRE(!(real{"nan", 6} >= real{"nan", 5}));
    // Integrals.
    REQUIRE(!(1u >= real{2}));
    REQUIRE(!(wchar_t{1} >= real{2}));
    REQUIRE((real{2} >= 1ull));
    REQUIRE((real{2} >= wchar_t{1}));
    REQUIRE((real{"inf", 64} >= 45));
    REQUIRE(!(real{"nan", 5} >= 1));
    REQUIRE(!(1 >= real{"nan", 5}));
    REQUIRE(!(real{"nan", 5} >= 1u));
    REQUIRE(!(1u >= real{"nan", 5}));
    // The bool specialisations.
    REQUIRE(real{2} >= true);
    REQUIRE(real{1} >= true);
    REQUIRE(!(real{-1} >= false));
    REQUIRE(!(real{"nan", 5} >= false));
    REQUIRE(true >= real{0});
    REQUIRE(true >= real{1});
    REQUIRE(false >= real{-1});
    REQUIRE(!(false >= real{"nan", 5}));
    // FP.
    REQUIRE((1. >= real{1}));
    REQUIRE((1.f >= real{1}));
    REQUIRE((1.l >= real{1}));
    REQUIRE(!(real{0.1} >= 1.));
    REQUIRE(!(real{0.1} >= 1.f));
    REQUIRE(!(real{0.1} >= 1.l));
    REQUIRE((real{"inf", 64} >= 45.));
    REQUIRE(!(real{"nan", 5} >= 1.));
    REQUIRE(!(real{"nan", 5} >= 1.f));
    REQUIRE(!(real{"nan", 5} >= 1.l));
    REQUIRE(!(1. >= real{"nan", 5}));
    REQUIRE(!(1.f >= real{"nan", 5}));
    REQUIRE(!(1.l >= real{"nan", 5}));
    if (std::numeric_limits<double>::has_quiet_NaN) {
        REQUIRE(!(std::numeric_limits<double>::quiet_NaN() >= real{"nan", 5}));
        REQUIRE(!(real{"nan", 5} >= std::numeric_limits<double>::quiet_NaN()));
    }
    // int/rat.
    REQUIRE(!(rat_t{0u} >= real{1}));
    REQUIRE((real{2} >= rat_t{1ull}));
    REQUIRE((real{"inf", 64} >= int_t{45}));
    REQUIRE(!(real{"nan", 5} >= int_t{1}));
    REQUIRE(!(int_t{1} >= real{"nan", 5}));
    REQUIRE(!(rat_t{1} >= real{"nan", 5}));
    REQUIRE(!(real{"nan", 5} >= rat_t{1}));
#if defined(MPPP_WITH_QUADMATH)
    REQUIRE(!(real128{} >= real{1}));
    REQUIRE((real{2} >= real128{1ull}));
    REQUIRE((real{"inf", 64} >= real128{45}));
    REQUIRE(!(real{"nan", 5} >= real128{1}));
    REQUIRE(!(real128{1} >= real{"nan", 5}));
    REQUIRE(!(real{"nan", 5} >= real128{"nan"}));
    REQUIRE(!(real128{"nan"} >= real{"nan", 5}));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE(real{2} >= __int128_t{-1});
    REQUIRE(__int128_t{2} >= real{2});
    REQUIRE(real{5} >= __uint128_t{3});
    REQUIRE(__uint128_t{5} >= real{5});
    REQUIRE(pow(2_r128, 65) + 1 >= (__uint128_t{1} << 65));
    REQUIRE((__uint128_t{1} << 65) + 1 >= pow(2_r128, 65));
    REQUIRE(pow(2_r128, 65) + 1 >= (__int128_t{1} << 65));
    REQUIRE((__int128_t{1} << 65) + 1 >= pow(2_r128, 65));
#endif
}

TEST_CASE("real incdec")
{
    real r0{0};
    REQUIRE(++r0 == 1);
    REQUIRE(r0++ == 1);
    REQUIRE(r0 == 2);
    REQUIRE(--r0 == 1);
    REQUIRE(r0-- == 1);
    REQUIRE(r0.zero_p());

    // Check precision handling.
    r0 = real{0, 4};
    ++r0;
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(1));
    r0 = real{0, 4};
    r0++;
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(1));
    r0 = real{0, 4};
    --r0;
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(1));
    r0 = real{0, 4};
    r0--;
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(1));
}
