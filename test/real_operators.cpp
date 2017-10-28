// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <algorithm>
#include <limits>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real.hpp>
#if defined(MPPP_WITH_QUADMATH)
#include <mp++/real128.hpp>
#endif
#include <utility>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

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
    REQUIRE(::mpfr_cmp_ui((+r0).get_mpfr_t(), 123ul) == 0);
    REQUIRE((+r0).get_prec() == std::numeric_limits<int>::digits + 1);
    REQUIRE(::mpfr_cmp_ui((+std::move(r0)).get_mpfr_t(), 123ul) == 0);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real binary add")
{
    real r0, r1;
    REQUIRE(real{} + real{} == real{});
    REQUIRE((real{} + real{}).get_prec() == real_prec_min());
    r0 = 23;
    r1 = -1;
    REQUIRE(r0 + r1 == real{22});
    REQUIRE(std::move(r0) + r1 == real{22});
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
    r0 = real{23};
    REQUIRE(r0 + std::move(r1) == real{22});
    REQUIRE(!r1.get_mpfr_t()->_mpfr_d);
    r1 = real{-1};
    REQUIRE(std::move(r0) + std::move(r1) == real{22});
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
    REQUIRE(r1.get_mpfr_t()->_mpfr_d);
    r0 = real{23};
    REQUIRE((real{1, 10} + real{2, 20} == real{3}));
    REQUIRE((real{1, 10} + real{2, 20}).get_prec() == 20);
    REQUIRE((real{1, 20} + real{2, 10} == real{3}));
    REQUIRE((real{1, 20} + real{2, 10}).get_prec() == 20);
    // Addition with integrals.
    REQUIRE((real{1, 10} + 10 == real{11}));
    REQUIRE((real{1, 10} + 10).get_prec() == std::numeric_limits<int>::digits + 1);
    REQUIRE((10 + real{1, 10} == real{11}));
    REQUIRE((10 + real{1, 10}).get_prec() == std::numeric_limits<int>::digits + 1);
    REQUIRE((real{1, 100} + 10 == real{11}));
    REQUIRE((real{1, 100} + 10).get_prec() == std::max(100, std::numeric_limits<int>::digits + 1));
    REQUIRE((10 + real{1, 100} == real{11}));
    REQUIRE((10 + real{1, 100}).get_prec() == std::max(100, std::numeric_limits<int>::digits + 1));
    real_set_default_prec(12);
    REQUIRE((real{1, 10} + 10 == real{11}));
    REQUIRE((real{1, 10} + 10).get_prec() == 12);
    REQUIRE((10 + real{1, 10} == real{11}));
    REQUIRE((10 + real{1, 10}).get_prec() == 12);
    REQUIRE((real{1, 100} + 10 == real{11}));
    REQUIRE((real{1, 100} + 10).get_prec() == 100);
    REQUIRE((10 + real{1, 100} == real{11}));
    REQUIRE((10 + real{1, 100}).get_prec() == 100);
    real_reset_default_prec();
    REQUIRE((real{1, 10} + true == real{2}));
    REQUIRE((real{1, 10} + true).get_prec() == 10);
    REQUIRE((false + real{1, 10} == real{1}));
    REQUIRE((false + real{1, 10}).get_prec() == 10);
    real_set_default_prec(12);
    REQUIRE((real{1, 10} + true == real{2}));
    REQUIRE((real{1, 10} + true).get_prec() == 12);
    REQUIRE((false + real{1, 10} == real{1}));
    REQUIRE((false + real{1, 10}).get_prec() == 12);
    real_reset_default_prec();
    REQUIRE((real{1, 10} + 10u == real{11}));
    REQUIRE((real{1, 10} + 10u).get_prec() == std::numeric_limits<unsigned>::digits);
    REQUIRE((10u + real{1, 10} == real{11}));
    REQUIRE((10u + real{1, 10}).get_prec() == std::numeric_limits<unsigned>::digits);
    REQUIRE((real{1, 100} + 10u == real{11}));
    REQUIRE((real{1, 100} + 10u).get_prec() == std::max(100, std::numeric_limits<unsigned>::digits));
    REQUIRE((10u + real{1, 100} == real{11}));
    REQUIRE((10u + real{1, 100}).get_prec() == std::max(100, std::numeric_limits<unsigned>::digits));
    real_set_default_prec(12);
    REQUIRE((real{1, 10} + 10u == real{11}));
    REQUIRE((real{1, 10} + 10u).get_prec() == 12);
    REQUIRE((10u + real{1, 10} == real{11}));
    REQUIRE((10u + real{1, 10}).get_prec() == 12);
    REQUIRE((real{1, 100} + 10u == real{11}));
    REQUIRE((real{1, 100} + 10u).get_prec() == 100);
    REQUIRE((10u + real{1, 100} == real{11}));
    REQUIRE((10u + real{1, 100}).get_prec() == 100);
    real_reset_default_prec();
    REQUIRE((real{1, 10} + 10ll == real{11}));
    REQUIRE((real{1, 10} + 10ll).get_prec() == std::numeric_limits<long long>::digits + 1);
    REQUIRE((10ll + real{1, 10} == real{11}));
    REQUIRE((10ll + real{1, 10}).get_prec() == std::numeric_limits<long long>::digits + 1);
    REQUIRE((real{1, 100} + 10ll == real{11}));
    REQUIRE((real{1, 100} + 10ll).get_prec() == std::max(100, std::numeric_limits<long long>::digits + 1));
    REQUIRE((10ll + real{1, 100} == real{11}));
    REQUIRE((10ll + real{1, 100}).get_prec() == std::max(100, std::numeric_limits<long long>::digits + 1));
    real_set_default_prec(12);
    REQUIRE((real{1, 10} + 10ll == real{11}));
    REQUIRE((real{1, 10} + 10ll).get_prec() == 12);
    REQUIRE((10ll + real{1, 10} == real{11}));
    REQUIRE((10ll + real{1, 10}).get_prec() == 12);
    REQUIRE((real{1, 100} + 10ll == real{11}));
    REQUIRE((real{1, 100} + 10ll).get_prec() == 100);
    REQUIRE((10ll + real{1, 100} == real{11}));
    REQUIRE((10ll + real{1, 100}).get_prec() == 100);
    real_reset_default_prec();
    REQUIRE((real{1, 10} + 10ull == real{11}));
    REQUIRE((real{1, 10} + 10ull).get_prec() == std::numeric_limits<unsigned long long>::digits);
    REQUIRE((10ull + real{1, 10} == real{11}));
    REQUIRE((10ull + real{1, 10}).get_prec() == std::numeric_limits<unsigned long long>::digits);
    REQUIRE((real{1, 100} + 10ull == real{11}));
    REQUIRE((real{1, 100} + 10ull).get_prec() == std::max(100, std::numeric_limits<unsigned long long>::digits));
    REQUIRE((10ull + real{1, 100} == real{11}));
    REQUIRE((10ull + real{1, 100}).get_prec() == std::max(100, std::numeric_limits<unsigned long long>::digits));
    real_set_default_prec(12);
    REQUIRE((real{1, 10} + 10ull == real{11}));
    REQUIRE((real{1, 10} + 10ull).get_prec() == 12);
    REQUIRE((10ull + real{1, 10} == real{11}));
    REQUIRE((10ull + real{1, 10}).get_prec() == 12);
    REQUIRE((real{1, 100} + 10ull == real{11}));
    REQUIRE((real{1, 100} + 10ull).get_prec() == 100);
    REQUIRE((10ull + real{1, 100} == real{11}));
    REQUIRE((10ull + real{1, 100}).get_prec() == 100);
    real_reset_default_prec();
    // Floating-point.
    REQUIRE((real{1, 10} + 10.f == real{11}));
    REQUIRE((real{1, 10} + 10.f).get_prec() == dig2mpfr_prec<float>());
    REQUIRE((10.f + real{1, 10} == real{11}));
    REQUIRE((10.f + real{1, 10}).get_prec() == dig2mpfr_prec<float>());
    REQUIRE((real{1, 100} + 10.f == real{11}));
    REQUIRE((real{1, 100} + 10.f).get_prec() == std::max<::mpfr_prec_t>(100, dig2mpfr_prec<float>()));
    REQUIRE((10.f + real{1, 100} == real{11}));
    REQUIRE((10.f + real{1, 100}).get_prec() == std::max<::mpfr_prec_t>(100, dig2mpfr_prec<float>()));
    real_set_default_prec(12);
    REQUIRE((real{1, 10} + 10.f == real{11}));
    REQUIRE((real{1, 10} + 10.f).get_prec() == 12);
    REQUIRE((10.f + real{1, 10} == real{11}));
    REQUIRE((10.f + real{1, 10}).get_prec() == 12);
    REQUIRE((real{1, 100} + 10.f == real{11}));
    REQUIRE((real{1, 100} + 10.f).get_prec() == 100);
    REQUIRE((10.f + real{1, 100} == real{11}));
    REQUIRE((10.f + real{1, 100}).get_prec() == 100);
    real_reset_default_prec();
    REQUIRE((real{1, 10} + 10. == real{11}));
    REQUIRE((real{1, 10} + 10.).get_prec() == dig2mpfr_prec<double>());
    REQUIRE((10. + real{1, 10} == real{11}));
    REQUIRE((10. + real{1, 10}).get_prec() == dig2mpfr_prec<double>());
    REQUIRE((real{1, 100} + 10. == real{11}));
    REQUIRE((real{1, 100} + 10.).get_prec() == std::max<::mpfr_prec_t>(100, dig2mpfr_prec<double>()));
    REQUIRE((10. + real{1, 100} == real{11}));
    REQUIRE((10. + real{1, 100}).get_prec() == std::max<::mpfr_prec_t>(100, dig2mpfr_prec<double>()));
    real_set_default_prec(12);
    REQUIRE((real{1, 10} + 10. == real{11}));
    REQUIRE((real{1, 10} + 10.).get_prec() == 12);
    REQUIRE((10. + real{1, 10} == real{11}));
    REQUIRE((10. + real{1, 10}).get_prec() == 12);
    REQUIRE((real{1, 100} + 10. == real{11}));
    REQUIRE((real{1, 100} + 10.).get_prec() == 100);
    REQUIRE((10. + real{1, 100} == real{11}));
    REQUIRE((10. + real{1, 100}).get_prec() == 100);
    real_reset_default_prec();
    REQUIRE((real{1, 10} + 10.l == real{11}));
    REQUIRE((real{1, 10} + 10.l).get_prec() == dig2mpfr_prec<long double>());
    REQUIRE((10.l + real{1, 10} == real{11}));
    REQUIRE((10.l + real{1, 10}).get_prec() == dig2mpfr_prec<long double>());
    REQUIRE((real{1, 100} + 10.l == real{11}));
    REQUIRE((real{1, 100} + 10.l).get_prec() == std::max<::mpfr_prec_t>(100, dig2mpfr_prec<long double>()));
    REQUIRE((10.l + real{1, 100} == real{11}));
    REQUIRE((10.l + real{1, 100}).get_prec() == std::max<::mpfr_prec_t>(100, dig2mpfr_prec<long double>()));
    real_set_default_prec(12);
    REQUIRE((real{1, 10} + 10.l == real{11}));
    REQUIRE((real{1, 10} + 10.l).get_prec() == 12);
    REQUIRE((10.l + real{1, 10} == real{11}));
    REQUIRE((10.l + real{1, 10}).get_prec() == 12);
    REQUIRE((real{1, 100} + 10.l == real{11}));
    REQUIRE((real{1, 100} + 10.l).get_prec() == 100);
    REQUIRE((10.l + real{1, 100} == real{11}));
    REQUIRE((10.l + real{1, 100}).get_prec() == 100);
    real_reset_default_prec();
    // Integer.
    REQUIRE((real{1, 10} + int_t{10} == real{11}));
    REQUIRE((real{1, 10} + int_t{10}).get_prec() == GMP_NUMB_BITS);
    REQUIRE((int_t{10} + real{1, 10} == real{11}));
    REQUIRE((int_t{10} + real{1, 10}).get_prec() == GMP_NUMB_BITS);
    REQUIRE((real{1, 100} + int_t{10} == real{11}));
    REQUIRE((real{1, 100} + int_t{10}).get_prec() == std::max(100, GMP_NUMB_BITS));
    REQUIRE((int_t{10} + real{1, 100} == real{11}));
    REQUIRE((int_t{10} + real{1, 100}).get_prec() == std::max(100, GMP_NUMB_BITS));
    real_set_default_prec(12);
    REQUIRE((real{1, 10} + int_t{10} == real{11}));
    REQUIRE((real{1, 10} + int_t{10}).get_prec() == 12);
    REQUIRE((int_t{10} + real{1, 10} == real{11}));
    REQUIRE((int_t{10} + real{1, 10}).get_prec() == 12);
    REQUIRE((real{1, 100} + int_t{10} == real{11}));
    REQUIRE((real{1, 100} + int_t{10}).get_prec() == 100);
    REQUIRE((int_t{10} + real{1, 100} == real{11}));
    REQUIRE((int_t{10} + real{1, 100}).get_prec() == 100);
    real_reset_default_prec();
    // Rational.
    REQUIRE((real{1, 10} + rat_t{10} == real{11}));
    REQUIRE((real{1, 10} + rat_t{10}).get_prec() == GMP_NUMB_BITS * 2);
    REQUIRE((rat_t{10} + real{1, 10} == real{11}));
    REQUIRE((rat_t{10} + real{1, 10}).get_prec() == GMP_NUMB_BITS * 2);
    REQUIRE((real{1, 100} + rat_t{10} == real{11}));
    REQUIRE((real{1, 100} + rat_t{10}).get_prec() == std::max(100, GMP_NUMB_BITS * 2));
    REQUIRE((rat_t{10} + real{1, 100} == real{11}));
    REQUIRE((rat_t{10} + real{1, 100}).get_prec() == std::max(100, GMP_NUMB_BITS * 2));
    real_set_default_prec(12);
    REQUIRE((real{1, 10} + rat_t{10} == real{11}));
    REQUIRE((real{1, 10} + rat_t{10}).get_prec() == 12);
    REQUIRE((rat_t{10} + real{1, 10} == real{11}));
    REQUIRE((rat_t{10} + real{1, 10}).get_prec() == 12);
    REQUIRE((real{1, 100} + rat_t{10} == real{11}));
    REQUIRE((real{1, 100} + rat_t{10}).get_prec() == 100);
    REQUIRE((rat_t{10} + real{1, 100} == real{11}));
    REQUIRE((rat_t{10} + real{1, 100}).get_prec() == 100);
    real_reset_default_prec();
#if defined(MPPP_WITH_QUADMATH)
    REQUIRE((real{1, 10} + real128{10} == real{11}));
    REQUIRE((real{1, 10} + real128{10}).get_prec() == 113);
    REQUIRE((real128{10} + real{1, 10} == real{11}));
    REQUIRE((real128{10} + real{1, 10}).get_prec() == 113);
    REQUIRE((real{1, 200} + real128{10} == real{11}));
    REQUIRE((real{1, 200} + real128{10}).get_prec() == 200);
    REQUIRE((real128{10} + real{1, 200} == real{11}));
    REQUIRE((real128{10} + real{1, 200}).get_prec() == 200);
    real_set_default_prec(12);
    REQUIRE((real{1, 10} + real128{10} == real{11}));
    REQUIRE((real{1, 10} + real128{10}).get_prec() == 12);
    REQUIRE((real128{10} + real{1, 10} == real{11}));
    REQUIRE((real128{10} + real{1, 10}).get_prec() == 12);
    REQUIRE((real{1, 200} + real128{10} == real{11}));
    REQUIRE((real{1, 200} + real128{10}).get_prec() == 200);
    REQUIRE((real128{10} + real{1, 200} == real{11}));
    REQUIRE((real128{10} + real{1, 200}).get_prec() == 200);
    real_reset_default_prec();
#endif
}

