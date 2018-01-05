// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <mp++/detail/mpfr.hpp>
#include <mp++/real.hpp>
#include <utility>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

#if defined(_MSC_VER)

template <typename... Args>
auto fma_wrap(Args &&... args) -> decltype(mppp::fma(std::forward<Args>(args)...))
{
    return mppp::fma(std::forward<Args>(args)...);
}

template <typename... Args>
auto fms_wrap(Args &&... args) -> decltype(mppp::fms(std::forward<Args>(args)...))
{
    return mppp::fms(std::forward<Args>(args)...);
}

#else

#define fma_wrap fma
#define fms_wrap fms

#endif

TEST_CASE("real arith nary steal")
{
    real r1, r2, r3;
    auto p = std::make_pair(static_cast<real *>(nullptr), r1.get_prec());
    mpfr_nary_op_check_steal(p, r2, r3);
    REQUIRE(!p.first);
    REQUIRE(p.second == r1.get_prec());
    r1.set_prec(r1.get_prec() + 1);
    p = std::make_pair(static_cast<real *>(nullptr), r1.get_prec());
    mpfr_nary_op_check_steal(p, r2, r3);
    REQUIRE(!p.first);
    REQUIRE(p.second == r1.get_prec());
    p = std::make_pair(static_cast<real *>(nullptr), r1.get_prec());
    mpfr_nary_op_check_steal(p, std::move(r2), r3);
    REQUIRE(p.first == &r2);
    REQUIRE(p.second == r1.get_prec());
    p = std::make_pair(static_cast<real *>(nullptr), r1.get_prec());
    mpfr_nary_op_check_steal(p, r2, std::move(r3));
    REQUIRE(p.first == &r3);
    REQUIRE(p.second == r1.get_prec());
    p = std::make_pair(static_cast<real *>(nullptr), r1.get_prec());
    mpfr_nary_op_check_steal(p, std::move(r2), std::move(r3));
    REQUIRE(p.first == &r2);
    REQUIRE(p.second == r1.get_prec());
    p = std::make_pair(&r1, r1.get_prec());
    mpfr_nary_op_check_steal(p, std::move(r2), std::move(r3));
    REQUIRE(p.first == &r1);
    REQUIRE(p.second == r1.get_prec());
    p = std::make_pair(&r1, r1.get_prec());
    mpfr_nary_op_check_steal(p, static_cast<const real &>(r2), r3);
    REQUIRE(p.first == &r1);
    REQUIRE(p.second == r1.get_prec());
    r3.set_prec(r1.get_prec() + 1);
    p = std::make_pair(&r1, r1.get_prec());
    mpfr_nary_op_check_steal(p, r2, r3);
    REQUIRE(p.first == &r1);
    REQUIRE(p.second == r3.get_prec());
    p = std::make_pair(&r1, r1.get_prec());
    mpfr_nary_op_check_steal(p, r2, std::move(r3));
    REQUIRE(p.first == &r3);
    REQUIRE(p.second == r3.get_prec());
    p = std::make_pair(static_cast<real *>(nullptr), r1.get_prec());
    mpfr_nary_op_check_steal(p, r2, std::move(r3));
    REQUIRE(p.first == &r3);
    REQUIRE(p.second == r3.get_prec());
}

TEST_CASE("real add")
{
    real r1, r2, r3;
    add(r1, r2, r3);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    r1 = 56;
    add(r1, r2, r3);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    r2 = 56;
    r3 = -45;
    r1 = -4;
    add(r1, r2, r3);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{11}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == r3.get_prec());
    r1.prec_round(real_prec_min());
    add(r1, r2, r3);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{11}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == r3.get_prec());
    add(r1, real{12, 123}, real{34, 128});
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{46}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    // Some tests with rvalue refs/overlapping arguments.
    add(r1, std::move(r1), std::move(r1));
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{92}.get_mpfr_t()));
    add(r1, std::move(r1), real{100, 150});
    REQUIRE(r1.get_prec() == 150);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{192}.get_mpfr_t()));
    add(r1, std::move(r1), real{100, 50});
    REQUIRE(r1.get_prec() == 150);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{292}.get_mpfr_t()));
    add(r1, real{100, 160}, std::move(r1));
    REQUIRE(r1.get_prec() == 160);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{392}.get_mpfr_t()));
    add(r1, real{100, 50}, std::move(r1));
    REQUIRE(r1.get_prec() == 160);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{492}.get_mpfr_t()));
    r1 = real{92, 128};
    add(r1, r1, std::move(r1));
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{184}.get_mpfr_t()));
    add(r1, std::move(r1), r1);
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{368}.get_mpfr_t()));
    r1 = real{};
    add(r1, real{10, 50}, real{12, 51});
    REQUIRE(r1.get_prec() == 51);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{22}.get_mpfr_t()));
    r1 = real{};
    add(r1, real{10, 52}, real{12, 51});
    REQUIRE(r1.get_prec() == 52);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{22}.get_mpfr_t()));
    r1 = real{0, 123};
    add(r1, real{10, 52}, real{12, 51});
    REQUIRE(r1.get_prec() == 52);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{22}.get_mpfr_t()));
}

