// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
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
#include <cmath>
#include <complex>
#include <cstdint>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#if defined(MPPP_HAVE_STRING_VIEW)

#include <string_view>

#endif

#if defined(MPPP_WITH_BOOST_S11N)

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/tracking.hpp>

#endif

#include <mp++/concepts.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/detail/visibility.hpp>
#include <mp++/fwd.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>

namespace mppp
{

namespace detail
{

// Machinery for low-level manipulation of __float128. Inspired by:
// https://github.com/gcc-mirror/gcc/blob/master/libquadmath/quadmath-imp.h
// Note that the union machinery is technically UB, but well-defined on GCC as
// an extension:
// https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#Type%2Dpunning
// quadmath so far is limited to GCC and perhaps clang, which I'd imagine
// implements the same extension for GCC compatibility. So we should be ok.

// The ieee fields.
struct ieee_t {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    std::uint_least8_t negative : 1;
    std::uint_least16_t exponent : 15;
    std::uint_least64_t mant_high : 48;
    std::uint_least64_t mant_low : 64;
#else
    std::uint_least64_t mant_low : 64;
    std::uint_least64_t mant_high : 48;
    std::uint_least16_t exponent : 15;
    std::uint_least8_t negative : 1;
#endif
}
#if defined(__MINGW32__)
// On mingw targets the ms-bitfields option is active by default.
// Therefore enforce gnu-bitfield style.
__attribute__((gcc_struct))
#endif
;

// The union.
union ieee_float128 {
    __float128 value;
    ieee_t i_eee;
};

// Wrappers for use in various functions below (so that
// we don't have to include quadmath.h here).
MPPP_DLL_PUBLIC __float128 scalbnq(__float128, int);
MPPP_DLL_PUBLIC __float128 scalblnq(__float128, long);

// For internal use only.
template <typename T>
using is_real128_mppp_interoperable = disjunction<is_integer<T>, is_rational<T>>;

// Story time!
//
// Since 3.9, clang supports the __float128 type. However, interactions between long double and __float128 are disabled.
// The rationale is given here:
//
// https://reviews.llvm.org/D15120
//
// Basically, it boils down to the fact that on at least one platform (PPC) the long double type (which is implemented
// as a double-double) can represent exactly some values that __float128 cannot (double-double is a strange beast).
// On the other hand, __float128 can also
// represent values that double-double cannot, so it is not clear which floating-point type should have the higher
// rank. Note that C and C++ mandate, for the standard FP types, that higher rank FPs can represent all values
// that lower rank FPs can. So, the decision was made to just disable the interaction. This is in line with the
// following TS for che C language ("Floating-point extensions for C"):
//
// http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1945.pdf (page 20)
//
// (this is not necessarily a very good way of doing things though, as it seems to create portability concerns)
//
// This is part of a larger problem having to do with the fact that, while the properties of __float128 are exactly
// specified (i.e., quad-precision IEEE), the properties of the standard floating-point types are *not*. So, a
// *truly* generic real128 class would need, for instance:
//
// - to check which C++ types can interact with __float128 and enable only those,
// - possibly return types other than real128 in binary operations (maybe long double is strictly bigger than quad
//   precision?),
// - etc. etc.
//
// For the moment, though, it seems like on all platforms __float128 is at least the same rank as double. For long
// double, clang and GCC diverge, and we follow whatever the compiler is doing. So we just hard-code the behaviour
// here, we can always write a more sophisticated solution later if the need arises.
//
// NOTE: since version 7 the behaviour of clang changed, now matching GCC. See the logic implemented
// in config.hpp.

// For internal use only.
template <typename T>
using is_real128_cpp_interoperable = detail::conjunction<is_cpp_arithmetic<T>
#if !defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
                                                         ,
                                                         detail::negation<std::is_same<T, long double>>
#endif
                                                         >;

} // namespace detail

template <typename T>
using is_real128_interoperable
    = detail::disjunction<detail::is_real128_cpp_interoperable<T>, detail::is_real128_mppp_interoperable<T>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL real128_interoperable = is_real128_interoperable<T>::value;

#endif

template <typename T>
using is_real128_cpp_complex = detail::conjunction<is_cpp_complex<T>
#if !defined(MPPP_FLOAT128_WITH_LONG_DOUBLE)
                                                   ,
                                                   detail::negation<std::is_same<T, std::complex<long double>>>
#endif
                                                   >;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T>
MPPP_CONCEPT_DECL real128_cpp_complex = is_real128_cpp_complex<T>::value;

#endif

template <typename T, typename U>
using are_real128_op_types
    = detail::disjunction<detail::conjunction<std::is_same<T, real128>, std::is_same<U, real128>>,
                          detail::conjunction<std::is_same<T, real128>, is_real128_interoperable<U>>,
                          detail::conjunction<std::is_same<U, real128>, is_real128_interoperable<T>>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, typename U>
MPPP_CONCEPT_DECL real128_op_types = are_real128_op_types<T, U>::value;

#endif

// Fwd declare the abs() function.
#if !defined(__INTEL_COMPILER)
constexpr
#endif
    real128
    abs(const real128 &);

// For the future:
// - the constructor from integer *may* be implemented in a faster way by reading directly the hi/lo parts
//   and writing them to the ieee union (instead right now we are using __float128 arithmetics and quadmath
//   functions). Make sure to benchmark first though...
// - should we change the cast operator to C++ types to check the result of the conversion?

// Quadruple-precision floating-point class.
class MPPP_DLL_PUBLIC real128
{
#if defined(MPPP_WITH_BOOST_S11N)
    // Boost serialization support.
    friend class boost::serialization::access;

    template <typename Archive>
    void save(Archive &ar, unsigned) const
    {
        ar << to_string();
    }

    template <typename Archive>
    void load(Archive &ar, unsigned)
    {
        std::string tmp;
        ar >> tmp;

        *this = real128{tmp};
    }

    // Overloads for binary archives.
    void save(boost::archive::binary_oarchive &, unsigned) const;
    void load(boost::archive::binary_iarchive &, unsigned);

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif

    // Number of digits in the significand.
    static constexpr unsigned sig_digits = 113;

public:
    // Default constructor.
    constexpr real128() : m_value(0) {}

    // Trivial copy constructor.
    real128(const real128 &) = default;
    // Trivial move constructor.
    real128(real128 &&) = default;

