// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <complex>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#if defined(MPPP_HAVE_STRING_VIEW)
#include <string_view>
#endif

#include <mp++/complex.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real.hpp>

#if defined(MPPP_WITH_QUADMATH)

#include <mp++/real128.hpp>

#endif

#include "catch.hpp"

using namespace mppp;

TEST_CASE("basic and generic constructors")
{
    using Catch::Matchers::Message;

    // Def ctor.
    {
        complex c;

        complex::const_re_extractor re{c};
        complex::const_im_extractor im{c};

        REQUIRE(re->zero_p());
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == real_prec_min());
        REQUIRE(im->get_prec() == real_prec_min());
        REQUIRE(!re->signbit());
        REQUIRE(!im->signbit());
    }

    // Generic ctor
    {
        complex c1{42};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 42);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == detail::real_deduce_precision(42));
        REQUIRE(im->get_prec() == detail::real_deduce_precision(42));
    }
    {
        complex c1{123.};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 123);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == detail::real_deduce_precision(123.));
        REQUIRE(im->get_prec() == detail::real_deduce_precision(123.));
    }
    {
        complex c1{-42_z1};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == -42);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == detail::real_deduce_precision(-42_z1));
        REQUIRE(im->get_prec() == detail::real_deduce_precision(-42_z1));
    }
    {
        complex c1{73_q1 / 2};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 73_q1 / 2);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == detail::real_deduce_precision(73_q1 / 2));
        REQUIRE(im->get_prec() == detail::real_deduce_precision(73_q1 / 2));
    }
    {
        complex c1{1.1_r512};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 1.1_r512);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 512);
        REQUIRE(im->get_prec() == 512);
    }
    {
        // Try moving in a real.
        auto r = 1.1_r512;

        complex c1{std::move(r)};
        REQUIRE(!r.is_valid());

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 1.1_r512);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 512);
        REQUIRE(im->get_prec() == 512);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1{-3.1_rq};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == -3.1_rq);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 113);
        REQUIRE(im->get_prec() == 113);
    }
#endif
    {
        complex c1{std::complex<double>{-4, 7}};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == -4);
        REQUIRE((*im) == 7);
        REQUIRE(re->get_prec() == detail::real_deduce_precision(-4.));
        REQUIRE(im->get_prec() == detail::real_deduce_precision(7.));
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1{-3.1_rq + 2.1_icq};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == -3.1_rq);
        REQUIRE((*im) == 2.1_rq);
        REQUIRE(re->get_prec() == 113);
        REQUIRE(im->get_prec() == 113);
    }
#endif

    // Copy ctor.
    {
        complex c1{std::complex<double>{-4, 7}}, c2 = c1;

        complex::const_re_extractor re{c2};
        complex::const_im_extractor im{c2};

        REQUIRE((*re) == -4);
        REQUIRE((*im) == 7);
        REQUIRE(re->get_prec() == detail::real_deduce_precision(-4.));
        REQUIRE(im->get_prec() == detail::real_deduce_precision(7.));
    }

    // Move ctor.
    {
        complex c1{std::complex<double>{-4, 7}}, c2{std::move(c1)};

        REQUIRE(!c1.is_valid());

        complex::const_re_extractor re{c2};
        complex::const_im_extractor im{c2};

        REQUIRE((*re) == -4);
        REQUIRE((*im) == 7);
        REQUIRE(re->get_prec() == detail::real_deduce_precision(-4.));
        REQUIRE(im->get_prec() == detail::real_deduce_precision(7.));
    }

    // Generic ctor with custom precision.
    {
        complex c1{42, complex_prec_t(123)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 42);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 123);
        REQUIRE(im->get_prec() == 123);
    }
    {
        complex c1{42.l, complex_prec_t(10)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 42);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 10);
        REQUIRE(im->get_prec() == 10);
    }
    {
        complex c1{-42_z1, complex_prec_t(768)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == -42);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 768);
        REQUIRE(im->get_prec() == 768);
    }
    {
        complex c1{73_q1 / 2, complex_prec_t(1768)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 73_q1 / 2);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 1768);
        REQUIRE(im->get_prec() == 1768);
    }
    {
        complex c1{1.1_r512, complex_prec_t(128)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 1.1_r128);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 128);
        REQUIRE(im->get_prec() == 128);
    }
    {
        // Try moving in a real.
        auto r = 1.1_r512;

        complex c1{std::move(r), complex_prec_t(1024)};
        REQUIRE(!r.is_valid());

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 1.1_r512);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 1024);
        REQUIRE(im->get_prec() == 1024);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1{-3.1_rq, complex_prec_t(1024)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == -3.1_rq);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 1024);
        REQUIRE(im->get_prec() == 1024);
    }
