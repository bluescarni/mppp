// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <complex>
#include <iomanip>
#include <limits>
#include <sstream>

#include <mp++/complex128.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("stream output")
{
    // Default setting.
    {
        std::ostringstream oss;
        oss << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-1.1,1.3)");
    }

    // Scientific format.
    {
        std::ostringstream oss;
        oss << std::scientific << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-1.100000e+00,1.300000e+00)");

        oss.str("");

        oss << std::uppercase << complex128{"(-1.1,1.3)"};
        REQUIRE(oss.str() == "(-1.100000E+00,1.300000E+00)");

        oss.str("");

        oss << complex128{"(inf,nan)"};
        REQUIRE(oss.str() == "(INF,NAN)");
    }

    // Fixed format.
    {
        std::ostringstream oss;
        oss << std::fixed << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-1.100000,1.300000)");

        oss.str("");

        oss << std::uppercase << complex128{"(inf,nan)"};
        REQUIRE(oss.str() == "(inf,nan)");
    }

    // Hexfloat.
    {
        std::ostringstream oss;
        oss << std::hexfloat << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-0x1.199999999999999999999999999ap+0,0x1.4ccccccccccccccccccccccccccdp+0)");

        oss.str("");

        oss << std::uppercase << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-0X1.199999999999999999999999999AP+0,0X1.4CCCCCCCCCCCCCCCCCCCCCCCCCCDP+0)");
    }

    // Test the showpoint bits.
    {
        std::ostringstream oss;
        oss << std::showpoint << complex128{-42, 31};

        REQUIRE(oss.str() == "(-42.0000,31.0000)");

        oss.str("");

        oss << std::scientific << complex128{-42, 31};

        REQUIRE(oss.str() == "(-4.200000e+01,3.100000e+01)");
    }

    // Test unconditional plus on front.
    {
        std::ostringstream oss;
        oss << std::showpos << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-1.1,+1.3)");

        oss.str("");

        oss << std::scientific << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-1.100000e+00,+1.300000e+00)");

        oss.str("");

        oss << std::hexfloat << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-0x1.199999999999999999999999999ap+0,+0x1.4ccccccccccccccccccccccccccdp+0)");
    }

    // Test altering the precision.
    {
        std::ostringstream oss;
        oss << std::showpoint << std::setprecision(10) << complex128{-42, 31};

        REQUIRE(oss.str() == "(-42.00000000,31.00000000)");

        oss.str("");

        oss << std::scientific << complex128{-42, 31};

        REQUIRE(oss.str() == "(-4.2000000000e+01,3.1000000000e+01)");

        oss.str("");

        oss << std::setprecision(40) << std::showpos << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str()
                == "(-1.1000000000000000000000000000000000770372e+00,+1.3000000000000000000000000000000000385186e+00)");
    }

    // Test right fill.
    {
        std::ostringstream oss;
        oss << std::setw(20) << std::setfill('*') << std::right << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "**********(-1.1,1.3)");

        oss.str("");

        oss << std::setw(20) << std::showpos << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "*********(-1.1,+1.3)");
    }

    // Test left fill.
    {
        std::ostringstream oss;
        oss << std::setw(20) << std::setfill('*') << std::left << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-1.1,1.3)**********");

        oss.str("");

        oss << std::setw(20) << std::showpos << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-1.1,+1.3)*********");
    }

    // Test internal fill (same as right).
    {
        std::ostringstream oss;
        oss << std::setw(20) << std::setfill('*') << std::internal << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "**********(-1.1,1.3)");

        oss.str("");

        oss << std::setw(20) << std::showpos << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "*********(-1.1,+1.3)");

        // Check the width is cleared out.
        oss.str("");

        oss << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-1.1,+1.3)");
    }

    // Negative precision.
    {
        std::ostringstream oss;
        oss << std::setprecision(-1) << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-1.1,1.3)");

        oss.str("");

        oss << std::scientific << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-1.100000e+00,1.300000e+00)");

        oss.str("");

        oss << std::fixed << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-1.100000,1.300000)");

        oss.str("");

        oss << std::hexfloat << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-0x1.199999999999999999999999999ap+0,0x1.4ccccccccccccccccccccccccccdp+0)");
    }

    // Zero precision.
    {
        std::ostringstream oss;
        oss << std::setprecision(0) << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-1,1)");

        oss.str("");

        oss << std::scientific << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-1e+00,1e+00)");

        oss.str("");

        oss << std::fixed << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-1,1)");

        oss.str("");

        oss << std::hexfloat << complex128{"(-1.1,1.3)"};

        REQUIRE(oss.str() == "(-0x1.199999999999999999999999999ap+0,0x1.4ccccccccccccccccccccccccccdp+0)");
    }

    // Print zero with zero precision.
    {
        std::ostringstream oss;
        oss << std::setprecision(0) << complex128{0, 0};

        REQUIRE(oss.str() == "(0,0)");
    }
}
