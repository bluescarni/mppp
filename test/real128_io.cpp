// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <iomanip>
#include <limits>
#include <random>
#include <sstream>

#include <mp++/real128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

static const int ntries = 1000;

// NOLINTNEXTLINE(cert-err58-cpp, cert-msc32-c, cert-msc51-cpp, cppcoreguidelines-avoid-non-const-global-variables)
static std::mt19937 rng;

TEST_CASE("stream output")
{
    // Default setting.
    {
        std::ostringstream oss;
        oss << real128{"1.1"};

        REQUIRE(oss.str() == "1.1");
    }

    // Scientific format.
    {
        std::ostringstream oss;
        oss << std::scientific << real128{"1.1"};

        REQUIRE(oss.str() == "1.100000e+00");

        oss.str("");

        oss << std::uppercase << real128{"1.1"};
        REQUIRE(oss.str() == "1.100000E+00");

        oss.str("");

        oss << real128{"inf"};
        REQUIRE(oss.str() == "INF");
    }

    // Fixed format.
    {
        std::ostringstream oss;
        oss << std::fixed << real128{"1.1"};

        REQUIRE(oss.str() == "1.100000");

        oss.str("");

        oss << std::uppercase << real128{"inf"};
        REQUIRE(oss.str() == "inf");
    }

    // Hexfloat.
    {
        std::ostringstream oss;
        oss << std::hexfloat << real128{"1.1"};

        REQUIRE(oss.str() == "0x1.199999999999999999999999999ap+0");

        oss.str("");

        oss << std::uppercase << real128{"1.1"};

        REQUIRE(oss.str() == "0X1.199999999999999999999999999AP+0");
    }

    // Test the showpoint bits.
    {
        std::ostringstream oss;
        oss << std::showpoint << real128{"42"};

        REQUIRE(oss.str() == "42.0000");

        oss.str("");

        oss << std::scientific << real128{"42"};

        REQUIRE(oss.str() == "4.200000e+01");
    }

    // Test unconditional plus on front.
    {
        std::ostringstream oss;
        oss << std::showpos << real128{"1.1"};

        REQUIRE(oss.str() == "+1.1");

        oss.str("");

        oss << std::scientific << real128{"1.1"};

        REQUIRE(oss.str() == "+1.100000e+00");

        oss.str("");

        oss << std::hexfloat << real128{"1.1"};

        REQUIRE(oss.str() == "+0x1.199999999999999999999999999ap+0");
    }

    // Test altering the precision.
    {
        std::ostringstream oss;
        oss << std::showpoint << std::setprecision(10) << real128{"42"};

        REQUIRE(oss.str() == "42.00000000");

        oss.str("");

        oss << std::scientific << real128{"42"};

        REQUIRE(oss.str() == "4.2000000000e+01");

        oss.str("");

        oss << std::setprecision(20) << std::showpos << real128{"1.1"};

        REQUIRE(oss.str() == "+1.10000000000000000000e+00");

        oss.str("");

        oss << std::setprecision(36) << std::showpos << real128{"1.1"};

        REQUIRE(oss.str() == "+1.100000000000000000000000000000000077e+00");
    }

    // Test right fill.
    {
        std::ostringstream oss;
        oss << std::setw(20) << std::setfill('*') << std::right << real128{"1.1"};

        REQUIRE(oss.str() == "*****************1.1");

        oss.str("");

        oss << std::setw(20) << std::showpos << real128{"1.1"};

        REQUIRE(oss.str() == "****************+1.1");

        oss.str("");

        oss << std::setw(20) << real128{"-1.1"};

        REQUIRE(oss.str() == "****************-1.1");
    }

    // Test left fill.
    {
        std::ostringstream oss;
        oss << std::setw(20) << std::setfill('*') << std::left << real128{"1.1"};

        REQUIRE(oss.str() == "1.1*****************");

        oss.str("");

        oss << std::setw(20) << std::showpos << real128{"1.1"};

        REQUIRE(oss.str() == "+1.1****************");

        oss.str("");

        oss << std::setw(20) << real128{"-1.1"};

        REQUIRE(oss.str() == "-1.1****************");
    }

    // Test internal fill.
    {
        std::ostringstream oss;
        oss << std::setw(20) << std::setfill('*') << std::internal << real128{"1.1"};

        REQUIRE(oss.str() == "*****************1.1");

        oss.str("");

        oss << std::setw(20) << std::showpos << real128{"1.1"};

        REQUIRE(oss.str() == "+****************1.1");

        oss.str("");

        oss << std::setw(20) << real128{"-1.1"};

        REQUIRE(oss.str() == "-****************1.1");

        oss.str("");

        oss << std::setw(20) << std::fixed << real128{"1.1"};

        REQUIRE(oss.str() == "+***********1.100000");

        oss.str("");

        oss << std::setw(20) << std::scientific << real128{"-1.1"};

        REQUIRE(oss.str() == "-*******1.100000e+00");

        oss.str("");

        oss << std::setw(20) << std::hexfloat << real128{"1.1"};

        REQUIRE(oss.str() == "+0x1.199999999999999999999999999ap+0");

        oss.str("");

        oss << std::setw(60) << std::hexfloat << real128{"-1.1"};

        REQUIRE(oss.str() == "-************************0x1.199999999999999999999999999ap+0");

        // Check the width is cleared out.
        oss.str("");

        oss << real128{"-1.1"};

        REQUIRE(oss.str() == "-0x1.199999999999999999999999999ap+0");
    }

    // Negative precision.
    {
        std::ostringstream oss;
        oss << std::setprecision(-1) << real128{"1.1"};

        REQUIRE(oss.str() == "1.1");

        oss.str("");

        oss << std::scientific << real128{"-1.1"};

        REQUIRE(oss.str() == "-1.100000e+00");

        oss.str("");

        oss << std::fixed << real128{"-1.1"};

        REQUIRE(oss.str() == "-1.100000");

        oss.str("");

        oss << std::hexfloat << real128{"-1.1"};

        REQUIRE(oss.str() == "-0x1.199999999999999999999999999ap+0");
    }

    // Zero precision.
    {
        std::ostringstream oss;
        oss << std::setprecision(0) << real128{"1.1"};

        REQUIRE(oss.str() == "1");

        oss.str("");

        oss << std::scientific << real128{"-1.1"};

        REQUIRE(oss.str() == "-1e+00");

        oss.str("");

        oss << std::fixed << real128{"-1.1"};

        REQUIRE(oss.str() == "-1");

        oss.str("");

        oss << std::hexfloat << real128{"-1.1"};

        REQUIRE(oss.str() == "-0x1.199999999999999999999999999ap+0");
    }

    // Print zero with zero precision.
    {
        std::ostringstream oss;
        oss << std::setprecision(0) << real128{"0"};

        REQUIRE(oss.str() == "0");
    }

    // Random testing.
    if (std::numeric_limits<double>::radix == 2) {
        std::uniform_real_distribution<double> rdist(-100., 100.);
        std::uniform_int_distribution<int> idist(0, 1), pdist(-1, std::numeric_limits<double>::max_digits10),
            wdist(-1, 100);

        for (auto i = 0; i < ntries; ++i) {
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
            oss2 << real128{x};

            REQUIRE(oss1.str() == oss2.str());
        }
    }
}

