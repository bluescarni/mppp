// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#if defined(__clang__) || defined(__GNUC__)

// NOTE: range warnings in the interop between
// real128 and __(u)int128_t.
#pragma GCC diagnostic ignored "-Wconversion"

#endif

#include <complex>
#include <tuple>
#include <type_traits>

#include <mp++/mp++.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp_test;

using mppp_types = std::tuple<integer<1>, rational<1>, real, real128, complex128, complex>;

using cpp_types = std::tuple<int, double, std::complex<double>
#if defined(MPPP_HAVE_GCC_INT128)
                             ,
                             __int128_t, __uint128_t
#endif
                             >;

template <typename T>
struct is_mppp_complex : std::false_type {
};

template <>
struct is_mppp_complex<complex128> : std::true_type {
};

template <>
struct is_mppp_complex<complex> : std::true_type {
};

struct mppp_interop_tester {
    template <typename A, typename B>
    static void run_ineq_cmp(const A &a, const B &b, std::false_type)
    {
        REQUIRE(!(a < b));
        REQUIRE(a <= b);
        REQUIRE(!(a > b));
        REQUIRE(a >= b);
    }
    template <typename A, typename B>
    static void run_ineq_cmp(const A &, const B &, std::true_type)
    {
    }

    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            // Construct T from U.
            T x1{U{42}};
            REQUIRE(x1 == 42);

            // Assign U to T.
            U y1{43};
            x1 = y1;
            REQUIRE(x1 == 43);

            // Convert T to U.
            // NOTE: this is probably not strictly necessary,
            // as it is covered by switching around T and U.
            REQUIRE(static_cast<U>(x1) == 43);

            // Basic binary arithmetic.
            REQUIRE(x1 + U{4} == 47);
            REQUIRE(x1 - U{4} == 39);
            REQUIRE(x1 * U{2} == 86);
            x1 = 10;
            REQUIRE(x1 / U{2} == 5);

            // Basic in-place arithmetics.
            REQUIRE((x1 += U{1}) == 11);
            REQUIRE((x1 -= U{1}) == 10);
            REQUIRE((x1 *= U{2}) == 20);
            REQUIRE((x1 /= U{2}) == 10);

            // Exponentiation.
            REQUIRE(mppp::pow(x1, U{2}) == 100);

            // Comparison.
            REQUIRE(x1 == U{10});
            REQUIRE(x1 != U{11});

            run_ineq_cmp(x1, U{10}, std::integral_constant < bool,
                         is_mppp_complex<T>::value || is_mppp_complex<U>::value > {});
        }
        void operator()(const T &) const {}
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(mppp_types{}, runner<T>{});
    }
};

TEST_CASE("mp++ interop")
{
    tuple_for_each(mppp_types{}, mppp_interop_tester{});
}

struct mppp_cpp_interop_tester {
    template <typename A, typename B>
    static void run_ineq_cmp(const A &a, const B &b, std::false_type)
    {
        REQUIRE(!(a < b));
        REQUIRE(a <= b);
        REQUIRE(!(a > b));
        REQUIRE(a >= b);
    }
    template <typename A, typename B>
    static void run_ineq_cmp(const A &, const B &, std::true_type)
    {
    }

    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            // NOTE: here T is an mp++ type,
            // U a C++ type.

            // Construct T from U.
            T x1{U{42}};
            REQUIRE(x1 == 42);

            // Assign U to T.
            U y1{43};
            x1 = y1;
            REQUIRE(x1 == 43);

            // Convert T to U.
            REQUIRE(static_cast<U>(x1) == U{43});

            // Basic binary arithmetic.
            REQUIRE(x1 + U{4} == 47.);
            REQUIRE(x1 - U{4} == 39.);
            REQUIRE(x1 * U{2} == 86.);
            x1 = 10;
            REQUIRE(x1 / U{2} == 5.);

            // Basic in-place arithmetics.
            REQUIRE((x1 += U{1}) == 11);
            REQUIRE((x1 -= U{1}) == 10);
            REQUIRE((x1 *= U{2}) == 20);
            REQUIRE((x1 /= U{2}) == 10);

            // Exponentiation.
            // NOTE: don't check the result due to roundoff errors.
            (void)mppp::pow(x1, U{1});

            // Comparison.
            REQUIRE(x1 == U{10});
            REQUIRE(x1 != U{11});

            run_ineq_cmp(x1, U{10}, std::integral_constant < bool,
                         is_mppp_complex<T>::value || is_cpp_complex<U>::value > {});
        }
        void operator()(const T &) const {}
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(cpp_types{}, runner<T>{});
    }
};

TEST_CASE("mppp cpp interop")
{
    tuple_for_each(mppp_types{}, mppp_cpp_interop_tester{});
}

struct cpp_mppp_interop_tester {
    template <typename A, typename B>
    static void run_ineq_cmp(const A &a, const B &b, std::false_type)
    {
        REQUIRE(!(a < b));
        REQUIRE(a <= b);
        REQUIRE(!(a > b));
        REQUIRE(a >= b);
    }
    template <typename A, typename B>
    static void run_ineq_cmp(const A &, const B &, std::true_type)
    {
    }

    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            // NOTE: here T is a C++ type,
            // U an mp++ type.

            // Construct T from U.
            T x1{U{42}};
            // NOLINTNEXTLINE(clang-diagnostic-implicit-int-float-conversion)
            REQUIRE(x1 == 42.);

            // Assign U to T.
            U y1{43};
            x1 = static_cast<T>(y1);
            // NOLINTNEXTLINE(clang-diagnostic-implicit-int-float-conversion)
            REQUIRE(x1 == 43.);

            // Convert T to U.
            REQUIRE(static_cast<U>(x1) == U{43});

            // Basic binary arithmetic.
            REQUIRE(x1 + U{4} == 47.);
            REQUIRE(x1 - U{4} == 39.);
            REQUIRE(x1 * U{2} == 86.);
            x1 = 10;
            REQUIRE(x1 / U{2} == 5.);

            // Basic in-place arithmetics.
            // NOLINTNEXTLINE(clang-diagnostic-implicit-int-float-conversion)
            REQUIRE((x1 += U{1}) == 11.);
            // NOLINTNEXTLINE(clang-diagnostic-implicit-int-float-conversion)
            REQUIRE((x1 -= U{1}) == 10.);
            // NOLINTNEXTLINE(clang-diagnostic-implicit-int-float-conversion)
            REQUIRE((x1 *= U{2}) == 20.);
            // NOLINTNEXTLINE(clang-diagnostic-implicit-int-float-conversion)
            REQUIRE((x1 /= U{2}) == 10.);

            // Exponentiation.
            // NOTE: don't check the result due to roundoff errors.
            (void)mppp::pow(x1, U{1});

            // Comparison.
            REQUIRE(x1 == U{10});
            REQUIRE(x1 != U{11});

            run_ineq_cmp(x1, U{10}, std::integral_constant < bool,
                         is_cpp_complex<T>::value || is_mppp_complex<U>::value > {});
        }
        void operator()(const T &) const {}
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(mppp_types{}, runner<T>{});
    }
};

TEST_CASE("cpp mppp interop")
{
    tuple_for_each(cpp_types{}, cpp_mppp_interop_tester{});
}