TEST_CASE("real sub")
{
    real r1, r2, r3;
    sub(r1, r2, r3);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    r1 = 56;
    sub(r1, r2, r3);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    r2 = 56;
    r3 = -45;
    r1 = -4;
    sub(r1, r2, r3);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{101}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == r3.get_prec());
    r1.prec_round(real_prec_min());
    sub(r1, r2, r3);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{101}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == r3.get_prec());
    sub(r1, real{12, 123}, real{34, 128});
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{-22}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    // Some tests with rvalue refs/overlapping arguments.
    sub(r1, std::move(r1), std::move(r1));
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{0}.get_mpfr_t()));
    r1 = real{123, 128};
    sub(r1, r1, std::move(r1));
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{0}.get_mpfr_t()));
    r1 = real{123, 128};
    sub(r1, std::move(r1), r1);
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{0}.get_mpfr_t()));
    r1 = real{};
    sub(r1, real{10, 50}, real{12, 51});
    REQUIRE(r1.get_prec() == 51);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{-2}.get_mpfr_t()));
    r1 = real{};
    sub(r1, real{10, 52}, real{12, 51});
    REQUIRE(r1.get_prec() == 52);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{-2}.get_mpfr_t()));
    r1 = real{0, 123};
    sub(r1, real{10, 52}, real{12, 51});
    REQUIRE(r1.get_prec() == 52);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{-2}.get_mpfr_t()));
}

TEST_CASE("real mul")
{
    real r1, r2, r3;
    mul(r1, r2, r3);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    r1 = 56;
    mul(r1, r2, r3);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    r2 = 56;
    r3 = -45;
    r1 = -4;
    mul(r1, r2, r3);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{-2520}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == r3.get_prec());
    r1.prec_round(real_prec_min());
    mul(r1, r2, r3);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{-2520}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == r3.get_prec());
    mul(r1, real{12, 123}, real{34, 128});
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{408}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    // Some tests with rvalue refs/overlapping arguments.
    r1 = real{2, 128};
    mul(r1, std::move(r1), std::move(r1));
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{4}.get_mpfr_t()));
    mul(r1, r1, std::move(r1));
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{16}.get_mpfr_t()));
    mul(r1, std::move(r1), r1);
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{256}.get_mpfr_t()));
    r1 = real{};
    mul(r1, real{10, 50}, real{12, 51});
    REQUIRE(r1.get_prec() == 51);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{120}.get_mpfr_t()));
    r1 = real{};
    mul(r1, real{10, 52}, real{12, 51});
    REQUIRE(r1.get_prec() == 52);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{120}.get_mpfr_t()));
    r1 = real{0, 123};
    mul(r1, real{10, 52}, real{12, 51});
    REQUIRE(r1.get_prec() == 52);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{120}.get_mpfr_t()));
}

TEST_CASE("real div")
{
    real r1, r2, r3;
    div(r1, r2, r3);
    REQUIRE(r1.nan_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    r1 = 56;
    div(r1, r2, r3);
    REQUIRE(r1.nan_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    r2 = 56;
    r3 = -7;
    r1 = -4;
    div(r1, r2, r3);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{-8}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == r3.get_prec());
    r1.prec_round(real_prec_min());
    div(r1, r2, r3);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{-8}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == r3.get_prec());
    div(r1, real{12, 123}, real{32, 128});
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{"0.375", 64}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    // Some tests with rvalue refs/overlapping arguments.
    r1 = real{256, 128};
    div(r1, std::move(r1), std::move(r1));
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{1}.get_mpfr_t()));
    r1 = real{256, 128};
    div(r1, r1, std::move(r1));
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{1}.get_mpfr_t()));
    r1 = real{256, 128};
    div(r1, std::move(r1), r1);
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{1}.get_mpfr_t()));
    r1 = real{};
    div(r1, real{10, 50}, real{5, 51});
    REQUIRE(r1.get_prec() == 51);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{2}.get_mpfr_t()));
    r1 = real{};
    div(r1, real{10, 52}, real{5, 51});
    REQUIRE(r1.get_prec() == 52);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{2}.get_mpfr_t()));
    r1 = real{0, 123};
    div(r1, real{10, 52}, real{5, 51});
    REQUIRE(r1.get_prec() == 52);
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{2}.get_mpfr_t()));
}