#endif
    {
        complex c1{std::complex<double>{-4, 7}, complex_prec_t(10)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == -4);
        REQUIRE((*im) == 7);
        REQUIRE(re->get_prec() == 10);
        REQUIRE(im->get_prec() == 10);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1{-3.1_rq + 2.1_icq, complex_prec_t(512)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == -3.1_rq);
        REQUIRE((*im) == 2.1_rq);
        REQUIRE(re->get_prec() == 512);
        REQUIRE(im->get_prec() == 512);
    }
#endif
    // Bad prec value.
    {
        REQUIRE_THROWS_MATCHES((complex{42.l, complex_prec_t(-1)}), std::invalid_argument,
                               Message("Cannot init a real with a precision of -1: the maximum allowed precision is "
                                       + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                       + detail::to_string(real_prec_min())));
        REQUIRE_THROWS_MATCHES((complex{std::complex<float>{1, 2}, complex_prec_t(-2)}), std::invalid_argument,
                               Message("Cannot init a real with a precision of -2: the maximum allowed precision is "
                                       + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                       + detail::to_string(real_prec_min())));
    }

    // Copy ctor with custom precision.
    {
        complex c{1.1_r512}, c1{c, 256};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 1.1_r256);
        REQUIRE((*re) != 1.1_r512);
        REQUIRE((*im) == 0);
        REQUIRE(re->get_prec() == 256);
        REQUIRE(im->get_prec() == 256);

        // Error checking.
        REQUIRE_THROWS_MATCHES((complex{c1, -1}), std::invalid_argument,
                               Message("Cannot init a complex with a precision of -1: the maximum allowed precision is "
                                       + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                       + detail::to_string(real_prec_min())));
        REQUIRE_THROWS_MATCHES((complex{c1, 0}), std::invalid_argument,
                               Message("Cannot init a complex with a precision of 0: the maximum allowed precision is "
                                       + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                       + detail::to_string(real_prec_min())));
    }

    // Move ctor with custom precision.
    {
        complex c{1.1_r512}, c1{std::move(c), 256};
        REQUIRE(!c.is_valid());

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 1.1_r256);
        REQUIRE((*re) != 1.1_r512);
        REQUIRE((*im) == 0);
        REQUIRE(re->get_prec() == 256);
        REQUIRE(im->get_prec() == 256);

        REQUIRE_THROWS_MATCHES((complex{std::move(c1), -1}), std::invalid_argument,
                               Message("Cannot init a complex with a precision of -1: the maximum allowed precision is "
                                       + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                       + detail::to_string(real_prec_min())));
        REQUIRE_THROWS_MATCHES((complex{std::move(c1), 0}), std::invalid_argument,
                               Message("Cannot init a complex with a precision of 0: the maximum allowed precision is "
                                       + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                       + detail::to_string(real_prec_min())));
    }

    // Binary ctors.
    {
        complex c1{45, -67.};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 45);
        REQUIRE(re->get_prec()
                == detail::c_max(detail::real_deduce_precision(45), detail::real_deduce_precision(-67.)));
        REQUIRE((*im) == -67);
        REQUIRE(im->get_prec()
                == detail::c_max(detail::real_deduce_precision(45), detail::real_deduce_precision(-67.)));
    }
    {
        complex c1{45_z1, -67 / 123_q1};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 45);
        REQUIRE(re->get_prec()
                == detail::c_max(detail::real_deduce_precision(45_z1), detail::real_deduce_precision(-67 / 123_q1)));
        REQUIRE((*im) == real{-67 / 123_q1});
        REQUIRE(im->get_prec()
                == detail::c_max(detail::real_deduce_precision(45_z1), detail::real_deduce_precision(-67 / 123_q1)));
    }
    {
        auto r = 1.23_r512;
        auto i = 4.56_r256;
        complex c1{r, i};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 1.23_r512);
        REQUIRE(re->get_prec() == 512);
        REQUIRE((*im) == 4.56_r256);
        REQUIRE(im->get_prec() == 512);
    }
    {
        auto r = 1.23_r512;
        auto i = 4.56_r256;
        complex c1{std::move(r), std::move(i)};

        REQUIRE(!r.is_valid());
        REQUIRE(!i.is_valid());

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 1.23_r512);
        REQUIRE(re->get_prec() == 512);
        REQUIRE((*im) == 4.56_r256);
        REQUIRE(im->get_prec() == 512);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1{45_rq, 12_rq};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 45);
        REQUIRE(re->get_prec() == 113);
        REQUIRE((*im) == 12);
        REQUIRE(im->get_prec() == 113);
    }
