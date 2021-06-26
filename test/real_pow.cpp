// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
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
    REQUIRE(rop.get_prec() == detail::nl_digits<int>() + 1);
    rop = real{};
    pow(rop, real{3}, r1);
    REQUIRE(rop == real{9});
    REQUIRE(rop.get_prec() == detail::nl_digits<int>() + 1);
    rop = real{};
    pow(rop, r0, real{2});
    REQUIRE(rop == real{9});
    REQUIRE(rop.get_prec() == detail::nl_digits<int>() + 1);
    rop = real{};
    pow(rop, real{3}, real{2});
    REQUIRE(rop == real{9});
    REQUIRE(rop.get_prec() == detail::nl_digits<int>() + 1);
    rop = real{};
    pow(rop, std::move(r0), r1);
    REQUIRE(rop == real{9});
    REQUIRE(rop.get_prec() == detail::nl_digits<int>() + 1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r0.zero_p());
    REQUIRE(r0.get_prec() == real_prec_min());
    r0 = 3;
    rop = real{};
    pow(rop, r0, std::move(r1));
    REQUIRE(rop == real{9});
    REQUIRE(rop.get_prec() == detail::nl_digits<int>() + 1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == real_prec_min());
    r1 = 2;
    REQUIRE(pow(r0, r1) == real{9});
    REQUIRE(pow(r0, r1).get_prec() == detail::nl_digits<int>() + 1);
    REQUIRE(pow(r0, real{2}) == real{9});
    REQUIRE(pow(real{3}, r1) == real{9});
    REQUIRE(pow(real{3}, real{2}) == real{9});
    REQUIRE(pow(r0, 2) == real{9});
    REQUIRE(pow(3, r1) == real{9});
    REQUIRE(pow(real{3}, 2) == real{9});
    REQUIRE(pow(3, real{2}) == real{9});
    REQUIRE(pow(real{3}, 2).get_prec() == detail::nl_digits<int>() + 1);
    REQUIRE(pow(3, real{2}).get_prec() == detail::nl_digits<int>() + 1);
    REQUIRE(pow(r0, 2u) == real{9});
    REQUIRE(pow(3u, r1) == real{9});
    REQUIRE(pow(real{3}, 2u) == real{9});
    REQUIRE(pow(3u, real{2}) == real{9});
    REQUIRE(pow(real{3}, 2u).get_prec() == detail::nl_digits<unsigned>());
    REQUIRE(pow(3u, real{2}).get_prec() == detail::nl_digits<unsigned>());
    REQUIRE(pow(r0, 2ll) == real{9});
    REQUIRE(pow(3ll, r1) == real{9});
    REQUIRE(pow(real{3}, 2ll) == real{9});
    REQUIRE(pow(3ll, real{2}) == real{9});
    REQUIRE(pow(real{3}, 2ll).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE(pow(3ll, real{2}).get_prec() == detail::nl_digits<long long>() + 1);
    REQUIRE(pow(r0, 2ull) == real{9});
    REQUIRE(pow(3ull, r1) == real{9});
    REQUIRE(pow(real{3}, 2ull) == real{9});
    REQUIRE(pow(3ull, real{2}) == real{9});
    REQUIRE(pow(real{3}, 2ull).get_prec() == detail::nl_digits<unsigned long long>());
    REQUIRE(pow(3ull, real{2}).get_prec() == detail::nl_digits<unsigned long long>());
    REQUIRE(pow(r0, 2.f) == real{9});
    REQUIRE(pow(3.f, r1) == real{9});
    REQUIRE(pow(real{3}, 2.f) == real{9});
    REQUIRE(pow(3.f, real{2}) == real{9});
    REQUIRE(pow(real{3}, 2.f).get_prec()
            == std::max<::mpfr_prec_t>(detail::dig2mpfr_prec<float>(), detail::nl_digits<int>() + 1));
    REQUIRE(pow(3.f, real{2}).get_prec()
            == std::max<::mpfr_prec_t>(detail::dig2mpfr_prec<float>(), detail::nl_digits<int>() + 1));
    REQUIRE(pow(r0, 2.) == real{9});
    REQUIRE(pow(3., r1) == real{9});
    REQUIRE(pow(real{3}, 2.) == real{9});
    REQUIRE(pow(3., real{2}) == real{9});
    REQUIRE(pow(real{3}, 2.).get_prec()
            == std::max<::mpfr_prec_t>(detail::dig2mpfr_prec<double>(), detail::nl_digits<int>() + 1));
    REQUIRE(pow(3., real{2}).get_prec()
            == std::max<::mpfr_prec_t>(detail::dig2mpfr_prec<double>(), detail::nl_digits<int>() + 1));
    REQUIRE(pow(r0, 2.l) == real{9});
    REQUIRE(pow(3.l, r1) == real{9});
    REQUIRE(pow(real{3}, 2.l) == real{9});
    REQUIRE(pow(3.l, real{2}) == real{9});
    REQUIRE(pow(real{3}, 2.l).get_prec()
            == std::max<::mpfr_prec_t>(detail::dig2mpfr_prec<long double>(), detail::nl_digits<int>() + 1));
    REQUIRE(pow(3.l, real{2}).get_prec()
            == std::max<::mpfr_prec_t>(detail::dig2mpfr_prec<long double>(), detail::nl_digits<int>() + 1));
    REQUIRE(pow(r0, int_t{2}) == real{9});
    REQUIRE(pow(int_t{3}, r1) == real{9});
    REQUIRE(pow(real{3}, int_t{2}) == real{9});
    REQUIRE(pow(int_t{3}, real{2}) == real{9});
    REQUIRE(pow(real{3}, int_t{2}).get_prec() == std::max<::mpfr_prec_t>(GMP_NUMB_BITS, detail::nl_digits<int>() + 1));
    REQUIRE(pow(int_t{3}, real{2}).get_prec() == std::max<::mpfr_prec_t>(GMP_NUMB_BITS, detail::nl_digits<int>() + 1));
    REQUIRE(pow(r0, rat_t{2}) == real{9});
    REQUIRE(pow(rat_t{3}, r1) == real{9});
    REQUIRE(pow(real{3}, rat_t{2}) == real{9});
    REQUIRE(pow(rat_t{3}, real{2}) == real{9});
    REQUIRE(pow(real{3}, rat_t{2}).get_prec()
            == std::max<::mpfr_prec_t>(GMP_NUMB_BITS * 2, detail::nl_digits<int>() + 1));
    REQUIRE(pow(rat_t{3}, real{2}).get_prec()
            == std::max<::mpfr_prec_t>(GMP_NUMB_BITS * 2, detail::nl_digits<int>() + 1));
#if defined(MPPP_WITH_QUADMATH)
    REQUIRE(pow(r0, real128{2}) == real{9});
    REQUIRE(pow(real128{3}, r1) == real{9});
    REQUIRE(pow(real{3}, real128{2}) == real{9});
    REQUIRE(pow(real128{3}, real{2}) == real{9});
    REQUIRE(pow(real{3}, real128{2}).get_prec() == std::max<::mpfr_prec_t>(113, detail::nl_digits<int>() + 1));
    REQUIRE(pow(real128{3}, real{2}).get_prec() == std::max<::mpfr_prec_t>(113, detail::nl_digits<int>() + 1));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE(pow(r0, __int128_t{2}) == real{9});
    REQUIRE(pow(r0, __uint128_t{2}) == real{9});
    REQUIRE(pow(__int128_t{3}, r1) == real{9});
    REQUIRE(pow(__uint128_t{3}, r1) == real{9});
    REQUIRE(pow(real{3}, __int128_t{2}).get_prec() == std::max(128, detail::nl_digits<int>() + 1));
    REQUIRE(pow(__uint128_t{3}, real{2}).get_prec() == std::max(128, detail::nl_digits<int>() + 1));

    // Try also with large values.
    REQUIRE(pow(1._r512, __int128_t{1} << 65) - pow(1._r512, pow(2_r128, 65)) == 0);
    REQUIRE(pow(__int128_t{1} << 65, 1._r512) - pow(pow(2_r128, 65), 1._r512) == 0);

    REQUIRE(pow(1._r512, __uint128_t{1} << 65) - pow(1._r512, pow(2_r128, 65)) == 0);
    REQUIRE(pow(__uint128_t{1} << 65, 1._r512) - pow(pow(2_r128, 65), 1._r512) == 0);
#endif

    // Ensure that x**(1/3) is almost identical to cbrt(1.1).
    REQUIRE(abs(pow(1.1_r512, 1_q1 / 3) - cbrt(1.1_r512)) <= pow(2_r512, -500));

    // Special casing for bool.
    REQUIRE(mppp::pow(real{123}, false) == 1);
    REQUIRE(mppp::pow(real{123}, false).get_prec()
            == std::max(detail::real_deduce_precision(123), detail::real_deduce_precision(false)));
    REQUIRE(mppp::pow(real{123}, true) == 123);
    REQUIRE(mppp::pow(real{123}, true).get_prec()
            == std::max(detail::real_deduce_precision(123), detail::real_deduce_precision(false)));

    REQUIRE(mppp::pow(false, real{123}) == 0);
    REQUIRE(mppp::pow(false, real{123}).get_prec()
            == std::max(detail::real_deduce_precision(123), detail::real_deduce_precision(false)));
    REQUIRE(mppp::pow(true, real{123}) == 1);
    REQUIRE(mppp::pow(true, real{123}).get_prec()
            == std::max(detail::real_deduce_precision(123), detail::real_deduce_precision(false)));
}

