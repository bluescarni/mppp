// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#if !defined(MPPP_DOXYGEN_INVOKED)

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

__float128 powq(__float128 x, __float128 y)
{
    return ::powq(x, y);
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
    return ::signbitq(m_value);
}

// In-place square root.
real128 &real128::sqrt()
{
    return *this = ::sqrtq(m_value);
}

// In-place cube root.
real128 &real128::cbrt()
{
    return *this = ::cbrtq(m_value);
}

/// In-place sine.
/**
 * This method will set \p this to its sine.
 *
 * @return a reference to \p this.
 */
real128 &real128::sin()
{
    return *this = ::sinq(m_value);
}

/// In-place cosine.
/**
 * This method will set \p this to its cosine.
 *
 * @return a reference to \p this.
 */
real128 &real128::cos()
{
    return *this = ::cosq(m_value);
}

/// In-place tangent.
/**
 * This method will set \p this to its trigonometric tangent.
 *
 * @return a reference to \p this.
 */
real128 &real128::tan()
{
    return *this = ::tanq(m_value);
}

/// In-place inverse sine.
/**
 * This method will set \p this to its inverse sine.
 *
 * @return a reference to \p this.
 */
real128 &real128::asin()
{
    return *this = ::asinq(m_value);
}

/// In-place inverse cosine.
/**
 * This method will set \p this to its inverse cosine.
 *
 * @return a reference to \p this.
 */
real128 &real128::acos()
{
    return *this = ::acosq(m_value);
}

/// In-place inverse tangent.
/**
 * This method will set \p this to its inverse trigonomotric tangent.
 *
 * @return a reference to \p this.
 */
real128 &real128::atan()
{
    return *this = ::atanq(m_value);
}

/// In-place hyperbolic sine.
/**
 * This method will set \p this to its hyperbolic sine.
 *
 * @return a reference to \p this.
 */
real128 &real128::sinh()
{
    return *this = ::sinhq(m_value);
}

/// In-place hyperbolic cosine.
/**
 * This method will set \p this to its hyperbolic cosine.
 *
 * @return a reference to \p this.
 */
real128 &real128::cosh()
{
    return *this = ::coshq(m_value);
}

/// In-place hyperbolic tangent.
/**
 * This method will set \p this to its hyperbolic tangent.
 *
 * @return a reference to \p this.
 */
real128 &real128::tanh()
{
    return *this = ::tanhq(m_value);
}

/// In-place inverse hyperbolic sine.
/**
 * This method will set \p this to its inverse hyperbolic sine.
 *
 * @return a reference to \p this.
 */
real128 &real128::asinh()
{
    return *this = ::asinhq(m_value);
}

/// In-place inverse hyperbolic cosine.
/**
 * This method will set \p this to its inverse hyperbolic cosine.
 *
 * @return a reference to \p this.
 */
real128 &real128::acosh()
{
    return *this = ::acoshq(m_value);
}

/// In-place inverse hyperbolic tangent.
/**
 * This method will set \p this to its inverse hyperbolic tangent.
 *
 * @return a reference to \p this.
 */
real128 &real128::atanh()
{
    return *this = ::atanhq(m_value);
}

/// In-place natural exponential function.
/**
 * This method will set \p this to \f$ \mathrm{e} \f$ raised to the power of \p this.
 *
 * @return a reference to \p this.
 */
real128 &real128::exp()
{
    return *this = ::expq(m_value);
}

/// In-place natural logarithm.
/**
 * This method will set \p this to its natural logarithm.
 *
 * @return a reference to \p this.
 */
real128 &real128::log()
{
    return *this = ::logq(m_value);
}

/// In-place base-10 logarithm.
/**
 * This method will set \p this to its base-10 logarithm.
 *
 * @return a reference to \p this.
 */
real128 &real128::log10()
{
    return *this = ::log10q(m_value);
}

/// In-place base-2 logarithm.
/**
 * This method will set \p this to its base-2 logarithm.
 *
 * @return a reference to \p this.
 */
real128 &real128::log2()
{
    return *this = ::log2q(m_value);
}

/// In-place lgamma function.
/**
 * This method will set \p this to the value of the natural logarithm of its gamma function.
 *
 * @return a reference to \p this.
 */
real128 &real128::lgamma()
{
    return *this = ::lgammaq(m_value);
}

/// In-place error function.
/**
 * This method will set \p this to the value of its error function.
 *
 * @return a reference to \p this.
 */
real128 &real128::erf()
{
    return *this = ::erfq(m_value);
}

/// Decompose a \link mppp::real128 real128\endlink into a normalized fraction and an integral power of two.
/**
 * \rststar
 * If ``x`` is zero, this function will return zero and store zero in ``exp``. Otherwise,
 * this function will return a :cpp:class:`~mppp::real128` :math:`r` with an absolute value in the
 * :math:`\left[0.5,1\right)` range, and it will store an integer value :math:`n` in ``exp``
 * such that :math:`r \times 2^n` equals to :math:`r`. If ``x`` is a non-finite value, the return
 * value will be ``x`` and an unspecified value will be stored in ``exp``.
 * \endrststar
 *
 * @param x the input \link mppp::real128 real128\endlink.
 * @param exp a pointer to the value that will store the exponent.
 *
 * @return the binary significand of ``x``.
 */
real128 frexp(const real128 &x, int *exp)
{
    return real128{::frexpq(x.m_value, exp)};
}

/// Fused multiply-add.
/**
 * \rststar
 * This function will return :math:`\left(x \times y\right) + z` as if calculated to infinite precision and
 * rounded once.
 * \endrststar
 *
 * @param x the first factor.
 * @param y the second factor.
 * @param z the addend.
 *
 * @return \f$ \left(x \times y\right) + z \f$.
 */
real128 fma(const real128 &x, const real128 &y, const real128 &z)
{
    return real128{::fmaq(x.m_value, y.m_value, z.m_value)};
}

/// Euclidean distance.
/**
 * The calculation is performed without undue overflow or underflow during the intermediate
 * steps of the calculation.
 *
 * @param x the first \link mppp::real128 real128\endlink argument.
 * @param y the second \link mppp::real128 real128\endlink argument.
 *
 * @return the euclidean distance \f$ \sqrt{x^2+y^2} \f$.
 */
real128 hypot(const real128 &x, const real128 &y)
{
    return real128{::hypotq(x.m_value, y.m_value)};
}

// Next real128 from 'from' to 'to'.
real128 nextafter(const real128 &from, const real128 &to)
{
    return real128{::nextafterq(from.m_value, to.m_value)};
}

/// Output stream operator.
/**
 * \rststar
 * This operator will print to the stream ``os`` the :cpp:class:`~mppp::real128` ``x``. The current implementation
 * ignores any formatting flag specified in ``os``, and the print format will be the one
 * described in :cpp:func:`mppp::real128::to_string()`.
 *
 * .. warning::
 *    In future versions of mp++, the behaviour of this operator will change to support the output stream's formatting
 *    flags. For the time being, users are encouraged to use the ``quadmath_snprintf()`` function from the quadmath
 *    library if precise and forward-compatible control on the printing format is needed.
 * \endrststar
 *
 * @param os the target stream.
 * @param x the input \link mppp::real128 real128\endlink.
 *
 * @return a reference to \p os.
 *
 * @throws unspecified any exception thrown by real128::to_string().
 */
std::ostream &operator<<(std::ostream &os, const real128 &x)
{
    detail::float128_stream(os, x.m_value);
    return os;
}

} // namespace mppp

#endif
