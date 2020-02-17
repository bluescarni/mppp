// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
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

#include <mp++/concepts.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/detail/visibility.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>

namespace mppp
{

// Fwd declaration.
class real128;

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

// String conversion helpers.
MPPP_DLL_PUBLIC void float128_stream(std::ostream &, const __float128 &);
MPPP_DLL_PUBLIC __float128 str_to_float128(const char *);

// Wrappers for use in various functions below (so that
// we don't have to include quadmath.h here).
MPPP_DLL_PUBLIC __float128 scalbnq(__float128, int);
MPPP_DLL_PUBLIC __float128 scalblnq(__float128, long);
MPPP_DLL_PUBLIC __float128 powq(__float128, __float128);

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

template <typename T>
using is_real128_cpp_interoperable =
#if defined(MPPP_WITH_MPFR) && defined(__clang__)
    conjunction<is_cpp_interoperable<T>, negation<std::is_same<T, long double>>>;
#else
    is_cpp_interoperable<T>;
#endif

template <typename T, typename U>
using are_real128_cpp_op_types = disjunction<conjunction<std::is_same<T, real128>, std::is_same<U, real128>>,
                                             conjunction<std::is_same<T, real128>, is_real128_cpp_interoperable<U>>,
                                             conjunction<std::is_same<U, real128>, is_real128_cpp_interoperable<T>>>;

template <typename T>
using is_real128_mppp_interoperable = disjunction<is_integer<T>, is_rational<T>>;

template <typename T, typename U>
using are_real128_mppp_op_types = disjunction<conjunction<std::is_same<T, real128>, is_real128_mppp_interoperable<U>>,
                                              conjunction<std::is_same<U, real128>, is_real128_mppp_interoperable<T>>>;
} // namespace detail

template <typename T>
#if defined(MPPP_HAVE_CONCEPTS)
MPPP_CONCEPT_DECL Real128CppInteroperable = detail::is_real128_cpp_interoperable<T>::value;
#else
using real128_cpp_interoperable_enabler = detail::enable_if_t<detail::is_real128_cpp_interoperable<T>::value, int>;
#endif

template <typename T>
#if defined(MPPP_HAVE_CONCEPTS)
MPPP_CONCEPT_DECL Real128MpppInteroperable = detail::is_real128_mppp_interoperable<T>::value;
#else
using real128_mppp_interoperable_enabler = detail::enable_if_t<detail::is_real128_mppp_interoperable<T>::value, int>;
#endif

template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
MPPP_CONCEPT_DECL Real128CppOpTypes = detail::are_real128_cpp_op_types<T, U>::value;
#else
using real128_cpp_op_types_enabler = detail::enable_if_t<detail::are_real128_cpp_op_types<T, U>::value, int>;
#endif

template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
MPPP_CONCEPT_DECL Real128MpppOpTypes = detail::are_real128_mppp_op_types<T, U>::value;
#else
using real128_mppp_op_types_enabler = detail::enable_if_t<detail::are_real128_mppp_op_types<T, U>::value, int>;
#endif

template <typename T, typename U>
#if defined(MPPP_HAVE_CONCEPTS)
MPPP_CONCEPT_DECL Real128OpTypes = Real128CppOpTypes<T, U> || Real128MpppOpTypes<T, U>;
#else
using real128_op_types_enabler = detail::enable_if_t<
    detail::disjunction<detail::are_real128_cpp_op_types<T, U>, detail::are_real128_mppp_op_types<T, U>>::value, int>;
#endif

// For the future:
// - in theory we could investigate a clang windows build as well, taking the libquadmath binary from mingw?
// - finish wrapping up the quadmath API
// - consider constexpr implementation of some basic functions (sqrt, log, etc.)
// - the constructor from integer *may* be implemented in a faster way by reading directly the hi/lo parts
//   and writing them to the ieee union (instead right now we are using __float128 arithmetics and quadmath
//   functions). Make sure to benchmark first though...
// - initial code for stream formatting at 687b86d9380a534048f62aac3815c31b094e52d2. The problem to be
//   solved is the segfault in MinGW.

/// Quadruple-precision floating-point class.
/**
 * \rststar
 * This class represents real values encoded in the quadruple-precision IEEE 754 floating-point format
 * (which features up to 36 decimal digits of precision).
 * The class is a thin wrapper around the :cpp:type:`__float128` type and the quadmath library, available in GCC
 * and recent Clang versions on most modern platforms, on top of which it provides the following additions:
 *
 * * interoperability with other mp++ classes,
 * * consistent behaviour with respect to the conventions followed elsewhere in mp++ (e.g., values are
 *   default-initialised to zero rather than to indefinite values, conversions must be explicit, etc.),
 * * enhanced compile-time (``constexpr``) capabilities,
 * * a generic C++ API.
 *
 * This class has the look and feel of a C++ builtin type: it can interact with most of C++'s integral and
 * floating-point primitive types (see the :cpp:concept:`~mppp::Real128CppInteroperable` concept for the full list),
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
 * will not work, and direct initialization should be used instead:
 *
 * .. code-block:: c++
 *
 *    real128 r{5.23};
 *    int m{r};
 *
 * Most of the functionality is exposed via plain :ref:`functions <real128_functions>`, with the
 * general convention that the functions are named after the corresponding quadmath functions minus the trailing ``q``
 * suffix. For instance, the quadmath code
 *
 * .. code-block:: c++
 *
 *    __float128 a = 1;
 *    auto b = ::sinq(a);
 *
 * that computes the sine of 1 in quadruple precision, storing the result in ``b``, becomes
 *
 * .. code-block:: c++
 *
 *    real128 a{1};
 *    auto b = sin(a);
 *
 * where the ``sin()`` function is resolved via argument-dependent lookup.
 *
 * Two ways of calling unary functions are usually provided:
 *
 * * a unary free function returning the result of the operation,
 * * a nullary member function that modifies the calling object in-place.
 *
 * For instance, here are two possible ways of computing the absolute value:
 *
 * .. code-block:: c++
 *
 *    real128 r1, r2{-5};
 *    r1 = abs(r2); // Unary abs(): returns the absolute value of r2, which is
 *                  // then assigned to r1.
 *    r2.abs();     // Member function abs(): replaces the value of r2 with its
 *                  // absolute value.
 *
 * Note that at this time a subset of the quadmath API has been wrapped by :cpp:class:`~mppp::real128`.
 *
 * Various :ref:`overloaded operators <real128_operators>` are provided.
 * The common arithmetic operators (``+``, ``-``, ``*`` and ``/``) always return :cpp:class:`~mppp::real128`
 * as a result, promoting at most one operand to :cpp:class:`~mppp::real128` before actually performing
 * the computation. Similarly, the relational operators, ``==``, ``!=``, ``<``, ``>``, ``<=`` and ``>=`` will promote at
 * most one argument to :cpp:class:`~mppp::real128` before performing the comparison. Alternative comparison functions
 * treating NaNs specially are provided for use in the C++ standard library (and wherever strict weak ordering relations
 * are needed).
 *
 * The :cpp:class:`~mppp::real128` class is a `literal type
 * <https://en.cppreference.com/w/cpp/named_req/LiteralType>`__, and, whenever possible, operations involving
 * :cpp:class:`~mppp::real128` are marked as ``constexpr``. Some functions which are not ``constexpr`` in the quadmath
 * library have been reimplemented as ``constexpr`` functions via compiler builtins.
 *
 * .. seealso::
 *    https://gcc.gnu.org/onlinedocs/gcc/Floating-Types.html
 *
 *    https://gcc.gnu.org/onlinedocs/libquadmath/
 * \endrststar
 */
