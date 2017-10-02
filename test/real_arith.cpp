// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
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

#else

#define fma_wrap fma

#endif

TEST_CASE("real arith nary rop")
{
    real r1, r2, r3;
    auto p = std::make_pair(true, r2.get_prec());
    mpfr_examine_precs(p, r1, r3);
    REQUIRE(p.first);
    REQUIRE(p.second == r1.get_prec());
    r1.set_prec(r1.get_prec() + 1);
    p = std::make_pair(r1.get_prec() == r2.get_prec(), r2.get_prec());
    mpfr_examine_precs(p, r1, r3);
    REQUIRE(!p.first);
    REQUIRE(p.second == r2.get_prec());
    r2.set_prec(r2.get_prec() + 1);
    p = std::make_pair(r1.get_prec() == r2.get_prec(), r2.get_prec());
    mpfr_examine_precs(p, r1, r3);
    REQUIRE(!p.first);
    REQUIRE(p.second == r3.get_prec() + 1);
    r3.set_prec(r3.get_prec() + 1);
    p = std::make_pair(r1.get_prec() == r2.get_prec(), r2.get_prec());
    mpfr_examine_precs(p, r1, r3);
    REQUIRE(p.first);
    REQUIRE(p.second == r1.get_prec());
    r3.set_prec(r3.get_prec() + 1);
    p = std::make_pair(r1.get_prec() == r2.get_prec(), r2.get_prec());
    mpfr_examine_precs(p, r1, r3);
    REQUIRE(!p.first);
    REQUIRE(p.second == r3.get_prec());
    r2.set_prec(r3.get_prec() + 2);
    p = std::make_pair(r1.get_prec() == r2.get_prec(), r2.get_prec());
    mpfr_examine_precs(p, r1, r3);
    REQUIRE(!p.first);
    REQUIRE(p.second == r2.get_prec());
    // Try with only 1 operand as well.
    r1 = real{};
    r2 = real{};
    p = std::make_pair(true, r2.get_prec());
    mpfr_examine_precs(p, r1);
    REQUIRE(p.first);
    REQUIRE(p.second == r1.get_prec());
    r1.set_prec(r1.get_prec() + 1);
    p = std::make_pair(r1.get_prec() == r2.get_prec(), r2.get_prec());
    mpfr_examine_precs(p, r1);
    REQUIRE(!p.first);
    REQUIRE(p.second == r2.get_prec());
    r2.set_prec(r2.get_prec() + 1);
    p = std::make_pair(r1.get_prec() == r2.get_prec(), r2.get_prec());
    mpfr_examine_precs(p, r1);
    REQUIRE(p.first);
    REQUIRE(p.second == r2.get_prec());
}

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
}
