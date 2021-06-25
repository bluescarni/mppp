// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <cassert>
#include <cstddef>
#include <ios>
#include <limits>
#include <locale>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#if defined(MPPP_HAVE_STRING_VIEW)

#include <string_view>

#endif

// NOTE: extern "C" is already included in quadmath.h since GCC 4.8:
// https://stackoverflow.com/questions/13780219/link-libquadmath-with-c-on-linux
#include <quadmath.h>

#include <mp++/complex128.hpp>
#include <mp++/detail/parse_complex.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/integer.hpp>
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
    oss.exceptions(std::ios_base::failbit | std::ios_base::badbit);

    // NOTE: use the same printing format as std::complex.
    oss << '(' << real().to_string() << ',' << imag().to_string() << ')';

    return oss.str();
}

std::ostream &operator<<(std::ostream &os, const complex128 &c)
{
    // Get the stream width.
    const auto width = os.width();

    // Fetch the stream's flags.
    const auto flags = os.flags();

    // Check if we are using scientific, fixed or both.
    const auto scientific = (flags & std::ios_base::scientific) != 0;
    const auto fixed = (flags & std::ios_base::fixed) != 0;
    const auto hexfloat = scientific && fixed;

    // Force decimal point character?
    const auto showpoint = (flags & std::ios_base::showpoint) != 0;

    // Force the plus sign?
    const bool with_plus = (flags & std::ios_base::showpos) != 0;

    // Uppercase?
    const bool uppercase = (flags & std::ios_base::uppercase) != 0;

    // Fetch the precision too.
    auto precision = os.precision();
    // NOTE: if the precision is negative, reset it
    // to the default value of 6.
    if (precision < 0) {
        precision = 6;
    }

    // Put together the format string.
    std::ostringstream oss;
    oss.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    oss.imbue(std::locale::classic());

    oss << '%';

    if (showpoint) {
        oss << '#';
    }

    if (with_plus) {
        oss << '+';
    }

    if (hexfloat) {
        // NOTE: in hexfloat mode, we want to ignore the precision
        // setting in the stream object and print the exact representation:
        // https://en.cppreference.com/w/cpp/locale/num_put/put
        oss << 'Q';
    } else {
        oss << '.' << precision << 'Q';
    }

    if (hexfloat) {
        oss << (uppercase ? 'A' : 'a');
    } else if (scientific) {
        oss << (uppercase ? 'E' : 'e');
    } else if (fixed) {
        // NOTE: in fixed format, the uppercase
        // setting is ignored for floating-point types:
        // https://en.cppreference.com/w/cpp/locale/num_put/put
        oss << 'f';
    } else {
        oss << (uppercase ? 'G' : 'g');
    }

    const auto fmt_str = oss.str();

    // Prepare the output string.
    std::string tmp = "(";

    // Execute quadmath_snprintf() once to determine the needed buffer size for the real part.
    // NOTE: this returns the number of characters *excluding* the terminator.
    const auto sz_real = ::quadmath_snprintf(nullptr, 0, fmt_str.c_str(), c.real().m_value);
    if (sz_real < 0) {
        // LCOV_EXCL_START
        throw std::invalid_argument("The quadmath_snprintf() returned an error code");
        // LCOV_EXCL_STOP
    }
    if (sz_real == std::numeric_limits<int>::max()) {
        // LCOV_EXCL_START
        throw std::overflow_error("An overflow condition was detected in the output stream operator of complex128");
        // LCOV_EXCL_STOP
    }

    // Prepare the char buffer.
    std::vector<char> buf;
    // NOTE: need +1 here because the buffer size passed to
    // quadmath_snprintf() must include the terminator.
    buf.resize(detail::safe_cast<decltype(buf.size())>(sz_real + 1));

    // Print out to buf.
    const auto ret_real = ::quadmath_snprintf(buf.data(), detail::safe_cast<std::size_t>(buf.size()), fmt_str.c_str(),
                                              c.real().m_value);
    if (ret_real < 0) {
        // LCOV_EXCL_START
        throw std::invalid_argument("The quadmath_snprintf() returned an error code");
        // LCOV_EXCL_STOP
    }
    // Make sure we wrote the expected number of chars.
    assert(ret_real == sz_real);

    // Append to the string.
    tmp += buf.data();
    tmp += ',';

    // Execute quadmath_snprintf() once to determine the needed buffer size for the imaginary part.
    // NOTE: this returns the number of characters *excluding* the terminator.
    const auto sz_imag = ::quadmath_snprintf(nullptr, 0, fmt_str.c_str(), c.imag().m_value);
    if (sz_imag < 0) {
        // LCOV_EXCL_START
        throw std::invalid_argument("The quadmath_snprintf() returned an error code");
        // LCOV_EXCL_STOP
    }
    if (sz_imag == std::numeric_limits<int>::max()) {
        // LCOV_EXCL_START
        throw std::overflow_error("An overflow condition was detected in the output stream operator of complex128");
        // LCOV_EXCL_STOP
    }

    // Prepare the char buffer.
    buf.clear();
    // NOTE: need +1 here because the buffer size passed to
    // quadmath_snprintf() must include the terminator.
    buf.resize(detail::safe_cast<decltype(buf.size())>(sz_imag + 1));

    // Print out to buf.
    const auto ret_imag = ::quadmath_snprintf(buf.data(), detail::safe_cast<std::size_t>(buf.size()), fmt_str.c_str(),
                                              c.imag().m_value);
    if (ret_imag < 0) {
        // LCOV_EXCL_START
        throw std::invalid_argument("The quadmath_snprintf() returned an error code");
        // LCOV_EXCL_STOP
    }
    // Make sure we wrote the expected number of chars.
    assert(ret_imag == sz_imag);

    // Finalise the string.
    tmp += buf.data();
    tmp += ')';

    // We are going to do the filling
    // only if the stream width is larger
    // than the total size of the string repr.
    if (width >= 0 && detail::make_unsigned(width) > tmp.size()) {
        // Determine the fill type.
        const auto fill = detail::stream_flags_to_fill(flags);

        // Compute how much fill we need.
        const auto fill_size = detail::safe_cast<decltype(tmp.size())>(detail::make_unsigned(width) - tmp.size());

        // Get the fill character.
        const auto fill_char = os.fill();

        if (fill == 1) {
            // Left fill: fill characters at the end.
            tmp.insert(tmp.end(), fill_size, fill_char);
        } else {
            assert(fill == 2 || fill == 3);

            // Right or internal fill: fill characters at the beginning.
            tmp.insert(tmp.begin(), fill_size, fill_char);
        }
    }

    os.write(tmp.data(), detail::safe_cast<std::streamsize>(tmp.size()));

    // Reset the stream width to zero, like the operator<<() does for builtin types.
    // https://en.cppreference.com/w/cpp/io/manip/setw
    // Do it here so we ensure we don't alter the state of the stream until the very end.
    os.width(0);

    return os;
}

// Helper to construct the internal m_value
// from a null-terminated string.
void complex128::construct_from_nts(const char *str)
{
    // NOTE: set the value to zero before doing
    // anything else, so that the set_real()/set_imag()
    // below operate on a well-defined internal value.
    m_value = 0;

    const auto res = detail::parse_complex(str);

    // Set the real part.
    set_real(real128{res[0], res[1]});

    // The imaginary part might not be present.
    if (res[2] == nullptr) {
        assert(res[3] == nullptr);

        set_imag(0);
    } else {
        assert(res[3] != nullptr);

        set_imag(real128{res[2], res[3]});
    }
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
complex128::complex128(const ptag &, const char *str)
{
    construct_from_nts(str);
}

complex128::complex128(const ptag &, const std::string &s) : complex128(s.c_str()) {}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
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

#if defined(MPPP_WITH_MPFR)

// Implementation of real's assignment
// from complex128.
real &real::operator=(const complex128 &x)
{
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
    return *this = static_cast<real>(x);
}

#endif

} // namespace mppp
