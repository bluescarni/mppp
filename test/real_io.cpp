// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <limits>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real.hpp>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;
using namespace mppp_test;

using int_types = std::tuple<char, signed char, unsigned char, short, unsigned short, int, unsigned, long,
                             unsigned long, long long, unsigned long long>;

using fp_types = std::tuple<float, double, long double>;

using int_t = integer<1>;
using rat_t = rational<1>;

static std::mt19937 rng;

static const int ntrials = 1000;

// NOTE: char types are not supported in uniform_int_distribution by the standard.
// Use a small wrapper to get an int distribution instead, with the min max limits
// from the char type. We will be casting back when using the distribution.
template <typename T, typename std::enable_if<!(std::is_same<char, T>::value || std::is_same<signed char, T>::value
                                                || std::is_same<unsigned char, T>::value),
                                              int>::type
                      = 0>
static inline std::uniform_int_distribution<T> get_int_dist(T min, T max)
{
    return std::uniform_int_distribution<T>(min, max);
}

template <typename T, typename std::enable_if<std::is_same<char, T>::value || std::is_same<signed char, T>::value
                                                  || std::is_same<unsigned char, T>::value,
                                              int>::type
                      = 0>
static inline std::uniform_int_distribution<typename std::conditional<is_signed<T>::value, int, unsigned>::type>
get_int_dist(T min, T max)
{
    return std::uniform_int_distribution<typename std::conditional<is_signed<T>::value, int, unsigned>::type>(min, max);
}

struct int_io_tester {
    template <typename T>
    void operator()(const T &) const
    {
        auto int_dist = get_int_dist(nl_min<T>(), nl_max<T>());
        auto prec_dist = get_int_dist(::mpfr_prec_t(real_prec_min()), ::mpfr_prec_t(200));
        auto base_dist = get_int_dist(2, 62);
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
        auto prec_dist = get_int_dist(::mpfr_prec_t(real_prec_min()), ::mpfr_prec_t(200));
        auto base_dist = get_int_dist(2, 62);
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
    {
        std::stringstream iss;
        iss.str("-1.1");
        real_set_default_prec(110);
        real r;
        iss >> r;
        real_reset_default_prec();
        REQUIRE(::mpfr_equal_p(real{"-1.1", 110}.get_mpfr_t(), r.get_mpfr_t()));
    }
}
