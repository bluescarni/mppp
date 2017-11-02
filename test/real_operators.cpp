// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real.hpp>
#if defined(MPPP_WITH_QUADMATH)
#include <mp++/real128.hpp>
#endif
#include <stdexcept>
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
    REQUIRE((real{1, 10} + 10 == real{1, 10} + real{10}));
    REQUIRE((real{1, 10} + std::numeric_limits<int>::max() == real{1, 10} + real{std::numeric_limits<int>::max()}));
    REQUIRE((real{-1, 10} + std::numeric_limits<int>::min() == real{-1, 10} + real{std::numeric_limits<int>::min()}));
    REQUIRE((10 + real{1, 10} == real{10} + real{1, 10}));
    REQUIRE((std::numeric_limits<int>::max() + real{1, 10} == real{std::numeric_limits<int>::max()} + real{1, 10}));
    REQUIRE((std::numeric_limits<int>::min() + real{-1, 10} == real{std::numeric_limits<int>::min()} + real{-1, 10}));
    REQUIRE((real{1, 100} + 10 == real{1, 100} + real{10}));
    REQUIRE((real{1, 100} + std::numeric_limits<int>::max() == real{1, 100} + real{std::numeric_limits<int>::max()}));
    REQUIRE((real{-1, 100} + std::numeric_limits<int>::min() == real{-1, 100} + real{std::numeric_limits<int>::min()}));
    REQUIRE((10 + real{1, 100} == real{10} + real{1, 100}));
    REQUIRE((std::numeric_limits<int>::max() + real{1, 100} == real{std::numeric_limits<int>::max()} + real{1, 100}));
    REQUIRE((std::numeric_limits<int>::min() + real{-1, 100} == real{std::numeric_limits<int>::min()} + real{-1, 100}));
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
    REQUIRE((real{1, 10} + 10u == real{1, 10} + real{10u}));
    REQUIRE((real{1, 10} + std::numeric_limits<unsigned>::max()
             == real{1, 10} + real{std::numeric_limits<unsigned>::max()}));
    REQUIRE((10u + real{1, 10} == real{10u} + real{1, 10}));
    REQUIRE((std::numeric_limits<unsigned>::max() + real{1, 10u}
             == real{std::numeric_limits<unsigned>::max()} + real{1, 10u}));
    REQUIRE((real{1, 100} + 10u == real{1, 100} + real{10u}));
    REQUIRE((real{1, 100} + std::numeric_limits<unsigned>::max()
             == real{1, 100} + real{std::numeric_limits<unsigned>::max()}));
    REQUIRE((10u + real{1, 100} == real{10u} + real{1, 100}));
    REQUIRE((std::numeric_limits<unsigned>::max() + real{1, 100}
             == real{std::numeric_limits<unsigned>::max()} + real{1, 100}));
    real_reset_default_prec();
    REQUIRE((real{1, 10} + 10ll == real{11}));
    REQUIRE((real{1, 10} + 10ll).get_prec() == std::numeric_limits<long long>::digits + 1);
    REQUIRE((real{0, 10} + std::numeric_limits<long long>::max() == real{std::numeric_limits<long long>::max()}));
    REQUIRE((real{0, 10} + std::numeric_limits<long long>::max()).get_prec()
            == std::numeric_limits<long long>::digits + 1);
    REQUIRE((real{0, 10} + std::numeric_limits<long long>::min() == real{std::numeric_limits<long long>::min()}));
    REQUIRE((real{0, 10} + std::numeric_limits<long long>::min()).get_prec()
            == std::numeric_limits<long long>::digits + 1);
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
    REQUIRE((real{1, 10} + 10ll == real{1, 10} + real{10ll}));
    REQUIRE((real{1, 10} + std::numeric_limits<long long>::max()
             == real{1, 10} + real{std::numeric_limits<long long>::max()}));
    REQUIRE((real{-1, 10} + std::numeric_limits<long long>::min()
             == real{-1, 10} + real{std::numeric_limits<long long>::min()}));
    REQUIRE((10ll + real{1, 10} == real{10ll} + real{1, 10}));
    REQUIRE((std::numeric_limits<long long>::max() + real{1, 10}
             == real{std::numeric_limits<long long>::max()} + real{1, 10}));
    REQUIRE((std::numeric_limits<long long>::min() + real{-1, 10}
             == real{std::numeric_limits<long long>::min()} + real{-1, 10}));
    REQUIRE((real{1, 100} + 10ll == real{1, 100} + real{10ll}));
    REQUIRE((real{1, 100} + std::numeric_limits<long long>::max()
             == real{1, 100} + real{std::numeric_limits<long long>::max()}));
    REQUIRE((real{-1, 100} + std::numeric_limits<long long>::min()
             == real{-1, 100} + real{std::numeric_limits<long long>::min()}));
    REQUIRE((10ll + real{1, 100} == real{10ll} + real{1, 100}));
    REQUIRE((std::numeric_limits<long long>::max() + real{1, 100}
             == real{std::numeric_limits<long long>::max()} + real{1, 100}));
    REQUIRE((std::numeric_limits<long long>::min() + real{-1, 100}
             == real{std::numeric_limits<long long>::min()} + real{-1, 100}));
    real_reset_default_prec();
    REQUIRE((real{1, 10} + 10ull == real{11}));
    REQUIRE((real{1, 10} + 10ull).get_prec() == std::numeric_limits<unsigned long long>::digits);
    REQUIRE((10ull + real{1, 10} == real{11}));
    REQUIRE((10ull + real{1, 10}).get_prec() == std::numeric_limits<unsigned long long>::digits);
    REQUIRE((real{0, 10} + std::numeric_limits<unsigned long long>::max()
             == real{std::numeric_limits<unsigned long long>::max()}));
    REQUIRE((real{0, 10} + std::numeric_limits<unsigned long long>::max()).get_prec()
            == std::numeric_limits<unsigned long long>::digits);
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
    REQUIRE((real{1, 10} + 10ull == real{1, 10} + real{10ull}));
    REQUIRE((real{1, 10} + std::numeric_limits<unsigned long long>::max()
             == real{1, 10} + real{std::numeric_limits<unsigned long long>::max()}));
    REQUIRE((10ull + real{1, 10} == real{10ull} + real{1, 10}));
    REQUIRE((std::numeric_limits<unsigned long long>::max() + real{1, 10u}
             == real{std::numeric_limits<unsigned long long>::max()} + real{1, 10u}));
    REQUIRE((real{1, 100} + 10ull == real{1, 100} + real{10ull}));
    REQUIRE((real{1, 100} + std::numeric_limits<unsigned long long>::max()
             == real{1, 100} + real{std::numeric_limits<unsigned long long>::max()}));
    REQUIRE((10ull + real{1, 100} == real{10ull} + real{1, 100}));
    REQUIRE((std::numeric_limits<unsigned long long>::max() + real{1, 100}
             == real{std::numeric_limits<unsigned long long>::max()} + real{1, 100}));
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
    REQUIRE((real{"32193821093809210101283092183091283092183", 10} + int_t{"32193821093809210101283092183091283092183"}
             == real{"32193821093809210101283092183091283092183", 10}
                    + real{int_t{"32193821093809210101283092183091283092183"}}));
    REQUIRE((int_t{"32193821093809210101283092183091283092183"} + real{"32193821093809210101283092183091283092183", 10}
             == real{int_t{"32193821093809210101283092183091283092183"}}
                    + real{"32193821093809210101283092183091283092183", 10}));
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
    REQUIRE((real{"32193821093809210101283092183091283092183", 10} + rat_t{"32193821093809210101283092183091283092183"}
             == real{"32193821093809210101283092183091283092183", 10}
                    + real{rat_t{"32193821093809210101283092183091283092183"}}));
    REQUIRE((rat_t{"32193821093809210101283092183091283092183"} + real{"32193821093809210101283092183091283092183", 10}
             == real{rat_t{"32193821093809210101283092183091283092183"}}
                    + real{"32193821093809210101283092183091283092183", 10}));
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

