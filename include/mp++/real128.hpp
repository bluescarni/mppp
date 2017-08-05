// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_REAL128_HPP
#define MPPP_REAL128_HPP

#include <mp++/config.hpp>

#if defined(MPPP_WITH_QUADMATH)

#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#if __cplusplus >= 201703L
#include <string_view>
#endif
#include <vector>

#include <mp++/concepts.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/quadmath.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>

namespace mppp
{

/// Quadruple-precision floating-point class.
/**
 * \rststar
 * This class represents real values encoded in the quadruple-precision IEEE 754 floating-point format
 * (which features up to 36 decimal digits of precision).
 * The class is a thin wrapper around the :cpp:type:`__float128` type and the quadmath library, available in GCC
 * on most contemporary platforms, on top of which it provides the following additions:
 *
 * * interoperability with other mp++ classes,
 * * consistent behaviour with respect to the conventions followed elsewhere in mp++ (e.g., values are
 *   default-initialised to zero rather than to indefinite values, conversions must be explicit, etc.),
 * * a generic C++ API.
 *
 * This class has the look and feel of a C++ builtin type: it can interact with most of C++'s integral and
 * floating-point primitive types (see the :cpp:concept:`~mppp::CppInteroperable` concept for the full list),
 * :cpp:class:`~mppp::integer` and :cpp:class:`~mppp::rational`,
 * and it provides overloaded :ref:`operators <real128_operators>`. Differently from the builtin types,
 * however, this class does not allow any implicit conversion to/from other types (apart from ``bool``): construction
 * from and conversion to primitive types must always be requested explicitly. As a side effect, syntax such as
 *
 * .. code-block:: c++
 *
 *    real128 r = 5.23;
 *    int m = r;
 *
 * will not work, and direct initialization and explicit casting should be used instead:
 *
 * .. code-block:: c++
 *
 *    real128 r{5.23};
 *    int m = static_cast<int>(r);
 *
 * Most of the functionality is exposed via plain :ref:`functions <real128_functions>`, with the
 * general convention that the functions are named after the corresponding quadmath functions minus the trailing ``q``
 * suffix. For instance, the quadmath call
 *
 * .. code-block:: c++
 *
 *    ::__float128 a = .5;
 *    auto b = ::sinq(a);
 *
 * that computes the sine of 0.5 in quadruple precision, storing the result in ``b``, becomes
 *
 * .. code-block:: c++
 *
 *    mppp::real128 a{.5};
 *    auto b = sin(a);
 *
 * where the ``sin()`` function is resolved via argument-dependent lookup.
 *
 * .. seealso::
 *    https://gcc.gnu.org/onlinedocs/gcc/Floating-Types.html
 *    https://gcc.gnu.org/onlinedocs/libquadmath/
 * \endrststar
 */
class real128
{
    // Number of digits in the significand.
    static constexpr unsigned sig_digits = 113;
    // Double check our assumption.
    static_assert(FLT128_MANT_DIG == sig_digits, "Invalid number of digits.");

public:
    /// Default constructor.
    /**
     * The default constructor will set \p this to zero.
     */
    constexpr real128() : m_value(0)
    {
    }
    real128(const real128 &) = default;
    real128(real128 &&) = default;
    constexpr explicit real128(::__float128 x) : m_value(x)
    {
    }
#if defined(MPPP_HAVE_CONCEPTS)
    constexpr explicit real128(CppInteroperable x)
#else
    template <typename T, cpp_interoperable_enabler<T> = 0>
    constexpr explicit real128(T x)
#endif
        : m_value(x)
    {
    }
    template <std::size_t SSize>
    explicit real128(const integer<SSize> &n)
    {
        // Special case for zero.
        const auto n_sgn = n.sgn();
        if (!n_sgn) {
            m_value = 0;
            return;
        }
        // Get the pointer to the limbs, and the size in limbs and bits.
        const ::mp_limb_t *ptr = n.is_static() ? n._get_union().g_st().m_limbs.data() : n._get_union().g_dy()._mp_d;
        const std::size_t n_bits = n.nbits();
        // Let's get the size in limbs from the size in bits, as we already made the effort.
        std::size_t ls = n_bits / unsigned(GMP_NUMB_BITS) + static_cast<bool>(n_bits % unsigned(GMP_NUMB_BITS));
        assert(ls && n_bits && ls == n.size());

        // Init value with the most significant limb, and move to the next limb.
        m_value = ptr[--ls] & GMP_NUMB_MASK;
        // Number of bits read so far from n: it is the size in bits of the top limb.
        auto read_bits = static_cast<unsigned>((n_bits % unsigned(GMP_NUMB_BITS)) ? (n_bits % unsigned(GMP_NUMB_BITS))
                                                                                  : unsigned(GMP_NUMB_BITS));
        assert(read_bits);
        // Keep on reading as long as we have limbs and as long as we haven't read enough bits
        // to fill up the significand of m_value.
        // NOTE: a paranoia check about possible overflow in read_bits += rbits.
        static_assert(sig_digits <= std::numeric_limits<unsigned>::max() - unsigned(GMP_NUMB_BITS), "Overflow error.");
        while (ls && read_bits < sig_digits) {
            // Number of bits to be read from the current limb: GMP_NUMB_BITS or less.
            const unsigned rbits = c_min(unsigned(GMP_NUMB_BITS), sig_digits - read_bits);
            // Shift m_value by rbits.
            // NOTE: safe to cast to int here, as rbits is not greater than GMP_NUMB_BITS which in turn fits in int.
            m_value = ::scalbnq(m_value, static_cast<int>(rbits));
            // Add the bottom part, and move to the next limb. We might need to remove lower bits
            // in case rbits is not exactly GMP_NUMB_BITS.
            m_value += (ptr[--ls] & GMP_NUMB_MASK) >> (unsigned(GMP_NUMB_BITS) - rbits);
            // Update the number of read bits.
            read_bits += rbits;
        }
        if (read_bits < n_bits) {
            // We did not read from n all its bits. This means that n has more bits than the quad-precision
            // significand, and thus we need to multiply this by 2**unread_bits.
            // Use the long variant of scalbn() to maximise the range.
            m_value = ::scalblnq(m_value, safe_cast<long>(n_bits - read_bits));
        }
        // Fix the sign as needed.
        if (n_sgn == -1) {
            m_value = -m_value;
        }
    }
    template <std::size_t SSize>
    explicit real128(const rational<SSize> &q)
    {
        const auto n_bits = q.get_num().nbits();
        const auto d_bits = q.get_den().nbits();
        if (n_bits <= sig_digits && d_bits <= sig_digits) {
            // Both num/den don't have more bits than quad's significand. We can just convert
            // them and divide.
            m_value = real128{q.get_num()}.m_value / real128{q.get_den()}.m_value;
        } else if (n_bits > sig_digits && d_bits <= sig_digits) {
            // Num's bit size is larger than quad's significand, den's is not. We will shift num down,
            // do the conversion, and then recover the shifted bits in the float128.
            integer<SSize> n;
            const auto shift = n_bits - sig_digits;
            tdiv_q_2exp(n, q.get_num(), safe_cast<::mp_bitcnt_t>(shift));
            m_value = real128{n}.m_value / real128{q.get_den()}.m_value;
            m_value = ::scalblnq(m_value, safe_cast<long>(shift));
        } else if (n_bits <= sig_digits && d_bits > sig_digits) {
            // The opposite of above.
            integer<SSize> d;
            const auto shift = d_bits - sig_digits;
            tdiv_q_2exp(d, q.get_den(), safe_cast<::mp_bitcnt_t>(shift));
            m_value = real128{q.get_num()}.m_value / real128{d}.m_value;
            m_value = ::scalblnq(m_value, negate_unsigned<long>(shift));
        } else {
            // Both num and den have more bits than quad's significand. We will downshift
            // both until they have 113 bits, do the division, and then recover the shifted bits.
            integer<SSize> n, d;
            const auto n_shift = n_bits - sig_digits;
            const auto d_shift = d_bits - sig_digits;
            tdiv_q_2exp(n, q.get_num(), safe_cast<::mp_bitcnt_t>(n_shift));
            tdiv_q_2exp(d, q.get_den(), safe_cast<::mp_bitcnt_t>(d_shift));
            m_value = real128{n}.m_value / real128{d}.m_value;
            if (n_shift >= d_shift) {
                m_value = ::scalblnq(m_value, safe_cast<long>(n_shift - d_shift));
            } else {
                m_value = ::scalblnq(m_value, negate_unsigned<long>(d_shift - n_shift));
            }
        }
    }
    explicit real128(const char *s) : m_value(str_to_float128(s))
    {
    }
    explicit real128(const std::string &s) : real128(s.c_str())
    {
    }
    explicit real128(const char *begin, const char *end)
    {
        MPPP_MAYBE_TLS std::vector<char> buffer;
        buffer.assign(begin, end);
        buffer.emplace_back('\0');
        m_value = str_to_float128(buffer.data());
    }
#if __cplusplus >= 201703L
    explicit real128(const std::string_view &s) : real128(s.data(), s.data() + s.size())
    {
    }
#endif
    real128 &operator=(const real128 &) = default;
    real128 &operator=(real128 &&) = default;
#if defined(MPPP_HAVE_CONCEPTS)
    template <CppInteroperable T>
#else
    template <typename T, cpp_interoperable_enabler<T> = 0>
#endif
    constexpr explicit operator T() const
    {
        return static_cast<T>(m_value);
    }
    constexpr explicit operator ::__float128() const
    {
        return m_value;
    }
    template <std::size_t SSize>
    explicit operator integer<SSize>() const
    {
        // Build the union and assign the value.
        ieee_float128 ief;
        ief.value = m_value;
        if (mppp_unlikely(ief.i_eee.exponent == 32767u)) {
            // Inf or nan, not representable by integer.
            throw std::domain_error("Cannot convert a non-finite real128 to an integer");
        }
        // Determine the real exponent. 16383 is the offset of the representation,
        // 112 is because we need to left shift the significand by 112 positions
        // in order to turn it into an integral value.
        const auto exponent = static_cast<long>(ief.i_eee.exponent) - (16383l + 112);
        if (!ief.i_eee.exponent || exponent < -112) {
            // Zero stored exponent means subnormal number, and if the real exponent is too small
            // we end up with a value with abs less than 1. In such cases, just return 0.
            return integer<SSize>{};
        }
        // Value is normalised and not less than 1 in abs. We can proceed.
        integer<SSize> retval{1u};
        if (exponent >= 0) {
            // Non-negative exponent means that we will have to take the significand
            // converted to an integer and further left shift it.
            retval <<= 112u;
            retval += integer<SSize>{ief.i_eee.mant_high} << 64u;
            retval += ief.i_eee.mant_low;
            retval <<= static_cast<unsigned long>(exponent);
        } else {
            // NOTE: the idea here is to avoid shifting up and then shifting back down,
            // as that might trigger a promotion to dynamic storage in retval. Instead,
            // we offset the shifts by the (negative) exponent, which is here guaranteed
            // to be in the [-112,-1] range.
            retval <<= static_cast<unsigned>(112 + exponent);
            if (exponent > -64) {
                // We need to right shift less than 64 bits. This means that some bits from the low
                // word of the significand survive.
                // NOTE: need to do the left shift in multiprecision here, as the final result
                // might spill over the 64 bit range.
                retval += integer<SSize>{ief.i_eee.mant_high} << static_cast<unsigned>(exponent + 64);
                retval += ief.i_eee.mant_low >> static_cast<unsigned>(-exponent);
            } else {
                // We need to right shift more than 64 bits, so none of the bits in the low word survive.
                // NOTE: here the right shift will be in the [0,48] range, so we can do it directly
                // on a C++ builtin type (i_eee.mant_high gives an ull, which is guaranteed to be
                // at least 64 bit).
                retval += ief.i_eee.mant_high >> static_cast<unsigned>(-(exponent + 64));
            }
        }
        // Adjust the sign.
        if (ief.i_eee.negative) {
            retval.neg();
        }
        return retval;
    }
    template <std::size_t SSize>
    explicit operator rational<SSize>() const
    {
        // Build the union and assign the value.
        ieee_float128 ief;
        ief.value = m_value;
        if (mppp_unlikely(ief.i_eee.exponent == 32767u)) {
            // Inf or nan, not representable by rational.
            throw std::domain_error("Cannot convert a non-finite real128 to a rational");
        }
        rational<SSize> retval;
        if (ief.i_eee.exponent) {
            // Normal number.
            // Determine the real exponent.
            const auto exponent = static_cast<long>(ief.i_eee.exponent) - (16383l + 112);
            retval._get_num() = 1u;
            retval._get_num() <<= 112u;
            retval._get_num() += integer<SSize>{ief.i_eee.mant_high} << 64u;
            retval._get_num() += ief.i_eee.mant_low;
            if (exponent >= 0) {
                // The result is an integer: no need to canonicalise or to try
                // to demote. Den is already set to 1.
                retval._get_num() <<= static_cast<unsigned long>(exponent);
            } else {
                retval._get_den() <<= static_cast<unsigned long>(-exponent);
                // Put in canonical form.
                canonicalise(retval);
                // Try demoting, after having possibly removed common factors.
                retval._get_num().demote();
                retval._get_den().demote();
            }
        } else {
            // Subnormal number.
            retval._get_num() = ief.i_eee.mant_high;
            retval._get_num() <<= 64u;
            retval._get_num() += ief.i_eee.mant_low;
            retval._get_den() <<= static_cast<unsigned long>(16382l + 112);
            canonicalise(retval);
            retval._get_num().demote();
            retval._get_den().demote();
        }
        // Adjust the sign.
        if (ief.i_eee.negative) {
            retval.neg();
        }
        return retval;
    }
    /// The internal value.
    /**
     * \rststar
     * This class member gives direct access to the :cpp:type:`__float128` instance stored
     * inside :cpp:class:`~mppp::real128`.
     * \endrststar
     */
    ::__float128 m_value;
};

inline std::ostream &operator<<(std::ostream &os, const real128 &r)
{
    float128_stream(os, r.m_value);
    return os;
}
}

#else

#error The real128.hpp header was included but mp++ was not configured with the MPPP_WITH_QUADMATH option.

#endif

#endif
