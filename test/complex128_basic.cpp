// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <complex>

#include <mp++/complex128.hpp>
#include <mp++/config.hpp>

#if defined(MPPP_WITH_MPFR)

#include <mp++/real.hpp>

#endif

#include "catch.hpp"

using namespace mppp;

TEST_CASE("complex128 constructors")
{
#if MPPP_CPLUSPLUS >= 201402L
    constexpr complex128 c{std::complex<double>{1, 2}};
#endif
    constexpr complex128 c2{3, 4};
    constexpr auto tmp = complex128{2}.creal();

#if MPPP_CPLUSPLUS >= 201402L
    std::cout << c << '\n';
#endif
    std::cout << c2 << '\n';
    std::cout << tmp << '\n';

    std::cout << complex128{(123_q1 * pow(2_z1, 512)) / 456} << '\n';
    std::cout << complex128{123_q1 / 2, 1} << '\n';
    complex128{real128{4}};

#if defined(MPPP_WITH_MPFR)
    std::cout << complex128{123_q1, 1.1_r256} << '\n';
#endif

    std::cout << (complex128{1, 2} == complex128{1, 2}) << '\n';
    std::cout << (complex128{1, 2} == real128{1}) << '\n';
    std::cout << (real128{1} == complex128{1, 2}) << '\n';
    std::cout << (complex128{1, 2} == 1.5) << '\n';
    std::cout << (45ull == complex128{1, 2}) << '\n';

    // Testing for string ctor:
    // - leading whitespaces (outside and inside the brackets),
    // - trailing whitespaces,
    // - empty components,
    // - non-closed bracket.
    std::cout << complex128{"1.23"} << '\n';
    std::cout << complex128{"(1.23)"} << '\n';
    std::cout << complex128{"(1.23,-4.56)"} << '\n';
    std::cout << complex128{"(1.23,-4.56)"} << '\n';

    std::cout << 1.23_cq << '\n';
    std::cout << 1.23_irq << '\n';
    std::cout << 1_irq << '\n';
    [[maybe_unused]] constexpr auto flop = 1_irq;
}
