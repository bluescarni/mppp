// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <cmath>
#include <complex>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(MPPP_HAVE_STRING_VIEW)
#include <string_view>
#endif

#if defined(MPPP_WITH_BOOST_S11N)

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#endif

#include <mp++/complex.hpp>
#include <mp++/detail/mpc.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real.hpp>
#include <mp++/type_name.hpp>

#if defined(MPPP_WITH_QUADMATH)

#include <mp++/complex128.hpp>
#include <mp++/real128.hpp>

#endif

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("basic and generic constructors")
{
    using Catch::Matchers::Message;

    // Def ctor.
    {
        complex c;

        complex::re_cref re{c};
        complex::im_cref im{c};

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

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 42);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == detail::real_deduce_precision(42));
        REQUIRE(im->get_prec() == detail::real_deduce_precision(42));
    }
    {
        complex c1{123.};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 123);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == detail::real_deduce_precision(123.));
        REQUIRE(im->get_prec() == detail::real_deduce_precision(123.));
    }
    {
        complex c1{-42_z1};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == -42);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == detail::real_deduce_precision(-42_z1));
        REQUIRE(im->get_prec() == detail::real_deduce_precision(-42_z1));
    }
    {
        complex c1{73_q1 / 2};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 73_q1 / 2);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == detail::real_deduce_precision(73_q1 / 2));
        REQUIRE(im->get_prec() == detail::real_deduce_precision(73_q1 / 2));
    }
    {
        complex c1{1.1_r512};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 1.1_r512);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 512);
        REQUIRE(im->get_prec() == 512);
    }
    {
        // Try moving in a real.
        auto r = 1.1_r512;

        complex c1{std::move(r)};
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 1.1_r512);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 512);
        REQUIRE(im->get_prec() == 512);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1{-3.1_rq};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == -3.1_rq);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 113);
        REQUIRE(im->get_prec() == 113);
    }
#endif
    {
        complex c1{std::complex<double>{-4, 7}};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == -4);
        REQUIRE((*im) == 7);
        REQUIRE(re->get_prec() == detail::real_deduce_precision(-4.));
        REQUIRE(im->get_prec() == detail::real_deduce_precision(7.));
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1{-3.1_rq + 2.1_icq};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == -3.1_rq);
        REQUIRE((*im) == 2.1_rq);
        REQUIRE(re->get_prec() == 113);
        REQUIRE(im->get_prec() == 113);
    }
#endif

    // Copy ctor.
    {
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        complex c1{std::complex<double>{-4, 7}}, c2 = c1;

        complex::re_cref re{c2};
        complex::im_cref im{c2};

        REQUIRE((*re) == -4);
        REQUIRE((*im) == 7);
        REQUIRE(re->get_prec() == detail::real_deduce_precision(-4.));
        REQUIRE(im->get_prec() == detail::real_deduce_precision(7.));
    }

    // Move ctor.
    {
        complex c1{std::complex<double>{-4, 7}}, c2{std::move(c1)};

        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());

        complex::re_cref re{c2};
        complex::im_cref im{c2};

        REQUIRE((*re) == -4);
        REQUIRE((*im) == 7);
        REQUIRE(re->get_prec() == detail::real_deduce_precision(-4.));
        REQUIRE(im->get_prec() == detail::real_deduce_precision(7.));
    }

    // Generic ctor with custom precision.
    {
        complex c1{42, complex_prec_t(123)};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 42);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 123);
        REQUIRE(im->get_prec() == 123);
    }
    {
        complex c1{42.l, complex_prec_t(10)};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 42);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 10);
        REQUIRE(im->get_prec() == 10);
    }
    {
        complex c1{-42_z1, complex_prec_t(768)};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == -42);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 768);
        REQUIRE(im->get_prec() == 768);
    }
    {
        complex c1{73_q1 / 2, complex_prec_t(1768)};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 73_q1 / 2);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 1768);
        REQUIRE(im->get_prec() == 1768);
    }
    {
        complex c1{1.1_r512, complex_prec_t(128)};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 1.1_r128);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 128);
        REQUIRE(im->get_prec() == 128);
    }
    {
        // Try moving in a real.
        auto r = 1.1_r512;

        complex c1{std::move(r), complex_prec_t(1024)};
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 1.1_r512);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 1024);
        REQUIRE(im->get_prec() == 1024);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1{-3.1_rq, complex_prec_t(1024)};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == -3.1_rq);
        REQUIRE(im->zero_p());
        REQUIRE(re->get_prec() == 1024);
        REQUIRE(im->get_prec() == 1024);
    }
#endif
    {
        complex c1{std::complex<double>{-4, 7}, complex_prec_t(10)};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == -4);
        REQUIRE((*im) == 7);
        REQUIRE(re->get_prec() == 10);
        REQUIRE(im->get_prec() == 10);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1{-3.1_rq + 2.1_icq, complex_prec_t(512)};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

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
        complex c{1.1_r512}, c1{c, complex_prec_t(256)};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 1.1_r256);
        REQUIRE((*re) != 1.1_r512);
        REQUIRE((*im) == 0);
        REQUIRE(re->get_prec() == 256);
        REQUIRE(im->get_prec() == 256);

        // Error checking.
        REQUIRE_THROWS_MATCHES((complex{c1, complex_prec_t(-1)}), std::invalid_argument,
                               Message("Cannot init a complex with a precision of -1: the maximum allowed precision is "
                                       + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                       + detail::to_string(real_prec_min())));
        REQUIRE_THROWS_MATCHES((complex{c1, complex_prec_t(0)}), std::invalid_argument,
                               Message("Cannot init a complex with a precision of 0: the maximum allowed precision is "
                                       + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                       + detail::to_string(real_prec_min())));
    }

    // Move ctor with custom precision.
    {
        complex c{1.1_r512}, c1{std::move(c), complex_prec_t(256)};
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 1.1_r256);
        REQUIRE((*re) != 1.1_r512);
        REQUIRE((*im) == 0);
        REQUIRE(re->get_prec() == 256);
        REQUIRE(im->get_prec() == 256);

        REQUIRE_THROWS_MATCHES((complex{std::move(c1), complex_prec_t(-1)}), std::invalid_argument,
                               Message("Cannot init a complex with a precision of -1: the maximum allowed precision is "
                                       + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                       + detail::to_string(real_prec_min())));
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE_THROWS_MATCHES((complex{std::move(c1), complex_prec_t(0)}), std::invalid_argument,
                               Message("Cannot init a complex with a precision of 0: the maximum allowed precision is "
                                       + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                       + detail::to_string(real_prec_min())));
    }

    // Binary ctors.
    {
        complex c1{45, -67.};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 45);
        REQUIRE(re->get_prec()
                == detail::c_max(detail::real_deduce_precision(45), detail::real_deduce_precision(-67.)));
        REQUIRE((*im) == -67);
        REQUIRE(im->get_prec()
                == detail::c_max(detail::real_deduce_precision(45), detail::real_deduce_precision(-67.)));
    }
    {
        complex c1{45_z1, -67 / 123_q1};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

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

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 1.23_r512);
        REQUIRE(re->get_prec() == 512);
        REQUIRE((*im) == 4.56_r256);
        REQUIRE(im->get_prec() == 512);
    }
    {
        auto r = 1.23_r512;
        auto i = 4.56_r256;
        complex c1{std::move(r), std::move(i)};

        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!i.is_valid());

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 1.23_r512);
        REQUIRE(re->get_prec() == 512);
        REQUIRE((*im) == 4.56_r256);
        REQUIRE(im->get_prec() == 512);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1{45_rq, 12_rq};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 45);
        REQUIRE(re->get_prec() == 113);
        REQUIRE((*im) == 12);
        REQUIRE(im->get_prec() == 113);
    }