static inline void check_round_trip(const real128 &r)
{
    const auto tmp = r.to_string();
    real128 r2{tmp};
    REQUIRE(((r.m_value == r2.m_value) || (r.isnan() && r2.isnan() && r.signbit() == r2.signbit())));
    std::ostringstream oss;
    oss << std::setprecision(std::numeric_limits<real128>::max_digits10) << r;
    REQUIRE(oss.str() == tmp);
    REQUIRE(oss.str() == r.to_string());
}

TEST_CASE("real128 io")
{
    check_round_trip(real128{});
    check_round_trip(real128{1.23});
    check_round_trip(real128{-4.56});
    check_round_trip(real128{"1.1"});
    check_round_trip(real128{"-1.1"});
    check_round_trip(real128{"inf"});
    check_round_trip(real128{"-inf"});
    check_round_trip(real128{"nan"});
    check_round_trip(real128{"-nan"});
    std::uniform_int_distribution<int> sdist(0, 1);
    std::uniform_real_distribution<double> dist1(100., 1000.);
    for (int i = 0; i < ntries; ++i) {
        check_round_trip(nextafter(real128{dist1(rng)}, real128{10000.}) * (sdist(rng) != 0 ? 1 : -1));
    }
    std::uniform_real_distribution<double> dist2(1E-6, 1E-1);
    for (int i = 0; i < ntries; ++i) {
        check_round_trip(nextafter(real128{dist2(rng)}, real128{1.}) * (sdist(rng) != 0 ? 1 : -1));
    }
    std::uniform_real_distribution<double> dist3(1E100, 1E120);
    for (int i = 0; i < ntries; ++i) {
        check_round_trip(nextafter(real128{dist3(rng)}, real128{1E121}) * (sdist(rng) != 0 ? 1 : -1));
    }
    // Some subnormals.
    check_round_trip(real128{"1E-4960"});
    check_round_trip(real128{"-1E-4960"});
}