    // Constructor from a quadruple-precision floating-point value.
    // NOTE: early GCC versions seem to exhibit constexpr
    // failures if x is passed by const reference.
    constexpr explicit real128(
#if defined(__GNUC__) && __GNUC__ < 5
        __float128 x
#else
        const __float128 &x
#endif
        )
        : m_value(x)
    {
    }

private:
    // Cast an integer to a __float128.
    template <std::size_t SSize>
    static __float128 cast_to_float128(const integer<SSize> &n)
    {
        // Special case for zero.
        const auto n_sgn = n.sgn();
        if (!n_sgn) {
            return 0;
        }

        // Get the pointer to the limbs, and the size in limbs and bits.
        const ::mp_limb_t *ptr = n.is_static() ? n._get_union().g_st().m_limbs.data() : n._get_union().g_dy()._mp_d;
        const std::size_t n_bits = n.nbits();

        // Let's get the size in limbs from the size in bits, as we already made the effort.
        const auto rem_bits = n_bits % unsigned(GMP_NUMB_BITS);
        std::size_t ls = n_bits / unsigned(GMP_NUMB_BITS) + static_cast<bool>(rem_bits);
        assert(ls && n_bits && ls == n.size());

        // Init value with the most significant limb, and move to the next limb.
        __float128 retval = ptr[--ls] & GMP_NUMB_MASK;

        // Number of bits read so far from n: it is the size in bits of the top limb.
        auto read_bits = static_cast<unsigned>(rem_bits ? rem_bits : unsigned(GMP_NUMB_BITS));
        assert(read_bits);

        // Keep on reading as long as we have limbs and as long as we haven't read enough bits
        // to fill up the significand of retval.
        while (ls && read_bits < sig_digits) {
            // Number of bits to be read from the current limb: GMP_NUMB_BITS or less.
            const unsigned rbits = detail::c_min(unsigned(GMP_NUMB_BITS), sig_digits - read_bits);

            // Shift retval by rbits.
            // NOTE: safe to cast to int here, as rbits is not greater than GMP_NUMB_BITS which in turn fits in int.
            retval = detail::scalbnq(retval, static_cast<int>(rbits));

            // Add the bottom part, and move to the next limb. We might need to remove lower bits
            // in case rbits is not exactly GMP_NUMB_BITS.
            retval += (ptr[--ls] & GMP_NUMB_MASK) >> (unsigned(GMP_NUMB_BITS) - rbits);

            // Update the number of read bits.
            // NOTE: read_bits can never be increased past sig_digits, due to the definition of rbits.
            // Hence, this addition can never overflow (as sig_digits is unsigned itself).
            read_bits += rbits;
        }

        if (read_bits < n_bits) {
            // We did not read from n all its bits. This means that n has more bits than the quad-precision
            // significand, and thus we need to multiply this by 2**unread_bits.
            // Use the long variant of scalbn() to maximise the range.
            retval = detail::scalblnq(retval, detail::safe_cast<long>(n_bits - read_bits));
        }

        // Fix the sign as needed.
        if (n_sgn == -1) {
            retval = -retval;
        }

        return retval;
    }

    // Cast a rational to a __float128.
    template <std::size_t SSize>
    static __float128 cast_to_float128(const rational<SSize> &q)
    {
        const auto n_bits = q.get_num().nbits();
        const auto d_bits = q.get_den().nbits();

        if (n_bits <= sig_digits && d_bits <= sig_digits) {
            // Both num/den don't have more bits than quad's significand. We can just convert
            // them and divide.
            return real128{q.get_num()}.m_value / real128{q.get_den()}.m_value;
        } else if (n_bits > sig_digits && d_bits <= sig_digits) {
            // Num's bit size is larger than quad's significand, den's is not. We will shift num down,
            // do the conversion, and then recover the shifted bits in the float128.
            MPPP_MAYBE_TLS integer<SSize> n;

            const auto shift = n_bits - sig_digits;
            tdiv_q_2exp(n, q.get_num(), detail::safe_cast<::mp_bitcnt_t>(shift));
            auto retval = real128{n}.m_value / real128{q.get_den()}.m_value;
            return detail::scalblnq(retval, detail::safe_cast<long>(shift));
        } else if (n_bits <= sig_digits && d_bits > sig_digits) {
            // The opposite of above.
            MPPP_MAYBE_TLS integer<SSize> d;

            const auto shift = d_bits - sig_digits;
            tdiv_q_2exp(d, q.get_den(), detail::safe_cast<::mp_bitcnt_t>(shift));
            auto retval = real128{q.get_num()}.m_value / real128{d}.m_value;
            return detail::scalblnq(retval, detail::negate_unsigned<long>(shift));
        } else {
            // Both num and den have more bits than quad's significand. We will downshift
            // both until they have 113 bits, do the division, and then recover the shifted bits.
            MPPP_MAYBE_TLS integer<SSize> n;
            MPPP_MAYBE_TLS integer<SSize> d;

            const auto n_shift = n_bits - sig_digits;
            const auto d_shift = d_bits - sig_digits;

            tdiv_q_2exp(n, q.get_num(), detail::safe_cast<::mp_bitcnt_t>(n_shift));
            tdiv_q_2exp(d, q.get_den(), detail::safe_cast<::mp_bitcnt_t>(d_shift));

            auto retval = real128{n}.m_value / real128{d}.m_value;
            if (n_shift >= d_shift) {
                return detail::scalblnq(retval, detail::safe_cast<long>(n_shift - d_shift));
            } else {
                return detail::scalblnq(retval, detail::negate_unsigned<long>(d_shift - n_shift));
            }
        }
    }

    // Cast C++ types to a __float128.
    template <typename T>
    static constexpr __float128 cast_to_float128(const T &x)
    {
        return static_cast<__float128>(x);
    }

public:
    // Constructor from interoperable types.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real128_interoperable T>
#else
    template <typename T, detail::enable_if_t<is_real128_interoperable<T>::value, int> = 0>
#endif
    constexpr real128(const T &x) : m_value(cast_to_float128(x))
    {
    }
    // Constructor from std::complex.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real128_cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_real128_cpp_complex<T>::value, int> = 0>
#endif
    MPPP_CONSTEXPR_14 explicit real128(const T &c)
        : m_value(c.imag() == 0
                      ? c.real()
                      : throw std::domain_error(
                          "Cannot construct a real128 from a complex C++ value with a non-zero imaginary part of "
                          + detail::to_string(c.imag())))
    {
    }

private:
    // A tag to call private ctors.
    struct ptag {
    };
    explicit real128(const ptag &, const char *);
    explicit real128(const ptag &, const std::string &);
#if defined(MPPP_HAVE_STRING_VIEW)
    explicit real128(const ptag &, const std::string_view &);
#endif

public:
    // Constructor from string.
#if defined(MPPP_HAVE_CONCEPTS)
    template <string_type T>
#else
    template <typename T, detail::enable_if_t<is_string_type<T>::value, int> = 0>
#endif
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    explicit real128(const T &s) : real128(ptag{}, s)
    {
    }
    // Constructor from range of characters.
    explicit real128(const char *, const char *);

    ~real128() = default;

    // Trivial copy assignment operator.
    real128 &operator=(const real128 &) = default;
    // Trivial move assignment operator.
    real128 &operator=(real128 &&) = default;

    // Assignment from a quadruple-precision floating-point value.
    MPPP_CONSTEXPR_14 real128 &operator=(const __float128 &x)
    {
        m_value = x;
        return *this;
    }

    // Assignment from interoperable types.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real128_interoperable T>
#else
    template <typename T, detail::enable_if_t<is_real128_interoperable<T>::value, int> = 0>
#endif
    MPPP_CONSTEXPR_14 real128 &operator=(const T &x)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
        return *this = real128{x};
    }

    // Assignment from C++ complex.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real128_cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_real128_cpp_complex<T>::value, int> = 0>
#endif
    MPPP_CONSTEXPR_14 real128 &operator=(const T &c)
    {
        // NOTE: icpc does not like __builtin_expect()
        // in constexpr contexts.
#if defined(__INTEL_COMPILER)
        if (c.imag() != 0) {
#else
        if (mppp_unlikely(c.imag()) != 0) {
#endif
            throw std::domain_error("Cannot assign a complex C++ value with a non-zero imaginary part of "
                                    + detail::to_string(c.imag()) + " to a real128");
        }
        // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
        return *this = c.real();
    }
    // Declaration of the assignments from
    // other mp++ classes.
#if defined(MPPP_WITH_MPFR)
    real128 &operator=(const real &);
#endif
    MPPP_CONSTEXPR_14 real128 &operator=(const complex128 &);
#if defined(MPPP_WITH_MPC)
    real128 &operator=(const complex &);
#endif

    // Assignment from string.
#if defined(MPPP_HAVE_CONCEPTS)
    template <string_type T>
#else
    template <typename T, detail::enable_if_t<is_string_type<T>::value, int> = 0>
#endif
    real128 &operator=(const T &s)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
        return *this = real128{s};
    }

    // Conversion to quadruple-precision floating-point.
    constexpr explicit operator __float128() const
    {
        return m_value;
    }

private:
    // Conversion to C++ types.
    template <typename T>
    MPPP_NODISCARD constexpr T dispatch_conversion(std::true_type) const
    {
        return static_cast<T>(m_value);
    }

