// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <limits>
#include <mp++/mp++.hpp>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>

#include "test_utils.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;
using namespace mppp_test;

using uint_types = std::tuple<unsigned char, unsigned short, unsigned, unsigned long, unsigned long long>;
using sint_types = std::tuple<signed char, short, int, long, long long>;

struct uint_uint_safe_cast_tester {
    template <typename T>
    struct runner {
        template <typename U>
        struct enabler {
            static const bool value = nl_max<T>() <= nl_max<U>();
        };
        template <typename U, enable_if_t<enabler<U>::value, int> = 0>
        void operator()(const U &) const
        {
        }
        template <typename U, enable_if_t<!enabler<U>::value, int> = 0>
        void operator()(const U &) const
        {
            REQUIRE((std::is_same<T, decltype(safe_cast<T>(U(0)))>::value));
            REQUIRE(safe_cast<T>(U(0)) == 0u);
            REQUIRE(safe_cast<T>(U(2)) == 2u);
            REQUIRE(safe_cast<T>(nl_max<U>()) == nl_max<U>());
            REQUIRE(safe_cast<T>(U(nl_max<U>() - 1u)) == nl_max<U>() - 1u);
            REQUIRE_THROWS_PREDICATE(
                safe_cast<U>(T(T(nl_max<U>()) + 1u)), std::overflow_error, [](const std::overflow_error &oe) {
                    return std::string(oe.what())
                           == "Error in the safe conversion between unsigned integral types: the input value "
                                  + std::to_string(T(nl_max<U>()) + 1u)
                                  + " does not fit in the range of the target type " + type_string<U>();
                });
            REQUIRE_THROWS_PREDICATE(safe_cast<U>(nl_max<T>()), std::overflow_error, [](const std::overflow_error &oe) {
                return std::string(oe.what())
                       == "Error in the safe conversion between unsigned integral types: the input value "
                              + std::to_string(nl_max<T>()) + " does not fit in the range of the target type "
                              + type_string<U>();
            });
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(uint_types{}, runner<T>{});
    }
};

TEST_CASE("uint_uint_safe_cast")
{
    tuple_for_each(uint_types{}, uint_uint_safe_cast_tester{});
}

struct sint_sint_safe_cast_tester {
    template <typename T>
    struct runner {
        template <typename U>
        struct enabler {
            static const bool value = nl_max<T>() <= nl_max<U>() && nl_min<T>() >= nl_min<U>();
        };
        template <typename U, enable_if_t<enabler<U>::value, int> = 0>
        void operator()(const U &) const
        {
        }
        template <typename U, enable_if_t<!enabler<U>::value, int> = 0>
        void operator()(const U &) const
        {
            REQUIRE((std::is_same<T, decltype(safe_cast<T>(U(0)))>::value));
            REQUIRE(safe_cast<T>(U(0)) == 0);
            REQUIRE(safe_cast<T>(U(2)) == 2);
            REQUIRE(safe_cast<T>(U(-2)) == -2);
            REQUIRE(safe_cast<T>(nl_max<U>()) == nl_max<U>());
            REQUIRE(safe_cast<T>(U(nl_max<U>() - 1)) == nl_max<U>() - 1);
            REQUIRE(safe_cast<T>(U(nl_min<U>() + 1)) == nl_min<U>() + 1);
            REQUIRE_THROWS_PREDICATE(
                safe_cast<U>(T(T(nl_max<U>()) + 1)), std::overflow_error, [](const std::overflow_error &oe) {
                    return std::string(oe.what())
                           == "Error in the safe conversion between signed integral types: the input value "
                                  + std::to_string(T(nl_max<U>()) + 1)
                                  + " does not fit in the range of the target type " + type_string<U>();
                });
            REQUIRE_THROWS_PREDICATE(
                safe_cast<U>(T(T(nl_min<U>()) - 1)), std::overflow_error, [](const std::overflow_error &oe) {
                    return std::string(oe.what())
                           == "Error in the safe conversion between signed integral types: the input value "
                                  + std::to_string(T(nl_min<U>()) - 1)
                                  + " does not fit in the range of the target type " + type_string<U>();
                });
            REQUIRE_THROWS_PREDICATE(safe_cast<U>(nl_max<T>()), std::overflow_error, [](const std::overflow_error &oe) {
                return std::string(oe.what())
                       == "Error in the safe conversion between signed integral types: the input value "
                              + std::to_string(nl_max<T>()) + " does not fit in the range of the target type "
                              + type_string<U>();
            });
            REQUIRE_THROWS_PREDICATE(safe_cast<U>(nl_min<T>()), std::overflow_error, [](const std::overflow_error &oe) {
                return std::string(oe.what())
                       == "Error in the safe conversion between signed integral types: the input value "
                              + std::to_string(nl_min<T>()) + " does not fit in the range of the target type "
                              + type_string<U>();
            });
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(sint_types{}, runner<T>{});
    }
};

TEST_CASE("sint_sint_safe_cast")
{
    tuple_for_each(sint_types{}, sint_sint_safe_cast_tester{});
}

struct sint_uint_safe_cast_tester {
    template <typename S>
    struct runner {
        template <typename U>
        void operator()(const U &) const
        {
            using uS = make_unsigned_t<S>;
            REQUIRE((std::is_same<U, decltype(safe_cast<U>(S(0)))>::value));
            REQUIRE(safe_cast<U>(S(0)) == U(0));
            REQUIRE_THROWS_PREDICATE(safe_cast<U>(S(-1)), std::overflow_error, [](const std::overflow_error &oe) {
                return std::string(oe.what())
                       == "Error in the safe conversion from a signed integral type to an unsigned integral type: the "
                          "input value "
                              + std::to_string(S(-1)) + " does not fit in the range of the target type "
                              + type_string<U>();
            });
            if (uS(nl_max<S>()) > nl_max<U>()) {
                REQUIRE(safe_cast<U>(S(nl_max<U>())) == nl_max<U>());
                REQUIRE_THROWS_PREDICATE(
                    safe_cast<U>(S(S(nl_max<U>()) + 1)), std::overflow_error, [](const std::overflow_error &oe) {
                        return std::string(oe.what())
                               == "Error in the safe conversion from a signed integral type to an "
                                  "unsigned integral type: the input value "
                                      + std::to_string(S(S(nl_max<U>()) + 1))
                                      + " does not fit in the range of the target type " + type_string<U>();
                    });
                REQUIRE_THROWS_PREDICATE(
                    safe_cast<U>(nl_max<S>()), std::overflow_error, [](const std::overflow_error &oe) {
                        return std::string(oe.what())
                               == "Error in the safe conversion from a signed integral type to an "
                                  "unsigned integral type: the input value "
                                      + std::to_string(nl_max<S>()) + " does not fit in the range of the target type "
                                      + type_string<U>();
                    });
            } else {
                REQUIRE(safe_cast<U>(nl_max<S>()) == uS(nl_max<S>()));
            }
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(uint_types{}, runner<T>{});
    }
};

TEST_CASE("sint_uint_safe_cast")
{
    tuple_for_each(sint_types{}, sint_uint_safe_cast_tester{});
}

struct uint_sint_safe_cast_tester {
    template <typename U>
    struct runner {
        template <typename S>
        void operator()(const S &) const
        {
            using uS = make_unsigned_t<S>;
            REQUIRE((std::is_same<S, decltype(safe_cast<S>(U(0)))>::value));
            REQUIRE(safe_cast<S>(U(0)) == S(0));
            REQUIRE(safe_cast<S>(U(10)) == S(10));
            if (nl_max<U>() > uS(nl_max<S>())) {
                REQUIRE(safe_cast<S>(U(nl_max<S>())) == nl_max<S>());
                REQUIRE_THROWS_PREDICATE(
                    safe_cast<S>(U(U(nl_max<S>()) + 1u)), std::overflow_error, [](const std::overflow_error &oe) {
                        return std::string(oe.what())
                               == "Error in the safe conversion from an unsigned integral type to a "
                                  "signed integral type: the input value "
                                      + std::to_string(U(U(nl_max<S>()) + 1))
                                      + " does not fit in the range of the target type " + type_string<S>();
                    });
                REQUIRE_THROWS_PREDICATE(
                    safe_cast<S>(nl_max<U>()), std::overflow_error, [](const std::overflow_error &oe) {
                        return std::string(oe.what())
                               == "Error in the safe conversion from an unsigned integral type to a "
                                  "signed integral type: the input value "
                                      + std::to_string(nl_max<U>()) + " does not fit in the range of the target type "
                                      + type_string<S>();
                    });
            } else {
                REQUIRE(uS(safe_cast<S>(nl_max<U>())) == nl_max<U>());
            }
        }
    };
    template <typename T>
    void operator()(const T &) const
    {
        tuple_for_each(sint_types{}, runner<T>{});
    }
};

TEST_CASE("uint_sint_safe_cast")
{
    tuple_for_each(uint_types{}, uint_sint_safe_cast_tester{});
}

#if defined(MPPP_HAVE_GCC_INT128)

TEST_CASE("int128 to_string")
{
    REQUIRE(to_string(__uint128_t(0)) == "0");
    REQUIRE(to_string(__uint128_t(1)) == "1");
    REQUIRE(to_string(__uint128_t(7)) == "7");
    REQUIRE(to_string(__uint128_t(9)) == "9");
    REQUIRE(to_string(__uint128_t(10)) == "10");
    REQUIRE(to_string(__uint128_t(11)) == "11");
    REQUIRE(to_string(__uint128_t(12)) == "12");
    REQUIRE(to_string(__uint128_t(19)) == "19");
    REQUIRE(to_string(__uint128_t(909)) == "909");
    REQUIRE(to_string(__uint128_t(910)) == "910");
    REQUIRE(to_string(__uint128_t(911)) == "911");
    REQUIRE(to_string(__uint128_t(999)) == "999");
    REQUIRE(to_string(__uint128_t(1000)) == "1000");
    REQUIRE(to_string(__uint128_t(9999)) == "9999");
    REQUIRE(to_string(__uint128_t(10000)) == "10000");
    REQUIRE(to_string(__uint128_t(18446744073709551615ull)) == "18446744073709551615");
    REQUIRE(to_string(nl_max<__uint128_t>()) == "340282366920938463463374607431768211455");
    REQUIRE(to_string(__int128_t(0)) == "0");
    REQUIRE(to_string(__int128_t(1)) == "1");
    REQUIRE(to_string(__int128_t(7)) == "7");
    REQUIRE(to_string(__int128_t(9)) == "9");
    REQUIRE(to_string(__int128_t(10)) == "10");
    REQUIRE(to_string(__int128_t(11)) == "11");
    REQUIRE(to_string(__int128_t(12)) == "12");
    REQUIRE(to_string(__int128_t(19)) == "19");
    REQUIRE(to_string(__int128_t(909)) == "909");
    REQUIRE(to_string(__int128_t(910)) == "910");
    REQUIRE(to_string(__int128_t(911)) == "911");
    REQUIRE(to_string(__int128_t(999)) == "999");
    REQUIRE(to_string(__int128_t(1000)) == "1000");
    REQUIRE(to_string(__int128_t(9999)) == "9999");
    REQUIRE(to_string(__int128_t(10000)) == "10000");
    REQUIRE(to_string(__int128_t(-1)) == "-1");
    REQUIRE(to_string(__int128_t(-7)) == "-7");
    REQUIRE(to_string(__int128_t(-9)) == "-9");
    REQUIRE(to_string(__int128_t(-10)) == "-10");
    REQUIRE(to_string(__int128_t(-11)) == "-11");
    REQUIRE(to_string(__int128_t(-12)) == "-12");
    REQUIRE(to_string(__int128_t(-19)) == "-19");
    REQUIRE(to_string(__int128_t(-909)) == "-909");
    REQUIRE(to_string(__int128_t(-910)) == "-910");
    REQUIRE(to_string(__int128_t(-911)) == "-911");
    REQUIRE(to_string(__int128_t(-999)) == "-999");
    REQUIRE(to_string(__int128_t(-1000)) == "-1000");
    REQUIRE(to_string(__int128_t(-9999)) == "-9999");
    REQUIRE(to_string(__int128_t(-10000)) == "-10000");
    REQUIRE(to_string(__int128_t(18446744073709551615ull)) == "18446744073709551615");
    REQUIRE(to_string(-__int128_t(18446744073709551615ull)) == "-18446744073709551615");
    REQUIRE(to_string(nl_max<__int128_t>()) == "170141183460469231731687303715884105727");
    REQUIRE(to_string(nl_max<__int128_t>() - 25) == "170141183460469231731687303715884105702");
    REQUIRE(to_string(nl_min<__int128_t>()) == "-170141183460469231731687303715884105728");
    REQUIRE(to_string(nl_min<__int128_t>() + 25) == "-170141183460469231731687303715884105703");
}

#endif
