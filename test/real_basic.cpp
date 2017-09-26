// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <string>
#if MPPP_CPLUSPLUS >= 201703L
#include <string_view>
#endif
#include <utility>
#include <vector>

#include <mp++/detail/mpfr.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>
#include <mp++/real.hpp>
#if defined(MPPP_WITH_QUADMATH)
#include <mp++/real128.hpp>
#endif

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

// #if defined(_MSC_VER)

// template <typename... Args>
// auto fma_wrap(Args &&... args) -> decltype(mppp::fma(std::forward<Args>(args)...))
// {
//     return mppp::fma(std::forward<Args>(args)...);
// }

// #else

// #define fma_wrap fma

// #endif

TEST_CASE("real default prec")
{
    REQUIRE(real_get_default_prec() == 0);
    real_set_default_prec(0);
    REQUIRE(real_get_default_prec() == 0);
    real_set_default_prec(100);
    REQUIRE(real_get_default_prec() == 100);
    real_reset_default_prec();
    REQUIRE(real_get_default_prec() == 0);
    REQUIRE_THROWS_PREDICATE(real_set_default_prec(-1), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot set the default precision to -1: the value must be either zero or between "
                      + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max());
    });
    if (real_prec_min() > 1) {
        REQUIRE_THROWS_PREDICATE(real_set_default_prec(1), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == "Cannot set the default precision to 1: the value must be either zero or between "
                          + std::to_string(real_prec_min()) + " and " + std::to_string(real_prec_max());
        });
    }
    if (real_prec_max() < std::numeric_limits<::mpfr_prec_t>::max()) {
        REQUIRE_THROWS_PREDICATE(real_set_default_prec(std::numeric_limits<::mpfr_prec_t>::max()),
                                 std::invalid_argument, [](const std::invalid_argument &ex) {
                                     return ex.what()
                                            == "Cannot set the default precision to "
                                                   + std::to_string(std::numeric_limits<::mpfr_prec_t>::max())
                                                   + ": the value must be either zero or between "
                                                   + std::to_string(real_prec_min()) + " and "
                                                   + std::to_string(real_prec_max());
                                 });
    }
    REQUIRE(real_get_default_prec() == 0);
}

