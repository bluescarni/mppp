// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <complex>

#include <mp++/complex.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/real.hpp>

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
        complex c1{42, complex_prec_t(123)};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == 42);
        REQUIRE(im.get().zero_p());
        REQUIRE(re.get().get_prec() == 123);
        REQUIRE(im.get().get_prec() == 123);
    }
    {
        complex c1{std::complex<double>{-4, 7}};

        complex::const_re_extractor re{c1};
        complex::const_im_extractor im{c1};

        REQUIRE(re.get() == -4);
        REQUIRE(im.get() == 7);
        REQUIRE(re.get().get_prec() == detail::real_deduce_precision(-4.));
        REQUIRE(im.get().get_prec() == detail::real_deduce_precision(7.));
    }
}