    // Conversion to integer.
    template <std::size_t SSize>
    bool mppp_conversion(integer<SSize> &rop) const
    {
        // Build the union and assign the value.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
        detail::ieee_float128 ief;
        ief.value = m_value;
        if (mppp_unlikely(ief.i_eee.exponent == 32767u)) {
            // Inf or nan, not representable by integer.
            return false;
        }
        // Determine the real exponent. 16383 is the offset of the representation,
        // 112 is because we need to left shift the significand by 112 positions
        // in order to turn it into an integral value.
        const auto exponent = static_cast<long>(ief.i_eee.exponent) - (16383l + 112);
        if (!ief.i_eee.exponent || exponent < -112) {
            // Zero stored exponent means subnormal number, and if the real exponent is too small
            // we end up with a value with abs less than 1. In such cases, just return 0.
            rop.set_zero();
            return true;
        }
        // Value is normalised and not less than 1 in abs. We can proceed.
        rop.set_one();
        if (exponent >= 0) {
            // Non-negative exponent means that we will have to take the significand
            // converted to an integer and further left shift it.
            rop <<= 112u;
            rop += integer<SSize>{ief.i_eee.mant_high} << 64u;
            rop += ief.i_eee.mant_low;
            rop <<= static_cast<unsigned long>(exponent);
        } else {
            // NOTE: the idea here is to avoid shifting up and then shifting back down,
            // as that might trigger a promotion to dynamic storage in retval. Instead,
            // we offset the shifts by the (negative) exponent, which is here guaranteed
            // to be in the [-112,-1] range.
            rop <<= static_cast<unsigned>(112 + exponent);
            if (exponent > -64) {
                // We need to right shift less than 64 bits. This means that some bits from the low
                // word of the significand survive.
                // NOTE: need to do the left shift in multiprecision here, as the final result
                // might spill over the 64 bit range.
                rop += integer<SSize>{ief.i_eee.mant_high} << static_cast<unsigned>(exponent + 64);
                rop += ief.i_eee.mant_low >> static_cast<unsigned>(-exponent);
            } else {
                // We need to right shift more than 64 bits, so none of the bits in the low word survive.
                // NOTE: here the right shift will be in the [0,48] range, so we can do it directly
                // on a C++ builtin type (i_eee.mant_high gives a 64bit int).
                rop += ief.i_eee.mant_high >> static_cast<unsigned>(-(exponent + 64));
            }
        }
        // Adjust the sign.
        if (ief.i_eee.negative) {
            rop.neg();
        }
        return true;
    }

    // Conversion to rational.
    template <std::size_t SSize>
    bool mppp_conversion(rational<SSize> &rop) const
    {
        // Build the union and assign the value.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
        detail::ieee_float128 ief;
        ief.value = m_value;
        if (mppp_unlikely(ief.i_eee.exponent == 32767u)) {
            // Inf or nan, not representable by rational.
            return false;
        }
        rop._get_num().set_zero();
        rop._get_den().set_one();
        if (ief.i_eee.exponent) {
            // Normal number.
            // Determine the real exponent.
            const auto exponent = static_cast<long>(ief.i_eee.exponent) - (16383l + 112);
            rop._get_num() = 1u;
            rop._get_num() <<= 112u;
            rop._get_num() += integer<SSize>{ief.i_eee.mant_high} << 64u;
            rop._get_num() += ief.i_eee.mant_low;
            if (exponent >= 0) {
                // The result is a large integer: no need to canonicalise or to try
                // to demote. Den is already set to 1.
                rop._get_num() <<= static_cast<unsigned long>(exponent);
            } else {
                rop._get_den() <<= static_cast<unsigned long>(-exponent);
                // Put in canonical form.
                canonicalise(rop);
                // Try demoting, after having possibly removed common factors.
                rop._get_num().demote();
                rop._get_den().demote();
            }
        } else {
            // Subnormal number.
            rop._get_num() = ief.i_eee.mant_high;
            rop._get_num() <<= 64u;
            rop._get_num() += ief.i_eee.mant_low;
            rop._get_den() <<= static_cast<unsigned long>(16382l + 112);
            canonicalise(rop);
            // Try demoting.
            rop._get_num().demote();
            rop._get_den().demote();
        }
        // Adjust the sign.
        if (ief.i_eee.negative) {
            rop.neg();
        }
        return true;
    }

    // Conversion to mp++ types.
    template <typename T>
    MPPP_NODISCARD T dispatch_conversion(std::false_type) const
    {
        T retval;
        if (mppp_unlikely(!mppp_conversion(retval))) {
            throw std::domain_error(std::string{"Cannot convert a non-finite real128 to "}
                                    + (detail::is_integer<T>::value ? "an integer" : "a rational"));
        }
        return retval;
    }

public:
    // Conversion operator to interoperable types.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real128_interoperable T>
#else
    template <typename T, detail::enable_if_t<is_real128_interoperable<T>::value, int> = 0>
#endif
    constexpr explicit operator T() const
    {
        return dispatch_conversion<T>(detail::is_real128_cpp_interoperable<T>{});
    }

    // Conversion operator to C++ complex types.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real128_cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_real128_cpp_complex<T>::value, int> = 0>
#endif
    MPPP_CONSTEXPR_14 explicit operator T() const
    {
        using value_t = typename T::value_type;

        return T{static_cast<value_t>(*this), value_t(0)};
    }

private:
    // get() implementation for C++ types.
    template <typename T>
    MPPP_CONSTEXPR_14 bool dispatch_get(T &rop, std::true_type) const
    {
        return rop = static_cast<T>(m_value), true;
    }
    // get() implementation for mp++ types.
    template <typename T>
    bool dispatch_get(T &rop, std::false_type) const
    {
        return mppp_conversion(rop);
    }

public:
    // Conversion member function to interoperable types.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real128_interoperable T>
#else
    template <typename T, detail::enable_if_t<is_real128_interoperable<T>::value, int> = 0>
#endif
    MPPP_CONSTEXPR_14 bool get(T &rop) const
    {
        return dispatch_get(rop, detail::is_real128_cpp_interoperable<T>{});
    }

    // Conversion member function to C++ complex types.
#if defined(MPPP_HAVE_CONCEPTS)
    template <real128_cpp_complex T>
#else
    template <typename T, detail::enable_if_t<is_real128_cpp_complex<T>::value, int> = 0>
#endif
    MPPP_CONSTEXPR_20 bool get(T &rop) const
    {
        using value_type = typename T::value_type;

        // NOTE: constexpr mutation of a std::complex
        // seems to be available only since C++20:
        // https://en.cppreference.com/w/cpp/numeric/complex/real
        // https://en.cppreference.com/w/cpp/numeric/complex/operator%3D
        rop.real(static_cast<value_type>(m_value));
        rop.imag(static_cast<value_type>(0));

        return true;
    }

    // Convert to string.
    MPPP_NODISCARD std::string to_string() const;

    // Unbiased exponent.
    MPPP_NODISCARD int ilogb() const;
#if defined(MPPP_QUADMATH_HAVE_LOGBQ)
    MPPP_NODISCARD real128 logb() const;
#endif

    // Get the IEEE representation of the value.
    MPPP_NODISCARD std::tuple<std::uint_least8_t, std::uint_least16_t, std::uint_least64_t, std::uint_least64_t>
    get_ieee() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
        detail::ieee_float128 ie;
        ie.value = m_value;
        return std::make_tuple(std::uint_least8_t(ie.i_eee.negative), std::uint_least16_t(ie.i_eee.exponent),
                               std::uint_least64_t(ie.i_eee.mant_high), std::uint_least64_t(ie.i_eee.mant_low));
    }
    // Sign bit.
    MPPP_NODISCARD bool signbit() const;
    // Categorise the floating point value.
    MPPP_NODISCARD