TEST_CASE("real constructors")
{
    // Default constructor.
    real r1;
    REQUIRE(r1.get_prec() == real_prec_min());
    REQUIRE(r1.zero_p());
    REQUIRE(!r1.signbit());
    real_set_default_prec(100);
    real r1a;
    REQUIRE(r1a.get_prec() == 100);
    REQUIRE(r1a.zero_p());
    REQUIRE(!r1a.signbit());
    // Constructor from prec.
    real r2{real_prec{42}};
    REQUIRE(r2.get_prec() == 42);
    REQUIRE(r2.nan_p());
    REQUIRE_THROWS_PREDICATE(real{real_prec{0}}, std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of 0: the maximum allowed precision is "
                      + std::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + std::to_string(real_prec_min());
    });
    REQUIRE_THROWS_PREDICATE(real{real_prec{-12}}, std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of -12: the maximum allowed precision is "
                      + std::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + std::to_string(real_prec_min());
    });
    if (real_prec_min() > 1) {
        REQUIRE_THROWS_PREDICATE(real{real_prec{1}}, std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == "Cannot init a real with a precision of 1: the maximum allowed precision is "
                          + std::to_string(real_prec_max()) + ", the minimum allowed precision is "
                          + std::to_string(real_prec_min());
        });
    }
    if (real_prec_max() < std::numeric_limits<::mpfr_prec_t>::max()) {
        REQUIRE_THROWS_PREDICATE(real{real_prec{std::numeric_limits<::mpfr_prec_t>::max()}}, std::invalid_argument,
                                 [](const std::invalid_argument &ex) {
                                     return ex.what()
                                            == "Cannot init a real with a precision of "
                                                   + std::to_string(std::numeric_limits<::mpfr_prec_t>::max())
                                                   + ": the maximum allowed precision is "
                                                   + std::to_string(real_prec_max())
                                                   + ", the minimum allowed precision is "
                                                   + std::to_string(real_prec_min());
                                 });
    }
    real_reset_default_prec();
    // Copy ctor.
    real r3{real{4}};
    REQUIRE(::mpfr_equal_p(r3.get_mpfr_t(), real{4}.get_mpfr_t()));
    REQUIRE(r3.get_prec() == real{4}.get_prec());
    real r4{real{4, 123}};
    REQUIRE(::mpfr_equal_p(r4.get_mpfr_t(), real{4, 123}.get_mpfr_t()));
    REQUIRE(r4.get_prec() == 123);
    // Copy ctor with different precision.
    real r5{real{4}, 512};
    REQUIRE(::mpfr_equal_p(r5.get_mpfr_t(), real{4}.get_mpfr_t()));
    REQUIRE(r5.get_prec() == 512);
    if (std::numeric_limits<double>::radix == 2 && std::numeric_limits<double>::digits > 12) {
        real r6{real{1.3}, 12};
        REQUIRE(!::mpfr_equal_p(r6.get_mpfr_t(), real{1.3}.get_mpfr_t()));
        REQUIRE(r6.get_prec() == 12);
    }
    if (real_prec_min() > 1) {
        REQUIRE_THROWS_PREDICATE((real{real{4}, 1}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == "Cannot init a real with a precision of 1: the maximum allowed precision is "
                          + std::to_string(real_prec_max()) + ", the minimum allowed precision is "
                          + std::to_string(real_prec_min());
        });
    }
    if (real_prec_max() < std::numeric_limits<::mpfr_prec_t>::max()) {
        REQUIRE_THROWS_PREDICATE((real{real{4}, std::numeric_limits<::mpfr_prec_t>::max()}), std::invalid_argument,
                                 [](const std::invalid_argument &ex) {
                                     return ex.what()
                                            == "Cannot init a real with a precision of "
                                                   + std::to_string(std::numeric_limits<::mpfr_prec_t>::max())
                                                   + ": the maximum allowed precision is "
                                                   + std::to_string(real_prec_max())
                                                   + ", the minimum allowed precision is "
                                                   + std::to_string(real_prec_min());
                                 });
    }
    // Move constructor.
    real r7{real{123}};
    REQUIRE(::mpfr_equal_p(r7.get_mpfr_t(), real{123}.get_mpfr_t()));
    REQUIRE(r7.get_prec() == real{123}.get_prec());
    real r8{42, 50}, r9{std::move(r8)};
    REQUIRE(::mpfr_equal_p(r9.get_mpfr_t(), real{42, 50}.get_mpfr_t()));
    REQUIRE(r9.get_prec() == 50);
    REQUIRE(r8.get_mpfr_t()->_mpfr_d == nullptr);
    // String constructors.
    REQUIRE(real_get_default_prec() == 0);
    REQUIRE((::mpfr_equal_p(real{"123", 10, 100}.get_mpfr_t(), real{123}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{std::string{"123"}, 10, 100}.get_mpfr_t(), real{123}.get_mpfr_t())));
#if MPPP_CPLUSPLUS >= 201703L
    REQUIRE((::mpfr_equal_p(real{std::string_view{"123"}, 10, 100}.get_mpfr_t(), real{123}.get_mpfr_t())));
#endif
    // Leading whitespaces are ok.
    REQUIRE((::mpfr_equal_p(real{"   123", 10, 100}.get_mpfr_t(), real{123}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{std::string{"   123"}, 10, 100}.get_mpfr_t(), real{123}.get_mpfr_t())));
#if MPPP_CPLUSPLUS >= 201703L
    REQUIRE((::mpfr_equal_p(real{std::string_view{"   123"}, 10, 100}.get_mpfr_t(), real{123}.get_mpfr_t())));
#endif
    REQUIRE((real{"123", 10, 100}.get_prec() == 100));
    REQUIRE((real{std::string{"123"}, 10, 100}.get_prec() == 100));
#if MPPP_CPLUSPLUS >= 201703L
    REQUIRE((real{std::string_view{"123"}, 10, 100}.get_prec() == 100));
#endif
    REQUIRE((::mpfr_equal_p(real{"-1.23E2", 10, 100}.get_mpfr_t(), real{-123}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{std::string{"-1.23E2"}, 10, 100}.get_mpfr_t(), real{-123}.get_mpfr_t())));
#if MPPP_CPLUSPLUS >= 201703L
    REQUIRE((::mpfr_equal_p(real{std::string_view{"-1.23E2"}, 10, 100}.get_mpfr_t(), real{-123}.get_mpfr_t())));
