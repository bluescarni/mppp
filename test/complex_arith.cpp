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
#include <mp++/config.hpp>
#include <mp++/rational.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"

#if defined(_MSC_VER)

template <typename... Args>
auto fma_wrap(Args &&...args) -> decltype(mppp::fma(std::forward<Args>(args)...))
{
    return mppp::fma(std::forward<Args>(args)...);
}

#else

#define fma_wrap fma

#endif

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("add")
{
    complex r1, r2, r3;
    add(r1, r2, r3);
    REQUIRE(std::is_same<complex &, decltype(add(r1, r2, r3))>::value);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    r1 = 56;
    add(r1, r2, r3);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    r2 = complex{56, 3};
    r3 = complex{-45, 4};
    r1 = complex{-4, complex_prec_t(128)};
    add(r1, r2, r3);
    REQUIRE(r1 == complex{11, 7});
    REQUIRE(r1.get_prec() == r3.get_prec());
    r1.prec_round(real_prec_min());
    add(r1, r2, r3);
    REQUIRE(r1 == complex{11, 7});
    REQUIRE(r1.get_prec() == r3.get_prec());
    add(r1, complex{12, complex_prec_t(123)}, complex{34, complex_prec_t(128)});
    REQUIRE(r1 == 46);
    REQUIRE(r1.get_prec() == 128);
    // Some tests with rvalue refs/overlapping arguments.
    add(r1, std::move(r1), std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(r1 == 92);
    add(r1, std::move(r1), complex{100, complex_prec_t(150)});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 150);
    REQUIRE(r1 == 192);
    add(r1, std::move(r1), complex{100, complex_prec_t(50)});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 150);
    REQUIRE(r1 == 292);
    add(r1, complex{100, complex_prec_t(160)}, std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 160);
    REQUIRE(r1 == 392);
    add(r1, complex{100, complex_prec_t(50)}, std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 160);
    REQUIRE(r1 == 492);
    r1 = complex{92, complex_prec_t(128)};
    add(r1, r1, std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(r1 == 184);
    add(r1, std::move(r1), r1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(r1 == 368);
    r1 = complex{};
    add(r1, complex{10, complex_prec_t(50)}, complex{12, complex_prec_t(51)});
    REQUIRE(r1.get_prec() == 51);
    REQUIRE(r1 == 22);
    r1 = complex{};
    add(r1, complex{10, complex_prec_t(52)}, complex{12, complex_prec_t(51)});
    REQUIRE(r1.get_prec() == 52);
    REQUIRE(r1 == 22);
    r1 = complex{0, 123};
    add(r1, complex{10, complex_prec_t(52)}, complex{12, complex_prec_t(51)});
    REQUIRE(r1.get_prec() == 52);
    REQUIRE(r1 == 22);
}

TEST_CASE("sub")
{
    complex r1, r2, r3;
    sub(r1, r2, r3);
    REQUIRE(std::is_same<complex &, decltype(sub(r1, r2, r3))>::value);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    r1 = 56;
    sub(r1, r2, r3);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    r2 = complex{56, 3};
    r3 = complex{-45, 4};
    r1 = complex{-4, complex_prec_t(128)};
    sub(r1, r2, r3);
    REQUIRE(r1 == complex{101, -1});
    REQUIRE(r1.get_prec() == r3.get_prec());
    r1.prec_round(real_prec_min());
    sub(r1, r2, r3);
    REQUIRE(r1 == complex{101, -1});
    REQUIRE(r1.get_prec() == r3.get_prec());
    sub(r1, complex{12, complex_prec_t(123)}, complex{34, complex_prec_t(128)});
    REQUIRE(r1 == -22);
    REQUIRE(r1.get_prec() == 128);
    // Some tests with rvalue refs/overlapping arguments.
    sub(r1, std::move(r1), std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(r1 == 0);
    sub(r1, std::move(r1), complex{100, complex_prec_t(150)});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 150);
    REQUIRE(r1 == -100);
    sub(r1, std::move(r1), complex{100, complex_prec_t(50)});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 150);
    REQUIRE(r1 == -200);
    sub(r1, complex{100, complex_prec_t(160)}, std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 160);
    REQUIRE(r1 == 300);
    sub(r1, complex{100, complex_prec_t(50)}, std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 160);
    REQUIRE(r1 == -200);
    r1 = complex{92, complex_prec_t(128)};
    sub(r1, r1, std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(r1 == 0);
    sub(r1, std::move(r1), r1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(r1 == 0);
    r1 = complex{};
    sub(r1, complex{10, complex_prec_t(50)}, complex{12, complex_prec_t(51)});
    REQUIRE(r1.get_prec() == 51);
    REQUIRE(r1 == -2);
    r1 = complex{};
    sub(r1, complex{10, complex_prec_t(52)}, complex{12, complex_prec_t(51)});
    REQUIRE(r1.get_prec() == 52);
    REQUIRE(r1 == -2);
    r1 = complex{0, 123};
    sub(r1, complex{10, complex_prec_t(52)}, complex{12, complex_prec_t(51)});
    REQUIRE(r1.get_prec() == 52);
    REQUIRE(r1 == -2);
}

TEST_CASE("mul")
{
    complex r1, r2, r3;
    mul(r1, r2, r3);
    REQUIRE(std::is_same<complex &, decltype(mul(r1, r2, r3))>::value);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    r1 = 56;
    mul(r1, r2, r3);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    r2 = complex{56, 3};
    r3 = complex{-45, 4};
    r1 = complex{-4, complex_prec_t(128)};
    mul(r1, r2, r3);
    REQUIRE(r1 == complex{-2532, 89});
    REQUIRE(r1.get_prec() == r3.get_prec());
    r1.prec_round(real_prec_min());
    mul(r1, r2, r3);
    REQUIRE(r1 == complex{-2532, 89});
    REQUIRE(r1.get_prec() == r3.get_prec());
    mul(r1, complex{12, complex_prec_t(123)}, complex{34, complex_prec_t(128)});
    REQUIRE(r1 == 408);
    REQUIRE(r1.get_prec() == 128);
    // Some tests with rvalue refs/overlapping arguments.
    mul(r1, std::move(r1), std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(r1 == 166464l);
    mul(r1, std::move(r1), complex{100, complex_prec_t(150)});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 150);
    REQUIRE(r1 == 16646400ll);
    mul(r1, std::move(r1), complex{100, complex_prec_t(50)});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 150);
    REQUIRE(r1 == 1664640000ll);
    mul(r1, complex{100, complex_prec_t(160)}, std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 160);
    REQUIRE(r1 == 166464000000ll);
    mul(r1, complex{100, complex_prec_t(50)}, std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 160);
    REQUIRE(r1 == 16646400000000ll);
    r1 = complex{92, complex_prec_t(128)};
    mul(r1, r1, std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(r1 == 8464);
    mul(r1, std::move(r1), r1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(r1 == 71639296);
    r1 = complex{};
    mul(r1, complex{10, complex_prec_t(50)}, complex{12, complex_prec_t(51)});
    REQUIRE(r1.get_prec() == 51);
    REQUIRE(r1 == 120);
    r1 = complex{};
    mul(r1, complex{10, complex_prec_t(52)}, complex{12, complex_prec_t(51)});
    REQUIRE(r1.get_prec() == 52);
    REQUIRE(r1 == 120);
    r1 = complex{0, 123};
    mul(r1, complex{10, complex_prec_t(52)}, complex{12, complex_prec_t(51)});
    REQUIRE(r1.get_prec() == 52);
    REQUIRE(r1 == 120);
}

TEST_CASE("div")
{
    complex r1, r2, r3{1};
    div(r1, r2, r3);
    REQUIRE(std::is_same<complex &, decltype(div(r1, r2, r3))>::value);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    r1 = 56;
    div(r1, r2, r3);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    r2 = complex{2, -4};
    r3 = complex{2, -4};
    r1 = complex{-4, complex_prec_t(128)};
    div(r1, r2, r3);
    REQUIRE(r1 == 1);
    REQUIRE(r1.get_prec() == r3.get_prec());
    r1.prec_round(real_prec_min());
    div(r1, r2, r3);
    REQUIRE(r1 == 1);
    REQUIRE(r1.get_prec() == r3.get_prec());
    div(r1, complex{12, complex_prec_t(123)}, complex{6, complex_prec_t(128)});
    REQUIRE(r1 == 2);
    REQUIRE(r1.get_prec() == 128);
    // Some tests with rvalue refs/overlapping arguments.
    div(r1, std::move(r1), std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(r1 == 1);
    div(r1, std::move(r1), complex{16, complex_prec_t(150)});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 150);
    REQUIRE(r1 == 1 / 16_q1);
    div(r1, std::move(r1), complex{2, complex_prec_t(50)});
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 150);
    REQUIRE(r1 == 1 / 32_q1);
    div(r1, complex{2, complex_prec_t(160)}, std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 160);
    REQUIRE(r1 == 64);
    div(r1, complex{2, complex_prec_t(50)}, std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 160);
    REQUIRE(r1 == 1 / 32_q1);
    r1 = complex{92, complex_prec_t(128)};
    div(r1, r1, std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(r1 == 1);
    div(r1, std::move(r1), r1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(r1 == 1);
    r1 = complex{};
    div(r1, complex{10, complex_prec_t(50)}, complex{20, complex_prec_t(51)});
    REQUIRE(r1.get_prec() == 51);
    REQUIRE(r1 == 1 / 2_q1);
    r1 = complex{};
    div(r1, complex{10, complex_prec_t(52)}, complex{5, complex_prec_t(51)});
    REQUIRE(r1.get_prec() == 52);
    REQUIRE(r1 == 2);
    r1 = complex{0, 123};
    div(r1, complex{6, complex_prec_t(52)}, complex{12, complex_prec_t(51)});
    REQUIRE(r1.get_prec() == 52);
    REQUIRE(r1 == 1 / 2_q1);
}

TEST_CASE("neg")
{
    {
        // Member function.
        complex c{1, 2};
        c.neg();
        REQUIRE(c == complex{-1, -2});
        REQUIRE(c.get_prec() == detail::real_deduce_precision(1));
    }
    {
        // rop overload.
        complex c1, c2{1, 2};
        const auto p = c2.get_prec();
        REQUIRE(&neg(c1, c2) == &c1);
        REQUIRE(std::is_same<decltype(neg(c1, c2)), complex &>::value);
        REQUIRE(c1 == complex{-1, -2});
        REQUIRE(c1.get_prec() == p);

        // Move, but won't steal because rop
        // has higher precision.
        c1 = complex{0, complex_prec_t(c2.get_prec() + 1)};
        neg(c1, std::move(c2));
        REQUIRE(c1 == complex{-1, -2});
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == complex{1, 2});

        // Move, will steal.
        c1 = complex{};
        neg(c1, std::move(c2));
        REQUIRE(c1 == complex{-1, -2});
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == complex{});
    }
    {
        // return overload.
        REQUIRE(neg(complex{1, 2}) == complex{-1, -2});
        REQUIRE(std::is_same<decltype(neg(complex{1, 2})), complex>::value);

        // move, will steal.
        complex c1{3, 4};
        const auto p = c1.get_prec();
        auto c2 = neg(std::move(c1));
        REQUIRE(c2 == complex{-3, -4});
        REQUIRE(c2.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }
}

TEST_CASE("conj")
{
    {
        // Member function.
        complex c{1, 2};
        c.conj();
        REQUIRE(c == complex{1, -2});
        REQUIRE(c.get_prec() == detail::real_deduce_precision(1));
    }
    {
        // rop overload.
        complex c1, c2{1, 2};
        const auto p = c2.get_prec();
        conj(c1, c2);
        REQUIRE(c1 == complex{1, -2});
        REQUIRE(c1.get_prec() == p);

        // Move, but won't steal because rop
        // has higher precision.
        c1 = complex{0, complex_prec_t(c2.get_prec() + 1)};
        REQUIRE(&conj(c1, std::move(c2)) == &c1);
        REQUIRE(std::is_same<decltype(conj(c1, c2)), complex &>::value);
        REQUIRE(c1 == complex{1, -2});
        REQUIRE(c1.get_prec() == p);
        REQUIRE(c2 == complex{1, 2});

        // Move, will steal.
        c1 = complex{};
        conj(c1, std::move(c2));
        REQUIRE(c1 == complex{1, -2});
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == complex{});
    }
    {
        // return overload.
        REQUIRE(conj(complex{1, 2}) == complex{1, -2});
        REQUIRE(std::is_same<decltype(conj(complex{1, 2})), complex>::value);

        // move, will steal.
        complex c1{3, 4};
        const auto p = c1.get_prec();
        auto c2 = conj(std::move(c1));
        REQUIRE(c2 == complex{3, -4});
        REQUIRE(c2.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }
}

TEST_CASE("abs")
{
    {
        // Member function.
        complex c{3, 4};
        c.abs();
        REQUIRE(c == complex{5, 0});
        REQUIRE(c.get_prec() == detail::real_deduce_precision(1));
    }
    {
        // Free function.
        REQUIRE(abs(complex{3, 4, complex_prec_t(112)}) == real{5});
        REQUIRE(abs(complex{3, 4, complex_prec_t(112)}).get_prec() == 112);
        REQUIRE(std::is_same<real, decltype(abs(complex{3, 4, complex_prec_t(112)}))>::value);
        real r;
        REQUIRE(&abs(r, complex{3, 4, complex_prec_t(112)}) == &r);
        REQUIRE(std::is_same<decltype(abs(r, complex{3, 4, complex_prec_t(112)})), real &>::value);
        REQUIRE(r == 5);
        REQUIRE(r.get_prec() == 112);
    }
}

TEST_CASE("norm")
{
    {
        // Member function.
        complex c{3, 4};
        c.norm();
        REQUIRE(c == complex{25, 0});
        REQUIRE(c.get_prec() == detail::real_deduce_precision(1));
    }
    {
        // Free function.
        REQUIRE(norm(complex{3, 4, complex_prec_t(112)}) == real{25});
        REQUIRE(norm(complex{3, 4, complex_prec_t(112)}).get_prec() == 112);
        REQUIRE(std::is_same<real, decltype(norm(complex{3, 4, complex_prec_t(112)}))>::value);
        real r;
        REQUIRE(&norm(r, complex{3, 4, complex_prec_t(112)}) == &r);
        REQUIRE(std::is_same<decltype(norm(r, complex{3, 4, complex_prec_t(112)})), real &>::value);
        REQUIRE(r == 25);
        REQUIRE(r.get_prec() == 112);
    }
}

TEST_CASE("arg")
{
    {
        // Member function.
        complex c{1, 1};
        c.arg();
        REQUIRE(c == real_pi(detail::real_deduce_precision(1)) / 4);
        REQUIRE(c.get_prec() == detail::real_deduce_precision(1));
    }
    {
        // Free function.
        REQUIRE(arg(complex{1, 1, complex_prec_t(112)}) == real_pi(112) / 4);
        REQUIRE(arg(complex{1, 1, complex_prec_t(112)}).get_prec() == 112);
        REQUIRE(std::is_same<real, decltype(arg(complex{3, 4, complex_prec_t(112)}))>::value);
        real r;
        REQUIRE(&arg(r, complex{1, 1, complex_prec_t(112)}) == &r);
        REQUIRE(std::is_same<decltype(arg(r, complex{3, 4, complex_prec_t(112)})), real &>::value);
        REQUIRE(r == real_pi(112) / 4);
        REQUIRE(r.get_prec() == 112);
    }
}

TEST_CASE("proj")
{
    {
        // Member function.
        complex c{42, -43};
        c.proj();
        REQUIRE(c == complex{42, -43});
        REQUIRE(c.get_prec() == detail::real_deduce_precision(1));

        c = complex{"(inf, 123)", complex_prec_t(42)};
        c.proj();
        REQUIRE(c == complex{"(inf, 0)", complex_prec_t(42)});
        REQUIRE(c.get_prec() == 42);
        {
            complex::im_cref im{c};

            REQUIRE(!im->signbit());
        }

        c = complex{"(inf, -123)", complex_prec_t(42)};
        c.proj();
        REQUIRE(c == complex{"(inf, 0)", complex_prec_t(42)});
        REQUIRE(c.get_prec() == 42);
        {
            complex::im_cref im{c};

            REQUIRE(im->signbit());
        }
    }
    {
        // rop overload.
        complex c1, c2{1, 2};
        const auto p = c2.get_prec();
        REQUIRE(&proj(c1, c2) == &c1);
        REQUIRE(std::is_same<decltype(proj(c1, c2)), complex &>::value);
        REQUIRE(c1 == complex{1, 2});
        REQUIRE(c1.get_prec() == p);

        // Move, but won't steal because rop
        // has higher precision.
        c1 = complex{0, complex_prec_t(c2.get_prec() + 1)};
        proj(c1, std::move(c2));
        REQUIRE(c1 == complex{1, 2});
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == complex{1, 2});

        // Move, will steal.
        c1 = complex{};
        proj(c1, std::move(c2));
        REQUIRE(c1 == complex{1, 2});
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == complex{});
    }
    {
        // return overload.
        REQUIRE(proj(complex{1, 2}) == complex{1, 2});
        REQUIRE(std::is_same<decltype(proj(complex{1, 2})), complex>::value);

        // move, will steal.
        complex c1{3, 4};
        const auto p = c1.get_prec();
        auto c2 = proj(std::move(c1));
        REQUIRE(c2 == complex{3, 4});
        REQUIRE(c2.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }
}

TEST_CASE("sqr")
{
    {
        // Member function.
        complex c{1, 2};
        c.sqr();
        REQUIRE(c == complex{-3, 4});
        REQUIRE(c.get_prec() == detail::real_deduce_precision(1));
    }
    {
        // rop overload.
        complex c1, c2{1, 2};
        const auto p = c2.get_prec();
        REQUIRE(&sqr(c1, c2) == &c1);
        REQUIRE(std::is_same<decltype(sqr(c1, c2)), complex &>::value);
        REQUIRE(c1 == complex{-3, 4});
        REQUIRE(c1.get_prec() == p);

        // Move, but won't steal because rop
        // has higher precision.
        c1 = complex{0, complex_prec_t(c2.get_prec() + 1)};
        sqr(c1, std::move(c2));
        REQUIRE(c1 == complex{-3, 4});
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == complex{1, 2});

        // Move, will steal.
        c1 = complex{};
        sqr(c1, std::move(c2));
        REQUIRE(c1 == complex{-3, 4});
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == complex{});
    }
    {
        // return overload.
        REQUIRE(sqr(complex{1, 2}) == complex{-3, 4});
        REQUIRE(std::is_same<decltype(sqr(complex{1, 2})), complex>::value);

        // move, will steal.
        complex c1{3, 4};
        const auto p = c1.get_prec();
        auto c2 = sqr(std::move(c1));
        REQUIRE(c2 == complex{-7, 24});
        REQUIRE(c2.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }
}

TEST_CASE("mul_i")
{
    {
        // Member function.
        complex c{1, 2};
        c.mul_i();
        REQUIRE(c == complex{-2, 1});
        REQUIRE(c.get_prec() == detail::real_deduce_precision(1));
        c.mul_i(-1);
        REQUIRE(c == complex{1, 2});
        REQUIRE(c.get_prec() == detail::real_deduce_precision(1));
    }
    {
        // rop overload.
        complex c1, c2{1, 2};
        const auto p = c2.get_prec();
        REQUIRE(&mul_i(c1, c2) == &c1);
        REQUIRE(std::is_same<decltype(mul_i(c1, c2)), complex &>::value);
        REQUIRE(c1 == complex{-2, 1});
        REQUIRE(c1.get_prec() == p);
        mul_i(c1, c2, -1);
        REQUIRE(c1 == complex{2, -1});

        // Move, but won't steal because rop
        // has higher precision.
        c1 = complex{0, complex_prec_t(c2.get_prec() + 1)};
        mul_i(c1, std::move(c2));
        REQUIRE(c1 == complex{-2, 1});
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == complex{1, 2});
        mul_i(c1, std::move(c2), -1);
        REQUIRE(c1 == complex{2, -1});
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == complex{1, 2});

        // Move, will steal.
        c1 = complex{};
        mul_i(c1, std::move(c2));
        REQUIRE(c1 == complex{-2, 1});
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == complex{});
        c2 = complex{1, 2};
        c1 = complex{};
        mul_i(c1, std::move(c2), -1);
        REQUIRE(c1 == complex{2, -1});
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == complex{});
    }
    {
        // return overload.
        REQUIRE(mul_i(complex{1, 2}) == complex{-2, 1});
        REQUIRE(std::is_same<decltype(mul_i(complex{1, 2})), complex>::value);
        REQUIRE(mul_i(complex{1, 2}, -2) == complex{2, -1});

        // move, will steal.
        complex c1{3, 4};
        const auto p = c1.get_prec();
        auto c2 = mul_i(std::move(c1));
        REQUIRE(c2 == complex{-4, 3});
        REQUIRE(c2.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
        c1 = complex{3, 4};
        c2 = mul_i(std::move(c1), -3);
        REQUIRE(c2 == complex{4, -3});
        REQUIRE(c2.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }
}

TEST_CASE("fma")
{
    complex r1, r2, r3, r4;
    REQUIRE(std::is_same<complex &, decltype(mppp::fma(r1, r2, r3, r4))>::value);
    fma_wrap(r1, r2, r3, r4);
    REQUIRE(r1.zero_p());
    REQUIRE(r1.get_prec() == r3.get_prec());
    fma_wrap(r1, complex{2, complex_prec_t(12)}, complex{3, complex_prec_t(7)}, complex{14, complex_prec_t(128)});
    REQUIRE(r1 == 20);
    REQUIRE(r1.get_prec() == 128);
    r1 = 0;
    fma_wrap(r1, complex{3, complex_prec_t(7)}, complex{2, complex_prec_t(12)}, complex{14, complex_prec_t(128)});
    REQUIRE(r1 == 20);
    REQUIRE(r1.get_prec() == 128);
    r1 = 0;
    fma_wrap(r1, complex{14, complex_prec_t(128)}, complex{3, complex_prec_t(7)}, complex{2, complex_prec_t(12)});
    REQUIRE(r1 == 44);
    REQUIRE(r1.get_prec() == 128);
    REQUIRE(std::is_same<complex, decltype(mppp::fma(r1, r2, r3))>::value);
    r1 = fma_wrap(complex{14, complex_prec_t(128)}, complex{3, complex_prec_t(7)}, complex{2, complex_prec_t(12)});
    REQUIRE(r1 == 44);
    REQUIRE(r1.get_prec() == 128);
    r1 = fma_wrap(static_cast<const complex &>(complex{14, complex_prec_t(128)}), complex{3, complex_prec_t(7)},
                  complex{2, complex_prec_t(12)});
    REQUIRE(r1 == 44);
    REQUIRE(r1.get_prec() == 128);
    r1 = fma_wrap(complex{14, complex_prec_t(128)}, static_cast<const complex &>(complex{3, complex_prec_t(7)}),
                  complex{2, complex_prec_t(12)});
    REQUIRE(r1 == 44);
    REQUIRE(r1.get_prec() == 128);
    r1 = fma_wrap(complex{14, complex_prec_t(128)}, complex{3, complex_prec_t(7)},
                  static_cast<const complex &>(complex{2, complex_prec_t(12)}));
    REQUIRE(r1 == 44);
    REQUIRE(r1.get_prec() == 128);
    r1 = fma_wrap(complex{14, complex_prec_t(128)}, static_cast<const complex &>(complex{3, complex_prec_t(7)}),
                  static_cast<const complex &>(complex{2, complex_prec_t(12)}));
    REQUIRE(r1 == 44);
    REQUIRE(r1.get_prec() == 128);
    r1 = fma_wrap(static_cast<const complex &>(complex{14, complex_prec_t(128)}), complex{3, complex_prec_t(7)},
                  static_cast<const complex &>(complex{2, complex_prec_t(12)}));
    REQUIRE(r1 == 44);
    REQUIRE(r1.get_prec() == 128);
    r1 = fma_wrap(static_cast<const complex &>(complex{14, complex_prec_t(128)}),
                  static_cast<const complex &>(complex{3, complex_prec_t(7)}),
                  static_cast<const complex &>(complex{2, complex_prec_t(12)}));
    REQUIRE(r1 == 44);
    REQUIRE(r1.get_prec() == 128);
    // Overlap + rvalue.
    r1 = 0;
    fma_wrap(r1, std::move(r1), std::move(r1), std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.zero_p());
    fma_wrap(r1, r1, std::move(r1), std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.zero_p());
    fma_wrap(r1, r1, r1, std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.zero_p());
    fma_wrap(r1, std::move(r1), r1, std::move(r1));
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.zero_p());
    fma_wrap(r1, std::move(r1), std::move(r1), r1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(r1.zero_p());
}

#if defined(MPPP_WITH_ARB)

TEST_CASE("inv")
{
    const auto cmp1
        = 0.16923076923076923076923076923076923076975_r128 + 0.3538461538461538461538461538461538461538_icr128;
    {
        // Member function.
        auto c = 1.1_r128 - 2.3_icr128;
        c.inv();
        REQUIRE(abs(c - cmp1) < pow(2_r128, -126));
        REQUIRE(c.get_prec() == 128);
    }
    {
        // rop overload.
        complex c1, c2 = 1.1_r128 - 2.3_icr128;
        const auto p = c2.get_prec();
        REQUIRE(&inv(c1, c2) == &c1);
        REQUIRE(std::is_same<decltype(inv(c1, c2)), complex &>::value);
        REQUIRE(abs(c1 - cmp1) < pow(2_r128, -126));
        REQUIRE(c1.get_prec() == p);

        // Move, but won't steal because rop
        // has higher precision.
        c1 = complex{0, complex_prec_t(c2.get_prec() + 1)};
        inv(c1, std::move(c2));
        REQUIRE(abs(c1 - cmp1) < pow(2_r128, -126));
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == 1.1_r128 - 2.3_icr128);

        // Move, will steal.
        c1 = complex{};
        inv(c1, std::move(c2));
        REQUIRE(abs(c1 - cmp1) < pow(2_r128, -126));
        REQUIRE(c1.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2 == complex{});
    }
    {
        // return overload.
        REQUIRE(abs(inv(1.1_r128 - 2.3_icr128) - cmp1) < pow(2_r128, -126));
        REQUIRE(std::is_same<decltype(inv(complex{1, 2})), complex>::value);

        // move, will steal.
        complex c1 = 1.1_r128 - 2.3_icr128;
        const auto p = c1.get_prec();
        auto c2 = inv(std::move(c1));
        REQUIRE(abs(c2 - cmp1) < pow(2_r128, -126));
        REQUIRE(c2.get_prec() == p);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
    }

    // Test the special cases.
    {
        complex c{0, complex_prec_t(128)};
        REQUIRE(c.inv().inf_p());
        REQUIRE(c.get_prec() == 128);
        REQUIRE(inv(complex{0, complex_prec_t(128)}).inf_p());
    }
    {
        complex c{"(inf, 0)", complex_prec_t(128)};
        REQUIRE(c.inv().zero_p());
        REQUIRE(c.get_prec() == 128);
        REQUIRE(inv(complex{"(inf, 0)", complex_prec_t(128)}).zero_p());
    }
    {
        complex c{"(inf, nan)", complex_prec_t(128)};
        REQUIRE(c.inv().zero_p());
        REQUIRE(c.get_prec() == 128);
        REQUIRE(inv(complex{"(inf, nan)", complex_prec_t(128)}).zero_p());
    }
    {
        complex c{"(0, inf)", complex_prec_t(128)};
        REQUIRE(c.inv().zero_p());
        REQUIRE(c.get_prec() == 128);
        REQUIRE(inv(complex{"(0, inf)", complex_prec_t(128)}).zero_p());
    }
    {
        complex c{"(nan, inf)", complex_prec_t(128)};
        REQUIRE(c.inv().zero_p());
        REQUIRE(c.get_prec() == 128);
        REQUIRE(inv(complex{"(nan, inf)", complex_prec_t(128)}).zero_p());
    }
    {
        complex c{"(inf, inf)", complex_prec_t(128)};
        REQUIRE(c.inv().zero_p());
        REQUIRE(c.get_prec() == 128);
        REQUIRE(inv(complex{"(inf, inf)", complex_prec_t(128)}).zero_p());
    }
}

#endif
