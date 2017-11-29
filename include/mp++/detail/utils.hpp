// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_UTILS_HPP
#define MPPP_DETAIL_UTILS_HPP

#include <mp++/config.hpp>

#if MPPP_CPLUSPLUS < 201402L
#include <algorithm>
#endif
#include <cassert>
#include <cstddef>
#if MPPP_CPLUSPLUS >= 201402L
#include <iterator>
#endif
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include <mp++/detail/type_traits.hpp>

namespace mppp
{

inline namespace detail
{

#if defined(_MSC_VER)

// Disable some warnings for MSVC.
#pragma warning(push)
#pragma warning(disable : 4146)

#endif

// These are overloads useful to treat in a generic way mppp classes and standard numeric types.

// Sign function for integral types.
template <typename T, enable_if_t<is_integral<T>::value, int> = 0>
constexpr int sgn(const T &n)
{
    return n ? (n > T(0) ? 1 : -1) : 0;
}

// Zero detection for integral types.
template <typename T, enable_if_t<is_integral<T>::value, int> = 0>
constexpr bool is_zero(const T &n)
{
    return n == T(0);
}

// Generic string conversion utility - will use std::to_string() for arithmetic types,
// x.to_string() otherwise.
template <typename T, enable_if_t<std::is_arithmetic<T>::value, int> = 0>
inline std::string to_string(const T &x)
{
    return std::to_string(x);
}

template <typename T, enable_if_t<!std::is_arithmetic<T>::value, int> = 0>
inline std::string to_string(const T &x)
{
    return x.to_string();
}

// Compute the absolute value of a negative integer, returning the result as an instance
// of the corresponding unsigned type. Requires T to be a signed integral type and n
// to be negative.
// NOTE: here we are using cast to unsigned + unary minus to extract the abs value of a signed negative
// integral. See:
// http://stackoverflow.com/questions/4536095/unary-minus-and-signed-to-unsigned-conversion
// This technique is not 100% portable: it requires an implementation
// of signed integers such that the absolute value of the minimum (negative) value is not greater than
// the maximum value of the unsigned counterpart. This is guaranteed on all computer architectures in use today,
// but in theory there could be architectures where the assumption is not respected. See for instance the
// discussion here:
// http://stackoverflow.com/questions/11372044/unsigned-vs-signed-range-guarantees
// Note that in any case we never run into UB, the only consequence is that for very large negative values
// we could init the integer with the wrong value, and we should be able to detect this in the unit tests.
// Let's keep this in mind in the remote case this ever becomes a problem.
template <typename T>
constexpr make_unsigned_t<T> nint_abs(T n)
{
// NOTE: we should assert about negative n, but this is guaranteed to work properly only
// from C++17:
// https://stackoverflow.com/questions/26072709/alternative-to-asserts-for-constexpr-functions
#if MPPP_CPLUSPLUS >= 201703L
    assert(n < T(0));
#endif
    static_assert(is_integral<T>::value && is_signed<T>::value,
                  "The sint_abs() function can be used only with signed integral types.");
    using uT = make_unsigned_t<T>;
    // NOTE: the potential cast to "unsigned", rather than uT, is for when uT is a short integral type.
    // In such a case, the unary minus will trigger integral promotion to int/unsigned
    // int, and I am *not* 100% sure in this case the technique still works. Written like this, the cast
    // is never to a type narrower than "unsigned".
    return static_cast<uT>(
        -static_cast<typename std::conditional<(nl_max<uT>() <= nl_max<unsigned>()), unsigned, uT>::type>(n));
}

// constexpr max/min implementations with copy semantics.
template <typename T>
constexpr T c_max(T a, T b)
{
    return a > b ? a : b;
}

template <typename T>
constexpr T c_min(T a, T b)
{
    return a < b ? a : b;
}

// A small helper to convert the input unsigned n to -n, represented as the signed T.
template <typename T, typename U>
// NOTE: C++17 because we are using assert().
#if MPPP_CPLUSPLUS >= 201703L
constexpr
#else
inline
#endif
    std::pair<bool, T>
    unsigned_to_nsigned(U n)
{
    static_assert(is_integral<T>::value && is_signed<T>::value, "Invalid type.");
    static_assert(is_integral<U>::value && is_unsigned<U>::value, "Invalid type.");
    // Cache a couple of quantities.
    constexpr auto Tmax = static_cast<make_unsigned_t<T>>(nl_max<T>());
    constexpr auto Tmin_abs = nint_abs(nl_min<T>());
    if (mppp_likely(n <= c_min(Tmax, Tmin_abs))) {
        // Optimise the case in which n fits both Tmax and Tmin_abs. This means
        // we can convert and negate safely.
        return std::make_pair(true, static_cast<T>(-static_cast<T>(n)));
    }
    // n needs to fit within the abs of min().
    if (n > Tmin_abs) {
        return std::make_pair(false, T(0));
    }
    // LCOV_EXCL_START
    if (Tmin_abs <= Tmax) {
        // The negative range of T is leq than the positive one: we can convert to T and negate safely.
        // NOTE: this is never hit on current architectures.
        return std::make_pair(true, static_cast<T>(-static_cast<T>(n)));
    }
    // LCOV_EXCL_STOP
    // NOTE: double check this, since:
    // - Tmin_abs > Tmax (as checked just above),
    // - n > c_min(Tmax, Tmin_abs) (as checked earlier).
    assert(n > Tmax);
    // The negative range is greater than the positive one and n larger than Tmax:
    // we cannot directly convert n to T. The idea then is to init retval to -Tmax
    // and then to subtract from it Tmax as many times as needed.
    auto retval = static_cast<T>(-static_cast<T>(Tmax));
    const auto q = static_cast<make_unsigned_t<T>>(n / Tmax), r = static_cast<make_unsigned_t<T>>(n % Tmax);
    for (make_unsigned_t<T> i = 0; i < q - 1u; ++i) {
        // LCOV_EXCL_START
        // NOTE: this is never hit on current archs, as Tmax differs from Tmin_abs
        // by just 1: we will use only the remainder r.
        retval = static_cast<T>(retval - static_cast<T>(Tmax));
        // LCOV_EXCL_STOP
    }
    retval = static_cast<T>(retval - static_cast<T>(r));
    return std::make_pair(true, retval);
}

// Like above, but throw on failure.
template <typename T, typename U>
#if MPPP_CPLUSPLUS >= 201703L
constexpr
#else
inline
#endif
    T
    negate_unsigned(U n)
{
    const auto retval = unsigned_to_nsigned<T>(n);
    return retval.first ? retval.second
                        : throw std::overflow_error(
                              "Error while trying to negate the unsigned integral value " + std::to_string(n)
                              + ": the result does not fit in the range of the target type " + typeid(T).name());
}

// Safe casting functionality between integral types. It will throw if the conversion overflows the range
// of the target type T.
template <typename T, typename U,
          enable_if_t<conjunction<is_integral<T>, is_integral<U>, is_unsigned<T>, is_unsigned<U>>::value, int> = 0>
constexpr T safe_cast(const U &n)
{
    return n <= nl_max<T>()
               ? static_cast<T>(n)
               : throw std::overflow_error(
                     "Error in the safe conversion between unsigned integral types: the input value "
                     + std::to_string(n) + " does not fit in the range of the target type " + typeid(T).name());
}

template <typename T, typename U,
          enable_if_t<conjunction<is_integral<T>, is_integral<U>, is_signed<T>, is_signed<U>>::value, int> = 0>
constexpr T safe_cast(const U &n)
{
    return (n <= nl_max<T>() && n >= nl_min<T>())
               ? static_cast<T>(n)
               : throw std::overflow_error(
                     "Error in the safe conversion between signed integral types: the input value " + std::to_string(n)
                     + " does not fit in the range of the target type " + typeid(T).name());
}

template <typename T, typename U,
          enable_if_t<conjunction<is_integral<T>, is_integral<U>, is_unsigned<T>, is_signed<U>>::value, int> = 0>
constexpr T safe_cast(const U &n)
{
    return (n >= U(0) && static_cast<make_unsigned_t<U>>(n) <= nl_max<T>())
               ? static_cast<T>(n)
               : throw std::overflow_error("Error in the safe conversion from a signed integral type to an unsigned "
                                           "integral type: the input value "
                                           + std::to_string(n) + " does not fit in the range of the target type "
                                           + typeid(T).name());
}

template <typename T, typename U,
          enable_if_t<conjunction<is_integral<T>, is_integral<U>, is_signed<T>, is_unsigned<U>>::value, int> = 0>
constexpr T safe_cast(const U &n)
{
    return n <= static_cast<make_unsigned_t<T>>(nl_max<T>())
               ? static_cast<T>(n)
               : throw std::overflow_error("Error in the safe conversion from an unsigned integral type to a signed "
                                           "integral type: the input value "
                                           + std::to_string(n) + " does not fit in the range of the target type "
                                           + typeid(T).name());
}

#if defined(MPPP_HAVE_GCC_INT128)

// Implementation of to_string() for 128bit integers.
template <std::size_t N>
inline char *to_string_impl(char (&output)[N], __uint128_t n)
{
    // Max 128 uint value needs 39 digits in base 10, plus the terminator.
    static_assert(N >= 40u,
                  "An array of at least 40 characters is needed to convert a 128 bit unsigned integer to string.");
    // Sequence of text representations of integers from 0 to 99 (2 digits per number).
    constexpr char d2_text[] = "000102030405060708091011121314151617181920212223242526272829303132333435363738394041424"
                               "344454647484950515253545556575859606162636465666768697071727374757677787980818283848586"
                               "87888990919293949596979899";
    static_assert(sizeof(d2_text) == 201u, "Invalid size.");
    // Place the terminator.
    std::size_t idx = 0;
    output[idx++] = '\0';
    // Reduce n iteratively by a factor of 100, and print the remainder at each iteration.
    auto r = static_cast<unsigned>(n % 100u);
    for (; n >= 100u; n = n / 100u, r = static_cast<unsigned>(n % 100u)) {
        output[idx++] = d2_text[r * 2u + 1u];
        output[idx++] = d2_text[r * 2u];
    }
    // Write the last two digits, skipping the second one if the current
    // remainder is not at least 10.
    output[idx++] = d2_text[r * 2u + 1u];
    if (r >= 10u) {
        output[idx++] = d2_text[r * 2u];
    }
    assert(idx <= 40u);
    return output + idx;
}

inline std::string to_string(__uint128_t n)
{
    char output[40];
    auto o = to_string_impl(output, n);
#if MPPP_CPLUSPLUS >= 201402L
    // Now build the string by reading backwards. When reverse iterators are created,
    // the original iterator is decreased by one. Hence, we can build the begin directly
    // from o (which points 1 past the last written char), and the end from output + 1
    // (so that it will point to the terminator).
    return std::string(std::make_reverse_iterator(o), std::make_reverse_iterator(output + 1));
#else
    // In C++11, we reverse output and then create the string.
    std::reverse(output, o);
    // NOTE: decrease by one as we don't want to init the string
    // with a terminator.
    return std::string(output, o - 1);
#endif
}

inline std::string to_string(__int128_t n)
{
    char output[41];
    const bool neg = n < 0;
    auto o = to_string_impl(output, neg ? nint_abs(n) : static_cast<__uint128_t>(n));
    // Add the sign, if needed.
    if (neg) {
        *(o++) = '-';
    }
#if MPPP_CPLUSPLUS >= 201402L
    return std::string(std::make_reverse_iterator(o), std::make_reverse_iterator(output + 1));
#else
    std::reverse(output, o);
    return std::string(output, o - 1);
#endif
}

#endif

#if defined(_MSC_VER)

#pragma warning(pop)

#endif
}
}

#endif
