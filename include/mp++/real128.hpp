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
#include <sstream>
#include <stdexcept>
#include <string>
#if MPPP_CPLUSPLUS >= 201703L
#include <string_view>
#endif
#include <type_traits>
#include <vector>

#include <mp++/concepts.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/quadmath.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>

namespace mppp
{

// Fwd declaration.
class real128;

inline namespace detail
{

template <typename T, typename U>
using are_real128_cpp_op_types
    = std::integral_constant<bool, disjunction<conjunction<std::is_same<T, real128>, std::is_same<U, real128>>,
                                               conjunction<std::is_same<T, real128>, is_cpp_interoperable<U>>,
                                               conjunction<std::is_same<U, real128>, is_cpp_interoperable<T>>>::value>;

template <typename T, typename U>
using are_real128_mppp_op_types
    = std::integral_constant<bool, disjunction<conjunction<std::is_same<T, real128>, is_integer<U>>,
                                               conjunction<std::is_same<U, real128>, is_integer<T>>,
                                               conjunction<std::is_same<T, real128>, is_rational<U>>,
                                               conjunction<std::is_same<U, real128>, is_rational<T>>>::value>;
}

template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool Real128CppOpTypes = are_real128_cpp_op_types<T, U>::value;
#else
using real128_cpp_op_types_enabler = enable_if_t<are_real128_cpp_op_types<T, U>::value, int>;
#endif

template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool Real128MpppOpTypes = are_real128_mppp_op_types<T, U>::value;
#else
using real128_mppp_op_types_enabler = enable_if_t<are_real128_mppp_op_types<T, U>::value, int>;
#endif

template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
concept bool Real128OpTypes = Real128CppOpTypes<T, U> || Real128MpppOpTypes<T, U>;
#else
using real128_op_types_enabler
    = enable_if_t<disjunction<are_real128_cpp_op_types<T, U>, are_real128_mppp_op_types<T, U>>::value, int>;
#endif

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
    /// Copy constructor.
    /**
     * @param other the real128 that will be copied.
     */
    constexpr real128(const real128 &other) : m_value(other)
    {
    }
    /// Move constructor.
    /**
     * @param other the real128 that will be moved.
     */
    constexpr real128(real128 &&other) : real128(other)
    {
    }
    /// Constructor from a quadruple-precision floating-point value.
    /**
     * This constructor will initialise the internal value with \p x.
     *
     * @param x the quadruple-precision floating-point variable that will be
     * used to initialise the internal value.
     */
    constexpr explicit real128(::__float128 x) : m_value(x)
    {
    }
/// Constructor from C++ interoperable types.
/**
 * This constructor will initialise the internal value with \p x.
 *
 * @param x the value that will be used for initialisation.
 */
#if defined(MPPP_HAVE_CONCEPTS)
    constexpr explicit real128(CppInteroperable x)
#else
    template <typename T, cpp_interoperable_enabler<T> = 0>
    constexpr explicit real128(T x)
#endif
        : m_value(x)
    {
    }
    /// Constructor from \link mppp::integer integer \endlink.
    /**
     * This constructor will initialise the internal value with \p n. If the absolute value of \p n is large
     * enough, \p this may not be exactly equal to \p n after initialisation.
     *
     * @param n the \link mppp::integer integer \endlink that will be used for the initialisation of the
     * internal value.
     *
     * @throws std::overflow_error if the absolute value of \p n is larger than an implementation-defined
     * limit.
     */
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
        const auto rem_bits = n_bits % unsigned(GMP_NUMB_BITS);
        std::size_t ls = n_bits / unsigned(GMP_NUMB_BITS) + static_cast<bool>(rem_bits);
        assert(ls && n_bits && ls == n.size());
        // Init value with the most significant limb, and move to the next limb.
        m_value = ptr[--ls] & GMP_NUMB_MASK;
        // Number of bits read so far from n: it is the size in bits of the top limb.
        auto read_bits = static_cast<unsigned>(rem_bits ? rem_bits : unsigned(GMP_NUMB_BITS));
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
    /// Constructor from \link mppp::rational rational \endlink.
    /**
     * This constructor will initialise the internal value with \p q.
     *
     * @param q the \link mppp::rational rational \endlink that will be used for the initialisation of the
     * internal value.
     *
     * @throws std::overflow_error if the absolute values of the numerator and/or denominator of \p q are larger than an
     * implementation-defined limit.
     */
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
            MPPP_MAYBE_TLS integer<SSize> n;
            const auto shift = n_bits - sig_digits;
            tdiv_q_2exp(n, q.get_num(), safe_cast<::mp_bitcnt_t>(shift));
            m_value = real128{n}.m_value / real128{q.get_den()}.m_value;
            m_value = ::scalblnq(m_value, safe_cast<long>(shift));
        } else if (n_bits <= sig_digits && d_bits > sig_digits) {
            // The opposite of above.
            MPPP_MAYBE_TLS integer<SSize> d;
            const auto shift = d_bits - sig_digits;
            tdiv_q_2exp(d, q.get_den(), safe_cast<::mp_bitcnt_t>(shift));
            m_value = real128{q.get_num()}.m_value / real128{d}.m_value;
            m_value = ::scalblnq(m_value, negate_unsigned<long>(shift));
        } else {
            // Both num and den have more bits than quad's significand. We will downshift
            // both until they have 113 bits, do the division, and then recover the shifted bits.
            MPPP_MAYBE_TLS integer<SSize> n;
            MPPP_MAYBE_TLS integer<SSize> d;
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
    /// Constructor from C string.
    /**
     * \rststar
     * This constructor will initialise \p this from the null-terminated string ``s``.
     * The accepted string formats are detailed in the quadmath library's documentation
     * (see the link below). Leading whitespaces are accepted (and ignored), but trailing whitespaces
     * will raise an error.
     *
     * .. seealso::
     *    https://gcc.gnu.org/onlinedocs/libquadmath/strtoflt128.html
     * \endrststar
     *
     * @param s the null-terminated string that will be used to initialise \p this.
     *
     * @throws std::invalid_argument if \p s does not represent a valid quadruple-precision
     * floating-point value.
     */
    explicit real128(const char *s) : m_value(str_to_float128(s))
    {
    }
    /// Constructor from C++ string (equivalent to the constructor from C string).
    /**
     * @param s the input string.
     *
     * @throws unspecified any exception thrown by the constructor from C string.
     */
    explicit real128(const std::string &s) : real128(s.c_str())
    {
    }
    /// Constructor from range of characters.
    /**
     * This constructor will initialise \p this from the content of the input half-open range, which is interpreted
     * as the string representation of a floating-point value.
     *
     * Internally, the constructor will copy the content of the range to a local buffer, add a string terminator, and
     * invoke the constructor from C string.
     *
     * @param begin the begin of the input range.
     * @param end the end of the input range.
     *
     * @throws unspecified any exception thrown by the constructor from C string.
     */
    explicit real128(const char *begin, const char *end)
    {
        MPPP_MAYBE_TLS std::vector<char> buffer;
        buffer.assign(begin, end);
        buffer.emplace_back('\0');
        m_value = str_to_float128(buffer.data());
    }
#if MPPP_CPLUSPLUS >= 201703L
    /// Constructor from string view.
    /**
     * This constructor will initialise \p this from the content of the input string view,
     * which is interpreted as the string representation of a floating-point value.
     *
     * Internally, the constructor will invoke the constructor from a range of characters.
     *
     * \rststar
     * .. note::
     *
     *   This constructor is available only if at least C++17 is being used.
     * \endrststar
     *
     * @param s the \p std::string_view that will be used for construction.
     *
     * @throws unspecified any exception thrown by the constructor from a range of characters.
     */
    explicit real128(const std::string_view &s) : real128(s.data(), s.data() + s.size())
    {
    }
#endif
    /// Defaulted copy assignment operator.
    /**
     * @param other the assignment argument.
     *
     * @return a reference to \p this.
     */
    real128 &operator=(const real128 &other) = default;
    /// Defaulted move assignment operator.
    /**
     * @param other the assignment argument.
     *
     * @return a reference to \p this.
     */
    real128 &operator=(real128 &&other) = default;
    /// Assignment from a quadruple-precision floating-point value.
    /**
     * \rststar
     * .. note::
     *
     *   This operator is marked as ``constexpr`` only if at least C++14 is being used.
     * \endrststar
     *
     * @param x the quadruple-precision floating-point variable that will be
     * assigned to the internal value.
     *
     * @return a reference to \p this.
     */
    MPPP_CONSTEXPR_14 real128 &operator=(const ::__float128 &x)
    {
        m_value = x;
        return *this;
    }
/// Assignment from C++ interoperable types.
/**
 * \rststar
 * .. note::
 *
 *   This operator is marked as ``constexpr`` only if at least C++14 is being used.
 * \endrststar
 *
 * @param x the assignment argument.
 *
 * @return a reference to \p this.
 */
#if defined(MPPP_HAVE_CONCEPTS)
    MPPP_CONSTEXPR_14 real128 &operator=(const CppInteroperable &x)
#else
    template <typename T, cpp_interoperable_enabler<T> = 0>
    MPPP_CONSTEXPR_14 real128 &operator=(const T &x)
#endif
    {
        m_value = x;
        return *this;
    }
    /// Assignment from \link mppp::integer integer \endlink.
    /**
     * \rststar
     * The body of this operator is equivalent to:
     *
     * .. code-block:: c++
     *
     *    return *this = real128{n};
     *
     * That is, a temporary :cpp:class:`~mppp::real128` is constructed from ``n`` and it is then move-assigned to
     * ``this``.
     * \endrststar
     *
     * @param n the assignment argument.
     *
     * @return a reference to \p this.
     *
     * @throws unspecified any exception thrown by the constructor from \link mppp::integer integer \endlink.
     */
    template <std::size_t SSize>
    real128 &operator=(const integer<SSize> &n)
    {
        return *this = real128{n};
    }
    /// Assignment from \link mppp::rational rational \endlink.
    /**
     * \rststar
     * The body of this operator is equivalent to:
     *
     * .. code-block:: c++
     *
     *    return *this = real128{q};
     *
     * That is, a temporary :cpp:class:`~mppp::real128` is constructed from ``q`` and it is then move-assigned to
     * ``this``.
     * \endrststar
     *
     * @param q the assignment argument.
     *
     * @return a reference to \p this.
     *
     * @throws unspecified any exception thrown by the constructor from \link mppp::rational rational \endlink.
     */
    template <std::size_t SSize>
    real128 &operator=(const rational<SSize> &q)
    {
        return *this = real128{q};
    }
    /// Assignment from C string.
    /**
     * \rststar
     * The body of this operator is equivalent to:
     *
     * .. code-block:: c++
     *
     *    return *this = real128{s};
     *
     * That is, a temporary :cpp:class:`~mppp::real128` is constructed from ``s`` and it is then move-assigned to
     * ``this``.
     * \endrststar
     *
     * @param s the C string that will be used for the assignment.
     *
     * @return a reference to \p this.
     *
     * @throws unspecified any exception thrown by the constructor from string.
     */
    real128 &operator=(const char *s)
    {
        return *this = real128{s};
    }
    /// Assignment from C++ string (equivalent to the assignment from C string).
    /**
     * @param s the C++ string that will be used for the assignment.
     *
     * @return a reference to \p this.
     *
     * @throws unspecified any exception thrown by the assignment operator from C string.
     */
    real128 &operator=(const std::string &s)
    {
        return operator=(s.c_str());
    }
#if MPPP_CPLUSPLUS >= 201703L
    /// Assignment from string view.
    /**
     * \rststar
     * The body of this operator is equivalent to:
     *
     * .. code-block:: c++
     *
     *    return *this = real128{s};
     *
     * That is, a temporary :cpp:class:`~mppp::real128` is constructed from ``s`` and it is then move-assigned to
     * ``this``.
     *
     * .. note::
     *
     *   This operator is available only if at least C++17 is being used.
     *
     * \endrststar
     *
     * @param s the \p std::string_view that will be used for the assignment.
     *
     * @return a reference to \p this.
     *
     * @throws unspecified any exception thrown by the constructor from string view.
     */
    real128 &operator=(const std::string_view &s)
    {
        return *this = real128{s};
    }
#endif
/// Conversion to interoperable C++ types.
/**
 * \rststar
 * This operator will convert ``this`` to a :cpp:concept:`~mppp::CppInteroperable` type. The conversion uses
 * a direct ``static_cast()`` of the internal :cpp:member:`~mppp::real128::m_value` member to the target type,
 * and thus no checks are performed to ensure that the value of ``this`` can be represented by the target type.
 * Conversion to integral types will produce the truncated counterpart of ``this``.
 * \endrststar
 *
 * @return \p this converted to \p T.
 */
#if defined(MPPP_HAVE_CONCEPTS)
    template <CppInteroperable T>
#else
    template <typename T, cpp_interoperable_enabler<T> = 0>
#endif
    constexpr explicit operator T() const
    {
        return static_cast<T>(m_value);
    }
    /// Conversion to quadruple-precision floating-point.
    /**
     * \rststar
     * This operator will convert ``this`` to :cpp:type:`__float128`.
     * \endrststar
     *
     * @return a copy of the quadruple-precision floating-point value stored internally.
     */
    constexpr explicit operator __float128() const
    {
        return m_value;
    }
    /// Conversion to \link mppp::integer integer \endlink.
    /**
     * \rststar
     * This operator will convert ``this`` to an :cpp:type:`~mppp::integer`. If ``this`` does not represent
     * an integral value, the conversion will yield the truncated counterpart of ``this``.
     * \endrststar
     *
     * @return \p this converted to \link mppp::integer integer \endlink.
     *
     * @throws std::domain_error if \p this represents a non-finite value.
     */
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
                // on a C++ builtin type (i_eee.mant_high gives a 64bit int).
                retval += ief.i_eee.mant_high >> static_cast<unsigned>(-(exponent + 64));
            }
        }
        // Adjust the sign.
        if (ief.i_eee.negative) {
            retval.neg();
        }
        return retval;
    }
    /// Conversion to \link mppp::rational rational \endlink.
    /**
     * \rststar
     * This operator will convert ``this`` to a :cpp:type:`~mppp::rational`. The conversion,
     * if successful, is exact.
     * \endrststar
     *
     * @return \p this converted to \link mppp::rational rational \endlink.
     *
     * @throws std::domain_error if \p this represents a non-finite value.
     */
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
            // Try demoting.
            retval._get_num().demote();
            retval._get_den().demote();
        }
        // Adjust the sign.
        if (ief.i_eee.negative) {
            retval.neg();
        }
        return retval;
    }
    /// Convert to string.
    /**
     * \rststar
     * This method will convert ``this`` to a decimal string representation in scientific format.
     * The number of significant digits in the output (36) guarantees that a :cpp:class:`~mppp::real128`
     * constructed from the returned string will have a value identical to the value of ``this``.
     *
     * The implementation uses the ``quadmath_snprintf()`` function from the quadmath library.
     *
     * .. seealso::
     *    https://gcc.gnu.org/onlinedocs/libquadmath/quadmath_005fsnprintf.html
     * \endrststar
     *
     * @return a decimal string representation of ``this``.
     *
     * @throws std::runtime_error if the internal call to the ``quadmath_snprintf()`` function fails.
     */
    std::string to_string() const
    {
        std::ostringstream oss;
        float128_stream(oss, m_value);
        return oss.str();
    }
    /// Sign bit.
    /**
     * This method will return the value of the sign bit of \p this. That is, if \p this
     * is not a NaN the method will return \p true if \p this is negative, \p false otherwise.
     * If \p this is NaN, the sign bit of the NaN value will be returned.
     *
     * @return \p true if the sign bit of \p this is set, \p false otherwise.
     */
    constexpr bool signbit() const
    {
        return __builtin_signbit(m_value);
    }
    /// Detect NaN.
    /**
     * @return \p true if \p this is NaN, \p false otherwise.
     */
    constexpr bool isnan() const
    {
        return __builtin_isnan(m_value);
    }
    /// Detect infinity.
    /**
     * @return \p true if \p this is infinite, \p false otherwise.
     */
    bool isinf() const
    {
        return ::isinfq(m_value);
    }
    /// Detect finite value.
    /**
     * @return \p true if \p this is finite, \p false otherwise.
     */
    bool finite() const
    {
        return ::finiteq(m_value);
    }
    /// In-place absolute value.
    /**
     * This method will set \p this to its absolute value.
     *
     * @return a reference to \p this.
     */
    real128 &abs()
    {
        return *this = ::fabsq(m_value);
    }
    /// In-place square root.
    /**
     * This method will set \p this to its nonnegative square root.
     * If \p this is less than negative zero, the result will be NaN.
     *
     * @return a reference to \p this.
     */
    real128 &sqrt()
    {
        return *this = ::sqrtq(m_value);
    }
    /// In-place cube root.
    /**
     * This method will set \p this to its real cube root.
     *
     * @return a reference to \p this.
     */
    real128 &cbrt()
    {
        return *this = ::cbrtq(m_value);
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

/** @defgroup real128_arithmetic real128_arithmetic
 *  @{
 */

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
inline real128 fma(const real128 &x, const real128 &y, const real128 &z)
{
    return real128{::fmaq(x.m_value, y.m_value, z.m_value)};
}

/// Unary absolute value.
/**
 * @param x the \link mppp::real128 real128 \endlink whose absolute value will be computed.
 *
 * @return the absolute value of \p x.
 */
inline real128 abs(real128 x)
{
    x.abs();
    return x;
}

/** @} */

/** @defgroup real128_io real128_io
 *  @{
 */

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
 * @param x the input \link mppp::real128 real128 \endlink.
 *
 * @return a reference to \p os.
 *
 * @throws unspecified any exception thrown by real128::to_string().
 */
inline std::ostream &operator<<(std::ostream &os, const real128 &x)
{
    float128_stream(os, x.m_value);
    return os;
}

/// Input stream operator.
/**
 * \rststar
 * This operator is equivalent to extracting a line from the stream, using it to construct a temporary
 * :cpp:class:`~mppp::real128` and then assigning the temporary to ``x``.
 * \endrststar
 *
 * @param is the input stream.
 * @param x the \link mppp::real128 real128 \endlink to which the contents of the stream will be assigned.
 *
 * @return a reference to \p is.
 *
 * @throws unspecified any exception thrown by the constructor from string of \link mppp::real128 real128 \endlink.
 */
inline std::istream &operator>>(std::istream &is, real128 &x)
{
    MPPP_MAYBE_TLS std::string tmp_str;
    std::getline(is, tmp_str);
    x = real128{tmp_str};
    return is;
}

/** @} */

/** @defgroup real128_comparison real128_comparison
 *  @{
 */

/// Sign bit of a \link mppp::real128 real128 \endlink.
/**
 * @param x the \link mppp::real128 real128 \endlink whose sign bit will be returned.
 *
 * @return the sign bit of \p x (as returned by mppp::real128::signbit()).
 */
constexpr bool signbit(const real128 &x)
{
    return x.signbit();
}

/// Detect if a \link mppp::real128 real128 \endlink is NaN.
/**
 * @param x the \link mppp::real128 real128 \endlink argument.
 *
 * @return \p true if \p x is NaN, \p false otherwise.
 */
constexpr bool isnan(const real128 &x)
{
    return x.isnan();
}

/// Detect if a \link mppp::real128 real128 \endlink is infinite.
/**
 * @param x the \link mppp::real128 real128 \endlink argument.
 *
 * @return \p true if \p x is infinite, \p false otherwise.
 */
inline bool isinf(const real128 &x)
{
    return x.isinf();
}

/// Detect if a \link mppp::real128 real128 \endlink is finite.
/**
 * @param x the \link mppp::real128 real128 \endlink argument.
 *
 * @return \p true if \p x is finite, \p false otherwise.
 */
inline bool finite(const real128 &x)
{
    return x.finite();
}

/** @} */

/** @defgroup real128_roots real128_roots
 *  @{
 */

/// Unary square root.
/**
 * If \p x is less than negative zero, the result will be NaN.
 *
 * @param x the \link mppp::real128 real128 \endlink whose square root will be returned.
 *
 * @return the nonnegative square root of \p x.
 */
inline real128 sqrt(real128 x)
{
    x.sqrt();
    return x;
}

/// Unary cube root.
/**
 * @param x the \link mppp::real128 real128 \endlink whose cube root will be returned.
 *
 * @return the real cube root of \p x.
 */
inline real128 cbrt(real128 x)
{
    x.cbrt();
    return x;
}

/// Euclidean distance.
/**
 * @param x the first \link mppp::real128 real128 \endlink argument.
 * @param y the second \link mppp::real128 real128 \endlink argument.
 *
 * @return the euclidean distance \f$ \sqrt{x^2+y^2} \f$.
 */
inline real128 hypot(const real128 &x, const real128 &y)
{
    return real128{::hypotq(x.m_value, y.m_value)};
}

/** @} */

/** @defgroup real128_exponentiation real128_exponentiation
 *  @{
 */

inline namespace detail
{

inline real128 dispatch_pow(const real128 &x, const real128 &y)
{
    return real128{::powq(x.m_value, y.m_value)};
}

template <typename T, enable_if_t<is_cpp_interoperable<T>::value, int> = 0>
inline real128 dispatch_pow(const real128 &x, const T &y)
{
    return real128{::powq(x.m_value, y)};
}

template <typename T, enable_if_t<is_cpp_interoperable<T>::value, int> = 0>
inline real128 dispatch_pow(const T &x, const real128 &y)
{
    return real128{::powq(x, y.m_value)};
}

template <typename T, enable_if_t<disjunction<is_integer<T>, is_rational<T>>::value, int> = 0>
inline real128 dispatch_pow(const real128 &x, const T &y)
{
    return dispatch_pow(x, real128{y});
}

template <typename T, enable_if_t<disjunction<is_integer<T>, is_rational<T>>::value, int> = 0>
inline real128 dispatch_pow(const T &x, const real128 &y)
{
    return dispatch_pow(real128{x}, y);
}
}

/// Exponentiation.
/**
 * \rststar
 * This function will raise the base ``x`` to the exponent ``y``. Internally,
 * the implementation uses the ``powq()`` function from the quadmath library,
 * after the conversion of one of the operands to :cpp:class:`~mppp::real128`
 * (if necessary).
 * \endrststar
 *
 * @param x the base.
 * @param y the exponent.
 *
 * @return \f$ x^y \f$.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline real128 pow(const T &x, const Real128OpTypes<T> &y)
#else
template <typename T, typename U, real128_op_types_enabler<T, U> = 0>
inline real128 pow(const T &x, const U &y)
#endif
{
    return dispatch_pow(x, y);
}

/** @} */

/** @defgroup real128_operators real128_operators
 *  @{
 */

/// Identity operator.
/**
 * @param x the \link mppp::real128 real128 \endlink that will be copied.
 *
 * @return a copy of \p x.
 */
constexpr real128 operator+(real128 x)
{
    return x;
}

inline namespace detail
{

constexpr real128 dispatch_add(const real128 &x, const real128 &y)
{
    return real128{x.m_value + y.m_value};
}

template <typename T, enable_if_t<is_cpp_interoperable<T>::value, int> = 0>
constexpr real128 dispatch_add(const real128 &x, const T &y)
{
    return real128{x.m_value + y};
}

template <typename T, enable_if_t<is_cpp_interoperable<T>::value, int> = 0>
constexpr real128 dispatch_add(const T &x, const real128 &y)
{
    return real128{x + y.m_value};
}
}

/// Binary addition involving \link mppp::real128 real128 \endlink and C++ types.
/**
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \f$ x + y \f$.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
constexpr real128 operator+(const T &x, const Real128CppOpTypes<T> &y)
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
constexpr real128 operator+(const T &x, const U &y)
#endif
{
    return dispatch_add(x, y);
}

inline namespace detail
{

template <typename T, enable_if_t<disjunction<is_integer<T>, is_rational<T>>::value, int> = 0>
inline real128 dispatch_add(const real128 &x, const T &y)
{
    return x + real128{y};
}

template <typename T, enable_if_t<disjunction<is_integer<T>, is_rational<T>>::value, int> = 0>
inline real128 dispatch_add(const T &x, const real128 &y)
{
    return real128{x} + y;
}
}

/// Binary addition involving \link mppp::real128 real128 \endlink and mp++ types.
/**
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \f$ x + y \f$.
 *
 * @throws unspecified any exception thrown by the constructor of \link mppp::real128 real128 \endlink
 * from the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline real128 operator+(const T &x, const Real128MpppOpTypes<T> &y)
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
inline real128 operator+(const T &x, const U &y)
#endif
{
    return dispatch_add(x, y);
}

/// Prefix increment.
/**
 * This operator will increment \p x by one.
 *
 * \rststar
 * .. note::
 *
 *   This operator is marked as ``constexpr`` only if at least C++14 is being used.
 * \endrststar
 *
 * @param x the \link mppp::real128 real128 \endlink that will be increased.
 *
 * @return a reference to \p x after the increment.
 */
MPPP_CONSTEXPR_14 real128 &operator++(real128 &x)
{
    ++x.m_value;
    return x;
}

/// Suffix increment.
/**
 * This operator will increment \p x by one and return a copy of \p x as it was before the increment.
 *
 * \rststar
 * .. note::
 *
 *   This operator is marked as ``constexpr`` only if at least C++14 is being used.
 * \endrststar
 *
 * @param x the \link mppp::real128 real128 \endlink that will be increased.
 *
 * @return a copy of \p x before the increment.
 */
MPPP_CONSTEXPR_14 real128 operator++(real128 &x, int)
{
    auto retval(x);
    ++x;
    return retval;
}

/// Negation operator.
/**
 * @param x the \link mppp::real128 real128 \endlink whose opposite will be returned.
 *
 * @return \f$ -x \f$.
 */
constexpr real128 operator-(const real128 &x)
{
    return real128{-x.m_value};
}

inline namespace detail
{

constexpr real128 dispatch_sub(const real128 &x, const real128 &y)
{
    return real128{x.m_value - y.m_value};
}

template <typename T, enable_if_t<is_cpp_interoperable<T>::value, int> = 0>
constexpr real128 dispatch_sub(const real128 &x, const T &y)
{
    return real128{x.m_value - y};
}

template <typename T, enable_if_t<is_cpp_interoperable<T>::value, int> = 0>
constexpr real128 dispatch_sub(const T &x, const real128 &y)
{
    return real128{x - y.m_value};
}
}

/// Binary subtraction involving \link mppp::real128 real128 \endlink and C++ types.
/**
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \f$ x - y \f$.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
constexpr real128 operator-(const T &x, const Real128CppOpTypes<T> &y)
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
constexpr real128 operator-(const T &x, const U &y)
#endif
{
    return dispatch_sub(x, y);
}

inline namespace detail
{

template <typename T, enable_if_t<disjunction<is_integer<T>, is_rational<T>>::value, int> = 0>
inline real128 dispatch_sub(const real128 &x, const T &y)
{
    return x - real128{y};
}

template <typename T, enable_if_t<disjunction<is_integer<T>, is_rational<T>>::value, int> = 0>
inline real128 dispatch_sub(const T &x, const real128 &y)
{
    return real128{x} - y;
}
}

/// Binary subtraction involving \link mppp::real128 real128 \endlink and mp++ types.
/**
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \f$ x + y \f$.
 *
 * @throws unspecified any exception thrown by the constructor of \link mppp::real128 real128 \endlink
 * from the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline real128 operator-(const T &x, const Real128MpppOpTypes<T> &y)
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
inline real128 operator-(const T &x, const U &y)
#endif
{
    return dispatch_sub(x, y);
}

/// Prefix decrement.
/**
 * This operator will decrement \p x by one.
 *
 * \rststar
 * .. note::
 *
 *   This operator is marked as ``constexpr`` only if at least C++14 is being used.
 * \endrststar
 *
 * @param x the \link mppp::real128 real128 \endlink that will be decreased.
 *
 * @return a reference to \p x after the decrement.
 */
MPPP_CONSTEXPR_14 real128 &operator--(real128 &x)
{
    --x.m_value;
    return x;
}

/// Suffix decrement.
/**
 * This operator will decrement \p x by one and return a copy of \p x as it was before the decrement.
 *
 * \rststar
 * .. note::
 *
 *   This operator is marked as ``constexpr`` only if at least C++14 is being used.
 * \endrststar
 *
 * @param x the \link mppp::real128 real128 \endlink that will be decreased.
 *
 * @return a copy of \p x before the decrement.
 */
MPPP_CONSTEXPR_14 real128 operator--(real128 &x, int)
{
    auto retval(x);
    --x;
    return retval;
}

inline namespace detail
{

constexpr real128 dispatch_mul(const real128 &x, const real128 &y)
{
    return real128{x.m_value * y.m_value};
}

template <typename T, enable_if_t<is_cpp_interoperable<T>::value, int> = 0>
constexpr real128 dispatch_mul(const real128 &x, const T &y)
{
    return real128{x.m_value * y};
}

template <typename T, enable_if_t<is_cpp_interoperable<T>::value, int> = 0>
constexpr real128 dispatch_mul(const T &x, const real128 &y)
{
    return real128{x * y.m_value};
}
}

/// Binary multiplication involving \link mppp::real128 real128 \endlink and C++ types.
/**
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \f$ x \times y \f$.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
constexpr real128 operator*(const T &x, const Real128CppOpTypes<T> &y)
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
constexpr real128 operator*(const T &x, const U &y)
#endif
{
    return dispatch_mul(x, y);
}

inline namespace detail
{

template <typename T, enable_if_t<disjunction<is_integer<T>, is_rational<T>>::value, int> = 0>
inline real128 dispatch_mul(const real128 &x, const T &y)
{
    return x * real128{y};
}

template <typename T, enable_if_t<disjunction<is_integer<T>, is_rational<T>>::value, int> = 0>
inline real128 dispatch_mul(const T &x, const real128 &y)
{
    return real128{x} * y;
}
}

/// Binary multiplication involving \link mppp::real128 real128 \endlink and mp++ types.
/**
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \f$ x \times y \f$.
 *
 * @throws unspecified any exception thrown by the constructor of \link mppp::real128 real128 \endlink
 * from the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline real128 operator*(const T &x, const Real128MpppOpTypes<T> &y)
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
inline real128 operator*(const T &x, const U &y)
#endif
{
    return dispatch_mul(x, y);
}

inline namespace detail
{

constexpr real128 dispatch_div(const real128 &x, const real128 &y)
{
    return real128{x.m_value / y.m_value};
}

template <typename T, enable_if_t<is_cpp_interoperable<T>::value, int> = 0>
constexpr real128 dispatch_div(const real128 &x, const T &y)
{
    return real128{x.m_value / y};
}

template <typename T, enable_if_t<is_cpp_interoperable<T>::value, int> = 0>
constexpr real128 dispatch_div(const T &x, const real128 &y)
{
    return real128{x / y.m_value};
}
}

/// Binary division involving \link mppp::real128 real128 \endlink and C++ types.
/**
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \f$ x / y \f$.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
constexpr real128 operator/(const T &x, const Real128CppOpTypes<T> &y)
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
constexpr real128 operator/(const T &x, const U &y)
#endif
{
    return dispatch_div(x, y);
}

inline namespace detail
{

template <typename T, enable_if_t<disjunction<is_integer<T>, is_rational<T>>::value, int> = 0>
inline real128 dispatch_div(const real128 &x, const T &y)
{
    return x / real128{y};
}

template <typename T, enable_if_t<disjunction<is_integer<T>, is_rational<T>>::value, int> = 0>
inline real128 dispatch_div(const T &x, const real128 &y)
{
    return real128{x} / y;
}
}

/// Binary division involving \link mppp::real128 real128 \endlink and mp++ types.
/**
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \f$ x / y \f$.
 *
 * @throws unspecified any exception thrown by the constructor of \link mppp::real128 real128 \endlink
 * from the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T>
inline real128 operator/(const T &x, const Real128MpppOpTypes<T> &y)
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
inline real128 operator/(const T &x, const U &y)
#endif
{
    return dispatch_div(x, y);
}

/** @} */

inline namespace detail
{

// Some constants useful in the implementation of real128 constants below.
template <typename = void>
struct real128_constants {
    // 2**-112.
    static constexpr real128 two_112 = real128{1} / (1ull << 32) / (1ull << 32) / (1ull << 48);
    // 2**-48.
    static constexpr real128 two_48 = real128{1} / (1ull << 48);
};

#if MPPP_CPLUSPLUS < 201703L

template <typename T>
constexpr real128 real128_constants<T>::two_112;

template <typename T>
constexpr real128 real128_constants<T>::two_48;

#endif
}

/** @defgroup real128_constants real128_constants
 *  @{
 */

/// The positive \f$ \infty \f$ constant.
/**
 * @return \f$ +\infty \f$.
 */
constexpr real128 real128_inf()
{
    // NOTE: here we cannot use 1/0 as that is apparently forbidden in a constant expression:
    // https://stackoverflow.com/questions/33916113/dividing-by-zero-in-a-constant-expression
    // The infinity value is technically not guaranteed to be there in all implementations.
    static_assert(std::numeric_limits<double>::has_infinity, "The 'double' type does not support infinity.");
    return real128{std::numeric_limits<double>::infinity()};
}

/// NaN constant.
/**
 * @return a NaN with sign bit unset.
 */
constexpr real128 real128_nan()
{
    // NOTE: make sure we return a NaN without sign bit set. On at least some implementations,
    // inf / inf returns -nan.
    return (real128_inf() / real128_inf()).signbit() ? -(real128_inf() / real128_inf())
                                                     : (real128_inf() / real128_inf());
}

/// The \f$ \pi \f$ constant.
/**
 * @return the quadruple-precision value of \f$ \pi \f$.
 */
constexpr real128 real128_pi()
{
    using c = real128_constants<>;
    return 2 * (9541308523256152504ull * c::two_112 + 160664882791121ull * c::two_48 + 1);
}

/// The \f$ \mathrm{e} \f$ constant (Euler's number).
/**
 * @return the quadruple-precision value of \f$ \mathrm{e} \f$.
 */
constexpr real128 real128_e()
{
    using c = real128_constants<>;
    return 2 * (10751604932185443962ull * c::two_112 + 101089180468598ull * c::two_48 + 1);
}

/// The \f$ \sqrt{2} \f$ constant.
/**
 * @return the quadruple-precision value of \f$ \sqrt{2} \f$.
 */
constexpr real128 real128_sqrt2()
{
    using c = real128_constants<>;
    return 14486024992869247637ull * c::two_112 + 116590752822204ull * c::two_48 + 1;
}

#if MPPP_CPLUSPLUS >= 201703L

// NOTE: namespace scope constexpr variables are *not* implicitly inline, so we need
// inline here:
// http://en.cppreference.com/w/cpp/language/inline
// Note that constexpr static member variables are implicitly inline instead.

/// Quadruple-precision \f$ +\infty \f$ constant.
inline constexpr real128 inf128 = real128_inf();

/// Quadruple-precision NaN constant.
inline constexpr real128 nan128 = real128_nan();

/// Quadruple-precision \f$ \pi \f$ constant.
inline constexpr real128 pi128 = real128_pi();

/// Quadruple-precision \f$ \mathrm{e} \f$ constant (Euler's number).
inline constexpr real128 e128 = real128_e();

/// Quadruple-precision \f$ \sqrt{2} \f$ constant.
inline constexpr real128 sqrt2128 = real128_sqrt2();

#endif

/** @} */
}

#else

#error The real128.hpp header was included but mp++ was not configured with the MPPP_WITH_QUADMATH option.

#endif

#endif
