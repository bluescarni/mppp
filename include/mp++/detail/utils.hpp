// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_UTILS_HPP
#define MPPP_DETAIL_UTILS_HPP

#include <cassert>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <mp++/config.hpp>
#include <mp++/detail/type_traits.hpp>
#include <mp++/detail/visibility.hpp>
#include <mp++/type_name.hpp>

namespace mppp
{

namespace detail
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

// Small helper to convert the non-negative signed integer n
// into its unsigned counterpart.
template <typename T>
constexpr make_unsigned_t<T> make_unsigned(T n) noexcept
{
    static_assert(is_integral<T>::value && is_signed<T>::value, "Invalid type.");
#if MPPP_CPLUSPLUS >= 201703L
    // LCOV_EXCL_START
    assert(n >= T(0));
    // LCOV_EXCL_STOP
#endif
    return static_cast<make_unsigned_t<T>>(n);
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

#if defined(MPPP_HAVE_GCC_INT128)

MPPP_DLL_PUBLIC std::string to_string(__uint128_t);
MPPP_DLL_PUBLIC std::string to_string(__int128_t);

#endif

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
constexpr make_unsigned_t<T> nint_abs(T n) noexcept
{
    // NOTE: we should assert about negative n, but this is guaranteed to work properly only
    // from C++17:
    // https://stackoverflow.com/questions/26072709/alternative-to-asserts-for-constexpr-functions
#if MPPP_CPLUSPLUS >= 201703L
    // LCOV_EXCL_START
    assert(n < T(0));
    // LCOV_EXCL_STOP
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
constexpr T c_max(T a, T b) noexcept
{
    return a > b ? a : b;
}

template <typename T>
constexpr T c_min(T a, T b) noexcept
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
    constexpr auto Tmax = make_unsigned(nl_max<T>());
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
    const auto q = static_cast<U>(n / Tmax), r = static_cast<U>(n % Tmax);
    for (U i = 0; i < q - 1u; ++i) {
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
    // LCOV_EXCL_START
    return retval.first ? retval.second
                        : throw std::overflow_error(
                            "Error while trying to negate the unsigned integral value " + to_string(n)
                            + ": the result does not fit in the range of the target type '" + type_name<T>() + "'");
    // LCOV_EXCL_STOP
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
                   "Error in the safe conversion between unsigned integral types: the input value " + to_string(n)
                   + " does not fit in the range of the target type '" + type_name<T>() + "'");
}

template <typename T, typename U,
          enable_if_t<conjunction<is_integral<T>, is_integral<U>, is_signed<T>, is_signed<U>>::value, int> = 0>
constexpr T safe_cast(const U &n)
{
    return (n <= nl_max<T>() && n >= nl_min<T>())
               ? static_cast<T>(n)
               : throw std::overflow_error(
                   "Error in the safe conversion between signed integral types: the input value " + to_string(n)
                   + " does not fit in the range of the target type '" + type_name<T>() + "'");
}

template <typename T, typename U,
          enable_if_t<conjunction<is_integral<T>, is_integral<U>, is_unsigned<T>, is_signed<U>>::value, int> = 0>
constexpr T safe_cast(const U &n)
{
    return (n >= U(0) && make_unsigned(n) <= nl_max<T>())
               ? static_cast<T>(n)
               : throw std::overflow_error("Error in the safe conversion from a signed integral type to an unsigned "
                                           "integral type: the input value "
                                           + to_string(n) + " does not fit in the range of the target type '"
                                           + type_name<T>() + "'");
}

template <typename T, typename U,
          enable_if_t<conjunction<is_integral<T>, is_integral<U>, is_signed<T>, is_unsigned<U>>::value, int> = 0>
constexpr T safe_cast(const U &n)
{
    return n <= make_unsigned(nl_max<T>())
               ? static_cast<T>(n)
               : throw std::overflow_error("Error in the safe conversion from an unsigned integral type to a signed "
                                           "integral type: the input value "
                                           + to_string(n) + " does not fit in the range of the target type '"
                                           + type_name<T>() + "'");
}

// Helper to ignore unused variables.
// NOTE: the return type has to be int, rather than void, for compatibility
// with C++11 constexpr.
template <typename... Args>
constexpr int ignore(Args &&...)
{
    return 0;
}

#if defined(_MSC_VER)

#pragma warning(pop)

#endif
} // namespace detail
} // namespace mppp

#endif
