// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <type_traits>
#include <utility>

#include <mp++/complex.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("exp")
{
    complex r0{0};
    r0.exp();
    REQUIRE(std::is_same<complex &, decltype(r0.exp())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    r0 = 0;
    complex rop;
    REQUIRE(exp(rop, r0) == 1);
    REQUIRE(std::is_same<complex &, decltype(exp(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    r0 = 0;
    REQUIRE(exp(r0) == 1);
    REQUIRE(std::is_same<complex, decltype(exp(r0))>::value);
    REQUIRE(exp(std::move(r0)) == 1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    r0 = complex{16, 17, complex_prec_t(128)};
    const auto cmp = -2445131.833118359643378588430813828510240021169288406198855703802785185164085466_r128
                     - 8543084.366980812241428998694463654837258755013240486378160291834679603293584698_icr128;
    REQUIRE(abs(exp(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(exp(r0).get_prec() == 128);
    rop = complex{16, 17, complex_prec_t(40)};
    exp(rop, r0);
    REQUIRE(abs(exp(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(rop.get_prec() == 128);
    r0.exp();
    REQUIRE(abs(r0 - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(r0.get_prec() == 128);
}

TEST_CASE("log")
{
    complex r0{1};
    r0.log();
    REQUIRE(std::is_same<complex &, decltype(r0.log())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    r0 = 1;
    complex rop;
    REQUIRE(log(rop, r0) == 0);
    REQUIRE(std::is_same<complex &, decltype(log(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    r0 = 1;
    REQUIRE(log(r0) == 0);
    REQUIRE(std::is_same<complex, decltype(log(r0))>::value);
    REQUIRE(log(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    r0 = complex{16, 17, complex_prec_t(128)};
    const auto cmp = 3.150392897331622037489037892717694645292975118885874437493760723077541135951419_r128
                     + 0.8156919233162234110214608387456458465668800463893672274043331243464113240668553_icr128;
    REQUIRE(abs(log(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(log(r0).get_prec() == 128);
    rop = complex{16, 17, complex_prec_t(40)};
    log(rop, r0);
    REQUIRE(abs(log(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(rop.get_prec() == 128);
    r0.log();
    REQUIRE(abs(r0 - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(r0.get_prec() == 128);
}

TEST_CASE("log10")
{
    complex r0{1};
    r0.log10();
    REQUIRE(std::is_same<complex &, decltype(r0.log10())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    r0 = 1;
    complex rop;
    REQUIRE(log10(rop, r0) == 0);
    REQUIRE(std::is_same<complex &, decltype(log10(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    r0 = 1;
    REQUIRE(log10(r0) == 0);
    REQUIRE(std::is_same<complex, decltype(log10(r0))>::value);
    REQUIRE(log10(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    r0 = complex{16, 17, complex_prec_t(128)};
    const auto cmp = 1.368198251138321219993387206444691129937007320682937570399986537071603805920224_r128
                     + 0.3542505012292862657052583097680702767133408029264230838139883294900267333560827_icr128;
    REQUIRE(abs(log10(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(log10(r0).get_prec() == 128);
    rop = complex{16, 17, complex_prec_t(40)};
    log10(rop, r0);
    REQUIRE(abs(log10(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(rop.get_prec() == 128);
    r0.log10();
    REQUIRE(abs(r0 - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(r0.get_prec() == 128);
}