class MPPP_DLL_PUBLIC real128
{
    // Number of digits in the significand.
    static constexpr unsigned sig_digits = 113;

public:
    /// Default constructor.
    /**
     * The default constructor will set \p this to zero.
     */
    constexpr real128() : m_value(0) {}
    /// Trivial copy constructor.
    /**
     * @param other the construction argument.
     */
    constexpr real128(const real128 &other) = default;
    /// Trivial move constructor.
    /**
     * @param other the construction argument.
     */
    constexpr real128(real128 &&other) = default;
    /// Constructor from a quadruple-precision floating-point value.
    /**
     * This constructor will initialise the internal value with \p x.
     *
     * @param x the quadruple-precision floating-point variable that will be
     * used to initialise the internal value.
     */
    constexpr explicit real128(__float128 x) : m_value(x) {}
    /// Constructor from interoperable C++ types.
    /**
     * This constructor will initialise the internal value with \p x.
     *
     * @param x the value that will be used for initialisation.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    template <Real128CppInteroperable T>
#else
    template <typename T, real128_cpp_interoperable_enabler<T> = 0>
#endif
    constexpr explicit real128(T x) : m_value(x)
    {
    }

private:
    // Construction from mp++ types.
    template <std::size_t SSize>
    void dispatch_mppp_construction(const integer<SSize> &n)
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
        while (ls && read_bits < sig_digits) {
            // Number of bits to be read from the current limb: GMP_NUMB_BITS or less.
            const unsigned rbits = detail::c_min(unsigned(GMP_NUMB_BITS), sig_digits - read_bits);
            // Shift m_value by rbits.
            // NOTE: safe to cast to int here, as rbits is not greater than GMP_NUMB_BITS which in turn fits in int.
            m_value = detail::scalbnq(m_value, static_cast<int>(rbits));
            // Add the bottom part, and move to the next limb. We might need to remove lower bits
            // in case rbits is not exactly GMP_NUMB_BITS.
            m_value += (ptr[--ls] & GMP_NUMB_MASK) >> (unsigned(GMP_NUMB_BITS) - rbits);
            // Update the number of read bits.
            // NOTE: read_bits can never be increased past sig_digits, due to the definition of rbits.
            // Hence, this addition can never overflow (as sig_digits is unsigned itself).
            read_bits += rbits;
        }
        if (read_bits < n_bits) {
            // We did not read from n all its bits. This means that n has more bits than the quad-precision
            // significand, and thus we need to multiply this by 2**unread_bits.
            // Use the long variant of scalbn() to maximise the range.
            m_value = detail::scalblnq(m_value, detail::safe_cast<long>(n_bits - read_bits));
        }
        // Fix the sign as needed.
        if (n_sgn == -1) {
            m_value = -m_value;
        }
    }
    template <std::size_t SSize>
    void dispatch_mppp_construction(const rational<SSize> &q)
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
            tdiv_q_2exp(n, q.get_num(), detail::safe_cast<::mp_bitcnt_t>(shift));
            m_value = real128{n}.m_value / real128{q.get_den()}.m_value;
            m_value = detail::scalblnq(m_value, detail::safe_cast<long>(shift));
        } else if (n_bits <= sig_digits && d_bits > sig_digits) {
            // The opposite of above.
            MPPP_MAYBE_TLS integer<SSize> d;
            const auto shift = d_bits - sig_digits;
            tdiv_q_2exp(d, q.get_den(), detail::safe_cast<::mp_bitcnt_t>(shift));
            m_value = real128{q.get_num()}.m_value / real128{d}.m_value;
            m_value = detail::scalblnq(m_value, detail::negate_unsigned<long>(shift));
        } else {
            // Both num and den have more bits than quad's significand. We will downshift
            // both until they have 113 bits, do the division, and then recover the shifted bits.
            MPPP_MAYBE_TLS integer<SSize> n;
            MPPP_MAYBE_TLS integer<SSize> d;
            const auto n_shift = n_bits - sig_digits;
            const auto d_shift = d_bits - sig_digits;
            tdiv_q_2exp(n, q.get_num(), detail::safe_cast<::mp_bitcnt_t>(n_shift));
            tdiv_q_2exp(d, q.get_den(), detail::safe_cast<::mp_bitcnt_t>(d_shift));
            m_value = real128{n}.m_value / real128{d}.m_value;
            if (n_shift >= d_shift) {
                m_value = detail::scalblnq(m_value, detail::safe_cast<long>(n_shift - d_shift));
            } else {
                m_value = detail::scalblnq(m_value, detail::negate_unsigned<long>(d_shift - n_shift));
            }
        }
    }

public:
    /// Constructor from mp++ types.
    /**
     * \rststar
     * This constructor will initialise the internal value with with the :cpp:concept:`~mppp::Real128MpppInteroperable`
     * ``x``. Depending on the value of ``x``, ``this`` may not be exactly equal to ``x`` after initialisation.
     * \endrststar
     *
     * @param x the value that will be used for the initialisation.
     *
     * @throws std::overflow_error if an overflow occurs during initialisation.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    template <Real128MpppInteroperable T>
#else
    template <typename T, real128_mppp_interoperable_enabler<T> = 0>
#endif
    explicit real128(const T &x)
    {
        dispatch_mppp_construction(x);
    }

private:
    // A tag to call private ctors.
    struct ptag {
    };
    explicit real128(const ptag &, const char *s) : m_value(detail::str_to_float128(s)) {}
    explicit real128(const ptag &, const std::string &s) : real128(s.c_str()) {}
#if defined(MPPP_HAVE_STRING_VIEW)
    explicit real128(const ptag &, const std::string_view &s) : real128(s.data(), s.data() + s.size()) {}
#endif
public:
    /// Constructor from string.
    /**
     * \rststar
     * This constructor will initialise \p this from the :cpp:concept:`~mppp::StringType` ``s``.
     * The accepted string formats are detailed in the quadmath library's documentation
     * (see the link below). Leading whitespaces are accepted (and ignored), but trailing whitespaces
     * will raise an error.
     *
     * .. seealso::
     *    https://gcc.gnu.org/onlinedocs/libquadmath/strtoflt128.html
     * \endrststar
     *
     * @param s the string that will be used to initialise \p this.
     *
     * @throws std::invalid_argument if \p s does not represent a valid quadruple-precision
     * floating-point value.
     * @throws unspecified any exception thrown by memory errors in standard containers.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    explicit real128(const StringType &s)
#else
    template <typename T, string_type_enabler<T> = 0>
    explicit real128(const T &s)
#endif
        : real128(ptag{}, s)
    {
    }
    // Constructor from range of characters.
    explicit real128(const char *, const char *);
    /// Trivial copy assignment operator.
    /**
     * @param other the assignment argument.
     *
     * @return a reference to \p this.
     */
    real128 &operator=(const real128 &other) = default;
    /// Trivial move assignment operator.
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
    MPPP_CONSTEXPR_14 real128 &operator=(const __float128 &x)
    {
        m_value = x;
        return *this;
    }
    /// Assignment from interoperable C++ types.
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
    template <Real128CppInteroperable T>
#else
    template <typename T, real128_cpp_interoperable_enabler<T> = 0>
#endif
    MPPP_CONSTEXPR_14 real128 &operator=(const T &x)
    {
        m_value = x;
        return *this;
    }
    /// Assignment from mp++ types.
    /**
     * \rststar
     * The body of this operator is equivalent to:
     *
     * .. code-block:: c++
     *
     *    return *this = real128{x};
     *
     * That is, a temporary :cpp:class:`~mppp::real128` is constructed from ``x`` and it is then move-assigned to
     * ``this``.
     * \endrststar
     *
     * @param x the assignment argument.
     *
     * @return a reference to \p this.
     *
     * @throws unspecified any exception thrown by the construction of a \link mppp::real128 real128\endlink from
     * ``x``.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    template <Real128MpppInteroperable T>
#else
    template <typename T, real128_mppp_interoperable_enabler<T> = 0>
#endif
    real128 &operator=(const T &x)
    {
        return *this = real128{x};
    }
    /// Assignment from string.
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
     * @param s the string that will be used for the assignment.
     *
     * @return a reference to \p this.
     *
     * @throws unspecified any exception thrown by the constructor from string.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    real128 &operator=(const StringType &s)
#else
    template <typename T, string_type_enabler<T> = 0>
    real128 &operator=(const T &s)
#endif
    {
        return *this = real128{s};
    }
/// Conversion operator to interoperable C++ types.
/**
 * \rststar
 * This operator will convert ``this`` to a :cpp:concept:`~mppp::Real128CppInteroperable` type. The conversion uses
 * a direct ``static_cast()`` of the internal :cpp:member:`~mppp::real128::m_value` member to the target type,
 * and thus no checks are performed to ensure that the value of ``this`` can be represented by the target type.
 * Conversion to integral types will produce the truncated counterpart of ``this``.
 * \endrststar
 *
 * @return \p this converted to \p T.
 */
#if defined(MPPP_HAVE_CONCEPTS)
    template <Real128CppInteroperable T>
#else
    template <typename T, real128_cpp_interoperable_enabler<T> = 0>
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

private:
    template <std::size_t SSize>
    bool mppp_conversion(integer<SSize> &rop) const
    {
        // Build the union and assign the value.
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
    template <std::size_t SSize>
    bool mppp_conversion(rational<SSize> &rop) const
    {
        // Build the union and assign the value.
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

public:
    /// Conversion operator to mp++ types.
    /**
     * \rststar
     * This operator will convert ``this`` to a :cpp:concept:`~mppp::Real128MpppInteroperable` type.
     *
     * For conversions to :cpp:class:`~mppp::integer`, if ``this`` does not represent
     * an integral value the conversion will yield the truncated counterpart of ``this``.
     *
     * For conversions to :cpp:class:`~mppp::rational`, the conversion,
     * if successful, is exact.
     * \endrststar
     *
     * @return \p this converted to the target type.
     *
     * @throws std::domain_error if \p this represents a non-finite value.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    template <Real128MpppInteroperable T>
#else
    template <typename T, real128_mppp_interoperable_enabler<T> = 0>
#endif
    explicit operator T() const
    {
        T retval;
        if (mppp_unlikely(!mppp_conversion(retval))) {
            throw std::domain_error(std::string{"Cannot convert a non-finite real128 to "}
                                    + (detail::is_integer<T>::value ? "an integer" : "a rational"));
        }
        return retval;
    }
    /// Conversion method to interoperable C++ types.
    /**
     * \rststar
     * This method will cast the :cpp:member:`~mppp::real128::m_value` member to the
     * :cpp:concept:`~mppp::Real128CppInteroperable` type T, and assign the result
     * to ``rop``.
     *
     * .. note::
     *    This method is provided for consistency with other getter methods,
     *    and it does not offer any additional functionality over the plain
     *    conversion operator.
     *
     * .. note::
     *
     *   This method is marked as ``constexpr`` only if at least C++14 is being used.
     * \endrststar
     *
     * @param rop the variable which will store the result of the conversion.
     *
     * @return always ``true``.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    template <Real128CppInteroperable T>
#else
    template <typename T, real128_cpp_interoperable_enabler<T> = 0>
#endif
    MPPP_CONSTEXPR_14 bool get(T &rop) const
    {
        return rop = static_cast<T>(m_value), true;
    }
    /// Conversion method to mp++ types.
    /**
     * \rststar
     * This method, similarly to the corresponding conversion operator, will convert ``this`` to a
     * :cpp:concept:`~mppp::Real128MpppInteroperable` type, storing the result of the conversion into ``rop``.
     * Differently from the conversion operator, this method does not raise any exception: if the conversion is
     * successful, the method will return ``true``, otherwise the method will return ``false``. If the conversion
     * fails, ``rop`` will not be altered.
     * \endrststar
     *
     * @param rop the variable which will store the result of the conversion.
     *
     * @return ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail only if
     * ``this`` does not represent a finite value.
     */
#if defined(MPPP_HAVE_CONCEPTS)
    template <Real128MpppInteroperable T>
#else
    template <typename T, real128_mppp_interoperable_enabler<T> = 0>
#endif
    bool get(T &rop) const
    {
        return mppp_conversion(rop);
    }
    // Convert to string.
    std::string to_string() const;
    /// Get the IEEE representation of the value.
    /**
     * This method will return a tuple containing the IEEE quadruple-precision floating-point representation
     * of the value. The returned tuple elements are, in order:
     * - the sign of the value (1 for a negative sign bit, 0 for a positive sign bit),
     * - the exponent (a 15-bit unsigned value),
     * - the high part of the significand (a 48-bit unsigned value),
     * - the low part of the significand (a 64-bit unsigned value).
     *
     * \rststar
     * .. seealso::
     *    https://en.wikipedia.org/wiki/Quadruple-precision_floating-point_format
     * \endrststar
     *
     * @return a tuple containing the IEEE quadruple-precision floating-point representation of the value stored
     * in \p this.
     */
    std::tuple<std::uint_least8_t, std::uint_least16_t, std::uint_least64_t, std::uint_least64_t> get_ieee() const
    {
        detail::ieee_float128 ie;
        ie.value = m_value;
        return std::make_tuple(std::uint_least8_t(ie.i_eee.negative), std::uint_least16_t(ie.i_eee.exponent),
                               std::uint_least64_t(ie.i_eee.mant_high), std::uint_least64_t(ie.i_eee.mant_low));
    }
    // Sign bit.
    bool signbit() const;
    /// Categorise the floating point value.
    /**
     * This method will categorise the floating-point value of \p this into the 5 categories,
     * represented as ``int`` values, defined by the standard:
     * - ``FP_NAN`` for NaN,
     * - ``FP_INFINITE`` for infinite,
     * - ``FP_NORMAL`` for normal values,
     * - ``FP_SUBNORMAL`` for subnormal values,
     * - ``FP_ZERO`` for zero.
     *
     * @return the category to which the value of \p this belongs.
     */
    constexpr int fpclassify() const
    {
        // NOTE: according to the docs the builtin accepts generic floating-point types:
        // https://gcc.gnu.org/onlinedocs/gcc-7.2.0/gcc/Other-Builtins.html
        // It is used internally in the quadmath library as well:
        // https://github.com/gcc-mirror/gcc/blob/master/libquadmath/quadmath-imp.h
        return __builtin_fpclassify(FP_NAN, FP_INFINITE, FP_NORMAL, FP_SUBNORMAL, FP_ZERO, m_value);
    }
    /// Detect NaN.
    /**
     * @return \p true if \p this is NaN, \p false otherwise.
     */
    constexpr bool isnan() const
    {
        return fpclassify() == FP_NAN;
    }
    /// Detect infinity.
    /**
     * @return \p true if \p this is infinite, \p false otherwise.
     */
    constexpr bool isinf() const
    {
        return fpclassify() == FP_INFINITE;
    }
    /// Detect finite value.
    /**
     * @return \p true if \p this is finite, \p false otherwise.
     */
    constexpr bool finite() const
    {
        return fpclassify() == FP_NORMAL || fpclassify() == FP_SUBNORMAL || fpclassify() == FP_ZERO;
    }
    /// In-place absolute value.
    /**
     * This method will set \p this to its absolute value.
     *
     * \rststar
     * .. note::
     *
     *   This operator is marked as ``constexpr`` only if at least C++14 is being used.
     * \endrststar
     *
     * @return a reference to \p this.
     */
    MPPP_CONSTEXPR_14 real128 &abs()
    {
        const auto fpc = fpclassify();
        if (fpc == FP_NORMAL || fpc == FP_SUBNORMAL || fpc == FP_INFINITE) {
            // If the number is normal, subnormal or infinite compute its
            // absolute value.
            if (m_value < 0) {
                m_value = -m_value;
            }
        } else if (fpc == FP_ZERO) {
            // If the value is zero, it could be negative zero. Make sure we
            // set it to positive zero.
            m_value = 0;
        }
        // NOTE: for NaN, don't do anything and leave the NaN.
        return *this;
    }
    // In-place square root.
    real128 &sqrt();
    // In-place cube root.
    real128 &cbrt();
    // In-place sine.
    real128 &sin();
    // In-place cosine.
    real128 &cos();
    // In-place tangent.
    real128 &tan();
    // In-place inverse sine.
    real128 &asin();
    // In-place inverse cosine.
    real128 &acos();
    // In-place inverse tangent.
    real128 &atan();
    // In-place hyperbolic sine.
    real128 &sinh();
    // In-place hyperbolic cosine.
    real128 &cosh();
    // In-place hyperbolic tangent.
    real128 &tanh();
    // In-place inverse hyperbolic sine.
    real128 &asinh();
    // In-place inverse hyperbolic cosine.
    real128 &acosh();
    // In-place inverse hyperbolic tangent.
    real128 &atanh();
    // In-place natural exponential function.
    real128 &exp();
    // In-place natural logarithm.
    real128 &log();
    // In-place base-10 logarithm.
    real128 &log10();
    // In-place base-2 logarithm.
    real128 &log2();
    // In-place lgamma function.
    real128 &lgamma();
    // In-place error function.
    real128 &erf();
    /// The internal value.
    /**
     * \rststar
     * This class member gives direct access to the :cpp:type:`__float128` instance stored
     * inside :cpp:class:`~mppp::real128`.
     * \endrststar
     */
    __float128 m_value;
};

// Double check that real128 is a standard layout class.
static_assert(std::is_standard_layout<real128>::value, "real128 is not a standard layout class.");

/** @defgroup real128_conversion real128_conversion
 *  @{
 */

/// Conversion function to C++ types for \link mppp::real128 real128\endlink.
/**
 * \rststar
 * This function will convert the input :cpp:class:`~mppp::real128` ``x`` to a
 * :cpp:concept:`~mppp::Real128CppInteroperable` type, storing the result of the conversion into ``rop``.
 * The conversion is always successful.
 *
 * .. note::
 *    This function is provided for consistency with other getter functions,
 *    and it does not offer any additional functionality over the plain
 *    conversion operator.
 *
 * .. note::
 *
 *   This function is marked as ``constexpr`` only if at least C++14 is being used.
 * \endrststar
 *
 * @param rop the variable which will store the result of the conversion.
 * @param x the input \link mppp::real128 real128\endlink.
 *
 * @return always ``true``.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <Real128CppInteroperable T>
#else
template <typename T, real128_cpp_interoperable_enabler<T> = 0>
#endif
inline MPPP_CONSTEXPR_14 bool get(T &rop, const real128 &x)
{
    return x.get(rop);
}

/// Conversion function to mp++ types for \link mppp::real128 real128\endlink.
/**
 * \rststar
 * This function will convert the input :cpp:class:`~mppp::real128` ``x`` to a
 * :cpp:concept:`~mppp::Real128MpppInteroperable` type, storing the result of the conversion into ``rop``.
 * If the conversion is successful, the function
 * will return ``true``, otherwise the function will return ``false``. If the conversion fails, ``rop`` will
 * not be altered.
 * \endrststar
 *
 * @param rop the variable which will store the result of the conversion.
 * @param x the input \link mppp::real128 real128\endlink.
 *
 * @return ``true`` if the conversion succeeded, ``false`` otherwise. The conversion can fail only if ``x``
 * does not represent a finite value.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <Real128MpppInteroperable T>
#else
template <typename T, real128_mppp_interoperable_enabler<T> = 0>
#endif
inline bool get(T &rop, const real128 &x)
{
    return x.get(rop);
}

// Decompose into a normalized fraction and an integral power of two.
MPPP_DLL_PUBLIC real128 frexp(const real128 &, int *);

/** @} */

/** @defgroup real128_arithmetic real128_arithmetic
 *  @{
 */

// Fused multiply-add.
MPPP_DLL_PUBLIC real128 fma(const real128 &, const real128 &, const real128 &);

/// Unary absolute value.
/**
 * @param x the \link mppp::real128 real128\endlink whose absolute value will be computed.
 *
 * @return the absolute value of \p x.
 */
constexpr real128 abs(const real128 &x)
{
    return x.fpclassify() == FP_NAN
               ? x
               : ((x.fpclassify() == FP_NORMAL || x.fpclassify() == FP_SUBNORMAL || x.fpclassify() == FP_INFINITE)
                      ? (x.m_value < 0 ? real128{-x.m_value} : x)
                      : real128{0});
}

/// Multiply by power of 2 (\p int overload).
/**
 * @param x the input \link mppp::real128 real128\endlink.
 * @param n the power of 2 by which \p x will be multiplied.
 *
 * @return \p x multiplied by \f$ 2^n \f$.
 */
inline real128 scalbn(const real128 &x, int n)
{
    return real128{detail::scalbnq(x.m_value, n)};
}

/// Multiply by power of 2 (\p long overload).
/**
 * @param x the input \link mppp::real128 real128\endlink.
 * @param n the power of 2 by which \p x will be multiplied.
 *
 * @return \p x multiplied by \f$ 2^n \f$.
 */
inline real128 scalbln(const real128 &x, long n)
{
    return real128{detail::scalblnq(x.m_value, n)};
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
 * @param x the input \link mppp::real128 real128\endlink.
 *
 * @return a reference to \p os.
 *
 * @throws unspecified any exception thrown by real128::to_string().
 */
inline std::ostream &operator<<(std::ostream &os, const real128 &x)
{
    detail::float128_stream(os, x.m_value);
    return os;
}

/** @} */

/** @defgroup real128_comparison real128_comparison
 *  @{
 */

/// Sign bit of a \link mppp::real128 real128\endlink.
/**
 * @param x the \link mppp::real128 real128\endlink whose sign bit will be returned.
 *
 * @return the sign bit of \p x (as returned by mppp::real128::signbit()).
 */
inline bool signbit(const real128 &x)
{
    return x.signbit();
}

/// Categorise a \link mppp::real128 real128\endlink.
/**
 * @param x the \link mppp::real128 real128\endlink whose floating-point category will be returned.
 *
 * @return the category of the value of \p x, as established by mppp::real128::fpclassify().
 */
constexpr int fpclassify(const real128 &x)
{
    return x.fpclassify();
}

/// Detect if a \link mppp::real128 real128\endlink is NaN.
/**
 * @param x the \link mppp::real128 real128\endlink argument.
 *
 * @return \p true if \p x is NaN, \p false otherwise.
 */
constexpr bool isnan(const real128 &x)
{
    return x.isnan();
}

/// Detect if a \link mppp::real128 real128\endlink is infinite.
/**
 * @param x the \link mppp::real128 real128\endlink argument.
 *
 * @return \p true if \p x is infinite, \p false otherwise.
 */
constexpr bool isinf(const real128 &x)
{
    return x.isinf();
}

/// Detect if a \link mppp::real128 real128\endlink is finite.
/**
 * @param x the \link mppp::real128 real128\endlink argument.
 *
 * @return \p true if \p x is finite, \p false otherwise.
 */
constexpr bool finite(const real128 &x)
{
    return x.finite();
}

/// Equality predicate with special NaN handling for \link mppp::real128 real128\endlink.
/**
 * \rststar
 * If both ``x`` and ``y`` are not NaN, this function is identical to the equality operator for
 * :cpp:class:`~mppp::real128`. If at least one operand is NaN, this function will return ``true``
 * if both operands are NaN, ``false`` otherwise.
 *
 * In other words, this function behaves like an equality operator which considers all NaN
 * values equal to each other.
 * \endrststar
 *
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \p true if \f$ x = y \f$ (including the case in which both operands are NaN),
 * \p false otherwise.
 */
constexpr bool real128_equal_to(const real128 &, const real128 &);

/// Less-than predicate with special NaN handling for \link mppp::real128 real128\endlink.
/**
 * \rststar
 * If both ``x`` and ``y`` are not NaN, this function is identical to the less-than operator for
 * :cpp:class:`~mppp::real128`. If at least one operand is NaN, this function will return ``true``
 * if ``x`` is not NaN, ``false`` otherwise.
 *
 * In other words, this function behaves like a less-than operator which considers NaN values
 * greater than non-NaN values. This function can be used as a comparator in various facilities of the
 * standard library (e.g., ``std::sort()``, ``std::set``, etc.).
 * \endrststar
 *
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \p true if \f$ x < y \f$ (with NaN values considered greather than non-NaN values),
 * \p false otherwise.
 */
constexpr bool real128_lt(const real128 &, const real128 &);

/// Greater-than predicate with special NaN handling for \link mppp::real128 real128\endlink.
/**
 * \rststar
 * If both ``x`` and ``y`` are not NaN, this function is identical to the greater-than operator for
 * :cpp:class:`~mppp::real128`. If at least one operand is NaN, this function will return ``true``
 * if ``y`` is not NaN, ``false`` otherwise.
 *
 * In other words, this function behaves like a greater-than operator which considers NaN values
 * greater than non-NaN values. This function can be used as a comparator in various facilities of the
 * standard library (e.g., ``std::sort()``, ``std::set``, etc.).
 * \endrststar
 *
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \p true if \f$ x > y \f$ (with NaN values considered greather than non-NaN values),
 * \p false otherwise.
 */
constexpr bool real128_gt(const real128 &, const real128 &);

/** @} */

/** @defgroup real128_roots real128_roots
 *  @{
 */

/// Unary square root.
/**
 * If \p x is less than negative zero, the result will be NaN.
 *
 * @param x the \link mppp::real128 real128\endlink whose square root will be returned.
 *
 * @return the nonnegative square root of \p x.
 */
inline real128 sqrt(real128 x)
{
    return x.sqrt();
}

/// Unary cube root.
/**
 * @param x the \link mppp::real128 real128\endlink whose cube root will be returned.
 *
 * @return the real cube root of \p x.
 */
inline real128 cbrt(real128 x)
{
    return x.cbrt();
}

// Euclidean distance.
MPPP_DLL_PUBLIC real128 hypot(const real128 &, const real128 &);

/** @} */

/** @defgroup real128_exponentiation real128_exponentiation
 *  @{
 */

namespace detail
{

inline real128 dispatch_pow(const real128 &x, const real128 &y)
{
    return real128{detail::powq(x.m_value, y.m_value)};
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
inline real128 dispatch_pow(const real128 &x, const T &y)
{
    return real128{detail::powq(x.m_value, y)};
}

template <typename T, enable_if_t<is_real128_cpp_interoperable<T>::value, int> = 0>
inline real128 dispatch_pow(const T &x, const real128 &y)
{
    return real128{detail::powq(x, y.m_value)};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline real128 dispatch_pow(const real128 &x, const T &y)
{
    return dispatch_pow(x, real128{y});
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline real128 dispatch_pow(const T &x, const real128 &y)
{
    return dispatch_pow(real128{x}, y);
}
} // namespace detail

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
template <typename T, typename U>
requires Real128OpTypes<T, U>
#else
template <typename T, typename U, real128_op_types_enabler<T, U> = 0>
#endif
    inline real128 pow(const T &x, const U &y)
{
    return detail::dispatch_pow(x, y);
}

/** @} */

/** @defgroup real128_logexp real128_logexp
 *  @{
 */

/// Unary natural exponential function.
/**
 * @param x the \link mppp::real128 real128\endlink whose natural exponential function be computed.
 *
 * @return \f$ \mathrm{e} \f$ raised to the power of \p x.
 */
inline real128 exp(real128 x)
{
    return x.exp();
}

/// Unary natural logarithm.
/**
 * @param x the \link mppp::real128 real128\endlink whose natural logarithm will be computed.
 *
 * @return the natural logarithm of \p x.
 */
inline real128 log(real128 x)
{
    return x.log();
}

/// Unary base-10 logarithm.
/**
 * @param x the \link mppp::real128 real128\endlink whose base-10 logarithm will be computed.
 *
 * @return the base-10 logarithm of \p x.
 */
inline real128 log10(real128 x)
{
    return x.log10();
}

/// Unary base-2 logarithm.
/**
 * @param x the \link mppp::real128 real128\endlink whose base-2 logarithm will be computed.
 *
 * @return the base-2 logarithm of \p x.
 */
inline real128 log2(real128 x)
{
    return x.log2();
}

/** @} */

/** @defgroup real128_trig real128_trig
 *  @{
 */

/// Unary sine.
/**
 * @param x the \link mppp::real128 real128\endlink whose sine will be computed.
 *
 * @return the sine of \p x.
 */
inline real128 sin(real128 x)
{
    return x.sin();
}

/// Unary cosine.
/**
 * @param x the \link mppp::real128 real128\endlink whose cosine will be computed.
 *
 * @return the cosine of \p x.
 */
inline real128 cos(real128 x)
{
    return x.cos();
}

/// Unary tangent.
/**
 * @param x the \link mppp::real128 real128\endlink whose tangent will be computed.
 *
 * @return the tangent of \p x.
 */
inline real128 tan(real128 x)
{
    return x.tan();
}

/// Unary inverse sine.
/**
 * @param x the \link mppp::real128 real128\endlink whose inverse sine will be computed.
 *
 * @return the inverse sine of \p x.
 */
inline real128 asin(real128 x)
{
    return x.asin();
}

/// Unary inverse cosine.
/**
 * @param x the \link mppp::real128 real128\endlink whose inverse cosine will be computed.
 *
 * @return the inverse cosine of \p x.
 */
inline real128 acos(real128 x)
{
    return x.acos();
}

/// Unary inverse tangent.
/**
 * @param x the \link mppp::real128 real128\endlink whose inverse tangent will be computed.
 *
 * @return the inverse tangent of \p x.
 */
inline real128 atan(real128 x)
{
    return x.atan();
}

/** @} */

/** @defgroup real128_hyper real128_hyper
 *  @{
 */

/// Unary hyperbolic sine.
/**
 * @param x the \link mppp::real128 real128\endlink whose hyperbolic sine will be computed.
 *
 * @return the hyperbolic sine of \p x.
 */
inline real128 sinh(real128 x)
{
    return x.sinh();
}

/// Unary hyperbolic cosine.
/**
 * @param x the \link mppp::real128 real128\endlink whose hyperbolic cosine will be computed.
 *
 * @return the hyperbolic cosine of \p x.
 */
inline real128 cosh(real128 x)
{
    return x.cosh();
}

/// Unary hyperbolic tangent.
/**
 * @param x the \link mppp::real128 real128\endlink whose hyperbolic tangent will be computed.
 *
 * @return the hyperbolic tangent of \p x.
 */
inline real128 tanh(real128 x)
{
    return x.tanh();
}

/// Unary inverse hyperbolic sine.
/**
 * @param x the \link mppp::real128 real128\endlink whose inverse hyperbolic sine will be computed.
 *
 * @return the inverse hyperbolic sine of \p x.
 */
inline real128 asinh(real128 x)
{
    return x.asinh();
}

/// Unary inverse hyperbolic cosine.
/**
 * @param x the \link mppp::real128 real128\endlink whose inverse hyperbolic cosine will be computed.
 *
 * @return the inverse hyperbolic cosine of \p x.
 */
inline real128 acosh(real128 x)
{
    return x.acosh();
}

/// Unary inverse hyperbolic tangent.
/**
 * @param x the \link mppp::real128 real128\endlink whose inverse hyperbolic tangent will be computed.
 *
 * @return the inverse hyperbolic tangent of \p x.
 */
inline real128 atanh(real128 x)
{
    return x.atanh();
}
/** @} */

/** @defgroup real128_gamma real128_gamma
 *  @{
 */

/// Natural logarithm of the gamma function.
/**
 * @param x the \link mppp::real128 real128\endlink whose lgamma will be computed.
 *
 * @return the lgamma of \p x.
 */
inline real128 lgamma(real128 x)
{
    return x.lgamma();
}

/** @} */

/** @defgroup real128_miscfuncts real128_miscfuncts
 *  @{
 */

/// Error function.
/**
 * @param x the \link mppp::real128 real128\endlink whose erf will be computed.
 *
 * @return the erf of \p x.
 */
inline real128 erf(real128 x)
{
    return x.erf();
}

/** @} */

// Next real128 from 'from' to 'to'.
MPPP_DLL_PUBLIC real128 nextafter(const real128 &, const real128 &);

/** @defgroup real128_operators real128_operators
 *  @{
 */

/// Identity operator.
/**
 * @param x the \link mppp::real128 real128\endlink that will be copied.
 *
 * @return a copy of \p x.
 */
constexpr real128 operator+(real128 x)
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
} // namespace detail

/// Binary addition involving \link mppp::real128 real128\endlink and C++ types.
/**
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \f$ x + y \f$.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128CppOpTypes<T, U>
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
#endif
    constexpr real128 operator+(const T &x, const U &y)
{
    return detail::dispatch_add(x, y);
}

namespace detail
{

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline real128 dispatch_add(const real128 &x, const T &y)
{
    return x + real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline real128 dispatch_add(const T &x, const real128 &y)
{
    return real128{x} + y;
}
} // namespace detail

/// Binary addition involving \link mppp::real128 real128\endlink and mp++ types.
/**
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \f$ x + y \f$.
 *
 * @throws unspecified any exception thrown by the constructor of \link mppp::real128 real128\endlink
 * from the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128MpppOpTypes<T, U>
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
#endif
    inline real128 operator+(const T &x, const U &y)
{
    return detail::dispatch_add(x, y);
}

namespace detail
{

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
} // namespace detail

/// In-place addition involving \link mppp::real128 real128\endlink and C++ types.
/**
 * \rststar
 * .. note::
 *
 *   This operator is marked as ``constexpr`` only if at least C++14 is being used.
 * \endrststar
 *
 * @param x the augend.
 * @param y the addend.
 *
 * @return a reference to \p x, after it has been incremented by \p y.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128CppOpTypes<T, U>
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
#endif
    inline MPPP_CONSTEXPR_14 T &operator+=(T &x, const U &y)
{
    detail::dispatch_in_place_add(x, y);
    return x;
}

namespace detail
{

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

/// In-place addition involving \link mppp::real128 real128\endlink and mp++ types.
/**
 * @param x the augend.
 * @param y the addend.
 *
 * @return a reference to \p x, after it has been incremented by \p y.
 *
 * @throws unspecified any exception thrown by the corresponding binary operator, or by the conversion
 * of \link mppp::real128 real128\endlink to the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128MpppOpTypes<T, U>
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
#endif
    inline T &operator+=(T &x, const U &y)
{
    detail::dispatch_in_place_add(x, y);
    return x;
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
 * @param x the \link mppp::real128 real128\endlink that will be increased.
 *
 * @return a reference to \p x after the increment.
 */
inline MPPP_CONSTEXPR_14 real128 &operator++(real128 &x)
{
    x.m_value += 1;
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
 * @param x the \link mppp::real128 real128\endlink that will be increased.
 *
 * @return a copy of \p x before the increment.
 */
inline MPPP_CONSTEXPR_14 real128 operator++(real128 &x, int)
{
    auto retval(x);
    ++x;
    return retval;
}

/// Negation operator.
/**
 * @param x the \link mppp::real128 real128\endlink whose opposite will be returned.
 *
 * @return \f$ -x \f$.
 */
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
} // namespace detail

/// Binary subtraction involving \link mppp::real128 real128\endlink and C++ types.
/**
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \f$ x - y \f$.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128CppOpTypes<T, U>
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
#endif
    constexpr real128 operator-(const T &x, const U &y)
{
    return detail::dispatch_sub(x, y);
}

namespace detail
{

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline real128 dispatch_sub(const real128 &x, const T &y)
{
    return x - real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline real128 dispatch_sub(const T &x, const real128 &y)
{
    return real128{x} - y;
}
} // namespace detail

/// Binary subtraction involving \link mppp::real128 real128\endlink and mp++ types.
/**
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \f$ x - y \f$.
 *
 * @throws unspecified any exception thrown by the constructor of \link mppp::real128 real128\endlink
 * from the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128MpppOpTypes<T, U>
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
#endif
    inline real128 operator-(const T &x, const U &y)
{
    return detail::dispatch_sub(x, y);
}

namespace detail
{

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
} // namespace detail

/// In-place subtraction involving \link mppp::real128 real128\endlink and C++ types.
/**
 * \rststar
 * .. note::
 *
 *   This operator is marked as ``constexpr`` only if at least C++14 is being used.
 * \endrststar
 *
 * @param x the minuend.
 * @param y the subtrahend.
 *
 * @return a reference to \p x, after it has been decremented by \p y.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128CppOpTypes<T, U>
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
#endif
    inline MPPP_CONSTEXPR_14 T &operator-=(T &x, const U &y)
{
    detail::dispatch_in_place_sub(x, y);
    return x;
}

namespace detail
{

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

/// In-place subtraction involving \link mppp::real128 real128\endlink and mp++ types.
/**
 * @param x the minuend.
 * @param y the subtrahend.
 *
 * @return a reference to \p x, after it has been decremented by \p y.
 *
 * @throws unspecified any exception thrown by the corresponding binary operator, or by the conversion
 * of \link mppp::real128 real128\endlink to the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128MpppOpTypes<T, U>
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
#endif
    inline T &operator-=(T &x, const U &y)
{
    detail::dispatch_in_place_sub(x, y);
    return x;
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
 * @param x the \link mppp::real128 real128\endlink that will be decreased.
 *
 * @return a reference to \p x after the decrement.
 */
inline MPPP_CONSTEXPR_14 real128 &operator--(real128 &x)
{
    x.m_value -= 1;
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
 * @param x the \link mppp::real128 real128\endlink that will be decreased.
 *
 * @return a copy of \p x before the decrement.
 */
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
} // namespace detail

/// Binary multiplication involving \link mppp::real128 real128\endlink and C++ types.
/**
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \f$ x \times y \f$.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128CppOpTypes<T, U>
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
#endif
    constexpr real128 operator*(const T &x, const U &y)
{
    return detail::dispatch_mul(x, y);
}

namespace detail
{

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline real128 dispatch_mul(const real128 &x, const T &y)
{
    return x * real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline real128 dispatch_mul(const T &x, const real128 &y)
{
    return real128{x} * y;
}
} // namespace detail

/// Binary multiplication involving \link mppp::real128 real128\endlink and mp++ types.
/**
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \f$ x \times y \f$.
 *
 * @throws unspecified any exception thrown by the constructor of \link mppp::real128 real128\endlink
 * from the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128MpppOpTypes<T, U>
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
#endif
    inline real128 operator*(const T &x, const U &y)
{
    return detail::dispatch_mul(x, y);
}

namespace detail
{

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
} // namespace detail

/// In-place multiplication involving \link mppp::real128 real128\endlink and C++ types.
/**
 * \rststar
 * .. note::
 *
 *   This operator is marked as ``constexpr`` only if at least C++14 is being used.
 * \endrststar
 *
 * @param x the multiplicand.
 * @param y the multiplicator.
 *
 * @return a reference to \p x, after it has been multiplied by \p y.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128CppOpTypes<T, U>
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
#endif
    inline MPPP_CONSTEXPR_14 T &operator*=(T &x, const U &y)
{
    detail::dispatch_in_place_mul(x, y);
    return x;
}

namespace detail
{

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

/// In-place multiplication involving \link mppp::real128 real128\endlink and mp++ types.
/**
 * @param x the multiplicand.
 * @param y the multiplicator.
 *
 * @return a reference to \p x, after it has been multiplied by \p y.
 *
 * @throws unspecified any exception thrown by the corresponding binary operator, or by the conversion
 * of \link mppp::real128 real128\endlink to the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128MpppOpTypes<T, U>
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
#endif
    inline T &operator*=(T &x, const U &y)
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
} // namespace detail

/// Binary division involving \link mppp::real128 real128\endlink and C++ types.
/**
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \f$ x / y \f$.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128CppOpTypes<T, U>
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
#endif
    constexpr real128 operator/(const T &x, const U &y)
{
    return detail::dispatch_div(x, y);
}

namespace detail
{

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline real128 dispatch_div(const real128 &x, const T &y)
{
    return x / real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline real128 dispatch_div(const T &x, const real128 &y)
{
    return real128{x} / y;
}
} // namespace detail

/// Binary division involving \link mppp::real128 real128\endlink and mp++ types.
/**
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \f$ x / y \f$.
 *
 * @throws unspecified any exception thrown by the constructor of \link mppp::real128 real128\endlink
 * from the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128MpppOpTypes<T, U>
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
#endif
    inline real128 operator/(const T &x, const U &y)
{
    return detail::dispatch_div(x, y);
}

namespace detail
{

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
} // namespace detail

/// In-place division involving \link mppp::real128 real128\endlink and C++ types.
/**
 * \rststar
 * .. note::
 *
 *   This operator is marked as ``constexpr`` only if at least C++14 is being used.
 * \endrststar
 *
 * @param x the dividend.
 * @param y the divisor.
 *
 * @return a reference to \p x, after it has been divided by \p y.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128CppOpTypes<T, U>
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
#endif
    inline MPPP_CONSTEXPR_14 T &operator/=(T &x, const U &y)
{
    detail::dispatch_in_place_div(x, y);
    return x;
}

namespace detail
{

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

/// In-place division involving \link mppp::real128 real128\endlink and mp++ types.
/**
 * @param x the dividend.
 * @param y the divisor.
 *
 * @return a reference to \p x, after it has been divided by \p y.
 *
 * @throws unspecified any exception thrown by the corresponding binary operator, or by the conversion
 * of \link mppp::real128 real128\endlink to the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128MpppOpTypes<T, U>
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
#endif
    inline T &operator/=(T &x, const U &y)
{
    detail::dispatch_in_place_div(x, y);
    return x;
}

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
} // namespace detail

/// Equality operator involving \link mppp::real128 real128\endlink and C++ types.
/**
 * \rststar
 * The implementation uses the comparison operator of the :cpp:type:`__float128` type.
 *
 * .. note::
 *    This operator does not handle NaN in a special way (that is, NaN compares
 *    different from any value, including NaN itself). See :cpp:func:`mppp::real128_equal_to()`
 *    for an equality predicate that handles NaN specially.
 * \endrststar
 *
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \p true if \f$ x = y \f$, \p false otherwise.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128CppOpTypes<T, U>
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
#endif
    constexpr bool operator==(const T &x, const U &y)
{
    return detail::dispatch_eq(x, y);
}

namespace detail
{

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline bool dispatch_eq(const real128 &x, const T &y)
{
    return x == real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline bool dispatch_eq(const T &x, const real128 &y)
{
    return real128{x} == y;
}
} // namespace detail

/// Equality operator involving \link mppp::real128 real128\endlink and mp++ types.
/**
 * \rststar
 * The implementation promotes the non-:cpp:class:`~mppp::real128` argument to :cpp:class:`~mppp::real128`,
 * and then uses the equality operator of :cpp:class:`~mppp::real128`.
 *
 * .. note::
 *    This operator does not handle NaN in a special way (that is, NaN compares
 *    different from any value, including NaN itself). See :cpp:func:`mppp::real128_equal_to()`
 *    for an equality predicate that handles NaN specially.
 * \endrststar
 *
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \p true if \f$ x = y \f$, \p false otherwise.
 *
 * @throws unspecified any exception thrown by the constructor of \link mppp::real128 real128\endlink
 * from the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128MpppOpTypes<T, U>
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
#endif
    inline bool operator==(const T &x, const U &y)
{
    return detail::dispatch_eq(x, y);
}

/// Inequality operator involving \link mppp::real128 real128\endlink and C++ types.
/**
 * \rststar
 * The implementation uses the comparison operator of the :cpp:type:`__float128` type.
 *
 * .. note::
 *    This operator does not handle NaN in a special way (that is, NaN compares
 *    different from any value, including NaN itself). See :cpp:func:`mppp::real128_equal_to()`
 *    for an equality predicate that handles NaN specially.
 * \endrststar
 *
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \p true if \f$ x = y \f$, \p false otherwise.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128CppOpTypes<T, U>
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
#endif
    constexpr bool operator!=(const T &x, const U &y)
{
    return !(x == y);
}

/// Inequality operator involving \link mppp::real128 real128\endlink and mp++ types.
/**
 * \rststar
 * The implementation promotes the non-:cpp:class:`~mppp::real128` argument to :cpp:class:`~mppp::real128`,
 * and then uses the equality operator of :cpp:class:`~mppp::real128`.
 *
 * .. note::
 *    This operator does not handle NaN in a special way (that is, NaN compares
 *    different from any value, including NaN itself). See :cpp:func:`mppp::real128_equal_to()`
 *    for an equality predicate that handles NaN specially.
 * \endrststar
 *
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \p true if \f$ x = y \f$, \p false otherwise.
 *
 * @throws unspecified any exception thrown by the constructor of \link mppp::real128 real128\endlink
 * from the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128MpppOpTypes<T, U>
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
#endif
    inline bool operator!=(const T &x, const U &y)
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
} // namespace detail

/// Less-than operator involving \link mppp::real128 real128\endlink and C++ types.
/**
 * \rststar
 * The implementation uses the less-than operator of the :cpp:type:`__float128` type.
 *
 * .. note::
 *    This operator does not handle NaN in a special way (that is, NaN is never
 *    less than any value, and no value is less than NaN).
 *    See :cpp:func:`mppp::real128_lt()` for a less-than predicate that handles
 *    NaN specially.
 * \endrststar
 *
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \p true if \f$ x < y \f$, \p false otherwise.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128CppOpTypes<T, U>
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
#endif
    constexpr bool operator<(const T &x, const U &y)
{
    return detail::dispatch_lt(x, y);
}

namespace detail
{
template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline bool dispatch_lt(const real128 &x, const T &y)
{
    return x < real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline bool dispatch_lt(const T &x, const real128 &y)
{
    return real128{x} < y;
}
} // namespace detail

/// Less-than operator involving \link mppp::real128 real128\endlink and mp++ types.
/**
 * \rststar
 * The implementation promotes the non-:cpp:class:`~mppp::real128` argument to :cpp:class:`~mppp::real128`,
 * and then uses the less-than operator of :cpp:class:`~mppp::real128`.
 *
 * .. note::
 *    This operator does not handle NaN in a special way (that is, NaN is never
 *    less than any value, and no value is less than NaN).
 *    See :cpp:func:`mppp::real128_lt()` for a less-than predicate that handles
 *    NaN specially.
 * \endrststar
 *
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \p true if \f$ x < y \f$, \p false otherwise.
 *
 * @throws unspecified any exception thrown by the constructor of \link mppp::real128 real128\endlink
 * from the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128MpppOpTypes<T, U>
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
#endif
    inline bool operator<(const T &x, const U &y)
{
    return detail::dispatch_lt(x, y);
}

namespace detail
{

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
} // namespace detail

/// Less-than or equal operator involving \link mppp::real128 real128\endlink and C++ types.
/**
 * \rststar
 * The implementation uses the less-than or equal operator of the :cpp:type:`__float128` type.
 *
 * .. note::
 *    This operator does not handle NaN in a special way (that is, NaN is never
 *    less than or equal to any value, and no value is less than or equal to NaN).
 * \endrststar
 *
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \p true if \f$ x \leq y \f$, \p false otherwise.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128CppOpTypes<T, U>
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
#endif
    constexpr bool operator<=(const T &x, const U &y)
{
    return detail::dispatch_lte(x, y);
}

namespace detail
{
template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline bool dispatch_lte(const real128 &x, const T &y)
{
    return x <= real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline bool dispatch_lte(const T &x, const real128 &y)
{
    return real128{x} <= y;
}
} // namespace detail

/// Less-than or equal operator involving \link mppp::real128 real128\endlink and mp++ types.
/**
 * \rststar
 * The implementation promotes the non-:cpp:class:`~mppp::real128` argument to :cpp:class:`~mppp::real128`,
 * and then uses the less-than or equal operator of :cpp:class:`~mppp::real128`.
 *
 * .. note::
 *    This operator does not handle NaN in a special way (that is, NaN is never
 *    less than or equal to any value, and no value is less than or equal to NaN).
 * \endrststar
 *
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \p true if \f$ x \leq y \f$, \p false otherwise.
 *
 * @throws unspecified any exception thrown by the constructor of \link mppp::real128 real128\endlink
 * from the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128MpppOpTypes<T, U>
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
#endif
    inline bool operator<=(const T &x, const U &y)
{
    return detail::dispatch_lte(x, y);
}

namespace detail
{

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
} // namespace detail

/// Greater-than operator involving \link mppp::real128 real128\endlink and C++ types.
/**
 * \rststar
 * The implementation uses the greater-than operator of the :cpp:type:`__float128` type.
 *
 * .. note::
 *    This operator does not handle NaN in a special way (that is, NaN is never
 *    greater than any value, and no value is greater than NaN).
 *    See :cpp:func:`mppp::real128_gt()` for a greater-than predicate that handles
 *    NaN specially.
 * \endrststar
 *
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \p true if \f$ x > y \f$, \p false otherwise.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128CppOpTypes<T, U>
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
#endif
    constexpr bool operator>(const T &x, const U &y)
{
    return detail::dispatch_gt(x, y);
}

namespace detail
{
template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline bool dispatch_gt(const real128 &x, const T &y)
{
    return x > real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline bool dispatch_gt(const T &x, const real128 &y)
{
    return real128{x} > y;
}
} // namespace detail

/// Greater-than operator involving \link mppp::real128 real128\endlink and mp++ types.
/**
 * \rststar
 * The implementation promotes the non-:cpp:class:`~mppp::real128` argument to :cpp:class:`~mppp::real128`,
 * and then uses the greater-than operator of :cpp:class:`~mppp::real128`.
 *
 * .. note::
 *    This operator does not handle NaN in a special way (that is, NaN is never
 *    greater than any value, and no value is greater than NaN).
 *    See :cpp:func:`mppp::real128_gt()` for a greater-than predicate that handles
 *    NaN specially.
 * \endrststar
 *
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \p true if \f$ x > y \f$, \p false otherwise.
 *
 * @throws unspecified any exception thrown by the constructor of \link mppp::real128 real128\endlink
 * from the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128MpppOpTypes<T, U>
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
#endif
    inline bool operator>(const T &x, const U &y)
{
    return detail::dispatch_gt(x, y);
}

namespace detail
{

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
} // namespace detail

/// Greater-than or equal operator involving \link mppp::real128 real128\endlink and C++ types.
/**
 * \rststar
 * The implementation uses the greater-than or equal operator of the :cpp:type:`__float128` type.
 *
 * .. note::
 *    This operator does not handle NaN in a special way (that is, NaN is never
 *    greater than or equal to any value, and no value is greater than or equal to NaN).
 * \endrststar
 *
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \p true if \f$ x \geq y \f$, \p false otherwise.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128CppOpTypes<T, U>
#else
template <typename T, typename U, real128_cpp_op_types_enabler<T, U> = 0>
#endif
    constexpr bool operator>=(const T &x, const U &y)
{
    return detail::dispatch_gte(x, y);
}

namespace detail
{
template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline bool dispatch_gte(const real128 &x, const T &y)
{
    return x >= real128{y};
}

template <typename T, enable_if_t<is_real128_mppp_interoperable<T>::value, int> = 0>
inline bool dispatch_gte(const T &x, const real128 &y)
{
    return real128{x} >= y;
}
} // namespace detail

/// Greater-than or equal operator involving \link mppp::real128 real128\endlink and mp++ types.
/**
 * \rststar
 * The implementation promotes the non-:cpp:class:`~mppp::real128` argument to :cpp:class:`~mppp::real128`,
 * and then uses the greater-than or equal operator of :cpp:class:`~mppp::real128`.
 *
 * .. note::
 *    This operator does not handle NaN in a special way (that is, NaN is never
 *    greater than or equal to any value, and no value is greater than or equal to NaN).
 * \endrststar
 *
 * @param x the first operand.
 * @param y the second operand.
 *
 * @return \p true if \f$ x \geq y \f$, \p false otherwise.
 *
 * @throws unspecified any exception thrown by the constructor of \link mppp::real128 real128\endlink
 * from the mp++ type.
 */
#if defined(MPPP_HAVE_CONCEPTS)
template <typename T, typename U>
requires Real128MpppOpTypes<T, U>
#else
template <typename T, typename U, real128_mppp_op_types_enabler<T, U> = 0>
#endif
    inline bool operator>=(const T &x, const U &y)
{
    return detail::dispatch_gte(x, y);
}

/** @} */

namespace detail
{

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

/** @defgroup real128_constants real128_constants
 *  @{
 */

/// The number of binary digits in the significand of a \link mppp::real128 real128\endlink.
/**
 * @return the integral constant 113.
 */
constexpr unsigned real128_sig_digits()
{
    return 113u;
}

/// The largest positive finite value representable by \link mppp::real128 real128\endlink.
/**
 * @return approximately \f$ 1.18973 \times 10^{4932}\f$.
 */
constexpr real128 real128_max()
{
    return (18446744073709551615ull * detail::two_112() + 281474976710655ull * detail::two_48() + 1)
           * detail::two_ptwo<8192>() * detail::two_ptwo<4096>() * detail::two_ptwo<2048>() * detail::two_ptwo<1024>()
           * detail::two_ptwo<512>() * detail::two_ptwo<256>() * detail::two_ptwo<128>() * detail::two_ptwo<64>()
           * detail::two_ptwo<32>() * (1ull << 31);
}

/// The smallest positive value representable by \link mppp::real128 real128\endlink with full precision.
/**
 * @return approximately \f$ 3.3621 \times 10^{-4932}\f$.
 */
constexpr real128 real128_min()
{
    return 1 / detail::two_ptwo<8192>() / detail::two_ptwo<4096>() / detail::two_ptwo<2048>() / detail::two_ptwo<1024>()
           / detail::two_ptwo<512>() / detail::two_ptwo<256>() / detail::two_ptwo<128>() / detail::two_ptwo<64>()
           / detail::two_ptwo<32>() / (1ull << 30);
}

/// The difference between 1 and the next larger number representable by \link mppp::real128 real128\endlink.
/**
 * @return \f$ 2^{-112}\f$.
 */
constexpr real128 real128_epsilon()
{
    return 1 / detail::two_ptwo<64>() / detail::two_ptwo<32>() / (1ull << 16);
}

/// The smallest positive denormalized number representable by \link mppp::real128 real128\endlink.
/**
 * @return \f$ 2^{-16494}\f$.
 */
constexpr real128 real128_denorm_min()
{
    return 1 / detail::two_ptwo<8192>() / detail::two_ptwo<8192>() / detail::two_ptwo<64>() / (1ull << 46);
}

/// The positive \f$ \infty \f$ constant.
/**
 * @return \f$ +\infty \f$.
 */
constexpr real128 real128_inf()
{
#if __GNUC__ < 7
    // NOTE: it seems like there's no way to arithmetically construct infinity in constexpr.
    // I tried 1/0 and repeated multiplications by a large int, but it always ends up in
    // a 'not a constant expression' error message.
    return real128{std::numeric_limits<double>::infinity()};
#else
    // This builtin is constexpr only in GCC 7 and later.
    // https://gcc.gnu.org/onlinedocs/gcc/x86-Built-in-Functions.html
    // Note that this and the nan builtins are arch-specific, but it seems they
    // might be available everywhere __float128 is available.
    return real128{__builtin_infq()};
#endif
}

/// NaN constant.
/**
 * @return a quiet NaN value with unspecified sign bit.
 */
constexpr real128 real128_nan()
{
#if __GNUC__ < 7
    // Same as above - GCC would accept arithmetic generation of NaN,
    // but Clang does not.
    return real128{std::numeric_limits<double>::quiet_NaN()};
#else
    // This builtin is constexpr only in GCC 7 and later.
    return real128{__builtin_nanq("")};
#endif
}

/// The \f$ \pi \f$ constant.
/**
 * @return the quadruple-precision value of \f$ \pi \f$.
 */
constexpr real128 real128_pi()
{
    return 2 * (9541308523256152504ull * detail::two_112() + 160664882791121ull * detail::two_48() + 1);
}

/// The \f$ \mathrm{e} \f$ constant (Euler's number).
/**
 * @return the quadruple-precision value of \f$ \mathrm{e} \f$.
 */
constexpr real128 real128_e()
{
    return 2 * (10751604932185443962ull * detail::two_112() + 101089180468598ull * detail::two_48() + 1);
}

/// The \f$ \sqrt{2} \f$ constant.
/**
 * @return the quadruple-precision value of \f$ \sqrt{2} \f$.
 */
constexpr real128 real128_sqrt2()
{
    return 14486024992869247637ull * detail::two_112() + 116590752822204ull * detail::two_48() + 1;
}

#if MPPP_CPLUSPLUS >= 201703L

// NOTE: namespace scope constexpr variables are *not* implicitly inline, so we need
// inline explicitly here:
// http://en.cppreference.com/w/cpp/language/inline
// Note that constexpr static member variables are implicitly inline instead.

/// The number of binary digits in the significand of a \link mppp::real128 real128\endlink (113).
inline constexpr unsigned sig_digits_128 = real128_sig_digits();

/// The largest positive finite value representable by \link mppp::real128 real128\endlink.
inline constexpr real128 max_128 = real128_max();

/// The smallest positive value representable by \link mppp::real128 real128\endlink with full precision.
inline constexpr real128 min_128 = real128_min();

/// The difference between 1 and the next larger representable number by \link mppp::real128 real128\endlink.
inline constexpr real128 epsilon_128 = real128_epsilon();

/// The smallest positive denormalized number representable by \link mppp::real128 real128\endlink.
inline constexpr real128 denorm_min_128 = real128_denorm_min();

/// Quadruple-precision \f$ +\infty \f$ constant.
inline constexpr real128 inf_128 = real128_inf();

/// Quadruple-precision quiet NaN constant.
inline constexpr real128 nan_128 = real128_nan();

/// Quadruple-precision \f$ \pi \f$ constant.
inline constexpr real128 pi_128 = real128_pi();

/// Quadruple-precision \f$ \mathrm{e} \f$ constant (Euler's number).
inline constexpr real128 e_128 = real128_e();

/// Quadruple-precision \f$ \sqrt{2} \f$ constant.
inline constexpr real128 sqrt2_128 = real128_sqrt2();

#endif

/** @} */

// Hash.
inline std::size_t hash(const real128 &x)
{
    // NOTE: in order to detect if x is zero/nan, resort to reading directly into the ieee fields.
    // This avoids calling the fpclassify() function, which internally invokes a compiler library function.
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
    retval = (retval & static_cast<unsigned long long>(-!is_zero)) | static_cast<unsigned long long>(-is_nan);
    return static_cast<std::size_t>(retval);
}

// NOTE: put these definitions here, as we need the comparison operators to be available.
constexpr bool real128_equal_to(const real128 &x, const real128 &y)
{
    return (!x.isnan() && !y.isnan()) ? (x == y) : (x.isnan() && y.isnan());
}

constexpr bool real128_lt(const real128 &x, const real128 &y)
{
    // NOTE: in case at least one op is NaN, we have the following possibilities:
    // - NaN vs NaN -> false,
    // - NaN vs not-NaN -> false,
    // - not-NaN vs NaN -> true.
    return (!x.isnan() && !y.isnan()) ? (x < y) : !x.isnan();
}

constexpr bool real128_gt(const real128 &x, const real128 &y)
{
    // NOTE: in case at least one op is NaN, we have the following possibilities:
    // - NaN vs NaN -> false,
    // - NaN vs not-NaN -> true,
    // - not-NaN vs NaN -> false.
    return (!x.isnan() && !y.isnan()) ? (x > y) : !y.isnan();
}
} // namespace mppp

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
