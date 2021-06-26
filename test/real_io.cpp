// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <iomanip>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>

#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp_test;

using int_types = std::tuple<char, signed char, unsigned char, short, unsigned short, int, unsigned, long,
                             unsigned long, long long, unsigned long long, wchar_t>;

using fp_types = std::tuple<float, double, long double>;

using int_t = integer<1>;
using rat_t = rational<1>;

// NOLINTNEXTLINE(cert-err58-cpp, cert-msc32-c, cert-msc51-cpp, cppcoreguidelines-avoid-non-const-global-variables)
static std::mt19937 rng;

static const int ntrials = 1000;

TEST_CASE("real ostream")
{
    // Default setting.
    {
        std::ostringstream oss;
        oss << real{"1.1", 53};

        REQUIRE(oss.str() == "1.1");
    }

    // Scientific format.
    {
        std::ostringstream oss;
        oss << std::scientific << real{"1.1", 53};

        REQUIRE(oss.str() == "1.100000e+00");

        oss.str("");

        oss << std::uppercase << real{"1.1", 53};
        REQUIRE(oss.str() == "1.100000E+00");

        oss.str("");

        oss << real{"inf", 53};
        REQUIRE(oss.str() == "INF");
    }

    // Fixed format.
    {
        std::ostringstream oss;
        oss << std::fixed << real{"1.1", 53};

        REQUIRE(oss.str() == "1.100000");

        oss.str("");

        oss << std::uppercase << real{"inf", 53};
        REQUIRE(oss.str() == "inf");
    }

    // Hexfloat.
    {
        std::ostringstream oss;
        oss << std::hexfloat << real{"1.1", 53};

        REQUIRE(oss.str() == "0x1.199999999999ap+0");

        oss.str("");

        oss << std::uppercase << real{"1.1", 53};

        REQUIRE(oss.str() == "0X1.199999999999AP+0");
    }

    // Test the showpoint bits.
    {
        std::ostringstream oss;
        oss << std::showpoint << real{"42", 53};

        REQUIRE(oss.str() == "42.0000");

        oss.str("");

        oss << std::scientific << real{"42", 53};

        REQUIRE(oss.str() == "4.200000e+01");
    }

    // Test unconditional plus on front.
    {
        std::ostringstream oss;
        oss << std::showpos << real{"1.1", 53};

        REQUIRE(oss.str() == "+1.1");

        oss.str("");

        oss << std::scientific << real{"1.1", 53};

        REQUIRE(oss.str() == "+1.100000e+00");

        oss.str("");

        oss << std::hexfloat << real{"1.1", 53};

        REQUIRE(oss.str() == "+0x1.199999999999ap+0");
    }

    // Test altering the precision.
    {
        std::ostringstream oss;
        oss << std::showpoint << std::setprecision(10) << real{"42", 53};

        REQUIRE(oss.str() == "42.00000000");

        oss.str("");

        oss << std::scientific << real{"42", 53};

        REQUIRE(oss.str() == "4.2000000000e+01");

        oss.str("");

        oss << std::setprecision(20) << std::showpos << real{"1.1", 53};

        REQUIRE(oss.str() == "+1.10000000000000008882e+00");
    }

    // Test right fill.
    {
        std::ostringstream oss;
        oss << std::setw(20) << std::setfill('*') << std::right << real{"1.1", 53};

        REQUIRE(oss.str() == "*****************1.1");

        oss.str("");

        oss << std::setw(20) << std::showpos << real{"1.1", 53};

        REQUIRE(oss.str() == "****************+1.1");

        oss.str("");

        oss << std::setw(20) << real{"-1.1", 53};

        REQUIRE(oss.str() == "****************-1.1");
    }

    // Test left fill.
    {
        std::ostringstream oss;
        oss << std::setw(20) << std::setfill('*') << std::left << real{"1.1", 53};

        REQUIRE(oss.str() == "1.1*****************");

        oss.str("");

        oss << std::setw(20) << std::showpos << real{"1.1", 53};

        REQUIRE(oss.str() == "+1.1****************");

        oss.str("");

        oss << std::setw(20) << real{"-1.1", 53};

        REQUIRE(oss.str() == "-1.1****************");
    }

    // Test internal fill.
    {
        std::ostringstream oss;
        oss << std::setw(20) << std::setfill('*') << std::internal << real{"1.1", 53};

        REQUIRE(oss.str() == "*****************1.1");

        oss.str("");

        oss << std::setw(20) << std::showpos << real{"1.1", 53};

        REQUIRE(oss.str() == "+****************1.1");

        oss.str("");

        oss << std::setw(20) << real{"-1.1", 53};

        REQUIRE(oss.str() == "-****************1.1");

        oss.str("");

        oss << std::setw(20) << std::fixed << real{"1.1", 53};

        REQUIRE(oss.str() == "+***********1.100000");

        oss.str("");

        oss << std::setw(20) << std::scientific << real{"-1.1", 53};

        REQUIRE(oss.str() == "-*******1.100000e+00");

        oss.str("");

        oss << std::setw(20) << std::hexfloat << real{"1.1", 53};

        REQUIRE(oss.str() == "+0x1.199999999999ap+0");

        oss.str("");

        oss << std::setw(30) << std::hexfloat << real{"-1.1", 53};

        REQUIRE(oss.str() == "-*********0x1.199999999999ap+0");

        // Check the width is cleared out.
        oss.str("");

        oss << real{"-1.1", 53};

        REQUIRE(oss.str() == "-0x1.199999999999ap+0");
    }

    // Negative precision.
    {
        std::ostringstream oss;
        oss << std::setprecision(-1) << real{"1.1", 53};

        REQUIRE(oss.str() == "1.1");

        oss.str("");

        oss << std::scientific << real{"-1.1", 53};

        REQUIRE(oss.str() == "-1.100000e+00");

        oss.str("");

        oss << std::fixed << real{"-1.1", 53};

        REQUIRE(oss.str() == "-1.100000");

        oss.str("");

        oss << std::hexfloat << real{"-1.1", 53};

        REQUIRE(oss.str() == "-0x1.199999999999ap+0");
    }

    // Zero precision.
    {
        std::ostringstream oss;
        oss << std::setprecision(0) << real{"1.1", 53};

        REQUIRE(oss.str() == "1");

        oss.str("");

        oss << std::scientific << real{"-1.1", 53};

        REQUIRE(oss.str() == "-1e+00");

        oss.str("");

        oss << std::fixed << real{"-1.1", 53};

        REQUIRE(oss.str() == "-1");

        oss.str("");

        oss << std::hexfloat << real{"-1.1", 53};

        REQUIRE(oss.str() == "-0x1.199999999999ap+0");
    }

    // Print zero with zero precision.
    {
        std::ostringstream oss;
        oss << std::setprecision(0) << real{"0", 53};

        REQUIRE(oss.str() == "0");
    }

    // NOTE: disable random testing on Windows as it seems in some cases
    // the stream operator formatting is not fully standard-compliant.
#if !defined(_WIN32)

    // Random testing.
    if (std::numeric_limits<double>::radix == 2) {
        std::uniform_real_distribution<double> rdist(-100., 100.);
        std::uniform_int_distribution<int> idist(0, 1), pdist(-1, std::numeric_limits<double>::max_digits10),
            wdist(-1, 100);

        for (auto i = 0; i < ntrials; ++i) {
            std::ostringstream oss1, oss2;

            if (idist(rng) == 0) {
                oss1 << std::scientific;
                oss2 << std::scientific;
            }

            if (idist(rng) == 0) {
                oss1 << std::fixed;
                oss2 << std::fixed;
            }

            if (idist(rng) == 0) {
                oss1 << std::showpoint;
                oss2 << std::showpoint;
            }

            if (idist(rng) == 0) {
                oss1 << std::showpos;
                oss2 << std::showpos;
            }

            if (idist(rng) == 0) {
                oss1 << std::uppercase;
                oss2 << std::uppercase;
            }

            const auto prec = pdist(rng);

            oss1 << std::setprecision(prec);
            oss2 << std::setprecision(prec);

            const auto w = wdist(rng);

            oss1 << std::setw(w);
            oss2 << std::setw(w);

            oss1 << std::setfill('*');
            oss2 << std::setfill('*');

            const auto x = rdist(rng);

            oss1 << x;
            oss2 << real{x};

            REQUIRE(oss1.str() == oss2.str());
        }
    }

#endif
}