TEST_CASE("real fma")
{
    real r1, r2, r3, r4;
    fma_wrap(r1, r2, r3, r4);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    fma_wrap(r1, real{2, 12}, real{3, 7}, real{14, 128});
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{20}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = 0;
    fma_wrap(r1, real{3, 7}, real{2, 12}, real{14, 128});
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{20}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = 0;
    fma_wrap(r1, real{14, 128}, real{3, 7}, real{2, 12});
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{44}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = fma_wrap(real{14, 128}, real{3, 7}, real{2, 12});
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{44}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = fma_wrap(static_cast<const real &>(real{14, 128}), real{3, 7}, real{2, 12});
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{44}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = fma_wrap(real{14, 128}, static_cast<const real &>(real{3, 7}), real{2, 12});
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{44}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = fma_wrap(real{14, 128}, real{3, 7}, static_cast<const real &>(real{2, 12}));
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{44}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = fma_wrap(real{14, 128}, static_cast<const real &>(real{3, 7}), static_cast<const real &>(real{2, 12}));
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{44}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = fma_wrap(static_cast<const real &>(real{14, 128}), real{3, 7}, static_cast<const real &>(real{2, 12}));
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{44}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = fma_wrap(static_cast<const real &>(real{14, 128}), static_cast<const real &>(real{3, 7}),
                  static_cast<const real &>(real{2, 12}));
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{44}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    // Overlap + rvalue.
    r1 = 0;
    fma_wrap(r1, std::move(r1), std::move(r1), std::move(r1));
    REQUIRE(r1.zero_p());
    fma_wrap(r1, r1, std::move(r1), std::move(r1));
    REQUIRE(r1.zero_p());
    fma_wrap(r1, r1, r1, std::move(r1));
    REQUIRE(r1.zero_p());
    fma_wrap(r1, std::move(r1), r1, std::move(r1));
    REQUIRE(r1.zero_p());
    fma_wrap(r1, std::move(r1), std::move(r1), r1);
    REQUIRE(r1.zero_p());
}

TEST_CASE("real fms")
{
    real r1, r2, r3, r4;
    fms_wrap(r1, r2, r3, r4);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    fms_wrap(r1, real{2, 12}, real{3, 7}, real{14, 128});
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{-8}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = 0;
    fms_wrap(r1, real{3, 7}, real{2, 12}, real{14, 128});
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{-8}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = 0;
    fms_wrap(r1, real{14, 128}, real{3, 7}, real{2, 12});
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{40}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = fms_wrap(real{14, 128}, real{3, 7}, real{2, 12});
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{40}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = fms_wrap(static_cast<const real &>(real{14, 128}), real{3, 7}, real{2, 12});
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{40}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = fms_wrap(real{14, 128}, static_cast<const real &>(real{3, 7}), real{2, 12});
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{40}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = fms_wrap(real{14, 128}, real{3, 7}, static_cast<const real &>(real{2, 12}));
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{40}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = fms_wrap(real{14, 128}, static_cast<const real &>(real{3, 7}), static_cast<const real &>(real{2, 12}));
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{40}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = fms_wrap(static_cast<const real &>(real{14, 128}), real{3, 7}, static_cast<const real &>(real{2, 12}));
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{40}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    r1 = fms_wrap(static_cast<const real &>(real{14, 128}), static_cast<const real &>(real{3, 7}),
                  static_cast<const real &>(real{2, 12}));
    REQUIRE(::mpfr_equal_p(r1.get_mpfr_t(), real{40}.get_mpfr_t()));
    REQUIRE(r1.get_prec() == 128);
    // Overlap + rvalue.
    r1 = 0;
    fms_wrap(r1, std::move(r1), std::move(r1), std::move(r1));
    REQUIRE(r1.zero_p());
    fms_wrap(r1, r1, std::move(r1), std::move(r1));
    REQUIRE(r1.zero_p());
    fms_wrap(r1, r1, r1, std::move(r1));
    REQUIRE(r1.zero_p());
    fms_wrap(r1, std::move(r1), r1, std::move(r1));
    REQUIRE(r1.zero_p());
    fms_wrap(r1, std::move(r1), std::move(r1), r1);
    REQUIRE(r1.zero_p());
}