#endif

    // Binary ctors with custom precision.
    {
        complex c1{45, -67., 36};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 45);
        REQUIRE(re->get_prec() == 36);
        REQUIRE((*im) == -67);
        REQUIRE(im->get_prec() == 36);
    }
    {
        complex c1{45_z1, -67 / 123_q1, 87};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 45);
        REQUIRE(re->get_prec() == 87);
        REQUIRE((*im) == real{-67 / 123_q1, 87});
        REQUIRE(im->get_prec() == 87);
    }
    {
        auto r = 1.23_r512;
        auto i = 4.56_r256;
        complex c1{r, i, 128};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 1.23_r128);
        REQUIRE(re->get_prec() == 128);
        REQUIRE((*im) == 4.56_r128);
        REQUIRE(im->get_prec() == 128);
    }
    {
        auto r = 1.23_r512;
        auto i = 4.56_r256;
        complex c1{std::move(r), std::move(i), 128};

        REQUIRE(!r.is_valid());
        REQUIRE(!i.is_valid());

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 1.23_r128);
        REQUIRE(re->get_prec() == 128);
        REQUIRE((*im) == 4.56_r128);
        REQUIRE(im->get_prec() == 128);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1{45_rq, 12_rq, 28};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE((*re) == 45);
        REQUIRE(re->get_prec() == 28);
        REQUIRE((*im) == 12);
        REQUIRE(im->get_prec() == 28);
    }
#endif
    // Bad prec value.
    {
        REQUIRE_THROWS_MATCHES((complex{42, 43, -1}), std::invalid_argument,
                               Message("Cannot init a real with a precision of -1: the maximum allowed precision is "
                                       + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                       + detail::to_string(real_prec_min())));
        REQUIRE_THROWS_MATCHES((complex{1_q1, 1.23_r512, -2}), std::invalid_argument,
                               Message("Cannot init a real with a precision of -2: the maximum allowed precision is "
                                       + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                       + detail::to_string(real_prec_min())));
    }
}