TEST_CASE("real left in-place add")
{
    real r0, r1;
    r0 += r1;
    REQUIRE(r0.zero_p());
    REQUIRE(!r0.signbit());
    r0 = 5;
    r1 = 6;
    r0 += r1;
    REQUIRE(r0 == real{11});
    r0 = real{};
    r0 += real{12345678ll};
    REQUIRE(r0 == real{12345678ll});
    REQUIRE(r0.get_prec() == std::numeric_limits<long long>::digits + 1);
    // Integrals.
    r0 = real{};
    r0 += 123;
    REQUIRE(r0 == real{123});
    REQUIRE(r0.get_prec() == std::numeric_limits<int>::digits + 1);
    real_set_default_prec(5);
    r0 = real{};
    r0 += 123;
    REQUIRE((r0 == real{123, 5}));
    REQUIRE(r0.get_prec() == 5);
    real_reset_default_prec();
    r0 = real{};
    r0 += 123u;
    REQUIRE(r0 == real{123u});
    REQUIRE(r0.get_prec() == std::numeric_limits<unsigned>::digits);
    real_set_default_prec(5);
    r0 = real{};
    r0 += 123u;
    REQUIRE((r0 == real{123u, 5}));
    REQUIRE(r0.get_prec() == 5);
    real_reset_default_prec();
    r0 = real{};
    r0 += true;
    REQUIRE(r0 == real{1});
    REQUIRE(r0.get_prec() == std::max<::mpfr_prec_t>(std::numeric_limits<bool>::digits, real_prec_min()));
    real_set_default_prec(5);
    r0 = real{};
    r0 += true;
    REQUIRE((r0 == real{1, 5}));
    REQUIRE(r0.get_prec() == 5);
    real_reset_default_prec();
    r0 = real{};
    r0 += 123ll;
    REQUIRE(r0 == real{123ll});
    REQUIRE(r0.get_prec() == std::numeric_limits<long long>::digits + 1);
    r0 = real{};
    r0 += std::numeric_limits<long long>::max();
    REQUIRE(r0 == real{std::numeric_limits<long long>::max()});
    REQUIRE(r0.get_prec() == std::numeric_limits<long long>::digits + 1);
    r0 = real{};
    r0 += std::numeric_limits<long long>::min();
    REQUIRE(r0 == real{std::numeric_limits<long long>::min()});
    REQUIRE(r0.get_prec() == std::numeric_limits<long long>::digits + 1);
    real_set_default_prec(5);
    r0 = real{};
    r0 += 123ll;
    REQUIRE((r0 == real{123ll, 5}));
    REQUIRE(r0.get_prec() == 5);
    real_reset_default_prec();
    r0 = real{};
    r0 += 123ull;
    REQUIRE(r0 == real{123ull});
    REQUIRE(r0.get_prec() == std::numeric_limits<unsigned long long>::digits);
    r0 = real{};
    r0 += std::numeric_limits<unsigned long long>::max();
    REQUIRE(r0 == real{std::numeric_limits<unsigned long long>::max()});
    REQUIRE(r0.get_prec() == std::numeric_limits<unsigned long long>::digits);
    real_set_default_prec(5);
    r0 = real{};
    r0 += 123ll;
    REQUIRE((r0 == real{123ll, 5}));
    REQUIRE(r0.get_prec() == 5);
    real_reset_default_prec();
    // Floating-point.
    r0 = real{};
    r0 += 123.f;
    REQUIRE(r0 == real{123.f});
    REQUIRE(r0.get_prec() == dig2mpfr_prec<float>());
    real_set_default_prec(5);
    r0 = real{};
    r0 += 123.f;
    REQUIRE((r0 == real{123.f, 5}));
    REQUIRE(r0.get_prec() == 5);
    real_reset_default_prec();
    r0 = real{};
    r0 += 123.;
    REQUIRE(r0 == real{123.});
    REQUIRE(r0.get_prec() == dig2mpfr_prec<double>());
    real_set_default_prec(5);
    r0 = real{};
    r0 += 123.;
    REQUIRE((r0 == real{123., 5}));
    REQUIRE(r0.get_prec() == 5);
    real_reset_default_prec();
    r0 = real{};
    r0 += 123.l;
    REQUIRE(r0 == real{123.l});
    REQUIRE(r0.get_prec() == dig2mpfr_prec<long double>());
    real_set_default_prec(5);
    r0 = real{};
    r0 += 123.l;
    REQUIRE((r0 == real{123.l, 5}));
    REQUIRE(r0.get_prec() == 5);
    real_reset_default_prec();
    // Integer.
    r0 = real{};
    r0 += int_t{123};
    REQUIRE(r0 == real{int_t{123}});
    REQUIRE(r0.get_prec() == GMP_NUMB_BITS);
    real_set_default_prec(5);
    r0 = real{};
    r0 += int_t{123};
    REQUIRE((r0 == real{int_t{123}, 5}));
    REQUIRE(r0.get_prec() == 5);
    real_reset_default_prec();
    // Rational.
    r0 = real{};
    r0 += rat_t{123};
    REQUIRE(r0 == real{rat_t{123}});
    REQUIRE(r0.get_prec() == GMP_NUMB_BITS * 2);
    real_set_default_prec(5);
    r0 = real{};
    r0 += rat_t{123};
    REQUIRE((r0 == real{rat_t{123}, 5}));
    REQUIRE(r0.get_prec() == 5);
    real_reset_default_prec();
#if defined(MPPP_WITH_QUADMATH)
    r0 = real{};
    r0 += real128{123};
    REQUIRE(r0 == real{real128{123}});
    REQUIRE(r0.get_prec() == 113);
    real_set_default_prec(5);
    r0 = real{};
    r0 += real128{123};
    REQUIRE((r0 == real{real128{123}, 5}));
    REQUIRE(r0.get_prec() == 5);
    real_reset_default_prec();
#endif
}

