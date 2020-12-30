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

#if defined(MPPP_WITH_BOOST_S11N)

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#include <boost/serialization/binary_object.hpp>

#endif

// NOTE: extern "C" is already included in quadmath.h since GCC 4.8:
// https://stackoverflow.com/questions/13780219/link-libquadmath-with-c-on-linux
#include <quadmath.h>

#include <mp++/detail/utils.hpp>
#include <mp++/real128.hpp>

namespace mppp
{

// Double check our assumption regarding
// the number of binary digits in the
// significand.
static_assert(FLT128_MANT_DIG == real128_sig_digits(), "Invalid number of digits.");

namespace detail
{

namespace
{

void float128_stream(std::ostream &os, const __float128 &x)
{
    char buf[100];
    // NOTE: 36 decimal digits ensure that reading back the string always produces the same value:
    // https://en.wikipedia.org/wiki/Quadruple-precision_floating-point_format
    // NOTE: when using the g/G format, the precision field represents the number
    // of significant digits:
    // https://linux.die.net/man/3/printf
    const auto n = ::quadmath_snprintf(buf, sizeof(buf), "%.36Qg", x);
    // LCOV_EXCL_START
    if (mppp_unlikely(n < 0)) {
        throw std::runtime_error("A call to quadmath_snprintf() failed: a negative exit status of " + to_string(n)
                                 + " was returned");
    }
    if (mppp_unlikely(unsigned(n) >= sizeof(buf))) {
        throw std::runtime_error("A call to quadmath_snprintf() failed: the exit status " + to_string(n)
                                 + " is not less than the size of the internal buffer " + to_string(sizeof(buf)));
    }
    // LCOV_EXCL_STOP
    os << &buf[0];
}

__float128 str_to_float128(const char *s)
{
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    char *endptr;
    auto retval = ::strtoflt128(s, &endptr);
    if (mppp_unlikely(endptr == s || *endptr != '\0')) {
        // NOTE: the first condition handles an empty string.
        // endptr will point to the first character in the string which
        // did not contribute to the construction of retval.
        throw std::invalid_argument("The string '" + std::string(s)
                                    + "' does not represent a valid quadruple-precision floating-point value");
    }
    return retval;
}

} // namespace

__float128 scalbnq(__float128 x, int exp)
{
    return ::scalbnq(x, exp);
}

__float128 scalblnq(__float128 x, long exp)
{
    return ::scalblnq(x, exp);
}

} // namespace detail

// Private string constructors.
real128::real128(const ptag &, const char *s) : m_value(detail::str_to_float128(s)) {}

real128::real128(const ptag &, const std::string &s) : real128(s.c_str()) {}

#if defined(MPPP_HAVE_STRING_VIEW)

real128::real128(const ptag &, const std::string_view &s) : real128(s.data(), s.data() + s.size()) {}

#endif

// Constructor from range of characters.
real128::real128(const char *begin, const char *end)
{
    MPPP_MAYBE_TLS std::vector<char> buffer;
    buffer.assign(begin, end);
    buffer.emplace_back('\0');
    m_value = detail::str_to_float128(buffer.data());
}

// Convert to string.
std::string real128::to_string() const
{
    std::ostringstream oss;
    detail::float128_stream(oss, m_value);
    return oss.str();
}

// Sign bit.
bool real128::signbit() const
{
    return ::signbitq(m_value) != 0;
}

// Decompose into a normalized fraction and an integral power of two.
real128 frexp(const real128 &x, int *exp)
{
    return real128{::frexpq(x.m_value, exp)};
}

// Multiply by a power of 2.
real128 ldexp(const real128 &x, int exp)
{
    return real128{::ldexpq(x.m_value, exp)};
}

real128 scalbn(const real128 &x, int n)
{
    return real128{detail::scalbnq(x.m_value, n)};
}

real128 scalbln(const real128 &x, long n)
{
    return real128{detail::scalblnq(x.m_value, n)};
}

// Fused multiply-add.
real128 fma(const real128 &x, const real128 &y, const real128 &z)
{
    return real128{::fmaq(x.m_value, y.m_value, z.m_value)};
}

// Next real128 from 'from' to 'to'.
real128 nextafter(const real128 &from, const real128 &to)
{
    return real128{::nextafterq(from.m_value, to.m_value)};
}

// Output stream operator.
std::ostream &operator<<(std::ostream &os, const real128 &x)
{
    detail::float128_stream(os, x.m_value);
    return os;
}

namespace detail
{

real128 dispatch_real128_hypot(const real128 &x, const real128 &y)
{
    return real128{::hypotq(x.m_value, y.m_value)};
}

real128 dispatch_real128_pow(const real128 &x, const real128 &y)
{
    return real128{::powq(x.m_value, y.m_value)};
}

real128 dispatch_real128_atan2(const real128 &y, const real128 &x)
{
    return real128{::atan2q(y.m_value, x.m_value)};
}

real128 dispatch_real128_copysign(const real128 &y, const real128 &x)
{
    return real128{::copysignq(y.m_value, x.m_value)};
}

real128 dispatch_real128_fdim(const real128 &y, const real128 &x)
{
    return real128{::fdimq(y.m_value, x.m_value)};
}

real128 dispatch_real128_fmax(const real128 &y, const real128 &x)
{
    return real128{::fmaxq(y.m_value, x.m_value)};
}

real128 dispatch_real128_fmin(const real128 &y, const real128 &x)
{
    return real128{::fminq(y.m_value, x.m_value)};
}

real128 dispatch_real128_fmod(const real128 &y, const real128 &x)
{
    return real128{::fmodq(y.m_value, x.m_value)};
}

real128 dispatch_real128_remainder(const real128 &y, const real128 &x)
{
    return real128{::remainderq(y.m_value, x.m_value)};
}

} // namespace detail

#define MPPP_REAL128_IMPLEMENT_UNARY(func)                                                                             \
    real128 func(const real128 &c)                                                                                     \
    {                                                                                                                  \
        return real128{::func##q(c.m_value)};                                                                          \
    }                                                                                                                  \
                                                                                                                       \
    real128 &real128::func()                                                                                           \
    {                                                                                                                  \
        return *this = mppp::func(*this);                                                                              \
    }

MPPP_REAL128_IMPLEMENT_UNARY(ceil)
MPPP_REAL128_IMPLEMENT_UNARY(floor)
MPPP_REAL128_IMPLEMENT_UNARY(nearbyint)
MPPP_REAL128_IMPLEMENT_UNARY(rint)
MPPP_REAL128_IMPLEMENT_UNARY(round)
MPPP_REAL128_IMPLEMENT_UNARY(trunc)

MPPP_REAL128_IMPLEMENT_UNARY(exp)
#if defined(MPPP_QUADMATH_HAVE_EXP2Q)
MPPP_REAL128_IMPLEMENT_UNARY(exp2)
#endif
MPPP_REAL128_IMPLEMENT_UNARY(expm1)
MPPP_REAL128_IMPLEMENT_UNARY(log)
MPPP_REAL128_IMPLEMENT_UNARY(log10)
MPPP_REAL128_IMPLEMENT_UNARY(log2)
MPPP_REAL128_IMPLEMENT_UNARY(log1p)

MPPP_REAL128_IMPLEMENT_UNARY(erf)
MPPP_REAL128_IMPLEMENT_UNARY(erfc)

MPPP_REAL128_IMPLEMENT_UNARY(lgamma)
MPPP_REAL128_IMPLEMENT_UNARY(tgamma)

MPPP_REAL128_IMPLEMENT_UNARY(j0)
MPPP_REAL128_IMPLEMENT_UNARY(j1)
MPPP_REAL128_IMPLEMENT_UNARY(y0)
MPPP_REAL128_IMPLEMENT_UNARY(y1)

MPPP_REAL128_IMPLEMENT_UNARY(sqrt)
MPPP_REAL128_IMPLEMENT_UNARY(cbrt)

MPPP_REAL128_IMPLEMENT_UNARY(sin)
MPPP_REAL128_IMPLEMENT_UNARY(cos)
MPPP_REAL128_IMPLEMENT_UNARY(tan)
MPPP_REAL128_IMPLEMENT_UNARY(asin)
MPPP_REAL128_IMPLEMENT_UNARY(acos)
MPPP_REAL128_IMPLEMENT_UNARY(atan)

MPPP_REAL128_IMPLEMENT_UNARY(sinh)
MPPP_REAL128_IMPLEMENT_UNARY(cosh)
MPPP_REAL128_IMPLEMENT_UNARY(tanh)
MPPP_REAL128_IMPLEMENT_UNARY(asinh)
MPPP_REAL128_IMPLEMENT_UNARY(acosh)
MPPP_REAL128_IMPLEMENT_UNARY(atanh)

#undef MPPP_REAL128_IMPLEMENT_UNARY

real128 jn(int n, const real128 &x)
{
    return real128{::jnq(n, x.m_value)};
}

real128 yn(int n, const real128 &x)
{
    return real128{::ynq(n, x.m_value)};
}

long long llrint(const real128 &x)
{
    return ::llrintq(x.m_value);
}

long lrint(const real128 &x)
{
    return ::lrintq(x.m_value);
}

long long llround(const real128 &x)
{
    return ::llroundq(x.m_value);
}

long lround(const real128 &x)
{
    return ::lroundq(x.m_value);
}

int real128::ilogb() const
{
    return ::ilogbq(m_value);
}

int ilogb(const real128 &x)
{
    return x.ilogb();
}

#if defined(MPPP_QUADMATH_HAVE_LOGBQ)

real128 real128::logb() const
{
    return real128{::logbq(m_value)};
}

real128 logb(const real128 &x)
{
    return x.logb();
}

#endif

real128 modf(const real128 &x, real128 *iptr)
{
    return real128{::modfq(x.m_value, &iptr->m_value)};
}

real128 remquo(const real128 &x, const real128 &y, int *quo)
{
    return real128{::remquoq(x.m_value, y.m_value, quo)};
}

void sincos(const real128 &x, real128 *s, real128 *c)
{
    ::sincosq(x.m_value, &s->m_value, &c->m_value);
}

#if defined(MPPP_WITH_BOOST_S11N)

// Fast serialization implementations for Boost's binary archives.
void real128::save(boost::archive::binary_oarchive &ar, unsigned) const
{
    ar << boost::serialization::make_binary_object(&m_value, sizeof(m_value));
}

void real128::save(boost::archive::polymorphic_binary_oarchive &ar, unsigned) const
{
    ar << boost::serialization::make_binary_object(&m_value, sizeof(m_value));
}

void real128::load(boost::archive::binary_iarchive &ar, unsigned)
{
    ar >> boost::serialization::make_binary_object(&m_value, sizeof(m_value));
}

void real128::load(boost::archive::polymorphic_binary_iarchive &ar, unsigned)
{
    ar >> boost::serialization::make_binary_object(&m_value, sizeof(m_value));
}

#endif

} // namespace mppp
