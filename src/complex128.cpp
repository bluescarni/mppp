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
#include <vector>

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

std::string complex128::to_string() const
{
    std::ostringstream oss;

    // NOTE: use the same printing format as std::complex.
    oss << '(' << real().to_string() << ',' << imag().to_string() << ')';

    return oss.str();
}

std::ostream &operator<<(std::ostream &os, const complex128 &c)
{
    return os << c.to_string();
}

// Helper to construct the internal m_value
// from a null-terminated string.
void complex128::construct_from_nts(const char *str)
{
    // NOTE: set the value to zero before doing
    // anything else, so that the set_real()/set_imag()
    // below operate on a well-defined internal value.
    m_value = 0;

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
            set_imag(0);
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
        set_imag(0);
    }
}

complex128::complex128(const ptag &, const char *str)
{
    construct_from_nts(str);
}

complex128::complex128(const ptag &, const std::string &s) : complex128(s.c_str()) {}

complex128::complex128(const char *begin, const char *end)
{
    MPPP_MAYBE_TLS std::vector<char> buffer;
    buffer.assign(begin, end);
    buffer.emplace_back('\0');
    construct_from_nts(buffer.data());
}

#if defined(MPPP_HAVE_STRING_VIEW)

complex128::complex128(const ptag &, const std::string_view &s) : complex128(s.data(), s.data() + s.size()) {}

#endif

#if defined(MPPP_WITH_MPFR)

bool complex128::get_impl(mppp::real &rop, const real128 &r)
{
    rop = r;
    return true;
}

#endif

#define MPPP_COMPLEX128_IMPLEMENT_UNARY(func)                                                                          \
    complex128 func(const complex128 &c)                                                                               \
    {                                                                                                                  \
        return complex128{::c##func##q(c.m_value)};                                                                    \
    }                                                                                                                  \
                                                                                                                       \
    complex128 &complex128::func()                                                                                     \
    {                                                                                                                  \
        return *this = mppp::func(*this);                                                                              \
    }

real128 abs(const complex128 &c)
{
    return real128{::cabsq(c.m_value)};
}

complex128 &complex128::abs()
{
    return *this = mppp::abs(*this);
}

MPPP_COMPLEX128_IMPLEMENT_UNARY(arg)
MPPP_COMPLEX128_IMPLEMENT_UNARY(proj)

MPPP_COMPLEX128_IMPLEMENT_UNARY(sqrt)

MPPP_COMPLEX128_IMPLEMENT_UNARY(sin)
MPPP_COMPLEX128_IMPLEMENT_UNARY(cos)
MPPP_COMPLEX128_IMPLEMENT_UNARY(tan)

MPPP_COMPLEX128_IMPLEMENT_UNARY(asin)
MPPP_COMPLEX128_IMPLEMENT_UNARY(acos)
MPPP_COMPLEX128_IMPLEMENT_UNARY(atan)

MPPP_COMPLEX128_IMPLEMENT_UNARY(sinh)
MPPP_COMPLEX128_IMPLEMENT_UNARY(cosh)
MPPP_COMPLEX128_IMPLEMENT_UNARY(tanh)

MPPP_COMPLEX128_IMPLEMENT_UNARY(asinh)
MPPP_COMPLEX128_IMPLEMENT_UNARY(acosh)
MPPP_COMPLEX128_IMPLEMENT_UNARY(atanh)

MPPP_COMPLEX128_IMPLEMENT_UNARY(exp)
MPPP_COMPLEX128_IMPLEMENT_UNARY(log)
MPPP_COMPLEX128_IMPLEMENT_UNARY(log10)

#undef MPPP_COMPLEX128_IMPLEMENT_UNARY

namespace detail
{

complex128 complex128_pow_impl(const complex128 &x, const complex128 &y)
{
    return complex128{::cpowq(x.m_value, y.m_value)};
}

} // namespace detail

} // namespace mppp