TEST_CASE("real sqr")
{
    real r0{2};
    r0.sqr();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 4);
    real rop;
    REQUIRE(sqr(rop, r0) == 16);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(sqr(r0) == 16);
    REQUIRE(sqr(std::move(r0)) == 16);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    r0 = real{-16, 128};
    REQUIRE(sqr(r0) == 256);
    REQUIRE(sqr(r0).get_prec() == 128);
    rop = real{12, 40};
    sqr(rop, r0);
    REQUIRE(rop == 256);
    REQUIRE(rop.get_prec() == 128);
    r0.sqr();
    REQUIRE(r0 == 256);
    REQUIRE(r0.get_prec() == 128);

    REQUIRE(sqr(real{0}).zero_p());
    REQUIRE(sqr(real{-0.}).zero_p());
    REQUIRE(sqr(real{1}) == 1);
    REQUIRE(sqr(real{"inf", 34}).inf_p());
    REQUIRE(sqr(real{"inf", 34}).get_prec() == 34);
    REQUIRE(sqr(real{"-inf", 34}).inf_p());
    REQUIRE(sqr(real{"-inf", 34}).get_prec() == 34);
    REQUIRE(sqr(real{"nan", 34}).nan_p());
    REQUIRE(sqr(real{"nan", 34}).get_prec() == 34);
}
