// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <mp++/concepts.hpp>
#include <mp++/detail/type_traits.hpp>
#if MPPP_CPLUSPLUS >= 201703L
#include <string_view>
#endif
#include <sstream>
#include <string>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mppp;

template <typename T, enable_if_t<is_string_type<T>::value, int> = 0>
void check_dispatch(const T &s)
{
    std::ostringstream o;
    o << s;
    REQUIRE(o.str() == s);
}

TEST_CASE("concepts")
{
    REQUIRE(is_string_type<char *>::value);
    REQUIRE(is_string_type<const char *>::value);
    REQUIRE(is_string_type<char[]>::value);
    REQUIRE(is_string_type<char[1]>::value);
    REQUIRE(is_string_type<char[2]>::value);
    REQUIRE(is_string_type<char[10]>::value);
    REQUIRE(!is_string_type<char>::value);
    REQUIRE(!is_string_type<const char>::value);
    REQUIRE(!is_string_type<int>::value);
    REQUIRE(is_string_type<std::string>::value);
    REQUIRE(!is_string_type<std::string &>::value);
    REQUIRE(!is_string_type<const std::string &>::value);
    REQUIRE(!is_string_type<const std::string>::value);
    REQUIRE(!is_string_type<char(&)[]>::value);
    REQUIRE(!is_string_type<char(&)[1]>::value);
    REQUIRE(!is_string_type<const char[2]>::value);
    REQUIRE(!is_string_type<char(&&)[10]>::value);
#if MPPP_CPLUSPLUS >= 201703L
    REQUIRE(is_string_type<std::string_view>::value);
    REQUIRE(!is_string_type<std::string_view &>::value);
    REQUIRE(!is_string_type<const std::string_view &>::value);
    REQUIRE(!is_string_type<const std::string_view>::value);
#endif
    std::string s{"foo"};
    check_dispatch(std::string{"foo"});
    check_dispatch(s);
    check_dispatch(static_cast<const std::string &>(s));
    const char s1[] = "blab";
    check_dispatch(s1);
    check_dispatch(&s1[0]);
    check_dispatch("blab");
    char s2[] = "blab";
    check_dispatch(s2);
    check_dispatch(&s2[0]);
#if MPPP_CPLUSPLUS >= 201703L
    const std::string_view sv1{"bubbbbba"};
    check_dispatch(sv1);
    std::string_view sv2{"bubbbba"};
    check_dispatch(sv2);
#endif
}
