// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <complex>
#include <utility>

#include <mp++/complex.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real.hpp>

#if defined(MPPP_WITH_QUADMATH)

#include <mp++/real128.hpp>

#endif

#include "catch.hpp"

using namespace mppp;

TEST_CASE("complex constructors")
{
    // Def ctor.
    {
        complex c;

        complex::const_re_extractor re{c};
        complex::const_im_extractor im{c};

        REQUIRE(re.get().zero_p());
        REQUIRE(im.get().zero_p());
        REQUIRE(re.get().get_prec() == real_prec_min());
        REQUIRE(im.get().get_prec() == real_prec_min());
        REQUIRE(!re.get().signbit());
        REQUIRE(!im.get().signbit());
    }

    // Generic ctor
    {
        complex c1{42};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == 42);
        REQUIRE(im.get().zero_p());
        REQUIRE(re.get().get_prec() == detail::real_deduce_precision(42));
        REQUIRE(im.get().get_prec() == detail::real_deduce_precision(42));
    }
    {
        complex c1{123.};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == 123);
        REQUIRE(im.get().zero_p());
        REQUIRE(re.get().get_prec() == detail::real_deduce_precision(123.));
        REQUIRE(im.get().get_prec() == detail::real_deduce_precision(123.));
    }
    {
        complex c1{-42_z1};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == -42);
        REQUIRE(im.get().zero_p());
        REQUIRE(re.get().get_prec() == detail::real_deduce_precision(-42_z1));
        REQUIRE(im.get().get_prec() == detail::real_deduce_precision(-42_z1));
    }
    {
        complex c1{73_q1 / 2};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == 73_q1 / 2);
        REQUIRE(im.get().zero_p());
        REQUIRE(re.get().get_prec() == detail::real_deduce_precision(73_q1 / 2));
        REQUIRE(im.get().get_prec() == detail::real_deduce_precision(73_q1 / 2));
    }
    {
        complex c1{1.1_r512};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == 1.1_r512);
        REQUIRE(im.get().zero_p());
        REQUIRE(re.get().get_prec() == 512);
        REQUIRE(im.get().get_prec() == 512);
    }
    {
        // Try moving in a real.
        auto r = 1.1_r512;

        complex c1{std::move(r)};
        REQUIRE(!r.is_valid());

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == 1.1_r512);
        REQUIRE(im.get().zero_p());
        REQUIRE(re.get().get_prec() == 512);
        REQUIRE(im.get().get_prec() == 512);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1{-3.1_rq};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == -3.1_rq);
        REQUIRE(im.get().zero_p());
        REQUIRE(re.get().get_prec() == 113);
        REQUIRE(im.get().get_prec() == 113);
    }
#endif
    {
        complex c1{std::complex<double>{-4, 7}};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == -4);
        REQUIRE(im.get() == 7);
        REQUIRE(re.get().get_prec() == detail::real_deduce_precision(-4.));
        REQUIRE(im.get().get_prec() == detail::real_deduce_precision(7.));
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1{-3.1_rq + 2.1_icq};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == -3.1_rq);
        REQUIRE(im.get() == 2.1_rq);
        REQUIRE(re.get().get_prec() == 113);
        REQUIRE(im.get().get_prec() == 113);
    }
#endif

    // Copy ctor.
    {
        complex c1{std::complex<double>{-4, 7}}, c2 = c1;

        complex::const_re_extractor re{c2};
        complex::const_im_extractor im{c2};

        REQUIRE(re.get() == -4);
        REQUIRE(im.get() == 7);
        REQUIRE(re.get().get_prec() == detail::real_deduce_precision(-4.));
        REQUIRE(im.get().get_prec() == detail::real_deduce_precision(7.));
    }

    // Move ctor.
    {
        complex c1{std::complex<double>{-4, 7}}, c2{std::move(c1)};

        REQUIRE(!c1.is_valid());

        complex::const_re_extractor re{c2};
        complex::const_im_extractor im{c2};

        REQUIRE(re.get() == -4);
        REQUIRE(im.get() == 7);
        REQUIRE(re.get().get_prec() == detail::real_deduce_precision(-4.));
        REQUIRE(im.get().get_prec() == detail::real_deduce_precision(7.));
    }

    // Generic ctor with custom precision.
    {
        complex c1{42, complex_prec_t(123)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == 42);
        REQUIRE(im.get().zero_p());
        REQUIRE(re.get().get_prec() == 123);
        REQUIRE(im.get().get_prec() == 123);
    }
    {
        complex c1{42.l, complex_prec_t(10)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == 42);
        REQUIRE(im.get().zero_p());
        REQUIRE(re.get().get_prec() == 10);
        REQUIRE(im.get().get_prec() == 10);
    }
    {
        complex c1{-42_z1, complex_prec_t(768)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == -42);
        REQUIRE(im.get().zero_p());
        REQUIRE(re.get().get_prec() == 768);
        REQUIRE(im.get().get_prec() == 768);
    }
    {
        complex c1{73_q1 / 2, complex_prec_t(1768)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == 73_q1 / 2);
        REQUIRE(im.get().zero_p());
        REQUIRE(re.get().get_prec() == 1768);
        REQUIRE(im.get().get_prec() == 1768);
    }
    {
        complex c1{1.1_r512, complex_prec_t(128)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == 1.1_r128);
        REQUIRE(im.get().zero_p());
        REQUIRE(re.get().get_prec() == 128);
        REQUIRE(im.get().get_prec() == 128);
    }
    {
        // Try moving in a real.
        auto r = 1.1_r512;

        complex c1{std::move(r), complex_prec_t(1024)};
        REQUIRE(!r.is_valid());

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == 1.1_r512);
        REQUIRE(im.get().zero_p());
        REQUIRE(re.get().get_prec() == 1024);
        REQUIRE(im.get().get_prec() == 1024);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1{-3.1_rq, complex_prec_t(1024)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == -3.1_rq);
        REQUIRE(im.get().zero_p());
        REQUIRE(re.get().get_prec() == 1024);
        REQUIRE(im.get().get_prec() == 1024);
    }
#endif
    {
        complex c1{std::complex<double>{-4, 7}, complex_prec_t(10)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == -4);
        REQUIRE(im.get() == 7);
        REQUIRE(re.get().get_prec() == 10);
        REQUIRE(im.get().get_prec() == 10);
    }
#if defined(MPPP_WITH_QUADMATH)
    {
        complex c1{-3.1_rq + 2.1_icq, complex_prec_t(512)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == -3.1_rq);
        REQUIRE(im.get() == 2.1_rq);
        REQUIRE(re.get().get_prec() == 512);
        REQUIRE(im.get().get_prec() == 512);
    }
#endif
}
