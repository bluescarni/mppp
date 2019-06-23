// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <utility>

#include <mp++/detail/mpfr.hpp>
#include <mp++/real.hpp>

using namespace mppp;

TEST_CASE("real eint")
{
    real r0{1};
    r0.eint();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(r0 - 1.89511781) < 1E-5);
    real rop;
    r0 = real{1};
    REQUIRE(abs(eint(rop, r0) - 1.89511781) < 1E-5);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(eint(r0) - 1.89511781) < 1E-5);
    REQUIRE(abs(eint(std::move(r0)) - 1.89511781) < 1E-5);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real li2")
{
    real r0{-1};
    r0.li2();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(r0 + 0.8224670334241132) < 1E-5);
    real rop;
    r0 = real{-1};
    REQUIRE(abs(li2(rop, r0) + 0.8224670334241132) < 1E-5);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(li2(r0) + 0.8224670334241132) < 1E-5);
    REQUIRE(abs(li2(std::move(r0)) + 0.8224670334241132) < 1E-5);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real zeta")
{
    real r0{-1};
    r0.zeta();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(r0 + 1. / 12) < 1E-5);
    real rop;
    r0 = real{-1};
    REQUIRE(abs(zeta(rop, r0) + 1. / 12) < 1E-5);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(zeta(r0) + 1. / 12) < 1E-5);
    REQUIRE(abs(zeta(std::move(r0)) + 1. / 12) < 1E-5);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real erf")
{
    real r0{1};
    r0.erf();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(r0 - 0.84270079295) < 1E-5);
    real rop;
    r0 = real{1};
    REQUIRE(abs(erf(rop, r0) - 0.84270079295) < 1E-5);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(erf(r0) - 0.84270079295) < 1E-5);
    REQUIRE(abs(erf(std::move(r0)) - 0.84270079295) < 1E-5);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

TEST_CASE("real erfc")
{
    real r0{1};
    r0.erfc();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(r0 - 0.15729920705) < 1E-5);
    real rop;
    r0 = real{1};
    REQUIRE(abs(erfc(rop, r0) - 0.15729920705) < 1E-5);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(abs(erfc(r0) - 0.15729920705) < 1E-5);
    REQUIRE(abs(erfc(std::move(r0)) - 0.15729920705) < 1E-5);
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);
}

#if MPFR_VERSION_MAJOR >= 4

TEST_CASE("real beta")
{
    real r0{12, 450};
    beta(r0, real{4}, real{5});
    REQUIRE(abs(r0 - gamma(real{4}) * gamma(real{5}) / gamma(real{9})) < 1E-8);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    real tmp1{4}, tmp2{5};
    r0 = real{12, detail::real_deduce_precision(0) / 2};
    beta(r0, std::move(tmp1), tmp2);
    REQUIRE(abs(r0 - gamma(real{4}) * gamma(real{5}) / gamma(real{9})) < 1E-8);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    // Check tmp1 was swapped for r0.
    REQUIRE(tmp1 == real{12, detail::real_deduce_precision(0) / 2});
    REQUIRE(tmp1.get_prec() == detail::real_deduce_precision(0) / 2);
    tmp1 = real{4};
    tmp2 = real{5};
    r0 = real{12, detail::real_deduce_precision(0) / 2};
    beta(r0, tmp1, std::move(tmp2));
    REQUIRE(abs(r0 - gamma(real{4}) * gamma(real{5}) / gamma(real{9})) < 1E-8);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    // Check tmp2 was swapped for r0.
    REQUIRE(tmp2 == real{12, detail::real_deduce_precision(0) / 2});
    REQUIRE(tmp2.get_prec() == detail::real_deduce_precision(0) / 2);

    // Some tests for the binary form too.
    REQUIRE(abs(beta(real{4}, real{5}) - gamma(real{4}) * gamma(real{5}) / gamma(real{9})) < 1E-8);
    REQUIRE(beta(real{4, 20}, real{5, 30}).get_prec() == 30);
    REQUIRE(beta(real{4}, 5.) == beta(real{4}, real{5.}));
    REQUIRE(beta(5., real{4}) == beta(real{5.}, real{4}));
    REQUIRE(beta(real{4}, 5) == beta(real{4}, real{5}));
    REQUIRE(beta(5, real{4}) == beta(real{5}, real{4}));
    REQUIRE(beta(-5., real{4}) == beta(real{-5.}, real{4}));
    REQUIRE(beta(-5, real{4}) == beta(real{-5}, real{4}));
    REQUIRE(beta(real{4}, integer<1>{5}) == beta(real{4}, real{integer<1>{5}}));
    REQUIRE(beta(integer<1>{-5}, real{4}) == beta(real{integer<1>{-5}}, real{4}));
    REQUIRE(beta(real{4, detail::real_deduce_precision(0.) / 2}, 5.).get_prec() == detail::real_deduce_precision(0.));
    REQUIRE(beta(4., real{5, detail::real_deduce_precision(0.) / 2}).get_prec() == detail::real_deduce_precision(0.));
    REQUIRE(beta(real{4, detail::real_deduce_precision(0) / 2}, 5).get_prec() == detail::real_deduce_precision(0));
    REQUIRE(beta(4, real{5, detail::real_deduce_precision(0) / 2}).get_prec() == detail::real_deduce_precision(0));
}

#endif
