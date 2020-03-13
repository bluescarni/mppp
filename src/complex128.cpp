// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

#if defined(MPPP_HAVE_STRING_VIEW)

#include <string_view>

#endif

// NOTE: extern "C" is already included in quadmath.h since GCC 4.8:
// https://stackoverflow.com/questions/13780219/link-libquadmath-with-c-on-linux
#include <quadmath.h>

#include <mp++/complex128.hpp>
#include <mp++/real128.hpp>

#if defined(MPPP_WITH_MPFR)

#include <mp++/real.hpp>

#endif

namespace mppp
{

static_assert(std::is_same<cplex128, __complex128>::value, "Mismatched __complex128 types.");

__float128 complex128::cast_to_f128(const real &x)
{
    return static_cast<real128>(x).m_value;
}

std::ostream &operator<<(std::ostream &os, const complex128 &c)
{
    // NOTE: use the same printing format as std::complex.
    os << '(' << c.creal() << ',' << c.cimag() << ')';

    return os;
}

std::string complex128::to_string() const
{
    std::ostringstream oss;

    oss << *this;

    return oss.str();
}

complex128::complex128(const ptag &, const char *str)
{
    // Small helper to raise an error in case
    // of a malformed string.
    auto raise_error = [str]() {
        throw std::invalid_argument(std::string("The string '") + str
                                    + "' is not a valid representation of a complex128");
    };

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
            set_real(real128{s + 1, p - 1});
            set_imag(real128{0});
        } else {
            // We reached a comma, the format must
            // be (real,imag).

            // Set the real part first.
            set_real(real128{s + 1, p});

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

            // Set the imaginary part.
            set_imag(real128{s, p - 1});
        }

    } else {
        // The string does not start with a round
        // bracket, interpret as a real value.
        set_real(real128{s});
        set_imag(real128{0});
    }
}

complex128::complex128(const ptag &, const std::string &s) : complex128(s.c_str()) {}

#if defined(MPPP_HAVE_STRING_VIEW)

complex128::complex128(const ptag &, const std::string_view &s) : complex128(s.data(), s.data() + s.size()) {}

#endif

} // namespace mppp