#if !defined(__INTEL_COMPILER)
    constexpr
#endif
        int
        fpclassify() const
    {
        // NOTE: according to the docs the builtin accepts generic floating-point types:
        // https://gcc.gnu.org/onlinedocs/gcc-7.2.0/gcc/Other-Builtins.html
        // It is used internally in the quadmath library as well:
        // https://github.com/gcc-mirror/gcc/blob/master/libquadmath/quadmath-imp.h
        return __builtin_fpclassify(FP_NAN, FP_INFINITE, FP_NORMAL, FP_SUBNORMAL, FP_ZERO, m_value);
    }
    // Detect NaN.
    MPPP_NODISCARD
#if !defined(__INTEL_COMPILER)
    constexpr
#endif
        bool
        isnan() const
    {
        return fpclassify() == FP_NAN;
    }
    // Detect infinity.
    MPPP_NODISCARD
#if !defined(__INTEL_COMPILER)
    constexpr
#endif
        bool
        isinf() const
    {
        return fpclassify() == FP_INFINITE;
    }
    // Detect finite value.
    MPPP_NODISCARD
#if !defined(__INTEL_COMPILER)
    constexpr
#endif
        bool
        finite() const
    {
        return !isnan() && !isinf();
    }
    MPPP_NODISCARD
#if !defined(__INTEL_COMPILER)
    constexpr
#endif
        bool
        isfinite() const
    {
        return finite();
    }
    // Detect normal value.
    MPPP_NODISCARD
#if !defined(__INTEL_COMPILER)
    constexpr
#endif
        bool
        isnormal() const
    {
        return fpclassify() == FP_NORMAL;
    }

    // In-place absolute value.
#if !defined(__INTEL_COMPILER)
    MPPP_CONSTEXPR_14
#endif
    real128 &abs()
    {
        return *this = mppp::abs(*this);
    }
#if !defined(__INTEL_COMPILER)
    MPPP_CONSTEXPR_14
#endif
    real128 &fabs()
    {
        return this->abs();
    }

    // In-place roots.
    real128 &sqrt();
    real128 &cbrt();

    // In-place trigonometric functions.
    real128 &sin();
    real128 &cos();
    real128 &tan();
    real128 &asin();
    real128 &acos();
    real128 &atan();

    // In-place hyperbolic functions.
    real128 &sinh();
    real128 &cosh();
    real128 &tanh();
    real128 &asinh();
    real128 &acosh();
    real128 &atanh();

    // In-place exponentials and logarithms.
    real128 &exp();
#if defined(MPPP_QUADMATH_HAVE_EXP2Q)
    real128 &exp2();
#endif
    real128 &expm1();
    real128 &log();
    real128 &log10();
    real128 &log2();
    real128 &log1p();

    // In-place gamma functions.
    real128 &lgamma();
    real128 &tgamma();

    // In-place Bessel functions.
    real128 &j0();
    real128 &j1();
    real128 &y0();
    real128 &y1();

    // In-place error functions.
    real128 &erf();
    real128 &erfc();

    // Integer and remainder-related functions.
    real128 &ceil();
    real128 &floor();
    real128 &nearbyint();
    real128 &rint();
    real128 &round();
    real128 &trunc();

    // The internal value.
    // NOLINTNEXTLINE(modernize-use-default-member-init)
    __float128 m_value;
};

// Double check that real128 is a standard layout class.
static_assert(std::is_standard_layout<real128>::value, "real128 is not a standard layout class.");

// Conversion function.
template <typename T>
inline MPPP_CONSTEXPR_14 auto get(T &rop, const real128 &x) -> decltype(x.get(rop))
{
    return x.get(rop);
}

// Decompose into a normalized fraction and an integral power of two.
MPPP_DLL_PUBLIC real128 frexp(const real128 &, int *);

// Unbiased exponent.
MPPP_DLL_PUBLIC int ilogb(const real128 &);
#if defined(MPPP_QUADMATH_HAVE_LOGBQ)
MPPP_DLL_PUBLIC real128 logb(const real128 &);
#endif

// Fused multiply-add.
MPPP_DLL_PUBLIC real128 fma(const real128 &, const real128 &, const real128 &);

// Absolute value.
#if defined(__INTEL_COMPILER)
inline
#else
constexpr
#endif
    real128
    abs(const real128 &x)
{
    return x.fpclassify() == FP_NAN
               ? x
               : (x.fpclassify() == FP_ZERO ? real128{} : (x.m_value < 0 ? real128{-x.m_value} : x));
}

#if defined(__INTEL_COMPILER)
inline
#else
constexpr
#endif
    real128
    fabs(const real128 &x)
{
    return mppp::abs(x);
}

// Multiply by power of 2.
MPPP_DLL_PUBLIC real128 scalbn(const real128 &, int);
MPPP_DLL_PUBLIC real128 scalbln(const real128 &, long);
MPPP_DLL_PUBLIC real128 ldexp(const real128 &, int);

// Output stream operator.
MPPP_DLL_PUBLIC std::ostream &operator<<(std::ostream &, const real128 &);

// Sign bit.
inline bool signbit(const real128 &x)
{
    return x.signbit();
}

// Categorisation.
#if defined(__INTEL_COMPILER)
inline
#else
constexpr
#endif
    int
    fpclassify(const real128 &x)
{
    return x.fpclassify();
}

// Detect NaN.
#if defined(__INTEL_COMPILER)
inline
#else
constexpr
#endif
    bool
    isnan(const real128 &x)
{
    return x.isnan();
}

// Detect infinity.
#if defined(__INTEL_COMPILER)
inline
#else
constexpr
#endif
    bool
    isinf(const real128 &x)
{
    return x.isinf();
}

// Detect finite value.
#if defined(__INTEL_COMPILER)
inline
#else
constexpr
#endif
    bool
    finite(const real128 &x)
{
    return x.finite();
}

#if defined(__INTEL_COMPILER)
inline
#else
constexpr
#endif
    bool
    isfinite(const real128 &x)
{
    return x.isfinite();
}

// Detect normal value.
#if defined(__INTEL_COMPILER)
inline
#else
constexpr
#endif
    bool
    isnormal(const real128 &x)
{
    return x.isnormal();
}

// Equality predicate with special NaN handling.
#if !defined(__INTEL_COMPILER)
constexpr
#endif
    bool
    real128_equal_to(const real128 &, const real128 &);

// Less-than predicate with special NaN handling.
#if !defined(__INTEL_COMPILER)
constexpr
#endif
    bool
    real128_lt(const real128 &, const real128 &);

// Greater-than predicate with special NaN handling.
#if !defined(__INTEL_COMPILER)
constexpr
#endif
    bool
    real128_gt(const real128 &, const real128 &);

// Roots.
MPPP_DLL_PUBLIC real128 sqrt(const real128 &);
MPPP_DLL_PUBLIC real128 cbrt(const real128 &);

// Machinery to define binary operations involving real128.
#if defined(MPPP_HAVE_CONCEPTS)
#define MPPP_REAL128_BINARY_OP_HEADER                                                                                  \
    template <typename T, typename U>                                                                                  \
    requires real128_op_types<T, U>
#else
#define MPPP_REAL128_BINARY_OP_HEADER                                                                                  \
    template <typename T, typename U, detail::enable_if_t<are_real128_op_types<T, U>::value, int> = 0>
#endif

