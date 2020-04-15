// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/complex128.hpp>
#include <mp++/config.hpp>

#include "catch.hpp"

using namespace mppp;

// Some tests involving constexpr result in an ICE on GCC < 6.
#if (defined(_MSC_VER) || defined(__clang__) || __GNUC__ >= 6) && MPPP_CPLUSPLUS >= 201402L

#define MPPP_ENABLE_CONSTEXPR_TESTS

#endif

#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)

static constexpr complex128 test_constexpr_incr()
{
    complex128 retval;
    ++retval;
    retval++;
    return retval;
}

static constexpr complex128 test_constexpr_decr()
{
    complex128 retval;
    --retval;
    retval--;
    return retval;
}

#if 0
static constexpr complex128 test_constexpr_ipa()
{
    complex128 retval{1};
    retval += real128{-2};
    retval += 1.;
    retval += -1;
    int n = 3;
    n += real128{-2};
    return retval + n;
}

static constexpr real128 test_constexpr_ips()
{
    real128 retval{1};
    retval -= real128{-2};
    retval -= 1.;
    retval -= -1;
    int n = 3;
    n -= real128{-2};
    return retval + n;
}

static constexpr real128 test_constexpr_ipm()
{
    real128 retval{1};
    retval *= real128{-2};
    retval *= 2.;
    retval *= -1;
    int n = 3;
    n *= real128{-2};
    return retval * n;
}

static constexpr real128 test_constexpr_ipd()
{
    real128 retval{12};
    retval /= real128{-2};
    retval /= 3.;
    retval /= -2;
    int n = 6;
    n /= real128{-2};
    return n / retval;
}
#endif

#endif

TEST_CASE("identity")
{
    REQUIRE(+complex128{3, 4}.m_value == cplex128{3, 4});
    REQUIRE((+complex128{3, -0.}).imag().signbit());

    constexpr auto c = +complex128{3, 4};
    REQUIRE(c.m_value == cplex128{3, 4});
}

TEST_CASE("negation")
{
    REQUIRE(-complex128{3, 4}.m_value == cplex128{-3, -4});
    REQUIRE(!(-complex128{3, -0.}).imag().signbit());
    REQUIRE(!(-complex128{-0., 3}).real().signbit());
    REQUIRE((-complex128{3, 0.}).imag().signbit());
    REQUIRE((-complex128{0., 3}).real().signbit());

    constexpr auto c = -complex128{3, 4};
    REQUIRE(c.m_value == cplex128{-3, -4});
}

TEST_CASE("incdec")
{
    complex128 x{5, 6};
    REQUIRE(((++x).m_value == cplex128{6, 6}));
    REQUIRE(((x++).m_value == cplex128{6, 6}));
    REQUIRE(((x).m_value == cplex128{7, 6}));
#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)
    constexpr auto z4 = test_constexpr_incr();
    REQUIRE((z4.m_value == 2));
#endif

    REQUIRE(((--x).m_value == cplex128{6, 6}));
    REQUIRE(((x--).m_value == cplex128{6, 6}));
    REQUIRE(((x).m_value == cplex128{5, 6}));
#if defined(MPPP_ENABLE_CONSTEXPR_TESTS)
    constexpr auto z6 = test_constexpr_decr();
    REQUIRE((z6.m_value == -2));
#endif
}
