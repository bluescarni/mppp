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
#include <vector>

#include <mp++/concepts.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/quadmath.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>

namespace mppp
{

class real128
{
    // Number of digits in the significand.
    static constexpr unsigned sig_digits = 113;
    // Double check our assumption.
    static_assert(FLT128_MANT_DIG == sig_digits, "Invalid number of digits.");

public:
    constexpr real128() : m_value(0)
    {
    }
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
            if (mppp_unlikely(n_bits - read_bits > static_cast<unsigned long>(std::numeric_limits<long>::max()))) {
                throw std::overflow_error(
                    "Overflow in the construction of a real128 from an integer: the second argument to scalblnq() is "
                    + std::to_string(n_bits - read_bits) + ", but the max allowed value is "
                    + std::to_string(std::numeric_limits<long>::max()));
            }
            // Use the long variant of scalbn() to maximise the range.
            m_value = ::scalblnq(m_value, static_cast<long>(n_bits - read_bits));
        }
        // Fix the sign as needed.
        if (n_sgn == -1) {
            m_value = -m_value;
        }
    }
    template <std::size_t SSize>
    explicit real128(const rational<SSize> &q) : m_value(real128{q.get_num()}.value() / real128{q.get_den()}.value())
    {
    }
    explicit real128(const char *s) : m_value(str_to_float128(s))
    {
    }
    explicit real128(const char *begin, const char *end)
    {
        MPPP_MAYBE_TLS std::vector<char> buffer;
        buffer.assign(begin, end);
        buffer.emplace_back('\0');
        m_value = str_to_float128(buffer.data());
    }
    explicit real128(const std::string &s) : real128(s.c_str())
    {
    }
    real128(const real128 &) = default;
    real128(real128 &&) = default;
    real128 &operator=(const real128 &) = default;
    real128 &operator=(real128 &&) = default;
    // NOTE: drop the "::" here as it confuses clang-format.
    MPPP_MAYBE_CONSTEXPR __float128 &value()
    {
        return m_value;
    }
    MPPP_MAYBE_CONSTEXPR const ::__float128 &value() const
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
        // Get the sign.
        const bool neg = ief.i_eee.negative;
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
                retval += (ief.i_eee.mant_high >> static_cast<unsigned>(-(exponent + 64)));
            }
        }
        // Adjust the sign.
        if (neg) {
            retval.neg();
        }
        return retval;
    }
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

private:
    ::__float128 m_value;
};

inline std::ostream &operator<<(std::ostream &os, const real128 &r)
{
    float128_stream(os, r.value());
    return os;
}
}

#else

#error The real128.hpp header was included but mp++ was not configured with the MPPP_WITH_QUADMATH option.

#endif

#endif
