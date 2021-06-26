// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>

#include <mp++/mp++.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp_test;

using uint_types = std::tuple<unsigned char, unsigned short, unsigned, unsigned long, unsigned long long
#if defined(MPPP_HAVE_GCC_INT128)
                              ,
                              __uint128_t
#endif
                              >;

using sint_types = std::tuple<signed char, short, int, long, long long
#if defined(MPPP_HAVE_GCC_INT128)
                              ,
                              __int128_t
#endif
                              >;

struct uint_uint_safe_cast_tester {
    template <typename T>
    struct runner {
        template <typename U>
        struct enabler {
            static const bool value = detail::nl_max<T>() <= detail::nl_max<U>();
        };
        template <typename U, detail::enable_if_t<enabler<U>::value, int> = 0>
        void operator()(const U &) const
        {
        }
        template <typename U, detail::enable_if_t<!enabler<U>::value, int> = 0>
        void operator()(const U &) const
        {
            REQUIRE((std::is_same<T, decltype(detail::safe_cast<T>(U(0)))>::value));
            REQUIRE(detail::safe_cast<T>(U(0)) == 0u);
            REQUIRE(detail::safe_cast<T>(U(2)) == 2u);
            REQUIRE(detail::safe_cast<T>(detail::nl_max<U>()) == detail::nl_max<U>());
            REQUIRE(detail::safe_cast<T>(U(detail::nl_max<U>() - 1u)) == detail::nl_max<U>() - 1u);
            REQUIRE_THROWS_PREDICATE(
                detail::safe_cast<U>(T(T(detail::nl_max<U>()) + 1u)), std::overflow_error,
                [](const std::overflow_error &oe) {
                    return std::string(oe.what())
                           == "Error in the safe conversion between unsigned integral types: the input value "
                                  + detail::to_string(T(detail::nl_max<U>()) + 1u)
                                  + " does not fit in the range of the target type '" + type_name<U>() + "'";
                });
            REQUIRE_THROWS_PREDICATE(
                detail::safe_cast<U>(detail::nl_max<T>()), std::overflow_error, [](const std::overflow_error &oe) {
                    return std::string(oe.what())
                           == "Error in the safe conversion between unsigned integral types: the input value "
                                  + detail::to_string(detail::nl_max<T>())
                                  + " does not fit in the range of the target type '" + type_name<U>() + "'";
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
            static const bool value
                = detail::nl_max<T>() <= detail::nl_max<U>() && detail::nl_min<T>() >= detail::nl_min<U>();
        };
        template <typename U, detail::enable_if_t<enabler<U>::value, int> = 0>
        void operator()(const U &) const
        {
        }
        template <typename U, detail::enable_if_t<!enabler<U>::value, int> = 0>
        void operator()(const U &) const
        {
            REQUIRE((std::is_same<T, decltype(detail::safe_cast<T>(U(0)))>::value));
            REQUIRE(detail::safe_cast<T>(U(0)) == 0);
            REQUIRE(detail::safe_cast<T>(U(2)) == 2);
            REQUIRE(detail::safe_cast<T>(U(-2)) == -2);
            REQUIRE(detail::safe_cast<T>(detail::nl_max<U>()) == detail::nl_max<U>());
            REQUIRE(detail::safe_cast<T>(U(detail::nl_max<U>() - 1)) == detail::nl_max<U>() - 1);
            REQUIRE(detail::safe_cast<T>(U(detail::nl_min<U>() + 1)) == detail::nl_min<U>() + 1);
            REQUIRE_THROWS_PREDICATE(
                detail::safe_cast<U>(T(T(detail::nl_max<U>()) + 1)), std::overflow_error,
                [](const std::overflow_error &oe) {
                    return std::string(oe.what())
                           == "Error in the safe conversion between signed integral types: the input value "
                                  + detail::to_string(T(detail::nl_max<U>()) + 1)
                                  + " does not fit in the range of the target type '" + type_name<U>() + "'";
                });
            REQUIRE_THROWS_PREDICATE(
                detail::safe_cast<U>(T(T(detail::nl_min<U>()) - 1)), std::overflow_error,
                [](const std::overflow_error &oe) {
                    return std::string(oe.what())
                           == "Error in the safe conversion between signed integral types: the input value "
                                  + detail::to_string(T(detail::nl_min<U>()) - 1)
                                  + " does not fit in the range of the target type '" + type_name<U>() + "'";
                });
            REQUIRE_THROWS_PREDICATE(
                detail::safe_cast<U>(detail::nl_max<T>()), std::overflow_error, [](const std::overflow_error &oe) {
                    return std::string(oe.what())
                           == "Error in the safe conversion between signed integral types: the input value "
                                  + detail::to_string(detail::nl_max<T>())
                                  + " does not fit in the range of the target type '" + type_name<U>() + "'";
                });
            REQUIRE_THROWS_PREDICATE(
                detail::safe_cast<U>(detail::nl_min<T>()), std::overflow_error, [](const std::overflow_error &oe) {
                    return std::string(oe.what())
                           == "Error in the safe conversion between signed integral types: the input value "
                                  + detail::to_string(detail::nl_min<T>())
                                  + " does not fit in the range of the target type '" + type_name<U>() + "'";
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
            using uS = detail::make_unsigned_t<S>;
            REQUIRE((std::is_same<U, decltype(detail::safe_cast<U>(S(0)))>::value));
            REQUIRE(detail::safe_cast<U>(S(0)) == U(0));
            REQUIRE_THROWS_PREDICATE(
                detail::safe_cast<U>(S(-1)), std::overflow_error, [](const std::overflow_error &oe) {
                    return std::string(oe.what())
                           == "Error in the safe conversion from a signed integral type to an unsigned integral type: "
                              "the "
                              "input value "
                                  + detail::to_string(S(-1)) + " does not fit in the range of the target type '"
                                  + type_name<U>() + "'";
                });
            if (uS(detail::nl_max<S>()) > detail::nl_max<U>()) {
                REQUIRE(detail::safe_cast<U>(S(detail::nl_max<U>())) == detail::nl_max<U>());
                REQUIRE_THROWS_PREDICATE(detail::safe_cast<U>(S(S(detail::nl_max<U>()) + 1)), std::overflow_error,
                                         [](const std::overflow_error &oe) {
                                             return std::string(oe.what())
                                                    == "Error in the safe conversion from a signed integral type to an "
                                                       "unsigned integral type: the input value "
                                                           + detail::to_string(S(S(detail::nl_max<U>()) + 1))
                                                           + " does not fit in the range of the target type '"
                                                           + type_name<U>() + "'";
                                         });
                REQUIRE_THROWS_PREDICATE(
                    detail::safe_cast<U>(detail::nl_max<S>()), std::overflow_error, [](const std::overflow_error &oe) {
                        return std::string(oe.what())
                               == "Error in the safe conversion from a signed integral type to an "
                                  "unsigned integral type: the input value "
                                      + detail::to_string(detail::nl_max<S>())
                                      + " does not fit in the range of the target type '" + type_name<U>() + "'";
                    });
            } else {
                REQUIRE(detail::safe_cast<U>(detail::nl_max<S>()) == uS(detail::nl_max<S>()));
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
            using uS = detail::make_unsigned_t<S>;
            REQUIRE((std::is_same<S, decltype(detail::safe_cast<S>(U(0)))>::value));
            REQUIRE(detail::safe_cast<S>(U(0)) == S(0));
            REQUIRE(detail::safe_cast<S>(U(10)) == S(10));
            if (detail::nl_max<U>() > uS(detail::nl_max<S>())) {
                REQUIRE(detail::safe_cast<S>(U(detail::nl_max<S>())) == detail::nl_max<S>());
                REQUIRE_THROWS_PREDICATE(
                    detail::safe_cast<S>(U(U(detail::nl_max<S>()) + 1u)), std::overflow_error,
                    [](const std::overflow_error &oe) {
                        return std::string(oe.what())
                               == "Error in the safe conversion from an unsigned integral type to a "
                                  "signed integral type: the input value "
                                      + detail::to_string(U(U(detail::nl_max<S>()) + 1))
                                      + " does not fit in the range of the target type '" + type_name<S>() + "'";
                    });
                REQUIRE_THROWS_PREDICATE(
                    detail::safe_cast<S>(detail::nl_max<U>()), std::overflow_error, [](const std::overflow_error &oe) {
                        return std::string(oe.what())
                               == "Error in the safe conversion from an unsigned integral type to a "
                                  "signed integral type: the input value "
                                      + detail::to_string(detail::nl_max<U>())
                                      + " does not fit in the range of the target type '" + type_name<S>() + "'";
                    });
            } else {
                REQUIRE(uS(detail::safe_cast<S>(detail::nl_max<U>())) == detail::nl_max<U>());
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
    REQUIRE(detail::to_string(__uint128_t(0)) == "0");
    REQUIRE(detail::to_string(__uint128_t(1)) == "1");
    REQUIRE(detail::to_string(__uint128_t(7)) == "7");
    REQUIRE(detail::to_string(__uint128_t(9)) == "9");
    REQUIRE(detail::to_string(__uint128_t(10)) == "10");
    REQUIRE(detail::to_string(__uint128_t(11)) == "11");
    REQUIRE(detail::to_string(__uint128_t(12)) == "12");
    REQUIRE(detail::to_string(__uint128_t(19)) == "19");
    REQUIRE(detail::to_string(__uint128_t(909)) == "909");
    REQUIRE(detail::to_string(__uint128_t(910)) == "910");
    REQUIRE(detail::to_string(__uint128_t(911)) == "911");
    REQUIRE(detail::to_string(__uint128_t(999)) == "999");
    REQUIRE(detail::to_string(__uint128_t(1000)) == "1000");
    REQUIRE(detail::to_string(__uint128_t(9999)) == "9999");
    REQUIRE(detail::to_string(__uint128_t(10000)) == "10000");
    REQUIRE(detail::to_string(__uint128_t(18446744073709551615ull)) == "18446744073709551615");
    REQUIRE(detail::to_string(detail::nl_max<__uint128_t>()) == "340282366920938463463374607431768211455");
    REQUIRE(detail::to_string(__int128_t(0)) == "0");
    REQUIRE(detail::to_string(__int128_t(1)) == "1");
    REQUIRE(detail::to_string(__int128_t(7)) == "7");
    REQUIRE(detail::to_string(__int128_t(9)) == "9");
    REQUIRE(detail::to_string(__int128_t(10)) == "10");
    REQUIRE(detail::to_string(__int128_t(11)) == "11");
    REQUIRE(detail::to_string(__int128_t(12)) == "12");
    REQUIRE(detail::to_string(__int128_t(19)) == "19");
    REQUIRE(detail::to_string(__int128_t(909)) == "909");
    REQUIRE(detail::to_string(__int128_t(910)) == "910");
    REQUIRE(detail::to_string(__int128_t(911)) == "911");
    REQUIRE(detail::to_string(__int128_t(999)) == "999");
    REQUIRE(detail::to_string(__int128_t(1000)) == "1000");
    REQUIRE(detail::to_string(__int128_t(9999)) == "9999");
    REQUIRE(detail::to_string(__int128_t(10000)) == "10000");
    REQUIRE(detail::to_string(__int128_t(-1)) == "-1");
    REQUIRE(detail::to_string(__int128_t(-7)) == "-7");
    REQUIRE(detail::to_string(__int128_t(-9)) == "-9");
    REQUIRE(detail::to_string(__int128_t(-10)) == "-10");
    REQUIRE(detail::to_string(__int128_t(-11)) == "-11");
    REQUIRE(detail::to_string(__int128_t(-12)) == "-12");
    REQUIRE(detail::to_string(__int128_t(-19)) == "-19");
    REQUIRE(detail::to_string(__int128_t(-909)) == "-909");
    REQUIRE(detail::to_string(__int128_t(-910)) == "-910");
    REQUIRE(detail::to_string(__int128_t(-911)) == "-911");
    REQUIRE(detail::to_string(__int128_t(-999)) == "-999");
    REQUIRE(detail::to_string(__int128_t(-1000)) == "-1000");
    REQUIRE(detail::to_string(__int128_t(-9999)) == "-9999");
    REQUIRE(detail::to_string(__int128_t(-10000)) == "-10000");
    REQUIRE(detail::to_string(__int128_t(18446744073709551615ull)) == "18446744073709551615");
    REQUIRE(detail::to_string(-__int128_t(18446744073709551615ull)) == "-18446744073709551615");
    REQUIRE(detail::to_string(detail::nl_max<__int128_t>()) == "170141183460469231731687303715884105727");
    REQUIRE(detail::to_string(detail::nl_max<__int128_t>() - 25) == "170141183460469231731687303715884105702");
    REQUIRE(detail::to_string(detail::nl_min<__int128_t>()) == "-170141183460469231731687303715884105728");
    REQUIRE(detail::to_string(detail::nl_min<__int128_t>() + 25) == "-170141183460469231731687303715884105703");
}

#endif