TEST_CASE("real plus")
{
    real r0{123};
    REQUIRE(::mpfr_cmp_ui((+r0).get_mpfr_t(), 123ul) == 0);
    REQUIRE(::mpfr_cmp_ui((+real{123}).get_mpfr_t(), 123ul) == 0);
    std::cout << (real{123} + real{4}) << '\n';
    std::cout << (real{123} + int_t{4}) << '\n';
    std::cout << (int_t{4} + real{123}) << '\n';
    std::cout << (real{123} + rat_t{4}) << '\n';
    std::cout << (rat_t{4} + real{123}) << '\n';
    std::cout << (real{123} + 34u) << '\n';
    std::cout << (36u + real{123}) << '\n';
    std::cout << (real{123} + -34) << '\n';
    std::cout << (-36 + real{123}) << '\n';
    std::cout << (real{123} + true) << '\n';
    std::cout << (false + real{123}) << '\n';
    std::cout << (real{123} + 1.2f) << '\n';
    std::cout << (1.2f + real{123}) << '\n';
    std::cout << (real{123} + 1.2) << '\n';
    std::cout << (1.2 + real{123}) << '\n';
    std::cout << (real{123} + 1.2l) << '\n';
    std::cout << (1.2l + real{123}) << '\n';
#if defined(MPPP_WITH_QUADMATH)
    std::cout << (real{123} + real128{"1.1"}) << '\n';
    std::cout << (real128{"1.1"} + real{123}) << '\n';
#endif
    std::cout << (r0 += real{45}) << '\n';
    std::cout << (r0 += int_t{45}) << '\n';
    int_t n0{56};
    n0 += real{45};
    std::cout << n0 << '\n';
    r0 += rat_t{1, 2};
    std::cout << r0 << '\n';
    rat_t q0{1, 2};
    q0 += real{1};
    std::cout << q0 << '\n';
    r0 += 1u;
    std::cout << r0 << '\n';
    unsigned un = 5;
    un += real{23};
    std::cout << un << '\n';
    r0 += -1;
    std::cout << r0 << '\n';
    int sn = -5;
    sn += real{-23};
    std::cout << sn << '\n';
    r0 = real{};
    r0 += 1.1f;
    std::cout << r0 << '\n';
    r0 = real{};
    r0 += 1.1;
    std::cout << r0 << '\n';
    r0 = real{};
    r0 += 1.1l;
    std::cout << r0 << '\n';
#if defined(MPPP_WITH_QUADMATH)
    r0 = real{};
    r0 += real128{"1.1"};
    std::cout << r0 << '\n';
#endif
    std::cout << std::setprecision(50);
    float f0 = 1.1f;
    f0 += real{"1.1", 100};
    std::cout << f0 << '\n';
    double d0 = 1.1;
    d0 += real{"1.1", 100};
    std::cout << d0 << '\n';
    long double ld0 = 1.1l;
    ld0 += real{"1.1", 100};
    std::cout << ld0 << '\n';
#if defined(MPPP_WITH_QUADMATH)
    real128 qd0{"1.1"};
    qd0 += real{"1.1", 100};
    std::cout << qd0 << '\n';
#endif
}