#endif
    if (std::numeric_limits<double>::radix == 2) {
        REQUIRE((::mpfr_equal_p(real{"5E-1", 10, 100}.get_mpfr_t(), real{.5}.get_mpfr_t())));
        REQUIRE((::mpfr_equal_p(real{"-25e-2", 10, 100}.get_mpfr_t(), real{-.25}.get_mpfr_t())));
        REQUIRE((::mpfr_equal_p(real{std::string{"-25e-2"}, 10, 100}.get_mpfr_t(), real{-.25}.get_mpfr_t())));
#if MPPP_CPLUSPLUS >= 201703L
        REQUIRE((::mpfr_equal_p(real{std::string_view{"-25e-2"}, 10, 100}.get_mpfr_t(), real{-.25}.get_mpfr_t())));
#endif
    }
    REQUIRE((::mpfr_equal_p(real{"-11120", 3, 100}.get_mpfr_t(), real{-123}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{"-11120", 3, 100}.get_mpfr_t(), real{-123}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{"1111011", 2, 100}.get_mpfr_t(), real{123}.get_mpfr_t())));
    real_set_default_prec(150);
    REQUIRE((::mpfr_equal_p(real{"123"}.get_mpfr_t(), real{123}.get_mpfr_t())));
    REQUIRE((real{"123"}.get_prec() == 150));
    REQUIRE((::mpfr_equal_p(real{"-11120", 3}.get_mpfr_t(), real{-123}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{std::string{"-11120"}, 3}.get_mpfr_t(), real{-123}.get_mpfr_t())));
#if MPPP_CPLUSPLUS >= 201703L
    REQUIRE((::mpfr_equal_p(real{std::string_view{"-11120"}, 3}.get_mpfr_t(), real{-123}.get_mpfr_t())));
#endif
    REQUIRE((real{"-11120", 3}.get_prec() == 150));
    REQUIRE((::mpfr_equal_p(real{"123", int(0), ::mpfr_prec_t(0)}.get_mpfr_t(), real{123}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{"0b1111011", int(0), ::mpfr_prec_t(0)}.get_mpfr_t(), real{123}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{"-0B1111011", int(0), ::mpfr_prec_t(0)}.get_mpfr_t(), real{-123}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{"0x7B", int(0), ::mpfr_prec_t(0)}.get_mpfr_t(), real{123}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{std::string{"0x7B"}, int(0), ::mpfr_prec_t(0)}.get_mpfr_t(), real{123}.get_mpfr_t())));
#if MPPP_CPLUSPLUS >= 201703L
    REQUIRE((
        ::mpfr_equal_p(real{std::string_view{"0x7B"}, int(0), ::mpfr_prec_t(0)}.get_mpfr_t(), real{123}.get_mpfr_t())));
#endif
    REQUIRE((::mpfr_equal_p(real{"-0X7B", int(0), ::mpfr_prec_t(0)}.get_mpfr_t(), real{-123}.get_mpfr_t())));
    REQUIRE_THROWS_PREDICATE((real{"12", -1}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string("Cannot construct a real from a string in base -1: the base must either be zero or in "
                              "the [2,62] range");
    });
    REQUIRE_THROWS_PREDICATE((real{"12", 80}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string("Cannot construct a real from a string in base 80: the base must either be zero or in "
                              "the [2,62] range");
    });
    REQUIRE_THROWS_PREDICATE((real{std::string{"12"}, -1}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string("Cannot construct a real from a string in base -1: the base must either be zero or in "
                              "the [2,62] range");
    });
#if MPPP_CPLUSPLUS >= 201703L
    REQUIRE_THROWS_PREDICATE(
        (real{std::string_view{"12"}, -1}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == std::string(
                          "Cannot construct a real from a string in base -1: the base must either be zero or in "
                          "the [2,62] range");
        });
#endif
    REQUIRE_THROWS_PREDICATE((real{std::string{"12"}, 80}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string("Cannot construct a real from a string in base 80: the base must either be zero or in "
                              "the [2,62] range");
    });
    real_reset_default_prec();
    REQUIRE_THROWS_PREDICATE((real{"12", 10}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string("Cannot construct a real from a string if the precision is not explicitly "
                              "specified and no default precision has been set");
    });
    REQUIRE_THROWS_PREDICATE((real{"123", 10, -100}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == "Cannot init a real with a precision of -100: the maximum allowed precision is "
                      + std::to_string(real_prec_max()) + ", the minimum allowed precision is "
                      + std::to_string(real_prec_min());
    });
    REQUIRE_THROWS_PREDICATE((real{"hell-o", 10, 100}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what() == std::string("The string 'hell-o' does not represent a valid real in base 10");
    });
    REQUIRE_THROWS_PREDICATE(
        (real{std::string{"123"}, 10, -100}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what()
                   == "Cannot init a real with a precision of -100: the maximum allowed precision is "
                          + std::to_string(real_prec_max()) + ", the minimum allowed precision is "
                          + std::to_string(real_prec_min());
        });
    REQUIRE_THROWS_PREDICATE(
        (real{std::string{"hell-o"}, 10, 100}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what() == std::string("The string 'hell-o' does not represent a valid real in base 10");
        });
    REQUIRE_THROWS_PREDICATE((real{"123 ", 10, 100}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what() == std::string("The string '123 ' does not represent a valid real in base 10");
    });
    REQUIRE_THROWS_PREDICATE((real{" 123 ", 10, 100}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what() == std::string("The string ' 123 ' does not represent a valid real in base 10");
    });
    const std::vector<char> vc = {',', '-', '1', '2', '3', '4'};
    REQUIRE((::mpfr_equal_p(real{vc.data() + 2, vc.data() + 6, 10, 100}.get_mpfr_t(), real{1234}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{vc.data() + 1, vc.data() + 5, 10, 100}.get_mpfr_t(), real{-123}.get_mpfr_t())));
    REQUIRE_THROWS_PREDICATE(
        (real{vc.data(), vc.data() + 6, 10, 100}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what() == std::string("The string ',-1234' does not represent a valid real in base 10");
        });