TEST_CASE("real right in-place add")
{
    // Integrals.
    {
        int n = 3;
        n += real{2};
        REQUIRE(n == 5);
        n = 1;
        REQUIRE_THROWS_AS(n += real{std::numeric_limits<int>::max()}, std::overflow_error);
        REQUIRE_THROWS_AS((n += real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1);
        n = -1;
        REQUIRE_THROWS_AS(n += real{std::numeric_limits<int>::min()}, std::overflow_error);
        REQUIRE(n == -1);
        real_set_default_prec(5);
        n = 5;
        n += real{123};
        REQUIRE(n == static_cast<int>(5 + real{123}));
        REQUIRE(n == static_cast<int>(real{5} + real{123}));
        real_reset_default_prec();
    }
    {
        unsigned n = 3;
        n += real{2};
        REQUIRE(n == 5);
        n = 1;
        REQUIRE_THROWS_AS(n += real{std::numeric_limits<unsigned>::max()}, std::overflow_error);
        REQUIRE_THROWS_AS((n += real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1u);
        real_set_default_prec(5);
        n = 5;
        n += real{123};
        REQUIRE(n == static_cast<unsigned>(5 + real{123}));
        REQUIRE(n == static_cast<unsigned>(real{5} + real{123}));
        real_reset_default_prec();
    }
    {
        bool n = true;
        n += real{2};
        REQUIRE(n);
        real_set_default_prec(5);
        n += real{123};
        REQUIRE(n);
        n += real{-1};
        REQUIRE(!n);
        real_reset_default_prec();
    }
    {
        long long n = 3;
        n += real{2};
        REQUIRE(n == 5);
        n = 1;
        REQUIRE_THROWS_AS(n += real{std::numeric_limits<long long>::max()}, std::overflow_error);
        REQUIRE_THROWS_AS((n += real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1);
        n = -1;
        REQUIRE_THROWS_AS(n += real{std::numeric_limits<long long>::min()}, std::overflow_error);
        REQUIRE(n == -1);
        real_set_default_prec(5);
        n = 5;
        n += real{123};
        REQUIRE(n == static_cast<long long>(5 + real{123}));
        REQUIRE(n == static_cast<long long>(real{5} + real{123}));
        real_reset_default_prec();
    }
    {
        unsigned long long n = 3;
        n += real{2};
        REQUIRE(n == 5);
        n = 1;
        REQUIRE_THROWS_AS(n += real{std::numeric_limits<unsigned long long>::max()}, std::overflow_error);
        REQUIRE_THROWS_AS((n += real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1u);
        real_set_default_prec(5);
        n = 5;
        n += real{123};
        REQUIRE(n == static_cast<unsigned long long>(5 + real{123}));
        REQUIRE(n == static_cast<unsigned long long>(real{5} + real{123}));
        real_reset_default_prec();
    }
    // Floating-point.
    {
        float x = 3;
        x += real{2};
        REQUIRE(x == 5.f);
        if (std::numeric_limits<float>::is_iec559) {
            x = std::numeric_limits<float>::max();
            x += real{std::numeric_limits<float>::max()};
            REQUIRE(std::isinf(x));
        }
    }
    {
        double x = 3;
        x += real{2};
        REQUIRE(x == 5.);
        if (std::numeric_limits<double>::is_iec559) {
            x = std::numeric_limits<double>::max();
            x += real{std::numeric_limits<double>::max()};
            REQUIRE(std::isinf(x));
        }
    }
    {
        long double x = 3;
        x += real{2};
        REQUIRE(x == 5.l);
        if (std::numeric_limits<long double>::is_iec559) {
            x = std::numeric_limits<long double>::max();
            x += real{std::numeric_limits<long double>::max()};
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
        real_set_default_prec(5);
        n = 5;
        n += real{123};
        REQUIRE(n == static_cast<int_t>(int_t{5} + real{123}));
        REQUIRE(n == static_cast<int_t>(real{int_t{5}} + real{123}));
        real_reset_default_prec();
    }
    // Rational.
    {
        rat_t n{3};
        n += real{2};
        REQUIRE(n == 5);
        n = 1;
        REQUIRE_THROWS_AS((n += real{"inf", 5}), std::domain_error);
        REQUIRE(n == 1);
        real_set_default_prec(5);
        n = 5;
        n += real{123};
        REQUIRE(n == static_cast<rat_t>(rat_t{5} + real{123}));
        REQUIRE(n == static_cast<rat_t>(real{rat_t{5}} + real{123}));
        real_reset_default_prec();
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
}
