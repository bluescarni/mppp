// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#if MPPP_CPLUSPLUS < 201402L
#include <algorithm>
#endif
#include <cassert>
#include <cstddef>
#if MPPP_CPLUSPLUS >= 201402L
#include <iterator>
#endif
#include <string>

#include <mp++/detail/utils.hpp>

// NOLINTNEXTLINE(modernize-concat-nested-namespaces)
namespace mppp
{

namespace detail
{

#if defined(MPPP_HAVE_GCC_INT128)

namespace
{

// Implementation of to_string() for 128bit integers.
template <std::size_t N>
char *to_string_impl(char (&output)[N], __uint128_t n)
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

} // namespace

std::string to_string(__uint128_t n)
{
    char output[40];
    // NOLINTNEXTLINE(llvm-qualified-auto, readability-qualified-auto)
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
    return std::string(output);
#endif
}

std::string to_string(__int128_t n)
{
    char output[41];
    const bool neg = n < 0;
    // NOLINTNEXTLINE(llvm-qualified-auto, readability-qualified-auto)
    auto o = to_string_impl(output, neg ? nint_abs(n) : static_cast<__uint128_t>(n));
    // Add the sign, if needed.
    if (neg) {
        *(o++) = '-';
    }
#if MPPP_CPLUSPLUS >= 201402L
    return std::string(std::make_reverse_iterator(o), std::make_reverse_iterator(output + 1));
#else
    std::reverse(output, o);
    return std::string(output);
#endif
}

#endif

} // namespace detail

} // namespace mppp