#if MPPP_CPLUSPLUS >= 201703L
    REQUIRE((::mpfr_equal_p(real{std::string_view{vc.data() + 2, 4}, 10, 100}.get_mpfr_t(), real{1234}.get_mpfr_t())));
    REQUIRE((::mpfr_equal_p(real{std::string_view{vc.data() + 1, 4}, 10, 100}.get_mpfr_t(), real{-123}.get_mpfr_t())));
    REQUIRE_THROWS_PREDICATE(
        (real{std::string_view{vc.data(), 6}, 10, 100}), std::invalid_argument, [](const std::invalid_argument &ex) {
            return ex.what() == std::string("The string ',-1234' does not represent a valid real in base 10");
        });
#endif
}

#if 0
TEST_CASE("real basic")
{
    std::cout << std::setprecision(20);
    std::cout << real{1ll} << '\n';
    std::cout << real{1.l} << '\n';
    real r{12356732};
    std::cout << r.prec_round(120) << '\n';
    std::cout << r.prec_round(12) << '\n';
    std::cout << real{true} << '\n';
    std::cout << real{integer<1>{1}} << '\n';
    std::cout << static_cast<integer<1>>(real{integer<1>{1}}) << '\n';
    std::cout << real{rational<1>{1, 3}} << '\n';
    std::cout << static_cast<rational<1>>(real{rational<1>{1, 3}}) << '\n';
    std::cout << static_cast<float>(real{rational<1>{1, 3}}) << '\n';
    std::cout << static_cast<double>(real{rational<1>{1, 3}}) << '\n';
    std::cout << static_cast<long double>(real{rational<1>{1, 3}}) << '\n';
    std::cout << static_cast<unsigned>(real{128}) << '\n';
    std::cout << static_cast<bool>(real{128}) << '\n';
    std::cout << static_cast<bool>(real{-128}) << '\n';
    std::cout << static_cast<bool>(real{0}) << '\n';
    std::cout << static_cast<int>(real{42}) << '\n';
    std::cout << static_cast<long long>(real{-42}) << '\n';
    std::cout << sqrt(r) << '\n';
    real flup{9876};
    std::cout << sqrt(std::move(flup)) << '\n';
    std::cout << fma_wrap(real{1}, real{2}, real{3}) << '\n';
#if defined(MPPP_WITH_QUADMATH)
    std::cout << static_cast<real128>(real{-42}) << '\n';
    std::cout << static_cast<real128>(real{-1}) << '\n';
    std::cout << static_cast<real128>(real{1}) << '\n';
    std::cout << static_cast<real128>(real{real128{"3.40917866435610111081769936359662259e-4957"}}) << '\n';
    std::cout << static_cast<real128>(real{real128{"3.40917866435610111081769936359662259e-4957"}}.prec_round(127))
              << '\n';
    std::cout << static_cast<real128>(real{real128{"3.40917866435610111081769936359662259e-4957"}}.prec_round(128))
              << '\n';
    std::cout << static_cast<real128>(real{real128{"3.40917866435610111081769936359662259e-4957"}}.prec_round(129))
              << '\n';
    std::cout << static_cast<real128>(real{real128{"3.40917866435610111081769936359662259e-4957"}}.prec_round(250))
              << '\n';
    std::cout << real{-real128{"1.3E200"}} << '\n';
    std::cout << -real128{"1.3E200"} << '\n';
    std::cout << real{-real128{"1.3E-200"}} << '\n';
    std::cout << -real128{"1.3E-200"} << '\n';
    std::cout << real128{"1E-4940"} << '\n';
    std::cout << real{real128{"1E-4940"}} << '\n';
#endif
}
#endif