#define MPPP_REAL128_IMPLEMENT_BINARY_OPERATION(fname)                                                                 \
    namespace detail                                                                                                   \
    {                                                                                                                  \
    MPPP_DLL_PUBLIC real128 dispatch_real128_##fname(const real128 &, const real128 &);                                \
    template <typename T>                                                                                              \
    inline real128 dispatch_real128_##fname(const real128 &x, const T &y)                                              \
    {                                                                                                                  \
        return dispatch_real128_##fname(x, real128{y});                                                                \
    }                                                                                                                  \
    template <typename T>                                                                                              \
    inline real128 dispatch_real128_##fname(const T &x, const real128 &y)                                              \
    {                                                                                                                  \
        return dispatch_real128_##fname(real128{x}, y);                                                                \
    }                                                                                                                  \
    }                                                                                                                  \
    MPPP_REAL128_BINARY_OP_HEADER                                                                                      \
    inline real128 fname(const T &x, const U &y)                                                                       \
    {                                                                                                                  \
        return detail::dispatch_real128_##fname(x, y);                                                                 \
    }

// Euclidean distance.
MPPP_REAL128_IMPLEMENT_BINARY_OPERATION(hypot)

// Exponentiation.
MPPP_REAL128_IMPLEMENT_BINARY_OPERATION(pow)

// Logarithms and exponentials.
MPPP_DLL_PUBLIC real128 exp(const real128 &);
#if defined(MPPP_QUADMATH_HAVE_EXP2Q)
MPPP_DLL_PUBLIC real128 exp2(const real128 &);
#endif
MPPP_DLL_PUBLIC real128 expm1(const real128 &);
MPPP_DLL_PUBLIC real128 log(const real128 &);
MPPP_DLL_PUBLIC real128 log10(const real128 &);
MPPP_DLL_PUBLIC real128 log2(const real128 &);
MPPP_DLL_PUBLIC real128 log1p(const real128 &);

// Trigonometric functions.
MPPP_DLL_PUBLIC real128 sin(const real128 &);
MPPP_DLL_PUBLIC real128 cos(const real128 &);
MPPP_DLL_PUBLIC real128 tan(const real128 &);
MPPP_DLL_PUBLIC real128 asin(const real128 &);
MPPP_DLL_PUBLIC real128 acos(const real128 &);
MPPP_DLL_PUBLIC real128 atan(const real128 &);

// atan2.
MPPP_REAL128_IMPLEMENT_BINARY_OPERATION(atan2)

// Sine and cosine at the same time.
MPPP_DLL_PUBLIC void sincos(const real128 &, real128 *, real128 *);

// Hyperbolic functions.
MPPP_DLL_PUBLIC real128 sinh(const real128 &);
MPPP_DLL_PUBLIC real128 cosh(const real128 &);
MPPP_DLL_PUBLIC real128 tanh(const real128 &);
MPPP_DLL_PUBLIC real128 asinh(const real128 &);
MPPP_DLL_PUBLIC real128 acosh(const real128 &);
MPPP_DLL_PUBLIC real128 atanh(const real128 &);

// Gamma functions.
MPPP_DLL_PUBLIC real128 lgamma(const real128 &);
MPPP_DLL_PUBLIC real128 tgamma(const real128 &);

// Bessel functions.
MPPP_DLL_PUBLIC real128 j0(const real128 &);
MPPP_DLL_PUBLIC real128 j1(const real128 &);
MPPP_DLL_PUBLIC real128 y0(const real128 &);
MPPP_DLL_PUBLIC real128 y1(const real128 &);

MPPP_DLL_PUBLIC real128 jn(int, const real128 &);
MPPP_DLL_PUBLIC real128 yn(int, const real128 &);

// Error functions.
MPPP_DLL_PUBLIC real128 erf(const real128 &);
MPPP_DLL_PUBLIC real128 erfc(const real128 &);

// Next real128 from 'from' to 'to'.
MPPP_DLL_PUBLIC real128 nextafter(const real128 &, const real128 &);

// Copy sign.
MPPP_REAL128_IMPLEMENT_BINARY_OPERATION(copysign)

// fdim.
MPPP_REAL128_IMPLEMENT_BINARY_OPERATION(fdim)

// fmax/fmin.
MPPP_REAL128_IMPLEMENT_BINARY_OPERATION(fmax)
MPPP_REAL128_IMPLEMENT_BINARY_OPERATION(fmin)

// Integer and remainder-related functions.
MPPP_DLL_PUBLIC real128 ceil(const real128 &);
MPPP_DLL_PUBLIC real128 floor(const real128 &);
MPPP_DLL_PUBLIC real128 nearbyint(const real128 &);
MPPP_DLL_PUBLIC real128 rint(const real128 &);
MPPP_DLL_PUBLIC real128 round(const real128 &);
MPPP_DLL_PUBLIC real128 trunc(const real128 &);
MPPP_DLL_PUBLIC long long llrint(const real128 &);
MPPP_DLL_PUBLIC long lrint(const real128 &);
MPPP_DLL_PUBLIC long long llround(const real128 &);
MPPP_DLL_PUBLIC long lround(const real128 &);

MPPP_REAL128_IMPLEMENT_BINARY_OPERATION(fmod)
MPPP_REAL128_IMPLEMENT_BINARY_OPERATION(remainder)

MPPP_DLL_PUBLIC real128 modf(const real128 &, real128 *);
MPPP_DLL_PUBLIC real128 remquo(const real128 &, const real128 &, int *);

#undef MPPP_REAL128_BINARY_OP_HEADER
#undef MPPP_REAL128_IMPLEMENT_BINARY_OPERATION

// Identity operator.
constexpr real128 operator+(const real128 &x)
{
    return x;
}

