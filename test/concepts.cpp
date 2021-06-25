// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <sstream>
#include <string>

#if defined(MPPP_HAVE_STRING_VIEW)
#include <string_view>
#endif

#include <mp++/concepts.hpp>
#include <mp++/detail/type_traits.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

template <typename T, detail::enable_if_t<is_string_type<T>::value, int> = 0>
void check_dispatch(const T &s)
{
    std::ostringstream o;
    o << s;
    REQUIRE(o.str() == s);
}

TEST_CASE("concepts")
{
    REQUIRE(is_cpp_integral<int>::value);
    REQUIRE(!is_cpp_integral<int &>::value);
    REQUIRE(!is_cpp_integral<float>::value);
    REQUIRE(!is_cpp_integral<double>::value);
    REQUIRE(!is_cpp_integral<double &>::value);
    REQUIRE(!is_cpp_integral<void>::value);
    REQUIRE(is_cpp_unsigned_integral<unsigned>::value);
    REQUIRE(is_cpp_unsigned_integral<bool>::value);
    REQUIRE(is_cpp_unsigned_integral<unsigned char>::value);
    REQUIRE(is_cpp_unsigned_integral<unsigned short>::value);
    REQUIRE(!is_cpp_unsigned_integral<int>::value);
    REQUIRE(!is_cpp_unsigned_integral<signed char>::value);
    REQUIRE(!is_cpp_unsigned_integral<unsigned &>::value);
    REQUIRE(!is_cpp_unsigned_integral<float>::value);
    REQUIRE(!is_cpp_unsigned_integral<double>::value);
    REQUIRE(!is_cpp_unsigned_integral<double &>::value);
    REQUIRE(!is_cpp_unsigned_integral<void>::value);
    REQUIRE(!is_cpp_signed_integral<unsigned>::value);
    REQUIRE(!is_cpp_signed_integral<bool>::value);
    REQUIRE(!is_cpp_signed_integral<unsigned char>::value);
    REQUIRE(!is_cpp_signed_integral<unsigned short>::value);
    REQUIRE(is_cpp_signed_integral<int>::value);
    REQUIRE(is_cpp_signed_integral<signed char>::value);
    REQUIRE(!is_cpp_signed_integral<int &>::value);
    REQUIRE(!is_cpp_signed_integral<float>::value);
    REQUIRE(!is_cpp_signed_integral<double>::value);
    REQUIRE(!is_cpp_signed_integral<double &>::value);
    REQUIRE(!is_cpp_signed_integral<void>::value);
#if defined(MPPP_HAVE_GCC_INT128)
    REQUIRE(is_cpp_arithmetic<__int128_t>::value);
    REQUIRE(is_cpp_arithmetic<__uint128_t>::value);
    REQUIRE(!is_cpp_arithmetic<__uint128_t &>::value);
    REQUIRE(!is_cpp_arithmetic<const __int128_t>::value);
    REQUIRE(!is_cpp_unsigned_integral<__int128_t>::value);
    REQUIRE(is_cpp_unsigned_integral<__uint128_t>::value);
    REQUIRE(!is_cpp_unsigned_integral<__uint128_t &>::value);
    REQUIRE(!is_cpp_unsigned_integral<const __uint128_t>::value);
    REQUIRE(is_cpp_signed_integral<__int128_t>::value);
    REQUIRE(!is_cpp_signed_integral<__uint128_t>::value);
    REQUIRE(!is_cpp_signed_integral<__int128_t &>::value);
    REQUIRE(!is_cpp_signed_integral<const __int128_t>::value);
#endif
    REQUIRE(is_cpp_floating_point<float>::value);
    REQUIRE(is_cpp_floating_point<double>::value);
    REQUIRE(!is_cpp_floating_point<float &>::value);
    REQUIRE(!is_cpp_floating_point<const double>::value);
    REQUIRE(!is_cpp_floating_point<const double &>::value);
    REQUIRE(!is_cpp_floating_point<void>::value);
    REQUIRE(!is_cpp_floating_point<std::string>::value);
    REQUIRE(is_cpp_floating_point<long double>::value);
    REQUIRE(!is_cpp_floating_point<long double &&>::value);
    REQUIRE(!is_cpp_arithmetic<void>::value);
    REQUIRE(!is_cpp_arithmetic<std::string>::value);
    REQUIRE(!is_cpp_arithmetic<const int>::value);
    REQUIRE(!is_cpp_arithmetic<char &&>::value);

    REQUIRE(is_string_type<char *>::value);
    REQUIRE(is_string_type<const char *>::value);
    REQUIRE(is_string_type<char *const>::value);
    REQUIRE(is_string_type<const char *>::value);
    REQUIRE(is_string_type<const char *const>::value);
    REQUIRE(!is_string_type<char *&>::value);
    REQUIRE(is_string_type<char[]>::value);
    REQUIRE(is_string_type<const char[]>::value);
    REQUIRE(is_string_type<char[1]>::value);
    REQUIRE(is_string_type<const char[1]>::value);
    REQUIRE(is_string_type<char[2]>::value);
    REQUIRE(is_string_type<char[10]>::value);
    REQUIRE(is_string_type<const char[10]>::value);
    REQUIRE(!is_string_type<const char(&)[10]>::value);
    REQUIRE(!is_string_type<char(&)[10]>::value);
    REQUIRE(!is_string_type<char>::value);
    REQUIRE(!is_string_type<const char>::value);
    REQUIRE(!is_string_type<int>::value);
    REQUIRE(is_string_type<std::string>::value);
    REQUIRE(!is_string_type<std::string &>::value);
    REQUIRE(!is_string_type<const std::string &>::value);
    REQUIRE(is_string_type<const std::string>::value);
    REQUIRE(!is_string_type<char(&)[]>::value);
    REQUIRE(!is_string_type<char(&)[1]>::value);
    REQUIRE(is_string_type<const char[2]>::value);
    REQUIRE(!is_string_type<char(&&)[10]>::value);
#if defined(MPPP_HAVE_STRING_VIEW)
    REQUIRE(is_string_type<std::string_view>::value);
    REQUIRE(!is_string_type<std::string_view &>::value);
    REQUIRE(!is_string_type<const std::string_view &>::value);
    REQUIRE(is_string_type<const std::string_view>::value);
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
#if defined(MPPP_HAVE_STRING_VIEW)
    const std::string_view sv1{"bubbbbba"};
    check_dispatch(sv1);
    std::string_view sv2{"bubbbba"};
    check_dispatch(sv2);
#endif
}
