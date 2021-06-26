// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <array>
#include <cstring>
#include <stdexcept>
#include <string>

#include <mp++/config.hpp>
#include <mp++/detail/parse_complex.hpp>

// NOLINTNEXTLINE(modernize-concat-nested-namespaces)
namespace mppp
{

namespace detail
{

// This function will try to parse the null-terminated
// string str as a complex number. The expected format is one of:
//
// - "x",
// - "(x)",
// - "(x,y)",
//
// where "x" and "y" are string representations of the real
// and imaginary parts.
//
// The return value is an array of 4 pointers representing
// two ranges in str encompassing "x" and "y" respectively.
// If the format is "x" or "(x)", the second range consists
// of two null pointers.
//
// If parsing fails, an exception will be raised.
std::array<const char *, 4> parse_complex(const char *str)
{

    // Small helper to raise an error in case
    // of a malformed string.
    auto raise_error = [str]() {
        throw std::invalid_argument(std::string("The string '") + str
                                    + "' is not a valid representation of a complex value");
    };

    // NOLINTNEXTLINE(llvm-qualified-auto, readability-qualified-auto)
    auto s = str;

    // Skip leading whitespaces.
    for (; *s == ' '; ++s) {
    }

    if (mppp_unlikely(*s == '\0')) {
        // The string consists only of whitespaces.
        raise_error();
    }

    if (*s == '(') {
        // The string starts with a round bracket. Try to
        // understand if we have only the real component, or
        // the imaginary as well.

        // Examine the string until we get either to a comma
        // (the separator between real and imaginary parts)
        // or the end of the string.
        // NOLINTNEXTLINE(llvm-qualified-auto, readability-qualified-auto)
        auto p = s + 1;
        for (; *p != ',' && *p != '\0'; ++p) {
        }

        if (*p == '\0') {
            // We reached the end of the string,
            // the format must be (real).
            if (mppp_unlikely(*(p - 1) != ')')) {
                // Unbalanced bracket.
                raise_error();
            }

            // NOTE: here we know that:
            //
            // - *s == '(',
            // - p > s,
            // - *(p-1) == ')'.
            //
            // Thus, p-1 > s >= s+1.
            return {{s + 1, p - 1, nullptr, nullptr}};
        } else {
            // We reached a comma, the format must
            // be (real,imag).

            // Record the char range for the real part.
            const auto re_start = s + 1, re_end = p;

            // Move p past the comma, assign to s.
            s = ++p;

            if (mppp_unlikely(*p == '\0')) {
                // There's nothing after the comma.
                raise_error();
            }

            // Look for the end of the string.
            for (++p; *p != '\0'; ++p) {
            }

            // NOTE: here we are sure that p > s.
            if (mppp_unlikely(*(p - 1) != ')')) {
                // Unbalanced bracket.
                raise_error();
            }

            return {{re_start, re_end, s, p - 1}};
        }
    } else {
        // The string does not start with a round
        // bracket, interpret as a real value.
        return {{s, s + std::strlen(s), nullptr, nullptr}};
    }
}

} // namespace detail

} // namespace mppp