struct int_io_tester {
    template <typename T>
    void operator()(const T &) const
    {
        integral_minmax_dist<T> int_dist;
        std::uniform_int_distribution<::mpfr_prec_t> prec_dist(::mpfr_prec_t(real_prec_min()), ::mpfr_prec_t(200));
        std::uniform_int_distribution<int> base_dist(2, 62);
        for (auto i = 0; i < ntrials; ++i) {
            const auto tmp = int_dist(rng);
            const auto prec = prec_dist(rng);
            const auto base = base_dist(rng);
            real tmp_r{tmp, prec};
            real tmp_cmp{tmp_r.to_string(base), base, prec};
            REQUIRE(::mpfr_equal_p(tmp_r.get_mpfr_t(), tmp_cmp.get_mpfr_t()));
        }
    }
};

struct fp_io_tester {
    template <typename T>
    void operator()(const T &) const
    {
        auto dist = std::uniform_real_distribution<T>(T(-100), T(100));
        std::uniform_int_distribution<::mpfr_prec_t> prec_dist(::mpfr_prec_t(real_prec_min()), ::mpfr_prec_t(200));
        std::uniform_int_distribution<int> base_dist(2, 62);
        for (auto i = 0; i < ntrials; ++i) {
            const auto tmp = dist(rng);
            const auto prec = prec_dist(rng);
            const auto base = base_dist(rng);
            real tmp_r{tmp, prec};
            real tmp_cmp{tmp_r.to_string(base), base, prec};
            REQUIRE(::mpfr_equal_p(tmp_r.get_mpfr_t(), tmp_cmp.get_mpfr_t()));
            tmp_r.set("0", base);
            REQUIRE(tmp_r.zero_p());
            REQUIRE(!tmp_r.signbit());
            tmp_cmp = real{tmp_r.to_string(base), base, prec};
            REQUIRE(tmp_cmp.zero_p());
            REQUIRE(!tmp_cmp.signbit());
            tmp_r.set("-0", base);
            REQUIRE(tmp_r.zero_p());
            REQUIRE(tmp_r.signbit());
            tmp_cmp = real{tmp_r.to_string(base), base, prec};
            REQUIRE(tmp_cmp.zero_p());
            REQUIRE(tmp_cmp.signbit());
            tmp_r.set("@inf@", base);
            REQUIRE(tmp_r.inf_p());
            REQUIRE(tmp_r.sgn() > 0);
            tmp_cmp = real{tmp_r.to_string(base), base, prec};
            REQUIRE(::mpfr_equal_p(tmp_r.get_mpfr_t(), tmp_cmp.get_mpfr_t()));
            tmp_r.set("-@inf@", base);
            REQUIRE(tmp_r.inf_p());
            REQUIRE(tmp_r.sgn() < 0);
            tmp_cmp = real{tmp_r.to_string(base), base, prec};
            REQUIRE(::mpfr_equal_p(tmp_r.get_mpfr_t(), tmp_cmp.get_mpfr_t()));
            tmp_r.set("@nan@", base);
            REQUIRE(tmp_r.nan_p());
            tmp_cmp = real{tmp_r.to_string(base), base, prec};
            REQUIRE(tmp_cmp.nan_p());
        }
    }
};

TEST_CASE("real io")
{
    tuple_for_each(int_types{}, int_io_tester{});
    tuple_for_each(fp_types{}, fp_io_tester{});
    REQUIRE_THROWS_PREDICATE(real{}.to_string(-1), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string{"Cannot convert a real to a string in base -1: the base must be in the [2,62] range"};
    });
    REQUIRE_THROWS_PREDICATE(real{}.to_string(70), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string{"Cannot convert a real to a string in base 70: the base must be in the [2,62] range"};
    });
    // A couple of small tests for the stream operators.
    {
        std::ostringstream oss;
        oss << real{123, 100};
        REQUIRE(::mpfr_equal_p(real{123, 100}.get_mpfr_t(), real{oss.str(), 100}.get_mpfr_t()));
    }
}
