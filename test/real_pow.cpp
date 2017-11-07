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
#if defined(MPPP_WITH_QUADMATH)
#include <mp++/real128.hpp>
#endif
#include <mp++/rational.hpp>
#include <mp++/real.hpp>
#include <utility>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

using int_t = integer<1>;
using rat_t = rational<1>;

TEST_CASE("real pow")
{
    real r0, r1, rop;
    rop.set_prec(123);
    pow(rop, r0, r1);
    REQUIRE(rop == real{1});
    REQUIRE(rop.get_prec() == real_prec_min());
    r0 = 3;
    r1 = 2;
    pow(rop, r0, r1);
    REQUIRE(rop == real{9});
    REQUIRE(rop.get_prec() == std::numeric_limits<int>::digits + 1);
    rop = real{};
    pow(rop, real{3}, r1);
    REQUIRE(rop == real{9});
    REQUIRE(rop.get_prec() == std::numeric_limits<int>::digits + 1);
    rop = real{};
    pow(rop, r0, real{2});
    REQUIRE(rop == real{9});
    REQUIRE(rop.get_prec() == std::numeric_limits<int>::digits + 1);
    rop = real{};
    pow(rop, real{3}, real{2});
    REQUIRE(rop == real{9});
    REQUIRE(rop.get_prec() == std::numeric_limits<int>::digits + 1);
    rop = real{};
    pow(rop, std::move(r0), r1);
    REQUIRE(rop == real{9});
    REQUIRE(rop.get_prec() == std::numeric_limits<int>::digits + 1);
    REQUIRE(r0.zero_p());
    REQUIRE(r0.get_prec() == real_prec_min());
    r0 = 3;
    rop = real{};
    pow(rop, r0, std::move(r1));
    REQUIRE(rop == real{9});
    REQUIRE(rop.get_prec() == std::numeric_limits<int>::digits + 1);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == real_prec_min());
    r1 = 2;
    REQUIRE(pow(r0, r1) == real{9});
    REQUIRE(pow(r0, r1).get_prec() == std::numeric_limits<int>::digits + 1);
    REQUIRE(pow(r0, real{2}) == real{9});
    REQUIRE(pow(real{3}, r1) == real{9});
    REQUIRE(pow(real{3}, real{2}) == real{9});
    REQUIRE(pow(r0, 2) == real{9});
    REQUIRE(pow(3, r1) == real{9});
    REQUIRE(pow(real{3}, 2) == real{9});
    REQUIRE(pow(3, real{2}) == real{9});
    REQUIRE(pow(real{3}, 2).get_prec() == std::numeric_limits<int>::digits + 1);
    REQUIRE(pow(3, real{2}).get_prec() == std::numeric_limits<int>::digits + 1);
    real_set_default_prec(10);
    REQUIRE(pow(real{3, 5}, 2) == real{9});
    REQUIRE(pow(real{3, 5}, 2).get_prec() == 10);
    REQUIRE(pow(3, real{2, 5}) == real{9});
    REQUIRE(pow(3, real{2, 5}).get_prec() == 10);
    real_reset_default_prec();
    REQUIRE(pow(r0, 2u) == real{9});
    REQUIRE(pow(3u, r1) == real{9});
    REQUIRE(pow(real{3}, 2u) == real{9});
    REQUIRE(pow(3u, real{2}) == real{9});
    REQUIRE(pow(real{3}, 2u).get_prec() == std::numeric_limits<unsigned>::digits);
    REQUIRE(pow(3u, real{2}).get_prec() == std::numeric_limits<unsigned>::digits);
    real_set_default_prec(10);
    REQUIRE(pow(real{3, 5}, 2u) == real{9});
    REQUIRE(pow(real{3, 5}, 2u).get_prec() == 10);
    REQUIRE(pow(3u, real{2, 5}) == real{9});
    REQUIRE(pow(3u, real{2, 5}).get_prec() == 10);
    real_reset_default_prec();
    REQUIRE(pow(r0, 2ll) == real{9});
    REQUIRE(pow(3ll, r1) == real{9});
    REQUIRE(pow(real{3}, 2ll) == real{9});
    REQUIRE(pow(3ll, real{2}) == real{9});
    REQUIRE(pow(real{3}, 2ll).get_prec() == std::numeric_limits<long long>::digits + 1);
    REQUIRE(pow(3ll, real{2}).get_prec() == std::numeric_limits<long long>::digits + 1);
    real_set_default_prec(10);
    REQUIRE(pow(real{3, 5}, 2ll) == real{9});
    REQUIRE(pow(real{3, 5}, 2ll).get_prec() == 10);
    REQUIRE(pow(3ll, real{2, 5}) == real{9});
    REQUIRE(pow(3ll, real{2, 5}).get_prec() == 10);
    real_reset_default_prec();
    REQUIRE(pow(r0, 2ull) == real{9});
    REQUIRE(pow(3ull, r1) == real{9});
    REQUIRE(pow(real{3}, 2ull) == real{9});
    REQUIRE(pow(3ull, real{2}) == real{9});
    REQUIRE(pow(real{3}, 2ull).get_prec() == std::numeric_limits<unsigned long long>::digits);
    REQUIRE(pow(3ull, real{2}).get_prec() == std::numeric_limits<unsigned long long>::digits);
    real_set_default_prec(10);
    REQUIRE(pow(real{3, 5}, 2ull) == real{9});
    REQUIRE(pow(real{3, 5}, 2ull).get_prec() == 10);
    REQUIRE(pow(3ull, real{2, 5}) == real{9});
    REQUIRE(pow(3ull, real{2, 5}).get_prec() == 10);
    real_reset_default_prec();
    REQUIRE(pow(r0, 2.f) == real{9});
    REQUIRE(pow(3.f, r1) == real{9});
    REQUIRE(pow(real{3}, 2.f) == real{9});
    REQUIRE(pow(3.f, real{2}) == real{9});
    REQUIRE(pow(real{3}, 2.f).get_prec()
            == std::max<::mpfr_prec_t>(dig2mpfr_prec<float>(), std::numeric_limits<int>::digits + 1));
    REQUIRE(pow(3.f, real{2}).get_prec()
            == std::max<::mpfr_prec_t>(dig2mpfr_prec<float>(), std::numeric_limits<int>::digits + 1));
    real_set_default_prec(10);
    REQUIRE(pow(real{3, 5}, 2.f) == real{9});
    REQUIRE(pow(real{3, 5}, 2.f).get_prec() == 10);
    REQUIRE(pow(3.f, real{2, 5}) == real{9});
    REQUIRE(pow(3.f, real{2, 5}).get_prec() == 10);
    real_reset_default_prec();
    REQUIRE(pow(r0, 2.) == real{9});
    REQUIRE(pow(3., r1) == real{9});
    REQUIRE(pow(real{3}, 2.) == real{9});
    REQUIRE(pow(3., real{2}) == real{9});
    REQUIRE(pow(real{3}, 2.).get_prec()
            == std::max<::mpfr_prec_t>(dig2mpfr_prec<double>(), std::numeric_limits<int>::digits + 1));
    REQUIRE(pow(3., real{2}).get_prec()
            == std::max<::mpfr_prec_t>(dig2mpfr_prec<double>(), std::numeric_limits<int>::digits + 1));
    real_set_default_prec(10);
    REQUIRE(pow(real{3, 5}, 2.) == real{9});
    REQUIRE(pow(real{3, 5}, 2.).get_prec() == 10);
    REQUIRE(pow(3., real{2, 5}) == real{9});
    REQUIRE(pow(3., real{2, 5}).get_prec() == 10);
    real_reset_default_prec();
    REQUIRE(pow(r0, 2.l) == real{9});
    REQUIRE(pow(3.l, r1) == real{9});
    REQUIRE(pow(real{3}, 2.l) == real{9});
    REQUIRE(pow(3.l, real{2}) == real{9});
    REQUIRE(pow(real{3}, 2.l).get_prec()
            == std::max<::mpfr_prec_t>(dig2mpfr_prec<long double>(), std::numeric_limits<int>::digits + 1));
    REQUIRE(pow(3.l, real{2}).get_prec()
            == std::max<::mpfr_prec_t>(dig2mpfr_prec<long double>(), std::numeric_limits<int>::digits + 1));
    real_set_default_prec(10);
    REQUIRE(pow(real{3, 5}, 2.l) == real{9});
    REQUIRE(pow(real{3, 5}, 2.l).get_prec() == 10);
    REQUIRE(pow(3.l, real{2, 5}) == real{9});
    REQUIRE(pow(3.l, real{2, 5}).get_prec() == 10);
    real_reset_default_prec();
    REQUIRE(pow(r0, int_t{2}) == real{9});
    REQUIRE(pow(int_t{3}, r1) == real{9});
    REQUIRE(pow(real{3}, int_t{2}) == real{9});
    REQUIRE(pow(int_t{3}, real{2}) == real{9});
    REQUIRE(pow(real{3}, int_t{2}).get_prec()
            == std::max<::mpfr_prec_t>(GMP_NUMB_BITS, std::numeric_limits<int>::digits + 1));
    REQUIRE(pow(int_t{3}, real{2}).get_prec()
            == std::max<::mpfr_prec_t>(GMP_NUMB_BITS, std::numeric_limits<int>::digits + 1));
    real_set_default_prec(10);
    REQUIRE(pow(real{3, 5}, int_t{2}) == real{9});
    REQUIRE(pow(real{3, 5}, int_t{2}).get_prec() == 10);
    REQUIRE(pow(int_t{3}, real{2, 5}) == real{9});
    REQUIRE(pow(int_t{3}, real{2, 5}).get_prec() == 10);
    real_reset_default_prec();
    REQUIRE(pow(r0, rat_t{2}) == real{9});
    REQUIRE(pow(rat_t{3}, r1) == real{9});
    REQUIRE(pow(real{3}, rat_t{2}) == real{9});
    REQUIRE(pow(rat_t{3}, real{2}) == real{9});
    REQUIRE(pow(real{3}, rat_t{2}).get_prec()
            == std::max<::mpfr_prec_t>(GMP_NUMB_BITS * 2, std::numeric_limits<int>::digits + 1));
    REQUIRE(pow(rat_t{3}, real{2}).get_prec()
            == std::max<::mpfr_prec_t>(GMP_NUMB_BITS * 2, std::numeric_limits<int>::digits + 1));
    real_set_default_prec(10);
    REQUIRE(pow(real{3, 5}, rat_t{2}) == real{9});
    REQUIRE(pow(real{3, 5}, rat_t{2}).get_prec() == 10);
    REQUIRE(pow(rat_t{3}, real{2, 5}) == real{9});
    REQUIRE(pow(rat_t{3}, real{2, 5}).get_prec() == 10);
    real_reset_default_prec();
#if defined(MPPP_WITH_QUADMATH)
    REQUIRE(pow(r0, real128{2}) == real{9});
    REQUIRE(pow(real128{3}, r1) == real{9});
    REQUIRE(pow(real{3}, real128{2}) == real{9});
    REQUIRE(pow(real128{3}, real{2}) == real{9});
    REQUIRE(pow(real{3}, real128{2}).get_prec() == std::max<::mpfr_prec_t>(113, std::numeric_limits<int>::digits + 1));
    REQUIRE(pow(real128{3}, real{2}).get_prec() == std::max<::mpfr_prec_t>(113, std::numeric_limits<int>::digits + 1));
    real_set_default_prec(10);
    REQUIRE(pow(real{3, 5}, real128{2}) == real{9});
    REQUIRE(pow(real{3, 5}, real128{2}).get_prec() == 10);
    REQUIRE(pow(real128{3}, real{2, 5}) == real{9});
    REQUIRE(pow(real128{3}, real{2, 5}).get_prec() == 10);
    real_reset_default_prec();
#endif
}
