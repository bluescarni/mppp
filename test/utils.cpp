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
#include <typeinfo>

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
        template <typename U, enable_if_t<(std::numeric_limits<T>::max() <= std::numeric_limits<U>::max()), int> = 0>
        void operator()(const U &) const
        {
        }
        template <typename U, enable_if_t<(std::numeric_limits<T>::max() > std::numeric_limits<U>::max()), int> = 0>
        void operator()(const U &) const
        {
            REQUIRE((std::is_same<T, decltype(safe_cast<T>(U(0)))>::value));
            REQUIRE(safe_cast<T>(U(0)) == 0u);
            REQUIRE(safe_cast<T>(U(2)) == 2u);
            REQUIRE(safe_cast<T>(std::numeric_limits<U>::max()) == std::numeric_limits<U>::max());
            REQUIRE(safe_cast<T>(U(std::numeric_limits<U>::max() - 1u)) == std::numeric_limits<U>::max() - 1u);
            REQUIRE_THROWS_PREDICATE(
                safe_cast<U>(T(T(std::numeric_limits<U>::max()) + 1u)), std::overflow_error,
                [](const std::overflow_error &oe) {
                    return std::string(oe.what())
                           == "Error in the safe conversion between unsigned integral types: the input value "
                                  + std::to_string(T(std::numeric_limits<U>::max()) + 1u)
                                  + " does not fit in the range of the target type " + typeid(U).name();
                });
            REQUIRE_THROWS_PREDICATE(
                safe_cast<U>(std::numeric_limits<T>::max()), std::overflow_error, [](const std::overflow_error &oe) {
                    return std::string(oe.what())
                           == "Error in the safe conversion between unsigned integral types: the input value "
                                  + std::to_string(std::numeric_limits<T>::max())
                                  + " does not fit in the range of the target type " + typeid(U).name();
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
        template <typename U,
                  enable_if_t<(std::numeric_limits<T>::max() <= std::numeric_limits<U>::max()
                               && std::numeric_limits<T>::min() >= std::numeric_limits<U>::min()),
                              int> = 0>
        void operator()(const U &) const
        {
        }
        template <typename U,
                  enable_if_t<(std::numeric_limits<T>::max() > std::numeric_limits<U>::max()
                               && std::numeric_limits<T>::min() < std::numeric_limits<U>::min()),
                              int> = 0>
        void operator()(const U &) const
        {
            REQUIRE((std::is_same<T, decltype(safe_cast<T>(U(0)))>::value));
            REQUIRE(safe_cast<T>(U(0)) == 0);
            REQUIRE(safe_cast<T>(U(2)) == 2);
            REQUIRE(safe_cast<T>(U(-2)) == -2);
            REQUIRE(safe_cast<T>(std::numeric_limits<U>::max()) == std::numeric_limits<U>::max());
            REQUIRE(safe_cast<T>(U(std::numeric_limits<U>::max() - 1)) == std::numeric_limits<U>::max() - 1);
            REQUIRE(safe_cast<T>(U(std::numeric_limits<U>::min() + 1)) == std::numeric_limits<U>::min() + 1);
            REQUIRE_THROWS_PREDICATE(
                safe_cast<U>(T(T(std::numeric_limits<U>::max()) + 1)), std::overflow_error,
                [](const std::overflow_error &oe) {
                    return std::string(oe.what())
                           == "Error in the safe conversion between signed integral types: the input value "
                                  + std::to_string(T(std::numeric_limits<U>::max()) + 1)
                                  + " does not fit in the range of the target type " + typeid(U).name();
                });
            REQUIRE_THROWS_PREDICATE(
                safe_cast<U>(T(T(std::numeric_limits<U>::min()) - 1)), std::overflow_error,
                [](const std::overflow_error &oe) {
                    return std::string(oe.what())
                           == "Error in the safe conversion between signed integral types: the input value "
                                  + std::to_string(T(std::numeric_limits<U>::min()) - 1)
                                  + " does not fit in the range of the target type " + typeid(U).name();
                });
            REQUIRE_THROWS_PREDICATE(
                safe_cast<U>(std::numeric_limits<T>::max()), std::overflow_error, [](const std::overflow_error &oe) {
                    return std::string(oe.what())
                           == "Error in the safe conversion between signed integral types: the input value "
                                  + std::to_string(std::numeric_limits<T>::max())
                                  + " does not fit in the range of the target type " + typeid(U).name();
                });
            REQUIRE_THROWS_PREDICATE(
                safe_cast<U>(std::numeric_limits<T>::min()), std::overflow_error, [](const std::overflow_error &oe) {
                    return std::string(oe.what())
                           == "Error in the safe conversion between signed integral types: the input value "
                                  + std::to_string(std::numeric_limits<T>::min())
                                  + " does not fit in the range of the target type " + typeid(U).name();
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
            using uS = make_unsigned<S>;
            REQUIRE((std::is_same<U, decltype(safe_cast<U>(S(0)))>::value));
            REQUIRE(safe_cast<U>(S(0)) == U(0));
            REQUIRE_THROWS_PREDICATE(safe_cast<U>(S(-1)), std::overflow_error, [](const std::overflow_error &oe) {
                return std::string(oe.what())
                       == "Error in the safe conversion from a signed integral type to an unsigned integral type: the "
                          "input value "
                              + std::to_string(S(-1)) + " does not fit in the range of the target type "
                              + typeid(U).name();
            });
            if (uS(std::numeric_limits<S>::max()) > std::numeric_limits<U>::max()) {
                REQUIRE(safe_cast<U>(S(std::numeric_limits<U>::max())) == std::numeric_limits<U>::max());
                REQUIRE_THROWS_PREDICATE(safe_cast<U>(S(S(std::numeric_limits<U>::max()) + 1)), std::overflow_error,
                                         [](const std::overflow_error &oe) {
                                             return std::string(oe.what())
                                                    == "Error in the safe conversion from a signed integral type to an "
                                                       "unsigned integral type: the input value "
                                                           + std::to_string(S(S(std::numeric_limits<U>::max()) + 1))
                                                           + " does not fit in the range of the target type "
                                                           + typeid(U).name();
                                         });
                REQUIRE_THROWS_PREDICATE(safe_cast<U>(std::numeric_limits<S>::max()), std::overflow_error,
                                         [](const std::overflow_error &oe) {
                                             return std::string(oe.what())
                                                    == "Error in the safe conversion from a signed integral type to an "
                                                       "unsigned integral type: the input value "
                                                           + std::to_string(std::numeric_limits<S>::max())
                                                           + " does not fit in the range of the target type "
                                                           + typeid(U).name();
                                         });
            } else {
                REQUIRE(safe_cast<U>(std::numeric_limits<S>::max()) == uS(std::numeric_limits<S>::max()));
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
            using uS = make_unsigned<S>;
            REQUIRE((std::is_same<S, decltype(safe_cast<S>(U(0)))>::value));
            REQUIRE(safe_cast<S>(U(0)) == S(0));
            REQUIRE(safe_cast<S>(U(10)) == S(10));
            if (std::numeric_limits<U>::max() > uS(std::numeric_limits<S>::max())) {
                REQUIRE(safe_cast<S>(U(std::numeric_limits<S>::max())) == std::numeric_limits<S>::max());
                REQUIRE_THROWS_PREDICATE(
                    safe_cast<S>(U(U(std::numeric_limits<S>::max()) + 1u)), std::overflow_error,
                    [](const std::overflow_error &oe) {
                        return std::string(oe.what())
                               == "Error in the safe conversion from an unsigned integral type to a "
                                  "signed integral type: the input value "
                                      + std::to_string(U(U(std::numeric_limits<S>::max()) + 1))
                                      + " does not fit in the range of the target type " + typeid(S).name();
                    });
                REQUIRE_THROWS_PREDICATE(
                    safe_cast<S>(std::numeric_limits<U>::max()), std::overflow_error,
                    [](const std::overflow_error &oe) {
                        return std::string(oe.what())
                               == "Error in the safe conversion from an unsigned integral type to a "
                                  "signed integral type: the input value "
                                      + std::to_string(std::numeric_limits<U>::max())
                                      + " does not fit in the range of the target type " + typeid(S).name();
                    });
            } else {
                REQUIRE(uS(safe_cast<S>(std::numeric_limits<U>::max())) == std::numeric_limits<U>::max());
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