#endif

    // Binary ctors with custom precision.
    {
        complex c1{45, -67., complex_prec_t(36)};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 45);
        REQUIRE(re->get_prec() == 36);
        REQUIRE((*im) == -67);
        REQUIRE(im->get_prec() == 36);
    }
    {
        complex c1{45_z1, -67 / 123_q1, complex_prec_t(87)};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 45);
        REQUIRE(re->get_prec() == 87);
        REQUIRE((*im) == real{-67 / 123_q1, 87});
        REQUIRE(im->get_prec() == 87);
    }
    {
        auto r = 1.23_r512;
        auto i = 4.56_r256;
        complex c1{r, i, complex_prec_t(128)};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 1.23_r128);
        REQUIRE(re->get_prec() == 128);
        REQUIRE((*im) == 4.56_r128);
        REQUIRE(im->get_prec() == 128);
    }
    {
        auto r = 1.23_r512;
        auto i = 4.56_r256;
        complex c1{std::move(r), std::move(i), complex_prec_t(128)};

        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!r.is_valid());
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!i.is_valid());

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 1.23_r128);
        REQUIRE(re->get_prec() == 128);
        REQUIRE((*im) == 4.56_r128);
        REQUIRE(im->get_prec() == 128);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1{45_rq, 12_rq, complex_prec_t(28)};

        complex::re_cref re{c1};
        complex::im_cref im{c1};

        REQUIRE((*re) == 45);
        REQUIRE(re->get_prec() == 28);
        REQUIRE((*im) == 12);
        REQUIRE(im->get_prec() == 28);
    }
#endif
    // Bad prec value.
    {
        REQUIRE_THROWS_MATCHES((complex{42, 43, complex_prec_t(-1)}), std::invalid_argument,
                               Message("Cannot init a real with a precision of -1: the maximum allowed precision is "
                                       + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                       + detail::to_string(real_prec_min())));
        REQUIRE_THROWS_MATCHES((complex{1_q1, 1.23_r512, complex_prec_t(-2)}), std::invalid_argument,
                               Message("Cannot init a real with a precision of -2: the maximum allowed precision is "
                                       + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                       + detail::to_string(real_prec_min())));
    }

    // Implicit generic ctors
    {
        complex c1 = 42;
        REQUIRE(c1 == 42);
    }
    {
        complex c1 = true;
        REQUIRE(c1 == 1);
    }
    {
        complex c1 = 123.;
        REQUIRE(c1 == 123);
    }
    {
        complex c1 = -56_z1;
        REQUIRE(c1 == -56);
    }
    {
        complex c1 = -56_q1;
        REQUIRE(c1 == -56);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1 = 123.5_rq;
        REQUIRE(c1 == 123.5_rq);
    }
#endif
    {
        complex c1 = 1.1_r256;
        REQUIRE(c1 == 1.1_r256);
    }
    {
        complex c1 = std::complex<double>(1, 2);
        REQUIRE(c1 == std::complex<double>(1, 2));
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1 = 1.1_rq - 2.1_icq;
        REQUIRE(c1 == 1.1_rq - 2.1_icq);
    }
#endif
}

