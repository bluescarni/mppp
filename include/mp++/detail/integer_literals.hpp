// Copyright 2016-2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_INTEGER_LITERALS_HPP
#define MPPP_DETAIL_INTEGER_LITERALS_HPP

#include <mp++/config.hpp>

#if MPPP_CPLUSPLUS >= 201703L

#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>

#include <mp++/detail/gmp.hpp>

namespace mppp
{

namespace detail
{

struct invalid_integer_literal {
};

template <std::size_t Size>
constexpr int integer_literal_check_str(const char (&arr)[Size])
{
    // We expect the size of the array to be
    // always at least 2, the null terminator
    // plus at least one char from the literal.
    static_assert(Size >= 2u);
    assert(arr[Size - 1u] == '\0');

    // The actual string length of the literal.
    constexpr auto str_length = Size - 1u;

    // Helpers to check a digit in various bases.
    auto base10_digit_check = [](char c) {
        if (c < '0' || c > '9') {
            throw invalid_integer_literal{};
        }
    };

    auto base8_digit_check = [](char c) {
        if (c < '0' || c > '7') {
            throw invalid_integer_literal{};
        }
    };

    auto base2_digit_check = [](char c) {
        if (c != '0' && c != '1') {
            throw invalid_integer_literal{};
        }
    };

    auto base16_digit_check = [](char c) {
        if (c < '0' || c > '9') {
            switch (c) {
                case 'a':
                    [[fallthrough]];
                case 'b':
                    [[fallthrough]];
                case 'c':
                    [[fallthrough]];
                case 'd':
                    [[fallthrough]];
                case 'e':
                    [[fallthrough]];
                case 'f':
                    [[fallthrough]];
                case 'A':
                    [[fallthrough]];
                case 'B':
                    [[fallthrough]];
                case 'C':
                    [[fallthrough]];
                case 'D':
                    [[fallthrough]];
                case 'E':
                    [[fallthrough]];
                case 'F':
                    return;
                default:
                    throw invalid_integer_literal{};
            }
        }
    };

    if (str_length == 1u) {
        // If the string length is 1, the only
        // character must be in the ['0', '9'] range.
        base10_digit_check(arr[0]);

        // NOTE: the literal "0" is actually an
        // octal, but it does not matter if we
        // construct it using base 10.
        return 10;
    }

    // The string length is at least 2. Infer the base
    // of the literal.
    const int base = [&arr, base8_digit_check, base10_digit_check]() {
        // Fetch the first 2 digits.
        const auto d0 = arr[0], d1 = arr[1];

        if (d0 == '0') {
            // Binary, octal or hex.
            if (d1 == 'b' || d1 == 'B') {
                return 2;
            } else if (d1 == 'x' || d1 == 'X') {
                return 16;
            } else {
                base8_digit_check(d1);
                return 8;
            }
        } else {
            // Decimal.
            base10_digit_check(d0);
            base10_digit_check(d1);
            return 10;
        }
    }();

    // Run the checks on the remainder of the literal,
    // after we already checked the first 2 digits.
    // For bases 2 and 16, we need to have other digits,
    // otherwise the literal is malformed.
    switch (base) {
        case 2:
            if (str_length == 2u) {
                throw invalid_integer_literal{};
            }
            for (std::size_t i = 2; i < str_length; ++i) {
                base2_digit_check(arr[i]);
            }
            break;
        case 8:
            for (std::size_t i = 2; i < str_length; ++i) {
                base8_digit_check(arr[i]);
            }
            break;
        case 10:
            for (std::size_t i = 2; i < str_length; ++i) {
                base10_digit_check(arr[i]);
            }
            break;
        case 16:
            if (str_length == 2u) {
                throw invalid_integer_literal{};
            }
            for (std::size_t i = 2; i < str_length; ++i) {
                base16_digit_check(arr[i]);
            }
    }

    return base;
}

template <int Base, std::size_t Size>
constexpr std::pair<bool, ::mp_limb_t> z1_literal_impl(const char (&arr)[Size])
{
    auto digit_to_value = [](char c) {
        if constexpr (Base == 16) {
            if (c >= '0' && c <= '9') {
                return static_cast<::mp_limb_t>(c - '0');
            } else {
                switch (c) {
                    case 'a':
                        [[fallthrough]];
                    case 'A':
                        return static_cast<::mp_limb_t>(10);
                    case 'b':
                        [[fallthrough]];
                    case 'B':
                        return static_cast<::mp_limb_t>(11);
                    case 'c':
                        [[fallthrough]];
                    case 'C':
                        return static_cast<::mp_limb_t>(12);
                    case 'd':
                        [[fallthrough]];
                    case 'D':
                        return static_cast<::mp_limb_t>(13);
                    case 'e':
                        [[fallthrough]];
                    case 'E':
                        return static_cast<::mp_limb_t>(14);
                    default:
                        assert(c == 'f' || c == 'F');
                        return static_cast<::mp_limb_t>(15);
                }
            }
        } else {
            return static_cast<::mp_limb_t>(c - '0');
        }
    };

    constexpr auto str_idx_limit = []() {
        if constexpr (Base == 16 || Base == 2) {
            return static_cast<std::size_t>(2);
        } else if constexpr (Base == 8) {
            return static_cast<std::size_t>(1);
        } else {
            return static_cast<std::size_t>(0);
        }
    }();

    ::mp_limb_t retval = 0, shifter = 1;

    constexpr auto intmax = std::numeric_limits<::mp_limb_t>::max();

    for (std::size_t i = Size - 1u; i > str_idx_limit; --i) {
        const auto cur_digit = digit_to_value(arr[i - 1u]);
        if (cur_digit > intmax / shifter) {
            return std::make_pair(false, ::mp_limb_t(0));
        }
        const auto adder = static_cast<::mp_limb_t>(cur_digit * shifter);
        if (retval > intmax - adder) {
            return std::make_pair(false, ::mp_limb_t(0));
        }
        retval += adder;
        if (i > 1u) {
            if (shifter > intmax / ::mp_limb_t(Base)) {
                return std::make_pair(false, ::mp_limb_t(0));
            }
            shifter *= ::mp_limb_t(Base);
        }
    }

    return std::make_pair(true, retval);
}

} // namespace detail

inline namespace literals
{

template <char... Chars>
inline integer<1> operator"" _z1()
{
    // Turn the sequence of input chars
    // into a null-terminated char array.
    constexpr char arr[] = {Chars..., '\0'};

    // Run the checks on the char sequence, and determine the base.
    constexpr auto base = detail::integer_literal_check_str(arr);

    // Attempt the compile-time implementation of the literal.
    constexpr auto ret = detail::z1_literal_impl<base>(arr);

    if constexpr (ret.first) {
        return integer<1>{ret.second};
    } else {
        return integer<1>{arr};
    }
}

} // namespace literals

} // namespace mppp

#endif

#endif
