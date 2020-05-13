// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
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

using namespace mppp;

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
        REQUIRE(c2 == complex{1, 2});

        // Move, will steal.
        c1 = complex{};
        neg(c1, std::move(c2));
        REQUIRE(c1 == complex{-1, -2});
        REQUIRE(c1.get_prec() == p);
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
        REQUIRE(c2 == complex{1, 2});

        // Move, will steal.
        c1 = complex{};
        proj(c1, std::move(c2));
        REQUIRE(c1 == complex{1, 2});
        REQUIRE(c1.get_prec() == p);
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
        REQUIRE(!c1.is_valid());
    }
}