TEST_CASE("string constructors")
{
    using Catch::Matchers::Message;

    std::vector<char> buffer;

    // Start with zeroes.
    {
        complex c{"0", 10, complex_prec_t(128)};
        REQUIRE(c == 0);
        REQUIRE(c.get_prec() == 128);

        complex::re_cref re{c};
        complex::im_cref im{c};

        REQUIRE(!re->signbit());
        REQUIRE(!(*im).signbit());
    }
    {
        complex c{std::string("(-0)"), 10, complex_prec_t(128)};
        REQUIRE(c == 0);
        REQUIRE(c.get_prec() == 128);

        complex::re_cref re{c};
        complex::im_cref im{c};

        REQUIRE(re->signbit());
        REQUIRE(!(*im).signbit());
    }
    {
        complex c{"(0,0)", 10, complex_prec_t(128)};
        REQUIRE(c == 0);
        REQUIRE(c.get_prec() == 128);

        complex::re_cref re{c};
        complex::im_cref im{c};

        REQUIRE(!re->signbit());
        REQUIRE(!(*im).signbit());
    }

    // Single value, no brackets.
    {
        complex c{"1.1", 10, complex_prec_t(128)};
        REQUIRE(c == 1.1_r128);
        REQUIRE(c.get_prec() == 128);

        complex::im_cref im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{"  1.1", 10, complex_prec_t(128)};
        REQUIRE(c == 1.1_r128);
        REQUIRE(c.get_prec() == 128);

        complex::im_cref im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{"  +1.1", complex_prec_t(128)};
        REQUIRE(c == 1.1_r128);
        REQUIRE(c.get_prec() == 128);

        complex::im_cref im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        buffer = {' ', '+', '1', '.', '1'};

        complex c{buffer.data(), buffer.data() + buffer.size(), 10, complex_prec_t(128)};

        REQUIRE(c == 1.1_r128);
        REQUIRE(c.get_prec() == 128);

        complex::im_cref im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        buffer = {' ', '+', '1', '.', '3'};

        complex c{buffer.data(), buffer.data() + buffer.size(), complex_prec_t(128)};

        REQUIRE(c == 1.3_r128);
        REQUIRE(c.get_prec() == 128);

        complex::im_cref im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
#if MPPP_CPLUSPLUS >= 201703L
    {
        constexpr char str[] = "  -0x2f2.1aa4p0";

        complex c{str, 16, complex_prec_t(128)};
        REQUIRE(c == -0x2f2.1aa4p0_r128);
        REQUIRE(c.get_prec() == 128);

        complex::im_cref im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{std::string("  -0x2f2.1aa4p0"), 0, complex_prec_t(128)};
        REQUIRE(c == -0x2f2.1aa4p0_r128);
        REQUIRE(c.get_prec() == 128);

        complex::im_cref im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
#endif
    REQUIRE_THROWS_MATCHES((complex{"1.1 ", 10, complex_prec_t(128)}), std::invalid_argument,
                           Message("The string '1.1 ' does not represent a valid real in base 10"));
    REQUIRE_THROWS_MATCHES((complex{"hello world", 12, complex_prec_t(128)}), std::invalid_argument,
                           Message("The string 'hello world' does not represent a valid real in base 12"));
    REQUIRE_THROWS_MATCHES((complex{"1.1 ", -2, complex_prec_t(128)}), std::invalid_argument,
                           Message("Cannot construct a complex from a string in base -2: the base must either be zero "
                                   "or in the [2,62] range"));

    // Single value, brackets.
    {
        complex c{"(1.1)", 10, complex_prec_t(128)};
        REQUIRE(c == 1.1_r128);
        REQUIRE(c.get_prec() == 128);

        complex::im_cref im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{" (1.1)", 10, complex_prec_t(128)};
        REQUIRE(c == 1.1_r128);
        REQUIRE(c.get_prec() == 128);

        complex::im_cref im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
#if MPPP_CPLUSPLUS >= 201703L
    {
        complex c{std::string(" ( -0x2f2.1aa4p0)"), 16, complex_prec_t(128)};
        REQUIRE(c == -0x2f2.1aa4p0_r128);
        REQUIRE(c.get_prec() == 128);

        complex::im_cref im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{std::string(" ( -0x2f2.1aa4p0)"), 0, complex_prec_t(128)};
        REQUIRE(c == -0x2f2.1aa4p0_r128);
        REQUIRE(c.get_prec() == 128);

        complex::im_cref im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
#endif
    {
        complex c{" ( 1.1)", complex_prec_t(128)};
        REQUIRE(c == 1.1_r128);
        REQUIRE(c.get_prec() == 128);

        complex::im_cref im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        buffer = {' ', '(', '+', '1', '.', '1', ')'};

        complex c{buffer.data(), buffer.data() + buffer.size(), 10, complex_prec_t(128)};

        REQUIRE(c == 1.1_r128);
        REQUIRE(c.get_prec() == 128);

        complex::im_cref im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        buffer = {' ', '(', '+', '1', '.', '3', ')'};

        complex c{buffer.data(), buffer.data() + buffer.size(), complex_prec_t(128)};

        REQUIRE(c == 1.3_r128);
        REQUIRE(c.get_prec() == 128);

        complex::im_cref im{c};
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    REQUIRE_THROWS_MATCHES((complex{" ( 1.1 )", 10, complex_prec_t(128)}), std::invalid_argument,
                           Message("The string ' 1.1 ' does not represent a valid real in base 10"));
    REQUIRE_THROWS_MATCHES((complex{"(hello world)", 12, complex_prec_t(128)}), std::invalid_argument,
                           Message("The string 'hello world' does not represent a valid real in base 12"));
    REQUIRE_THROWS_MATCHES((complex{"(1.1)", -20, complex_prec_t(128)}), std::invalid_argument,
                           Message("Cannot construct a complex from a string in base -20: the base must either be zero "
                                   "or in the [2,62] range"));

    // Two values.
    {
        complex c{"(-1.1,-2.3)", 10, complex_prec_t(256)};
        REQUIRE(c == complex{-1.1_r256, -2.3_r256});
        REQUIRE(c.get_prec() == 256);
    }
#if defined(MPPP_HAVE_STRING_VIEW)
    {
        complex c{std::string_view("(-1.1,-2.3)"), 10, complex_prec_t(256)};
        REQUIRE(c == complex{-1.1_r256, -2.3_r256});
        REQUIRE(c.get_prec() == 256);
    }
#endif
    {
        complex c{" (-1.1,-2.3)", 0, complex_prec_t(256)};
        REQUIRE(c == complex{-1.1_r256, -2.3_r256});
        REQUIRE(c.get_prec() == 256);
    }
    {
        complex c{std::string(" ( -1.1, -2.3)"), 0, complex_prec_t(256)};
        REQUIRE(c == complex{-1.1_r256, -2.3_r256});
        REQUIRE(c.get_prec() == 256);
    }
    {
        buffer = {' ', '(', '-', '1', '.', '3', ',', '0', '.', '7', ')'};

        complex c{buffer.data(), buffer.data() + buffer.size(), complex_prec_t(128)};

        REQUIRE(c == complex{-1.3_r128, 0.7_r128});
        REQUIRE(c.get_prec() == 128);
    }
    {
        buffer = {' ', '(', '-', '1', '.', '3', ',', '0', '.', '7', ')'};

        complex c{buffer.data(), buffer.data() + buffer.size(), 10, complex_prec_t(128)};

        REQUIRE(c == complex{-1.3_r128, 0.7_r128});
        REQUIRE(c.get_prec() == 128);
    }
#if MPPP_CPLUSPLUS >= 201703L
    {
        complex c{"(   -0x2f2.1aa4p0, 0x123.aaap4)", 16, complex_prec_t(128)};
        REQUIRE(c == complex{-0x2f2.1aa4p0_r128, 0x123.aaap4_r128});
        REQUIRE(c.get_prec() == 128);
    }
    {
        complex c{"(   -0x2f2.1aa4p0, 0x123.aaap4)", 0, complex_prec_t(128)};
        REQUIRE(c == complex{-0x2f2.1aa4p0_r128, 0x123.aaap4_r128});
        REQUIRE(c.get_prec() == 128);
    }
#endif
    REQUIRE_THROWS_MATCHES((complex{" (hello, 2)", 10, complex_prec_t(128)}), std::invalid_argument,
                           Message("The string 'hello' does not represent a valid real in base 10"));
    REQUIRE_THROWS_MATCHES((complex{"(2, world )", 12, complex_prec_t(128)}), std::invalid_argument,
                           Message("The string ' world ' does not represent a valid real in base 12"));
}

TEST_CASE("ref getters")
{
    complex c{1, -2};
    {
        complex::re_cref re{c};
        complex::im_cref im{c};

        REQUIRE((*re) == 1);
        REQUIRE((*im) == -2);
    }
#if MPPP_CPLUSPLUS >= 201703L
    {
        REQUIRE(*c.real_cref() == 1);
        REQUIRE(*c.imag_cref() == -2);

        REQUIRE(*real_cref(c) == 1);
        REQUIRE(*imag_cref(c) == -2);
    }
#endif
    {
        complex::re_ref re{c};
        complex::im_ref im{c};

        *re = -1;
        *im = 100;

        REQUIRE((*re) == -1);
        REQUIRE((*im) == 100);
    }
    REQUIRE(c == complex{-1, 100});
#if MPPP_CPLUSPLUS >= 201703L
    {
        *c.real_ref() = 42;
        *c.imag_ref() = -43;

        REQUIRE(*c.real_ref() == 42);
        REQUIRE(*c.imag_ref() == -43);

        *real_ref(c) = -5;
        *imag_ref(c) = -6;

        REQUIRE(*real_ref(c) == -5);
        REQUIRE(*imag_ref(c) == -6);
    }
    REQUIRE(c == complex{-5, -6});
#endif
}

#if !defined(_MSC_VER) || defined(__clang__)

TEST_CASE("mpc move ctor")
{
    ::mpc_t c;
    ::mpc_init2(c, 14);
    ::mpfr_set_d(mpc_realref(c), 1.1, MPFR_RNDN);
    ::mpfr_set_d(mpc_imagref(c), -2.3, MPFR_RNDN);

    // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
    complex c2{std::move(c)};

    REQUIRE(c2.get_prec() == 14);
    REQUIRE(c2 == complex{1.1, -2.3, complex_prec_t(14)});
}

#endif

TEST_CASE("copy move ass")
{
    {
        complex c1, c2{3, 4};
        c1 = c2;
        REQUIRE(c1 == complex{3, 4});

        // Revive moved-from object via copy assignment.
        complex c3{std::move(c1)};
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
        c1 = c2;
        REQUIRE(c1 == complex{3, 4});

        // Self copy assignment.
        c1 = *&c1;
        REQUIRE(c1 == complex{3, 4});
    }

    {
        complex c1, c2{3, 4};
        c1 = std::move(c2);
        REQUIRE(c1 == complex{3, 4});
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c2.is_valid());
        REQUIRE(c2 == complex{});

        // Revive moved-from object via move assignment.
        complex c3{std::move(c1)};
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c1.is_valid());
        c1 = complex{45, 46};
        REQUIRE(c1 == complex{45, 46});

        // Self move assignment.
        c1 = std::move(*&c1);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(c1 == complex{45, 46});
    }
}

TEST_CASE("generic assignment")
{
    {
        complex c{12, 13, complex_prec_t(12)};
        c = 45;
        REQUIRE(c.get_prec() == detail::real_deduce_precision(45));

        complex::re_cref re(c);
        complex::im_cref im(c);

        REQUIRE(*re == 45);
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{12, 13, complex_prec_t(12)};
        c = 45.;
        REQUIRE(c.get_prec() == detail::real_deduce_precision(45.));

        complex::re_cref re(c);
        complex::im_cref im(c);

        REQUIRE(*re == 45.);
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{12, 13, complex_prec_t(12)};
        c = 45_z1;
        REQUIRE(c.get_prec() == detail::real_deduce_precision(45_z1));

        complex::re_cref re(c);
        complex::im_cref im(c);

        REQUIRE(*re == 45);
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{12, 13, complex_prec_t(12)};
        c = 1 / 3_q1;
        REQUIRE(c.get_prec() == detail::real_deduce_precision(1 / 3_q1));

        complex::re_cref re(c);
        complex::im_cref im(c);

        REQUIRE(*re == real{1 / 3_q1});
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c{12, 13, complex_prec_t(12)};
        c = 1.1_rq;
        REQUIRE(c.get_prec() == 113);

        complex::re_cref re(c);
        complex::im_cref im(c);

        REQUIRE(*re == 1.1_rq);
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
#endif
    {
        complex c{12, 13, complex_prec_t(12)};
        c = 1.1_r256;
        REQUIRE(c.get_prec() == 256);

        complex::re_cref re(c);
        complex::im_cref im(c);

        REQUIRE(*re == 1.1_r256);
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    // Test moving too.
    {
        complex c{12, 13, complex_prec_t(12)};
        auto r = 1.1_r256;
        c = std::move(r);
        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(r.is_valid());
        REQUIRE(r == 12);
        REQUIRE(r.get_prec() == 12);
        REQUIRE(c.get_prec() == 256);

        complex::re_cref re(c);
        complex::im_cref im(c);

        REQUIRE(*re == 1.1_r256);
        REQUIRE(im->zero_p());
        REQUIRE(!im->signbit());
    }
    {
        complex c{12, 13, complex_prec_t(12)};
        c = std::complex<double>{1.1, -2.3};
        REQUIRE(c.get_prec() == detail::real_deduce_precision(1.1));

        complex::re_cref re(c);
        complex::im_cref im(c);

        REQUIRE(*re == 1.1);
        REQUIRE(*im == -2.3);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c{12, 13, complex_prec_t(12)};
        c = complex128{1.1_rq, -2.3_rq};
        REQUIRE(c.get_prec() == 113);

        complex::re_cref re(c);
        complex::im_cref im(c);

        REQUIRE(*re == 1.1_rq);
        REQUIRE(*im == -2.3_rq);
    }
#endif
}

TEST_CASE("mpc_t assignment")
{
    complex c1, c2{41, 42};
    c1 = c2.get_mpc_t();

    REQUIRE(c1 == complex{41, 42});
#if !defined(_MSC_VER) || defined(__clang__)
    complex c3;

    mpc_t o;
    ::mpc_init2(o, 150);
    ::mpc_set_d_d(o, 1.1, 2.3, MPC_RNDNN);

    // NOLINTNEXTLINE(hicpp-move-const-arg, performance-move-const-arg)
    c3 = std::move(o);

    REQUIRE(c3 == complex{1.1, 2.3, complex_prec_t(150)});
    REQUIRE(c3.get_prec() == 150);
#endif
}

TEST_CASE("is_valid")
{
    complex c{1, 2};
    REQUIRE(c.is_valid());
    complex c2{std::move(c)};
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!c.is_valid());
}

TEST_CASE("set")
{
    using Catch::Matchers::Message;

    {
        complex c{1, 2, complex_prec_t(14)};
        c.set(complex{3, 4, complex_prec_t(42)});
        REQUIRE(c == complex{3, 4, complex_prec_t(14)});
        REQUIRE(c.get_prec() == 14);
    }
    {
        complex c{1, 2, complex_prec_t(14)};
        c.set(42);
        REQUIRE(c == 42);
        REQUIRE(c.get_prec() == 14);
    }
    {
        complex c{1, 2, complex_prec_t(14)};
        c.set(-1.3);
        REQUIRE(c == complex{-1.3, complex_prec_t(14)});
        REQUIRE(c.get_prec() == 14);
    }
    {
        complex c{1, 2, complex_prec_t(14)};
        c.set(42_z1);
        REQUIRE(c == 42);
        REQUIRE(c.get_prec() == 14);
    }
    {
        complex c{1, 2, complex_prec_t(14)};
        c.set(1 / 10_q1);
        REQUIRE(c == complex{1 / 10_q1, complex_prec_t(14)});
        REQUIRE(c.get_prec() == 14);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c{1, 2, complex_prec_t(14)};
        c.set(1.1_rq);
        REQUIRE(c == complex{1.1_rq, complex_prec_t(14)});
        REQUIRE(c.get_prec() == 14);
    }
#endif
    {
        complex c{1, 2, complex_prec_t(14)};
        c.set(1.1_r256);
        REQUIRE(c == complex{1.1_r256, complex_prec_t(14)});
        REQUIRE(c.get_prec() == 14);
    }

    {
        complex c{1, 2, complex_prec_t(14)};
        c.set(std::complex<double>(-1.3, 1.1));
        REQUIRE(c == complex{std::complex<double>(-1.3, 1.1), complex_prec_t(14)});
        REQUIRE(c.get_prec() == 14);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c{1, 2, complex_prec_t(14)};
        c.set(1.1_rq + 2.3_icq);
        REQUIRE(c == complex{1.1_rq + 2.3_icq, complex_prec_t(14)});
        REQUIRE(c.get_prec() == 14);
    }
#endif

    // String setters.
    {
        complex c{1, 2, complex_prec_t(14)};
        c.set("123");
        REQUIRE(c.get_prec() == 14);
        REQUIRE(c == complex{123, complex_prec_t(14)});

        c.set(std::string("(456)"));
        REQUIRE(c.get_prec() == 14);
        REQUIRE(c == complex{456, complex_prec_t(14)});

#if defined(MPPP_HAVE_STRING_VIEW)
        c.set(std::string_view("(456)"));
        REQUIRE(c.get_prec() == 14);
        REQUIRE(c == complex{456, complex_prec_t(14)});
#endif

        // Try different base as well.
        c.set("(1111011,111001000)", 2);
        REQUIRE(c.get_prec() == 14);
        REQUIRE(c == complex{"(123,456)", complex_prec_t(14)});

        c.set("(1c8)", 16);
        REQUIRE(c.get_prec() == 14);
        REQUIRE(c == complex{"(456)", complex_prec_t(14)});

        // Detect base.
        c.set("(0x1c8)", 0);
        REQUIRE(c.get_prec() == 14);
        REQUIRE(c == complex{"(456)", complex_prec_t(14)});

        c.set("(0b1111011,0x1c8)", 0);
        REQUIRE(c.get_prec() == 14);
        REQUIRE(c == complex{"(123,456)", complex_prec_t(14)});

        c.set("(1.1,2.3)");
        REQUIRE(c.get_prec() == 14);
        REQUIRE(c == complex{"(1.1,2.3)", complex_prec_t(14)});

        std::vector<char> buffer;
        constexpr char s[] = "(1.1,2.3)";
        buffer.assign(s, s + sizeof(s) - 1u);
        buffer.emplace_back('a');
        buffer.emplace_back('b');
        buffer.emplace_back('c');

        c.set(buffer.data(), buffer.data() + sizeof(s) - 1u);
        REQUIRE(c == complex{"(1.1,2.3)", complex_prec_t(14)});

        // Error handling.
        REQUIRE_THROWS_MATCHES(c.set("456", -1), std::invalid_argument,
                               Message("Cannot assign a complex from a string in base -1: the base must either be zero "
                                       "or in the [2,62] range"));
        REQUIRE_THROWS_MATCHES(
            c.set("456", 128), std::invalid_argument,
            Message("Cannot assign a complex from a string in base 128: the base must either be zero "
                    "or in the [2,62] range"));

        {
            REQUIRE_THROWS_MATCHES(
                c.set("hello"), std::invalid_argument,
                Message("The string 'hello' cannot be interpreted as a floating-point value in base 10"));

            complex::re_cref re{c};
            complex::im_cref im{c};

            REQUIRE(re->nan_p());
            REQUIRE(im->nan_p());
        }
        REQUIRE(c.get_prec() == 14);
        c.set("(1.1,2.3)");
        {
            REQUIRE_THROWS_MATCHES(
                c.set("(123,hello)"), std::invalid_argument,
                Message("The string 'hello' cannot be interpreted as a floating-point value in base 10"));

            complex::re_cref re{c};
            complex::im_cref im{c};

            REQUIRE(re->nan_p());
            REQUIRE(im->nan_p());
        }
        REQUIRE(c.get_prec() == 14);
        c.set("(1.1,2.3)");
        {
            REQUIRE_THROWS_MATCHES(c.set("(123,"), std::invalid_argument,
                                   Message("The string '(123,' is not a valid representation of a complex value"));

            complex::re_cref re{c};
            complex::im_cref im{c};

            REQUIRE(re->nan_p());
            REQUIRE(im->nan_p());
        }
        REQUIRE(c.get_prec() == 14);
        c.set("(1.1,2.3)");
        {
            REQUIRE_THROWS_MATCHES(c.set(""), std::invalid_argument,
                                   Message("The string '' is not a valid representation of a complex value"));

            complex::re_cref re{c};
            complex::im_cref im{c};

            REQUIRE(re->nan_p());
            REQUIRE(im->nan_p());
        }
    }

    // Try also the free-function overload.
    {
        complex c{1, 2, complex_prec_t(14)};
        set(c, -1.3);
        REQUIRE(c == complex{-1.3, complex_prec_t(14)});
        REQUIRE(c.get_prec() == 14);
    }
    {
        complex c{1, 2, complex_prec_t(14)};
        set(c, "(1111011,111001000)", 2);
        REQUIRE(c.get_prec() == 14);
        REQUIRE(c == complex{"(123,456)", complex_prec_t(14)});
    }
}

TEST_CASE("set_nan")
{
    complex c{1, 2};
    c.set_nan();
    {
        complex::re_cref re{c};
        complex::im_cref im{c};

        REQUIRE(re->nan_p());
        REQUIRE(im->nan_p());
    }

    c = complex{4, 5};
    set_nan(c);
    {
        complex::re_cref re{c};
        complex::im_cref im{c};

        REQUIRE(re->nan_p());
        REQUIRE(im->nan_p());
    }
}

TEST_CASE("mpc_t getters")
{
    complex c{1, 2};
    REQUIRE(mpfr_cmp_ui(mpc_realref(c.get_mpc_t()), 1u) == 0);
    REQUIRE(mpfr_cmp_ui(mpc_imagref(c.get_mpc_t()), 2u) == 0);
    ::mpc_add_ui(c._get_mpc_t(), c.get_mpc_t(), 3u, MPC_RNDNN);
    REQUIRE(mpfr_cmp_ui(mpc_realref(c.get_mpc_t()), 4u) == 0);
    REQUIRE(mpfr_cmp_ui(mpc_imagref(c.get_mpc_t()), 2u) == 0);
}

TEST_CASE("special values")
{
    complex c;
    REQUIRE(c.zero_p());
    REQUIRE(zero_p(c));

    c = 1;
    REQUIRE(!c.zero_p());
    REQUIRE(!zero_p(c));

    c = complex{0, 1};
    REQUIRE(!c.zero_p());
    REQUIRE(!zero_p(c));

    c = complex{1, 1};
    REQUIRE(!c.zero_p());
    REQUIRE(!zero_p(c));

    c = complex{0, 0};
    REQUIRE(c.zero_p());
    REQUIRE(zero_p(c));

    REQUIRE(!c.is_one());
    REQUIRE(!is_one(c));

    c = complex{2, 0};
    REQUIRE(!c.is_one());
    REQUIRE(!is_one(c));

    c = complex{2, 1};
    REQUIRE(!c.is_one());
    REQUIRE(!is_one(c));

    c = complex{1, 1};
    REQUIRE(!c.is_one());
    REQUIRE(!is_one(c));

    c = complex{1, 0};
    REQUIRE(c.is_one());
    REQUIRE(is_one(c));

    c = complex{-1, 0};
    REQUIRE(!c.is_one());
    REQUIRE(!is_one(c));
}

TEST_CASE("precision handling")
{
    using Catch::Matchers::Message;

    complex c;
    REQUIRE(c.get_prec() == real_prec_min());
    REQUIRE(get_prec(c) == real_prec_min());

    c = complex{1, 2, complex_prec_t(42)};
    REQUIRE(c.get_prec() == 42);
    REQUIRE(get_prec(c) == 42);

    c.set_prec(128);
    REQUIRE(c.get_prec() == 128);
    {
        complex::re_cref re{c};
        complex::im_cref im{c};

        REQUIRE(re->nan_p());
        REQUIRE(im->nan_p());
    }

    set_prec(c, 129);
    REQUIRE(c.get_prec() == 129);
    {
        complex::re_cref re{c};
        complex::im_cref im{c};

        REQUIRE(re->nan_p());
        REQUIRE(im->nan_p());
    }

    c = complex{"(1.1,2.3)", complex_prec_t(128)};
    c.prec_round(64);
    REQUIRE(c.get_prec() == 64);
    REQUIRE(c != complex{"(1.1,2.3)", complex_prec_t(128)});
    REQUIRE(c == complex{"(1.1,2.3)", complex_prec_t(64)});
    prec_round(c, 32);
    REQUIRE(c.get_prec() == 32);
    REQUIRE(c != complex{"(1.1,2.3)", complex_prec_t(64)});
    REQUIRE(c == complex{"(1.1,2.3)", complex_prec_t(32)});

    // Error handling.
    REQUIRE_THROWS_MATCHES(
        c.set_prec(-1), std::invalid_argument,
        Message("Cannot set the precision of a complex to the value -1: the maximum allowed precision is "
                + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                + detail::to_string(real_prec_min())));
    REQUIRE_THROWS_MATCHES(
        c.prec_round(0), std::invalid_argument,
        Message("Cannot set the precision of a complex to the value 0: the maximum allowed precision is "
                + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                + detail::to_string(real_prec_min())));
    REQUIRE_THROWS_MATCHES(c.prec_round(real_prec_max() + 1), std::invalid_argument,
                           Message("Cannot set the precision of a complex to the value "
                                   + detail::to_string(real_prec_max() + 1) + ": the maximum allowed precision is "
                                   + detail::to_string(real_prec_max()) + ", the minimum allowed precision is "
                                   + detail::to_string(real_prec_min())));
}

TEST_CASE("conversions")
{
    using Catch::Matchers::Message;

    {
        REQUIRE(static_cast<int>(complex{42, 0}) == 42);
        REQUIRE_THROWS_MATCHES(static_cast<int>(complex{42, 1}), std::domain_error,
                               Message("Cannot convert the complex value " + complex{42, 1}.to_string()
                                       + " to the real-valued type '" + type_name<int>()
                                       + "': the imaginary part is not zero"));
    }
    {
        REQUIRE(static_cast<double>(complex{-63, 0}) == -63.);
        REQUIRE_THROWS_MATCHES(static_cast<double>(complex{-63, 1}), std::domain_error,
                               Message("Cannot convert the complex value " + complex{-63, 1}.to_string()
                                       + " to the real-valued type '" + type_name<double>()
                                       + "': the imaginary part is not zero"));
    }
    {
        REQUIRE(static_cast<integer<1>>(complex{-63, 0}) == -63);
        REQUIRE_THROWS_MATCHES(static_cast<integer<1>>(complex{-63, 1}), std::domain_error,
                               Message("Cannot convert the complex value " + complex{-63, 1}.to_string()
                                       + " to the real-valued type '" + type_name<integer<1>>()
                                       + "': the imaginary part is not zero"));
    }
    {
        REQUIRE(static_cast<rational<1>>(complex{-63, 0}) == -63);
        REQUIRE_THROWS_MATCHES(static_cast<rational<1>>(complex{-63, 1}), std::domain_error,
                               Message("Cannot convert the complex value " + complex{-63, 1}.to_string()
                                       + " to the real-valued type '" + type_name<rational<1>>()
                                       + "': the imaginary part is not zero"));
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        REQUIRE(static_cast<real128>(complex{-63, 0}) == -63);
        REQUIRE_THROWS_MATCHES(static_cast<real128>(complex{-63, 1}), std::domain_error,
                               Message("Cannot convert the complex value " + complex{-63, 1}.to_string()
                                       + " to the real-valued type '" + type_name<real128>()
                                       + "': the imaginary part is not zero"));
    }
#endif
    {
        REQUIRE(static_cast<real>(complex{-63, 0}) == -63);
        REQUIRE(static_cast<real>(complex{-63, 0, complex_prec_t(78)}).get_prec() == 78);
        REQUIRE_THROWS_MATCHES(static_cast<real>(complex{-63, 1}), std::domain_error,
                               Message("Cannot convert the complex value " + complex{-63, 1}.to_string()
                                       + " to the real-valued type '" + type_name<real>()
                                       + "': the imaginary part is not zero"));
    }
    {
        REQUIRE(static_cast<std::complex<double>>(complex{-63, 12}) == std::complex<double>{-63, 12});
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        REQUIRE(static_cast<complex128>(complex{-63, 12}) == complex128{-63, 12});
    }
#endif

    // Make sure we prevent conversion to ref/cv qualified
    // interoperable types.
    REQUIRE(!std::is_convertible<complex, const int>::value);
    REQUIRE(!std::is_convertible<complex, double &>::value);
    REQUIRE(!std::is_convertible<const complex &, integer<1> &&>::value);

    // Special casing for bool.
    REQUIRE(static_cast<bool>(complex{0, 0}) == false);
    REQUIRE(static_cast<bool>(complex{1, 0}));
    REQUIRE(static_cast<bool>(complex{1, 1}));
    REQUIRE(static_cast<bool>(complex{0, 1}));
    REQUIRE(static_cast<bool>(complex{0, real{"nan", 42}}));
    REQUIRE(static_cast<bool>(complex{real{"nan", 42}, 0}));
    REQUIRE(static_cast<bool>(complex{real{"nan", 42}, real{"nan", 42}}));
}

TEST_CASE("get conversions")
{
    {
        int n = -1;
        REQUIRE(complex{42, 0}.get(n));
        REQUIRE(n == 42);

        REQUIRE(get(n, complex{-43, 0}));
        REQUIRE(n == -43);

        REQUIRE(!complex{42, -1}.get(n));
        REQUIRE(n == -43);
        REQUIRE(!get(n, complex{42, 1}));
        REQUIRE(n == -43);
        REQUIRE(!get(n, complex{"nan", complex_prec_t(12)}));
        REQUIRE(n == -43);
    }
    {
        double x = -1;
        REQUIRE(complex{42, 0}.get(x));
        REQUIRE(x == 42);

        REQUIRE(get(x, complex{-43, 0}));
        REQUIRE(x == -43);

        REQUIRE(!complex{42, -1}.get(x));
        REQUIRE(x == -43);
        REQUIRE(!get(x, complex{42, 1}));
        REQUIRE(x == -43);

        if (std::numeric_limits<double>::has_quiet_NaN) {
            REQUIRE(get(x, complex{"nan", complex_prec_t(12)}));
            REQUIRE(std::isnan(x));
        }
    }
    {
        integer<1> x{-1};
        REQUIRE(complex{42, 0}.get(x));
        REQUIRE(x == 42);

        REQUIRE(get(x, complex{-43, 0}));
        REQUIRE(x == -43);

        REQUIRE(!complex{42, -1}.get(x));
        REQUIRE(x == -43);
        REQUIRE(!get(x, complex{42, 1}));
        REQUIRE(x == -43);
        REQUIRE(!get(x, complex{"nan", complex_prec_t(12)}));
        REQUIRE(x == -43);
    }
    {
        rational<1> x{-1};
        REQUIRE(complex{42, 0}.get(x));
        REQUIRE(x == 42);

        REQUIRE(get(x, complex{-43, 0}));
        REQUIRE(x == -43);

        REQUIRE(!complex{42, -1}.get(x));
        REQUIRE(x == -43);
        REQUIRE(!get(x, complex{42, 1}));
        REQUIRE(x == -43);
        REQUIRE(!get(x, complex{"nan", complex_prec_t(12)}));
        REQUIRE(x == -43);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        real128 x{-1};
        REQUIRE(complex{42.1_rq, 0}.get(x));
        REQUIRE(x == 42.1_rq);

        REQUIRE(get(x, complex{-43.3_rq, 0}));
        REQUIRE(x == -43.3_rq);

        REQUIRE(!complex{42, -1}.get(x));
        REQUIRE(x == -43.3_rq);
        REQUIRE(!get(x, complex{42, 1}));
        REQUIRE(x == -43.3_rq);
        REQUIRE(get(x, complex{"nan", complex_prec_t(12)}));
        REQUIRE(isnan(x));
    }
#endif
    {
        real x{1, 56};
        REQUIRE(complex{42, 0, complex_prec_t(67)}.get(x));
        REQUIRE(x == 42);
        REQUIRE(x.get_prec() == 67);

        REQUIRE(get(x, complex{-43, 0, complex_prec_t(34)}));
        REQUIRE(x == -43);
        REQUIRE(x.get_prec() == 34);

        REQUIRE(!complex{42, -1}.get(x));
        REQUIRE(x == -43);
        REQUIRE(x.get_prec() == 34);
        REQUIRE(!get(x, complex{42, 1}));
        REQUIRE(x == -43);
        REQUIRE(x.get_prec() == 34);
        REQUIRE(get(x, complex{"nan", complex_prec_t(12)}));
        REQUIRE(x.nan_p());
        REQUIRE(x.get_prec() == 12);
    }
    {
        std::complex<double> c{1, 2};
        REQUIRE(complex{42, -37}.get(c));
        REQUIRE(c == std::complex<double>{42, -37});

        REQUIRE(get(c, complex{-43, 35}));
        REQUIRE(c == std::complex<double>{-43, 35});
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex128 c{1, 2};
        REQUIRE(complex{42, -37}.get(c));
        REQUIRE(c == complex128{42, -37});

        REQUIRE(get(c, complex{-43, 35}));
        REQUIRE(c == complex128{-43, 35});
    }
#endif
}

TEST_CASE("swap")
{
    complex c1{123, -45, complex_prec_t(45)}, c2{67, 89, complex_prec_t(23)};
    swap(c1, c2);
    REQUIRE(c1.get_prec() == 23);
    REQUIRE(c2.get_prec() == 45);
    REQUIRE(c1 == complex{67, 89, complex_prec_t(23)});
    REQUIRE(c2 == complex{123, -45, complex_prec_t(45)});

#if MPPP_CPLUSPLUS >= 201703L
    REQUIRE(std::is_nothrow_swappable_v<complex>);
#endif
}

TEST_CASE("to_string")
{
    complex c{45, 67, complex_prec_t(12)};
    REQUIRE(c.to_string() == '(' + real{45, 12}.to_string() + ',' + real{67, 12}.to_string() + ')');
    REQUIRE(c.to_string(16) == '(' + real{45, 12}.to_string(16) + ',' + real{67, 12}.to_string(16) + ')');
    REQUIRE(c.to_string(11) == '(' + real{45, 12}.to_string(11) + ',' + real{67, 12}.to_string(11) + ')');
}

// Assignment to other mp++ classes.
TEST_CASE("mppp_ass")
{
    {
        integer<1> n;
        n = complex{3, 0};
        REQUIRE(n == 3);
        REQUIRE_THROWS_AS((n = complex{3, 1}), std::domain_error);
        REQUIRE(n == 3);
    }
    {
        rational<1> n;
        n = complex{3, 0};
        REQUIRE(n == 3);
        REQUIRE_THROWS_AS((n = complex{3, 1}), std::domain_error);
        REQUIRE(n == 3);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        real128 r;
        r = complex{-42, 0};
        REQUIRE(r == -42);
        REQUIRE_THROWS_AS((r = complex{3, 1}), std::domain_error);
        REQUIRE(r == -42);
    }
    {
        complex128 r;
        r = complex{-42, 0};
        REQUIRE(r == -42);
        r = complex{3, 1};
        REQUIRE(r == complex128{3, 1});
    }
#endif
    {
        real r;
        r = complex{-42, 0};
        REQUIRE(r == -42);
        REQUIRE_THROWS_AS((r = complex{3, 1}), std::domain_error);
        REQUIRE(r == -42);
    }
}

TEST_CASE("get_real_imag")
{
    {
        complex c{1, 2};

        auto p = std::move(c).get_real_imag();

        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        REQUIRE(p.first == 1);
        REQUIRE(p.second == 2);

        c = complex{3, 4};
        REQUIRE(c.is_valid());

        p = std::move(c).get_real_imag();

        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        REQUIRE(p.first == 3);
        REQUIRE(p.second == 4);

        c = complex{-5, -6};
        REQUIRE(c.is_valid());

        p = get_real_imag(std::move(c));

        // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
        REQUIRE(!c.is_valid());
        REQUIRE(p.first == -5);
        REQUIRE(p.second == -6);
    }
    {
        complex c{1, 2};

        auto p = c.get_real_imag();

        REQUIRE(c.is_valid());
        REQUIRE(c == complex{1, 2});
        REQUIRE(p.first == 1);
        REQUIRE(p.second == 2);

        c = complex{3, 4};
        REQUIRE(c.is_valid());

        p = c.get_real_imag();

        REQUIRE(c.is_valid());
        REQUIRE(c == complex{3, 4});
        REQUIRE(p.first == 3);
        REQUIRE(p.second == 4);

        c = complex{-5, -6};
        REQUIRE(c.is_valid());

        p = get_real_imag(c);

        REQUIRE(c.is_valid());
        REQUIRE(c == complex{-5, -6});
        REQUIRE(p.first == -5);
        REQUIRE(p.second == -6);
    }
}

#if defined(MPPP_WITH_BOOST_S11N)

template <typename OA, typename IA>
void test_s11n()
{
    std::stringstream ss;

    auto x = 1.1_r512 - 1.3_icr512;
    {
        OA oa(ss);
        oa << x;
    }

    x = real{};
    {
        IA ia(ss);
        ia >> x;
    }

    REQUIRE(x == 1.1_r512 - 1.3_icr512);
    REQUIRE(x.get_prec() == 512);
}

TEST_CASE("boost_s11n")
{
    test_s11n<boost::archive::text_oarchive, boost::archive::text_iarchive>();
    test_s11n<boost::archive::binary_oarchive, boost::archive::binary_iarchive>();
}

#endif

#if defined(MPPP_MPFR_HAVE_MPFR_GET_STR_NDIGITS)

TEST_CASE("real str_ndigits")
{
    using Catch::Matchers::Message;

    complex c0{"(1.1,1.3)", complex_prec_t(53)};

    REQUIRE(c0.get_str_ndigits() == 17u);
    REQUIRE(c0.get_str_ndigits(10) == 17u);

    c0 = complex{"(1.1,1.3)", complex_prec_t(24)};

    REQUIRE(get_str_ndigits(c0) == 9u);
    REQUIRE(get_str_ndigits(c0, 10) == 9u);

    REQUIRE_THROWS_MATCHES(
        c0.get_str_ndigits(1), std::invalid_argument,
        Message("Invalid base value for get_str_ndigits(): the base must be in the [2,62] range, but it is 1 instead"));
    REQUIRE_THROWS_MATCHES(
        get_str_ndigits(c0, -100), std::invalid_argument,
        Message(
            "Invalid base value for get_str_ndigits(): the base must be in the [2,62] range, but it is -100 instead"));
    REQUIRE_THROWS_MATCHES(
        get_str_ndigits(c0, 63), std::invalid_argument,
        Message(
            "Invalid base value for get_str_ndigits(): the base must be in the [2,62] range, but it is 63 instead"));
}

#endif
