// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <type_traits>
#include <utility>

#include <mp++/real.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("real mul_2ui")
{
    // The return form.
    REQUIRE(mul_2ui(2_r128, 0) == 2);
    REQUIRE(std::is_same<real, decltype(mul_2ui(2_r128, 0))>::value);
    REQUIRE(mul_2ui(2_r128, 1) == 4);
    REQUIRE(mul_2ui(2_r128, 2) == 8);
    REQUIRE(mul_2ui(2_r128, 2).get_prec() == 128);
    const auto c = 36077725286.399999999999999999999999999919_r128;
    REQUIRE(abs(mul_2ui(2.1_r128, 34) - c) / c < mppp::pow(2_r128, -126));
    auto r0 = 2.1_r128;
    auto r1 = mul_2ui(std::move(r0), 34);
    REQUIRE(abs(r1 - c) / c < mppp::pow(2_r128, -126));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    r0 = 2.1_r128;
    r1 = mul_2ui(r0, 34);
    REQUIRE(abs(r1 - c) / c < mppp::pow(2_r128, -126));
    REQUIRE(r0.is_valid());

    // The form with rop.
    r0 = real{0, 40};
    mul_2ui(r0, 2.1_r128, 34);
    REQUIRE(std::is_same<real &, decltype(mul_2ui(r0, 2.1_r128, 34))>::value);
    REQUIRE(abs(r0 - c) / c < mppp::pow(2_r128, -126));
    REQUIRE(r0.get_prec() == 128);
    r1 = 2.1_r128;
    r0 = real{1, 12};
    mul_2ui(r0, std::move(r1), 34);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.is_valid());
    REQUIRE(r1.get_prec() == 12);
    REQUIRE(r1 == 1);
    REQUIRE(abs(r0 - c) / c < mppp::pow(2_r128, -126));
    REQUIRE(r0.get_prec() == 128);
    r1 = 2.1_r128;
    r0 = real{1, 12};
    mul_2ui(r0, r1, 34);
    REQUIRE(r1.is_valid());
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(r1 == 2.1_r128);
    REQUIRE(abs(r0 - c) / c < mppp::pow(2_r128, -126));
    REQUIRE(r0.get_prec() == 128);
}

TEST_CASE("real mul_2si")
{
    // The return form.
    REQUIRE(mul_2si(2_r128, 0) == 2);
    REQUIRE(std::is_same<real, decltype(mul_2si(2_r128, 0))>::value);
    REQUIRE(mul_2si(2_r128, -1) == 1);
    REQUIRE(mul_2si(2_r128, -2) == 2_r128 / 4);
    REQUIRE(mul_2si(2_r128, 2).get_prec() == 128);
    const auto c = 0.00000000012223608791828155517578124999999999999973_r128;
    REQUIRE(abs(mul_2si(2.1_r128, -34) - c) / c < mppp::pow(2_r128, -126));
    auto r0 = 2.1_r128;
    auto r1 = mul_2si(std::move(r0), -34);
    REQUIRE(abs(r1 - c) / c < mppp::pow(2_r128, -126));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    r0 = 2.1_r128;
    r1 = mul_2si(r0, -34);
    REQUIRE(abs(r1 - c) / c < mppp::pow(2_r128, -126));
    REQUIRE(r0.is_valid());

    // The form with rop.
    r0 = real{0, 40};
    mul_2si(r0, 2.1_r128, -34);
    REQUIRE(std::is_same<real &, decltype(mul_2si(r0, 2.1_r128, -34))>::value);
    REQUIRE(abs(r0 - c) / c < mppp::pow(2_r128, -126));
    REQUIRE(r0.get_prec() == 128);
    r1 = 2.1_r128;
    r0 = real{1, 12};
    mul_2si(r0, std::move(r1), -34);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.is_valid());
    REQUIRE(r1.get_prec() == 12);
    REQUIRE(r1 == 1);
    REQUIRE(abs(r0 - c) / c < mppp::pow(2_r128, -126));
    REQUIRE(r0.get_prec() == 128);
    r1 = 2.1_r128;
    r0 = real{1, 12};
    mul_2si(r0, r1, -34);
    REQUIRE(r1.is_valid());
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(r1 == 2.1_r128);
    REQUIRE(abs(r0 - c) / c < mppp::pow(2_r128, -126));
    REQUIRE(r0.get_prec() == 128);
}