namespace detail
{

constexpr real128 dispatch_add(const real128 &x, const real128 &y)
{
    return real128{x.m_value + y.m_value};
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr real128 dispatch_add(const real128 &x, const T &y)
{
    return real128{x.m_value + y};
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr real128 dispatch_add(const T &x, const real128 &y)
{
    return real128{x + y.m_value};
}

// NOTE: dispatching for mp++ types needs to be declared here
// but implemented below, as it needs the presence of an operator+()
// for real128.
template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
real128 dispatch_add(const real128 &, const T &);

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
real128 dispatch_add(const T &, const real128 &);

} // namespace detail

// Binary addition.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real128_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real128_op_types<T, U>::value, int> = 0>
#endif
constexpr real128 operator+(const T &x, const U &y)
{
    return detail::dispatch_add(x, y);
}

namespace detail
{

// Definitions for the binary dispatch functions above.
template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline real128 dispatch_add(const real128 &x, const T &y)
{
    return x + real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline real128 dispatch_add(const T &x, const real128 &y)
{
    return real128{x} + y;
}

// NOTE: we need the MPPP_CONSTEXPR_14 construct in the implementation detail as well,
// as returning void in a constexpr function is not allowed in C++11.
inline MPPP_CONSTEXPR_14 void dispatch_in_place_add(real128 &x, const real128 &y)
{
    x.m_value += y.m_value;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
inline MPPP_CONSTEXPR_14 void dispatch_in_place_add(real128 &x, const T &y)
{
    x.m_value += y;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
inline MPPP_CONSTEXPR_14 void dispatch_in_place_add(T &x, const real128 &y)
{
    x = static_cast<T>(x + y.m_value);
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_add(real128 &x, const T &y)
{
    x = x + y;
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_add(T &x, const real128 &y)
{
    x = static_cast<T>(x + y);
}

} // namespace detail

// In-place addition.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real128_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real128_op_types<T, U>::value, int> = 0>
#endif
inline MPPP_CONSTEXPR_14 T &operator+=(T &x, const U &y)
{
    detail::dispatch_in_place_add(x, y);
    return x;
}

// Prefix increment.
inline MPPP_CONSTEXPR_14 real128 &operator++(real128 &x)
{
    x.m_value += 1;
    return x;
}

// Suffix increment.
inline MPPP_CONSTEXPR_14 real128 operator++(real128 &x, int)
{
    auto retval(x);
    ++x;
    return retval;
}

// Negation operator.
constexpr real128 operator-(const real128 &x)
{
    return real128{-x.m_value};
}

namespace detail
{

constexpr real128 dispatch_sub(const real128 &x, const real128 &y)
{
    return real128{x.m_value - y.m_value};
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr real128 dispatch_sub(const real128 &x, const T &y)
{
    return real128{x.m_value - y};
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr real128 dispatch_sub(const T &x, const real128 &y)
{
    return real128{x - y.m_value};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
real128 dispatch_sub(const real128 &, const T &);

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
real128 dispatch_sub(const T &, const real128 &);

} // namespace detail

// Binary subtraction.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real128_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real128_op_types<T, U>::value, int> = 0>
#endif
constexpr real128 operator-(const T &x, const U &y)
{
    return detail::dispatch_sub(x, y);
}

namespace detail
{

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline real128 dispatch_sub(const real128 &x, const T &y)
{
    return x - real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline real128 dispatch_sub(const T &x, const real128 &y)
{
    return real128{x} - y;
}

inline MPPP_CONSTEXPR_14 void dispatch_in_place_sub(real128 &x, const real128 &y)
{
    x.m_value -= y.m_value;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
inline MPPP_CONSTEXPR_14 void dispatch_in_place_sub(real128 &x, const T &y)
{
    x.m_value -= y;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
inline MPPP_CONSTEXPR_14 void dispatch_in_place_sub(T &x, const real128 &y)
{
    x = static_cast<T>(x - y.m_value);
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_sub(real128 &x, const T &y)
{
    x = x - y;
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_sub(T &x, const real128 &y)
{
    x = static_cast<T>(x - y);
}

} // namespace detail

// In-place subtraction.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real128_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real128_op_types<T, U>::value, int> = 0>
#endif
inline MPPP_CONSTEXPR_14 T &operator-=(T &x, const U &y)
{
    detail::dispatch_in_place_sub(x, y);
    return x;
}

// Prefix decrement.
inline MPPP_CONSTEXPR_14 real128 &operator--(real128 &x)
{
    x.m_value -= 1;
    return x;
}

// Suffix decrement.
inline MPPP_CONSTEXPR_14 real128 operator--(real128 &x, int)
{
    auto retval(x);
    --x;
    return retval;
}

namespace detail
{

constexpr real128 dispatch_mul(const real128 &x, const real128 &y)
{
    return real128{x.m_value * y.m_value};
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr real128 dispatch_mul(const real128 &x, const T &y)
{
    return real128{x.m_value * y};
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr real128 dispatch_mul(const T &x, const real128 &y)
{
    return real128{x * y.m_value};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
real128 dispatch_mul(const real128 &, const T &);

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
real128 dispatch_mul(const T &, const real128 &);

} // namespace detail

// Binary multiplication.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real128_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real128_op_types<T, U>::value, int> = 0>
#endif
constexpr real128 operator*(const T &x, const U &y)
{
    return detail::dispatch_mul(x, y);
}

namespace detail
{

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline real128 dispatch_mul(const real128 &x, const T &y)
{
    return x * real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline real128 dispatch_mul(const T &x, const real128 &y)
{
    return real128{x} * y;
}

inline MPPP_CONSTEXPR_14 void dispatch_in_place_mul(real128 &x, const real128 &y)
{
    x.m_value *= y.m_value;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
inline MPPP_CONSTEXPR_14 void dispatch_in_place_mul(real128 &x, const T &y)
{
    x.m_value *= y;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
inline MPPP_CONSTEXPR_14 void dispatch_in_place_mul(T &x, const real128 &y)
{
    x = static_cast<T>(x * y.m_value);
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_mul(real128 &x, const T &y)
{
    x = x * y;
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_mul(T &x, const real128 &y)
{
    x = static_cast<T>(x * y);
}

} // namespace detail

// In-place multiplication.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real128_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real128_op_types<T, U>::value, int> = 0>
#endif
inline MPPP_CONSTEXPR_14 T &operator*=(T &x, const U &y)
{
    detail::dispatch_in_place_mul(x, y);
    return x;
}

namespace detail
{

constexpr real128 dispatch_div(const real128 &x, const real128 &y)
{
    return real128{x.m_value / y.m_value};
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr real128 dispatch_div(const real128 &x, const T &y)
{
    return real128{x.m_value / y};
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr real128 dispatch_div(const T &x, const real128 &y)
{
    return real128{x / y.m_value};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
real128 dispatch_div(const real128 &, const T &);

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
real128 dispatch_div(const T &, const real128 &);

} // namespace detail

// Binary division.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real128_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real128_op_types<T, U>::value, int> = 0>
#endif
constexpr real128 operator/(const T &x, const U &y)
{
    return detail::dispatch_div(x, y);
}

namespace detail
{

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline real128 dispatch_div(const real128 &x, const T &y)
{
    return x / real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline real128 dispatch_div(const T &x, const real128 &y)
{
    return real128{x} / y;
}

inline MPPP_CONSTEXPR_14 void dispatch_in_place_div(real128 &x, const real128 &y)
{
    x.m_value /= y.m_value;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
inline MPPP_CONSTEXPR_14 void dispatch_in_place_div(real128 &x, const T &y)
{
    x.m_value /= y;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
inline MPPP_CONSTEXPR_14 void dispatch_in_place_div(T &x, const real128 &y)
{
    x = static_cast<T>(x / y.m_value);
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_div(real128 &x, const T &y)
{
    x = x / y;
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline void dispatch_in_place_div(T &x, const real128 &y)
{
    x = static_cast<T>(x / y);
}

} // namespace detail

// In-place division.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real128_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real128_op_types<T, U>::value, int> = 0>
#endif
inline MPPP_CONSTEXPR_14 T &operator/=(T &x, const U &y)
{
    detail::dispatch_in_place_div(x, y);
    return x;
}

template <typename T, typename U>
using are_real128_eq_op_types
    = detail::disjunction<are_real128_op_types<T, U>,
                          detail::conjunction<std::is_same<T, real128>, is_real128_cpp_complex<U>>,
                          detail::conjunction<std::is_same<U, real128>, is_real128_cpp_complex<T>>>;

#if defined(MPPP_HAVE_CONCEPTS)

template <typename T, typename U>
MPPP_CONCEPT_DECL real128_eq_op_types = are_real128_eq_op_types<T, U>::value;

#endif

namespace detail
{

constexpr bool dispatch_eq(const real128 &x, const real128 &y)
{
    return x.m_value == y.m_value;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr bool dispatch_eq(const real128 &x, const T &y)
{
    return x.m_value == y;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr bool dispatch_eq(const T &x, const real128 &y)
{
    return x == y.m_value;
}

template <typename T, enable_if_t<is_real128_cpp_complex<T>::value, int> = 0>
constexpr bool dispatch_eq(const real128 &x, const T &y)
{
    // NOTE: follow what std::complex does here, that is, real arguments are treated as
    // complex numbers with the real part equal to the argument and the imaginary part set to zero.
    // https://en.cppreference.com/w/cpp/numeric/complex/operator_cmp
    return y.imag() == 0 && y.real() == x.m_value;
}

template <typename T, enable_if_t<is_real128_cpp_complex<T>::value, int> = 0>
constexpr bool dispatch_eq(const T &x, const real128 &y)
{
    return dispatch_eq(y, x);
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
bool dispatch_eq(const real128 &, const T &);

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
bool dispatch_eq(const T &, const real128 &);

} // namespace detail

// Equality operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real128_eq_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real128_eq_op_types<T, U>::value, int> = 0>
#endif
constexpr bool operator==(const T &x, const U &y)
{
    return detail::dispatch_eq(x, y);
}

namespace detail
{

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline bool dispatch_eq(const real128 &x, const T &y)
{
    return x == real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline bool dispatch_eq(const T &x, const real128 &y)
{
    return real128{x} == y;
}

} // namespace detail

// Inequality operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real128_eq_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real128_eq_op_types<T, U>::value, int> = 0>
#endif
constexpr bool operator!=(const T &x, const U &y)
{
    return !(x == y);
}

namespace detail
{

constexpr bool dispatch_lt(const real128 &x, const real128 &y)
{
    return x.m_value < y.m_value;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr bool dispatch_lt(const real128 &x, const T &y)
{
    return x.m_value < y;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr bool dispatch_lt(const T &x, const real128 &y)
{
    return x < y.m_value;
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
bool dispatch_lt(const real128 &, const T &);

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
bool dispatch_lt(const T &, const real128 &);

} // namespace detail

// Less-than operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real128_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real128_op_types<T, U>::value, int> = 0>
#endif
constexpr bool operator<(const T &x, const U &y)
{
    return detail::dispatch_lt(x, y);
}

namespace detail
{

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline bool dispatch_lt(const real128 &x, const T &y)
{
    return x < real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline bool dispatch_lt(const T &x, const real128 &y)
{
    return real128{x} < y;
}

constexpr bool dispatch_lte(const real128 &x, const real128 &y)
{
    return x.m_value <= y.m_value;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr bool dispatch_lte(const real128 &x, const T &y)
{
    return x.m_value <= y;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr bool dispatch_lte(const T &x, const real128 &y)
{
    return x <= y.m_value;
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
bool dispatch_lte(const real128 &, const T &);

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
bool dispatch_lte(const T &, const real128 &);

} // namespace detail

// Less-than or equal operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real128_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real128_op_types<T, U>::value, int> = 0>
#endif
constexpr bool operator<=(const T &x, const U &y)
{
    return detail::dispatch_lte(x, y);
}

namespace detail
{

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline bool dispatch_lte(const real128 &x, const T &y)
{
    return x <= real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline bool dispatch_lte(const T &x, const real128 &y)
{
    return real128{x} <= y;
}

constexpr bool dispatch_gt(const real128 &x, const real128 &y)
{
    return x.m_value > y.m_value;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr bool dispatch_gt(const real128 &x, const T &y)
{
    return x.m_value > y;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr bool dispatch_gt(const T &x, const real128 &y)
{
    return x > y.m_value;
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
bool dispatch_gt(const real128 &, const T &);

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
bool dispatch_gt(const T &, const real128 &);

} // namespace detail

// Greater-than operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real128_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real128_op_types<T, U>::value, int> = 0>
#endif
constexpr bool operator>(const T &x, const U &y)
{
    return detail::dispatch_gt(x, y);
}

namespace detail
{

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline bool dispatch_gt(const real128 &x, const T &y)
{
    return x > real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline bool dispatch_gt(const T &x, const real128 &y)
{
    return real128{x} > y;
}

constexpr bool dispatch_gte(const real128 &x, const real128 &y)
{
    return x.m_value >= y.m_value;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr bool dispatch_gte(const real128 &x, const T &y)
{
    return x.m_value >= y;
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
constexpr bool dispatch_gte(const T &x, const real128 &y)
{
    return x >= y.m_value;
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
bool dispatch_gte(const real128 &, const T &);

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
bool dispatch_gte(const T &, const real128 &);

} // namespace detail

// Greater-than or equal operator.
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires real128_op_types<T, U>
#else
template <typename T, typename U, detail::enable_if_t<are_real128_op_types<T, U>::value, int> = 0>
#endif
constexpr bool operator>=(const T &x, const U &y)
{
    return detail::dispatch_gte(x, y);
}

namespace detail
{

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline bool dispatch_gte(const real128 &x, const T &y)
{
    return x >= real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int>>
inline bool dispatch_gte(const T &x, const real128 &y)
{
    return real128{x} >= y;
}

// Functions useful for the implementation of real128 constants below.
constexpr real128 two_112()
{
    return real128{1} / (1ull << 32) / (1ull << 32) / (1ull << 48);
}

constexpr real128 two_48()
{
    return real128{1} / (1ull << 48);
}

// Recursively calculate 2**N, where N is a power of two greater than 32.
template <unsigned long N>
constexpr real128 two_ptwo()
{
    static_assert(N > 32u && !(N & (N - 1u)), "Invalid value for N.");
    return two_ptwo<N / 2u>() * two_ptwo<N / 2u>();
}

template <>
constexpr real128 two_ptwo<32ul>()
{
    return real128{1ull << 32};
}
} // namespace detail

// The number of binary digits in the significand.
constexpr unsigned real128_sig_digits()
{
    return 113u;
}

// The largest positive finite value.
constexpr real128 real128_max()
{
    return (18446744073709551615ull * detail::two_112() + 281474976710655ull * detail::two_48() + 1)
           * detail::two_ptwo<8192>() * detail::two_ptwo<4096>() * detail::two_ptwo<2048>() * detail::two_ptwo<1024>()
           * detail::two_ptwo<512>() * detail::two_ptwo<256>() * detail::two_ptwo<128>() * detail::two_ptwo<64>()
           * detail::two_ptwo<32>() * (1ull << 31);
}

// The smallest positive value.
constexpr real128 real128_min()
{
    return 1 / detail::two_ptwo<8192>() / detail::two_ptwo<4096>() / detail::two_ptwo<2048>() / detail::two_ptwo<1024>()
           / detail::two_ptwo<512>() / detail::two_ptwo<256>() / detail::two_ptwo<128>() / detail::two_ptwo<64>()
           / detail::two_ptwo<32>() / (1ull << 30);
}

// The difference between 1 and the next larger number.
constexpr real128 real128_epsilon()
{
    return 1 / detail::two_ptwo<64>() / detail::two_ptwo<32>() / (1ull << 16);
}

// The smallest positive denormalized number.
constexpr real128 real128_denorm_min()
{
    return 1 / detail::two_ptwo<8192>() / detail::two_ptwo<8192>() / detail::two_ptwo<64>() / (1ull << 46);
}

// Positive inf.
constexpr real128 real128_inf()
{
#if defined(__INTEL_COMPILER) || !defined(__GNUC__) || __GNUC__ < 7
    // NOTE: it seems like there's no way to arithmetically construct infinity in constexpr.
    // I tried 1/0 and repeated multiplications by a large int, but it always ends up in
    // a 'not a constant expression' error message.
    return real128{std::numeric_limits<double>::infinity()};
#else
    // This builtin is constexpr only on GCC 7 and later.
    // https://gcc.gnu.org/onlinedocs/gcc/x86-Built-in-Functions.html
    // Note that this and the nan builtins are arch-specific, but it seems they
    // might be available everywhere __float128 is available.
    return real128{__builtin_infq()};
#endif
}

// NaN.
constexpr real128 real128_nan()
{
#if defined(__INTEL_COMPILER) || !defined(__GNUC__) || __GNUC__ < 7
    // Same as above - GCC would accept arithmetic generation of NaN,
    // but Clang does not.
    return real128{std::numeric_limits<double>::quiet_NaN()};
#else
    // This builtin is constexpr only on GCC 7 and later.
    return real128{__builtin_nanq("")};
#endif
}

// Pi.
constexpr real128 real128_pi()
{
    return 2 * (9541308523256152504ull * detail::two_112() + 160664882791121ull * detail::two_48() + 1);
}

// Euler's number.
constexpr real128 real128_e()
{
    return 2 * (10751604932185443962ull * detail::two_112() + 101089180468598ull * detail::two_48() + 1);
}

// Sqrt2.
constexpr real128 real128_sqrt2()
{
    return 14486024992869247637ull * detail::two_112() + 116590752822204ull * detail::two_48() + 1;
}

#if MPPP_CPLUSPLUS >= 201703L

// NOTE: namespace scope constexpr variables are *not* implicitly inline, so we need
// inline explicitly here:
// http://en.cppreference.com/w/cpp/language/inline
// Note that constexpr static member variables are implicitly inline instead.

// The number of binary digits in the significand.
inline constexpr unsigned sig_digits_128 = real128_sig_digits();

// The largest positive finite value.
inline constexpr real128 max_128 = real128_max();

// The smallest positive value representable with full precision.
inline constexpr real128 min_128 = real128_min();

// The difference between 1 and the next larger representable number.
inline constexpr real128 epsilon_128 = real128_epsilon();

// The smallest positive denormalized number.
inline constexpr real128 denorm_min_128 = real128_denorm_min();

// Inf.
inline constexpr real128 inf_128 = real128_inf();

// NaN.
inline constexpr real128 nan_128 = real128_nan();

// Pi.
inline constexpr real128 pi_128 = real128_pi();

// Euler's number.
inline constexpr real128 e_128 = real128_e();

// Quadruple-precision sqrt2.
inline constexpr real128 sqrt2_128 = real128_sqrt2();

#endif

// Hash.
inline std::size_t hash(const real128 &x)
{
    // NOTE: in order to detect if x is zero/nan, resort to reading directly into the ieee fields.
    // This avoids calling the fpclassify() function, which internally invokes a compiler library function.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    detail::ieee_float128 ief;
    ief.value = x.m_value;
    const auto is_zero = ief.i_eee.exponent == 0u && ief.i_eee.mant_low == 0u && ief.i_eee.mant_high == 0u;
    const auto is_nan = ief.i_eee.exponent == 32767ul && (ief.i_eee.mant_low != 0u || ief.i_eee.mant_high != 0u);
    // Read the bit-level representation of x and mix it up using a hash combiner.
    struct float128_split_t {
        // NOTE: unsigned long long is guaranteed to be at least 64-bit wide.
        unsigned long long part1 : 64;
        unsigned long long part2 : 64;
    };
    union float128_split {
        __float128 value;
        float128_split_t split;
    };
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
    float128_split fs;
    fs.value = x.m_value;
    auto retval = fs.split.part1;
    // The hash combiner. This is lifted directly from Boost. See also:
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3876.pdf
    retval ^= fs.split.part2 + 0x9e3779b9ull + (retval << 6) + (retval >> 2);
    // This last step will set retval to zero if x is zero, and to -1 if x is NaN. We need this because:
    // - +0.0 and -0.0 have a different bit-level representation, but they are mathematically equal
    //   and they are equal according to operator==();
    // - we want to ensure that all NaN values produce the same hash.
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    retval = (retval & static_cast<unsigned long long>(-!is_zero)) | static_cast<unsigned long long>(-is_nan);
    return static_cast<std::size_t>(retval);
}

// NOTE: put these definitions here, as we need the comparison operators to be available.
#if defined(__INTEL_COMPILER)
inline
#else
constexpr
#endif
    bool
    real128_equal_to(const real128 &x, const real128 &y)
{
    return (!x.isnan() && !y.isnan()) ? (x == y) : (x.isnan() && y.isnan());
}

#if defined(__INTEL_COMPILER)
inline
#else
constexpr
#endif
    bool
    real128_lt(const real128 &x, const real128 &y)
{
    // NOTE: in case at least one op is NaN, we have the following possibilities:
    // - NaN vs NaN -> false,
    // - NaN vs not-NaN -> false,
    // - not-NaN vs NaN -> true.
    return (!x.isnan() && !y.isnan()) ? (x < y) : !x.isnan();
}

#if defined(__INTEL_COMPILER)
inline
#else
constexpr
#endif
    bool
    real128_gt(const real128 &x, const real128 &y)
{
    // NOTE: in case at least one op is NaN, we have the following possibilities:
    // - NaN vs NaN -> false,
    // - NaN vs not-NaN -> true,
    // - not-NaN vs NaN -> false.
    return (!x.isnan() && !y.isnan()) ? (x > y) : !y.isnan();
}

// Implementation of integer's assignment
// from real128.
template <std::size_t SSize>
inline integer<SSize> &integer<SSize>::operator=(const real128 &x)
{
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
    return *this = static_cast<integer<SSize>>(x);
}

// Implementation of rational's assignment
// from real128.
template <std::size_t SSize>
inline rational<SSize> &rational<SSize>::operator=(const real128 &x)
{
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
    return *this = static_cast<rational<SSize>>(x);
}

} // namespace mppp

#if defined(MPPP_WITH_BOOST_S11N)

// Never track the address of real128 objects
// during serialization.
BOOST_CLASS_TRACKING(mppp::real128, boost::serialization::track_never)

#endif

namespace std
{

// Specialisation of std::numeric_limits for mppp::real128.
template <>
class numeric_limits<mppp::real128>
{
public:
    // NOTE: in C++17 and later, constexpr implies inline. That is,
    // we have no need for the initialisation of the static member outside the class.
    // Before C++17, however, we *should* do the external initialisation, but, since
    // this is a full specialisation, that would lead to multiply-defined references
    // when dealing with multiple translation units. So we avoid that, but that means
    // that certain uses of the static data members may lead to undefined references
    // before C++17. See:
    // http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#454
    // NOTE: this is mostly lifted from Boost.Multiprecision.
    static constexpr bool is_specialized = true;
    static constexpr mppp::real128(min)()
    {
        return mppp::real128_min();
    }
    static constexpr mppp::real128(max)()
    {
        return mppp::real128_max();
    }
    static constexpr mppp::real128 lowest()
    {
        return -mppp::real128_max();
    }
    static constexpr int digits = 113;
    static constexpr int digits10 = 33;
    static constexpr int max_digits10 = 36;
    static constexpr bool is_signed = true;
    static constexpr bool is_integer = false;
    static constexpr bool is_exact = false;
    static constexpr int radix = 2;
    static constexpr mppp::real128 epsilon()
    {
        return mppp::real128_epsilon();
    }
    static constexpr mppp::real128 round_error()
    {
        return mppp::real128{.5};
    }
    static constexpr int min_exponent = -16381;
    static constexpr int min_exponent10 = min_exponent * 301L / 1000L;
    static constexpr int max_exponent = 16384;
    static constexpr int max_exponent10 = max_exponent * 301L / 1000L;
    static constexpr bool has_infinity = true;
    static constexpr bool has_quiet_NaN = true;
    static constexpr bool has_signaling_NaN = false;
    static constexpr float_denorm_style has_denorm = denorm_present;
    static constexpr bool has_denorm_loss = true;
    static constexpr mppp::real128 infinity()
    {
        return mppp::real128_inf();
    }
    static constexpr mppp::real128 quiet_NaN()
    {
        return mppp::real128_nan();
    }
    static constexpr mppp::real128 signaling_NaN()
    {
        return mppp::real128{0};
    }
    static constexpr mppp::real128 denorm_min()
    {
        return mppp::real128_denorm_min();
    }
    static constexpr bool is_iec559 = true;
    static constexpr bool is_bounded = false;
    static constexpr bool is_modulo = false;
    static constexpr bool traps = false;
    static constexpr bool tinyness_before = false;
    static constexpr float_round_style round_style = round_to_nearest;
};

} // namespace std

#include <mp++/detail/real128_literal.hpp>

// Support for pretty printing in xeus-cling.
#if defined(__CLING__)

#if __has_include(<nlohmann/json.hpp>)

#include <nlohmann/json.hpp>

namespace mppp
{

inline nlohmann::json mime_bundle_repr(const real128 &x)
{
    auto bundle = nlohmann::json::object();

    bundle["text/plain"] = x.to_string();

    return bundle;
}

} // namespace mppp

#endif

#endif

#else

#error The real128.hpp header was included but mp++ was not configured with the MPPP_WITH_QUADMATH option.

#endif

#endif