TEST_CASE("string constructors")
{
    using Catch::Matchers::Message;

    std::vector<char> buffer;

    // Start with zeroes.
    {
        complex c{"0", 10, 128};
        REQUIRE(c == 0);
        REQUIRE(c.get_prec() == 128);

        complex::const_re_extractor re{c};
        complex::const_im_extractor im{c};

        REQUIRE(!re->signbit());
        REQUIRE(!(*im).signbit());
    }
    {
        complex c{std::string("(-0)"), 10, 128};
        REQUIRE(c == 0);
        REQUIRE(c.get_prec() == 128);

        complex::const_re_extractor re{c};
        complex::const_im_extractor im{c};

        REQUIRE(re->signbit());
        REQUIRE(!(*im).signbit());
    }
    {
        complex c{"(0,0)", 10, 128};
        REQUIRE(c == 0);
        REQUIRE(c.get_prec() == 128);

        complex::const_re_extractor re{c};
        complex::const_im_extractor im{c};

        REQUIRE(!re->signbit());
        REQUIRE(!(*im).signbit());
    }

    // Single value, no brackets.
    {
        complex c{"1.1", 10, 128};
        REQUIRE(c == 1.1_r128);
        REQUIRE(c.get_prec() == 128);

        complex::const_im_extractor im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{"  1.1", 10, 128};
        REQUIRE(c == 1.1_r128);
        REQUIRE(c.get_prec() == 128);

        complex::const_im_extractor im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{"  +1.1", 128};
        REQUIRE(c == 1.1_r128);
        REQUIRE(c.get_prec() == 128);

        complex::const_im_extractor im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        buffer = {' ', '+', '1', '.', '1'};

        complex c{buffer.data(), buffer.data() + buffer.size(), 10, 128};

        REQUIRE(c == 1.1_r128);
        REQUIRE(c.get_prec() == 128);

        complex::const_im_extractor im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        buffer = {' ', '+', '1', '.', '3'};

        complex c{buffer.data(), buffer.data() + buffer.size(), 128};

        REQUIRE(c == 1.3_r128);
        REQUIRE(c.get_prec() == 128);

        complex::const_im_extractor im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{std::string("  -0x2f2.1aa4p0"), 16, 128};
        REQUIRE(c == -0x2f2.1aa4p0_r128);
        REQUIRE(c.get_prec() == 128);

        complex::const_im_extractor im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{std::string("  -0x2f2.1aa4p0"), 0, 128};
        REQUIRE(c == -0x2f2.1aa4p0_r128);
        REQUIRE(c.get_prec() == 128);

        complex::const_im_extractor im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    REQUIRE_THROWS_MATCHES((complex{"1.1 ", 10, 128}), std::invalid_argument,
                           Message("The string '1.1 ' does not represent a valid real in base 10"));
    REQUIRE_THROWS_MATCHES((complex{"hello world", 12, 128}), std::invalid_argument,
                           Message("The string 'hello world' does not represent a valid real in base 12"));
    REQUIRE_THROWS_MATCHES((complex{"1.1 ", -2, 128}), std::invalid_argument,
                           Message("Cannot construct a complex from a string in base -2: the base must either be zero "
                                   "or in the [2,62] range"));

    // Single value, brackets.
    {
        complex c{"(1.1)", 10, 128};
        REQUIRE(c == 1.1_r128);
        REQUIRE(c.get_prec() == 128);

        complex::const_im_extractor im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{" (1.1)", 10, 128};
        REQUIRE(c == 1.1_r128);
        REQUIRE(c.get_prec() == 128);

        complex::const_im_extractor im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{std::string(" ( -0x2f2.1aa4p0)"), 16, 128};
        REQUIRE(c == -0x2f2.1aa4p0_r128);
        REQUIRE(c.get_prec() == 128);

        complex::const_im_extractor im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{std::string(" ( -0x2f2.1aa4p0)"), 0, 128};
        REQUIRE(c == -0x2f2.1aa4p0_r128);
        REQUIRE(c.get_prec() == 128);

        complex::const_im_extractor im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{" ( 1.1)", 128};
        REQUIRE(c == 1.1_r128);
        REQUIRE(c.get_prec() == 128);

        complex::const_im_extractor im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        buffer = {' ', '(', '+', '1', '.', '1', ')'};

        complex c{buffer.data(), buffer.data() + buffer.size(), 10, 128};

        REQUIRE(c == 1.1_r128);
        REQUIRE(c.get_prec() == 128);

        complex::const_im_extractor im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        buffer = {' ', '(', '+', '1', '.', '3', ')'};

        complex c{buffer.data(), buffer.data() + buffer.size(), 128};

        REQUIRE(c == 1.3_r128);
        REQUIRE(c.get_prec() == 128);

        complex::const_im_extractor im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    REQUIRE_THROWS_MATCHES((complex{" ( 1.1 )", 10, 128}), std::invalid_argument,
                           Message("The string ' 1.1 ' does not represent a valid real in base 10"));
    REQUIRE_THROWS_MATCHES((complex{"(hello world)", 12, 128}), std::invalid_argument,
                           Message("The string 'hello world' does not represent a valid real in base 12"));
    REQUIRE_THROWS_MATCHES((complex{"(1.1)", -20, 128}), std::invalid_argument,
                           Message("Cannot construct a complex from a string in base -20: the base must either be zero "
                                   "or in the [2,62] range"));

    // Two values.
    {
        complex c{"(-1.1,-2.3)", 10, 256};
        REQUIRE(c == complex{-1.1_r256, -2.3_r256});
        REQUIRE(c.get_prec() == 256);
    }
#if defined(MPPP_HAVE_STRING_VIEW)
    {
        complex c{std::string_view("(-1.1,-2.3)"), 10, 256};
        REQUIRE(c == complex{-1.1_r256, -2.3_r256});
        REQUIRE(c.get_prec() == 256);
    }
#endif
    {
        complex c{" (-1.1,-2.3)", 0, 256};
        REQUIRE(c == complex{-1.1_r256, -2.3_r256});
        REQUIRE(c.get_prec() == 256);
    }
    {
        complex c{std::string(" ( -1.1, -2.3)"), 0, 256};
        REQUIRE(c == complex{-1.1_r256, -2.3_r256});
        REQUIRE(c.get_prec() == 256);
    }
    {
        buffer = {' ', '(', '-', '1', '.', '3', ',', '0', '.', '7', ')'};

        complex c{buffer.data(), buffer.data() + buffer.size(), 128};

        REQUIRE(c == complex{-1.3_r128, 0.7_r128});
        REQUIRE(c.get_prec() == 128);
    }
    {
        buffer = {' ', '(', '-', '1', '.', '3', ',', '0', '.', '7', ')'};

        complex c{buffer.data(), buffer.data() + buffer.size(), 10, 128};

        REQUIRE(c == complex{-1.3_r128, 0.7_r128});
        REQUIRE(c.get_prec() == 128);
    }
    {
        complex c{"(   -0x2f2.1aa4p0, 0x123.aaap4)", 16, 128};
        REQUIRE(c == complex{-0x2f2.1aa4p0_r128, 0x123.aaap4_r128});
        REQUIRE(c.get_prec() == 128);
    }
    {
        complex c{"(   -0x2f2.1aa4p0, 0x123.aaap4)", 0, 128};
        REQUIRE(c == complex{-0x2f2.1aa4p0_r128, 0x123.aaap4_r128});
        REQUIRE(c.get_prec() == 128);
    }
    REQUIRE_THROWS_MATCHES((complex{" (hello, 2)", 10, 128}), std::invalid_argument,
                           Message("The string 'hello' does not represent a valid real in base 10"));
    REQUIRE_THROWS_MATCHES((complex{"(2, world )", 12, 128}), std::invalid_argument,
                           Message("The string ' world ' does not represent a valid real in base 12"));
}
