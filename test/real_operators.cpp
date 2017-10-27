// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <limits>
#include <mp++/detail/mpfr.hpp>
#include <mp++/integer.hpp>
#include <mp++/real.hpp>
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
