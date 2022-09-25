// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <functional>
#include <random>
#include <tuple>
#include <type_traits>

#include <gmp.h>

#include <mp++/integer.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

static const int ntries = 1000;

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp_test;

using sizes = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                         std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 6>,
                         std::integral_constant<std::size_t, 10>>;

// NOLINTNEXTLINE(cert-err58-cpp, cert-msc32-c, cert-msc51-cpp, cppcoreguidelines-avoid-non-const-global-variables)
static std::mt19937 rng;

struct hash_tester {
    template <typename S>
    inline void operator()(const S &) const
    {
        using integer = integer<S::value>;
        const std::hash<integer> hasher{};
        auto n1 = integer{12}, n2 = n1;
        REQUIRE(n2.is_static());
        n1.promote();
        REQUIRE(n1.is_dynamic());
        REQUIRE((hash(n1) == hash(n2)));
        REQUIRE((hasher(n1) == hash(n2)));
        n1 = integer{-12};
        n2 = n1;
        REQUIRE(n2.is_static());
        n1.promote();
        REQUIRE(n1.is_dynamic());
        REQUIRE((hash(n1) == hash(n2)));
        REQUIRE((hash(n1) == hasher(n2)));
        detail::mpz_raii tmp;
        std::uniform_int_distribution<int> sdist(0, 1);
        // Run a variety of tests with operands with x number of limbs.
        //
        // NOTE: capture n1/n2 by value because there seems to be
        // a strict aliasing bug in GCC, perhaps related to the union
        // with common initial sequence used to implement integer.
        // Essentially, what seems to happen is that the compiler ignores
        // the negation of n1 via n1.neg(), and the subsequent assignment
        // n2 = n1 will result in n1 and n2 having the same absolute
        // value but different sign. It seems like the "if (sdist(rng)) {"
        // is skipped altogether - adding a print statement in it is enough
        // to avoid the issue. Compiling with -fno-strict-aliasing also seems
        // to hide the bug, as well as capturing only n2 by reference.
        auto random_xy = [&, n1, n2](unsigned x) mutable {
            for (int i = 0; i < ntries; ++i) {
                random_integer(tmp, x, rng);
                n1 = integer(detail::mpz_to_str(&tmp.m_mpz));
                if (sdist(rng)) {
                    // std::cout << "Negated!\n";
                    n1.neg();
                }
                n2 = n1;
                if (n2.is_static()) {
                    n1.promote();
                }
                REQUIRE((hash(n1) == hash(n2)));
                REQUIRE((hasher(n1) == hash(n2)));
            }
        };

        random_xy(0);
        random_xy(1);
        random_xy(2);
        random_xy(3);
        random_xy(4);
    }
};

TEST_CASE("hash")
{
    tuple_for_each(sizes{}, hash_tester{});
}