TEST_CASE("real div_2ui")
{
    // The return form.
    REQUIRE(div_2ui(2_r128, 0) == 2);
    REQUIRE(std::is_same<real, decltype(div_2ui(2_r128, 0))>::value);
    REQUIRE(div_2ui(2_r128, 1) == 1);
    REQUIRE(div_2ui(2_r128, 2) == 2_r128 / 4);
    REQUIRE(div_2ui(2_r128, 2).get_prec() == 128);
    const auto c = 0.00000000012223608791828155517578124999999999999973_r128;
    REQUIRE(abs(div_2ui(2.1_r128, 34) - c) / c < mppp::pow(2_r128, -126));
    auto r0 = 2.1_r128;
    auto r1 = div_2ui(std::move(r0), 34);
    REQUIRE(abs(r1 - c) / c < mppp::pow(2_r128, -126));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    r0 = 2.1_r128;
    r1 = div_2ui(r0, 34);
    REQUIRE(abs(r1 - c) / c < mppp::pow(2_r128, -126));
    REQUIRE(r0.is_valid());

    // The form with rop.
    r0 = real{0, 40};
    div_2ui(r0, 2.1_r128, 34);
    REQUIRE(std::is_same<real &, decltype(div_2ui(r0, 2.1_r128, 34))>::value);
    REQUIRE(abs(r0 - c) / c < mppp::pow(2_r128, -126));
    REQUIRE(r0.get_prec() == 128);
    r1 = 2.1_r128;
    r0 = real{1, 12};
    div_2ui(r0, std::move(r1), 34);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.is_valid());
    REQUIRE(r1.get_prec() == 12);
    REQUIRE(r1 == 1);
    REQUIRE(abs(r0 - c) / c < mppp::pow(2_r128, -126));
    REQUIRE(r0.get_prec() == 128);
    r1 = 2.1_r128;
    r0 = real{1, 12};
    div_2ui(r0, r1, 34);
    REQUIRE(r1.is_valid());
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(r1 == 2.1_r128);
    REQUIRE(abs(r0 - c) / c < mppp::pow(2_r128, -126));
    REQUIRE(r0.get_prec() == 128);
}

TEST_CASE("real div_2si")
{
    // The return form.
    REQUIRE(div_2si(2_r128, 0) == 2);
    REQUIRE(std::is_same<real, decltype(div_2si(2_r128, 0))>::value);
    REQUIRE(div_2si(2_r128, 1) == 1);
    REQUIRE(div_2si(2_r128, -1) == 4);
    REQUIRE(div_2si(2_r128, -2) == 8);
    REQUIRE(div_2si(2_r128, 2) == 2_r128 / 4);
    REQUIRE(div_2si(2_r128, 2).get_prec() == 128);
    const auto c = 36077725286.399999999999999999999999999919_r128;
    REQUIRE(abs(div_2si(2.1_r128, -34) - c) / c < mppp::pow(2_r128, -126));
    auto r0 = 2.1_r128;
    auto r1 = div_2si(std::move(r0), -34);
    REQUIRE(abs(r1 - c) / c < mppp::pow(2_r128, -126));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    r0 = 2.1_r128;
    r1 = div_2si(r0, -34);
    REQUIRE(abs(r1 - c) / c < mppp::pow(2_r128, -126));
    REQUIRE(r0.is_valid());

    // The form with rop.
    r0 = real{0, 40};
    div_2si(r0, 2.1_r128, -34);
    REQUIRE(std::is_same<real &, decltype(div_2si(r0, 2.1_r128, -34))>::value);
    REQUIRE(abs(r0 - c) / c < mppp::pow(2_r128, -126));
    REQUIRE(r0.get_prec() == 128);
    r1 = 2.1_r128;
    r0 = real{1, 12};
    div_2si(r0, std::move(r1), -34);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.is_valid());
    REQUIRE(r1.get_prec() == 12);
    REQUIRE(r1 == 1);
    REQUIRE(abs(r0 - c) / c < mppp::pow(2_r128, -126));
    REQUIRE(r0.get_prec() == 128);
    r1 = 2.1_r128;
    r0 = real{1, 12};
    div_2si(r0, r1, -34);
    REQUIRE(r1.is_valid());
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(r1 == 2.1_r128);
    REQUIRE(abs(r0 - c) / c < mppp::pow(2_r128, -126));
    REQUIRE(r0.get_prec() == 128);
}
