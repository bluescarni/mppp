// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_INTEGER_LITERALS_HPP
#define MPPP_DETAIL_INTEGER_LITERALS_HPP

#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <mp++/config.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/utils.hpp>

namespace mppp
{

namespace detail
{

// Helper to check if an integral literal is valid.
// It will return the base of the literal. Supported
// bases are 2, 8, 10 and 16.
// NOTE: we need this function in the C++11/C++14 implementation
// as well, so use MPPP_CONSTEXPR_14, MPPP_FALLTHROUGH, etc.
template <std::size_t Size>
inline
#if !defined(_MSC_VER) || defined(__clang__)
    MPPP_CONSTEXPR_14
#endif
    int
    integer_literal_check_str(const char (&arr)[Size])
{
    // We expect the size of the array to be
    // always at least 2, the null terminator
    // plus at least one char from the literal.
    static_assert(Size >= 2u, "Invalid array size.");
#if MPPP_CPLUSPLUS >= 201703L
    assert(arr[Size - 1u] == '\0');
#endif

    // The actual string length of the literal.
    constexpr auto str_length = Size - 1u;

    // Helpers to check a digit in various bases.
    auto base10_digit_check = [&arr](char c) {
        if (c < '0' || c > '9') {
            throw std::invalid_argument("Invalid integral literal: " + std::string(arr));
        }
    };

    auto base8_digit_check = [&arr](char c) {
        if (c < '0' || c > '7') {
            throw std::invalid_argument("Invalid integral literal: " + std::string(arr));
        }
    };

    auto base2_digit_check = [&arr](char c) {
        if (c != '0' && c != '1') {
            throw std::invalid_argument("Invalid integral literal: " + std::string(arr));
        }
    };

    auto base16_digit_check = [&arr](char c) {
        if (c < '0' || c > '9') {
            switch (c) {
                case 'a':
                    MPPP_FALLTHROUGH;
                case 'b':
                    MPPP_FALLTHROUGH;
                case 'c':
                    MPPP_FALLTHROUGH;
                case 'd':
                    MPPP_FALLTHROUGH;
                case 'e':
                    MPPP_FALLTHROUGH;
                case 'f':
                    MPPP_FALLTHROUGH;
                case 'A':
                    MPPP_FALLTHROUGH;
                case 'B':
                    MPPP_FALLTHROUGH;
                case 'C':
                    MPPP_FALLTHROUGH;
                case 'D':
                    MPPP_FALLTHROUGH;
                case 'E':
                    MPPP_FALLTHROUGH;
                case 'F':
                    return;
                default:
                    throw std::invalid_argument("Invalid integral literal: " + std::string(arr));
            }
        }
    };

    if (str_length == 1u) {
        // If the string length is 1, the only
        // character must be in the ['0', '9'] range.
        base10_digit_check(arr[0]);

        // NOTE: the literal "0" is actually an
        // octal, but it does not matter if we
        // construct it using base 10.
        return 10;
    }

    // The string length is at least 2. Infer the base
    // of the literal.
    const int base = [&arr, base8_digit_check, base10_digit_check]() -> int {
        // Fetch the first 2 digits.
        const auto d0 = arr[0], d1 = arr[1];

        if (d0 == '0') {
            // Binary, octal or hex.
            if (d1 == 'b' || d1 == 'B') {
                return 2;
            } else if (d1 == 'x' || d1 == 'X') {
                return 16;
            } else {
                base8_digit_check(d1);
                return 8;
            }
        } else {
            // Decimal.
            base10_digit_check(d0);
            base10_digit_check(d1);
            return 10;
        }
    }();

#if MPPP_CPLUSPLUS >= 201703L
    assert(base == 2 || base == 8 || base == 10 || base == 16);
#endif

    // Run the checks on the rest of the literal,
    // after we already checked the first 2 digits.
    // For bases 2 and 16, we need to have other digits,
    // otherwise the literal is malformed.
    // NOLINTNEXTLINE(hicpp-multiway-paths-covered)
    switch (base) {
        case 2:
            if (str_length == 2u) {
                throw std::invalid_argument("Invalid integral literal: " + std::string(arr));
            }
            for (std::size_t i = 2; i < str_length; ++i) {
                base2_digit_check(arr[i]);
            }
            break;
        case 8:
            for (std::size_t i = 2; i < str_length; ++i) {
                base8_digit_check(arr[i]);
            }
            break;
        case 10:
            for (std::size_t i = 2; i < str_length; ++i) {
                base10_digit_check(arr[i]);
            }
            break;
        case 16:
            if (str_length == 2u) {
                throw std::invalid_argument("Invalid integral literal: " + std::string(arr));
            }
            for (std::size_t i = 2; i < str_length; ++i) {
                base16_digit_check(arr[i]);
            }
    }

    return base;
}

#if MPPP_CPLUSPLUS >= 201703L && (!defined(_MSC_VER) || defined(__clang__))

// Small helper to convert the char representation
// of a digit into the corresponding numerical
// value for a given Base. Base must be one
// of 2, 8, 10 or 16, and Int must be some integral
// type.
template <int Base, typename Int>
constexpr Int digit_to_value(char c)
{
    static_assert(std::is_integral_v<Int>);
    static_assert(Base == 2 || Base == 8 || Base == 10 || Base == 16);

    if constexpr (Base == 16) {
        if (c >= '0' && c <= '9') {
            return static_cast<Int>(c - '0');
        } else {
            switch (c) {
                case 'a':
                    [[fallthrough]];
                case 'A':
                    return static_cast<Int>(10);
                case 'b':
                    [[fallthrough]];
                case 'B':
                    return static_cast<Int>(11);
                case 'c':
                    [[fallthrough]];
                case 'C':
                    return static_cast<Int>(12);
                case 'd':
                    [[fallthrough]];
                case 'D':
                    return static_cast<Int>(13);
                case 'e':
                    [[fallthrough]];
                case 'E':
                    return static_cast<Int>(14);
                default:
                    assert(c == 'f' || c == 'F');
                    return static_cast<Int>(15);
            }
        }
    } else {
        if constexpr (Base == 2) {
            assert(c == '0' || c == '1');
        } else if constexpr (Base == 8) {
            assert(c >= '0' && c <= '7');
        } else {
            assert(c >= '0' && c <= '9');
        }
        return static_cast<Int>(c - '0');
    }
}

#endif

template <std::size_t SSize, char... Chars>
inline integer<SSize> integer_literal_impl()
{
    // Turn the sequence of input chars
    // into a null-terminated char array.
    // NOTE: earlier clang versions won't properly
    // capture arr in the lambdas below if it is
    // not marked as static.
#if defined(__clang__) && (defined(__apple_build_version__) || __clang_major__ <= 8)
    static
#endif
        constexpr char arr[]
        = {Chars..., '\0'};

#if MPPP_CPLUSPLUS >= 201703L && (!defined(_MSC_VER) || defined(__clang__)) && !defined(__INTEL_COMPILER)
    // Run the checks on the char sequence, and determine the base.
    constexpr auto base = integer_literal_check_str(arr);
    static_assert(base == 2 || base == 8 || base == 10 || base == 16);

    // The actual number of digits in the literal
    // (discounting prefixes).
    constexpr auto ndigits = (base == 2 || base == 16) ? (sizeof...(Chars) - 2u)
                                                       : (base == 8 ? (sizeof...(Chars) - 1u) : sizeof...(Chars));

    // Determine how many digits in the given base we can always fit
    // into a single mp_limb_t.
    constexpr auto nd_limb = static_cast<unsigned>([]() {
        [[maybe_unused]] constexpr auto nbits = std::numeric_limits<::mp_limb_t>::digits;

        if constexpr (base == 2) {
            // Base 2, just return the number of bits.
            return nbits;
        } else if constexpr (base == 8) {
            // Base 8, need 3 bits per digit.
            return nbits / 3;
        } else if constexpr (base == 16) {
            // Base 16, need 4 bits per digit.
            return nbits / 4;
        } else {
            // Base 10, use digits10.
            return std::numeric_limits<::mp_limb_t>::digits10;
        }
    }());

    // Determine how many instances of mp_limb_t we need
    // to represent the literal.
    constexpr auto nlimbs = ndigits / nd_limb + static_cast<unsigned>(ndigits % nd_limb != 0u);
    static_assert(nlimbs > 0u);

    // Helper function to parse a single limb from a subset
    // of digits at indices [begin, end) in the literal.
    auto parse_limb = [](std::size_t begin, std::size_t end) {
        ::mp_limb_t retval = 0, shifter = 1;

        for (auto i = end; i > begin; --i) {
            const auto cur_digit = digit_to_value<base, ::mp_limb_t>(arr[i - 1u]);
            retval += cur_digit * shifter;

            if (i > begin + 1u) {
                // If this is not the last iteration, update the shifter.
                shifter *= static_cast<::mp_limb_t>(base);
            }
        }

        return retval;
    };

    if constexpr (nlimbs == 1u) {
        // Fast path if the literal fits into
        // a single mp_limb_t.
        constexpr auto l = parse_limb(sizeof...(Chars) - ndigits, sizeof...(Chars));

        return integer<SSize>{l};
    } else {
        // The literal needs more than 1 limb:
        // build an array of limbs from the literal
        // and use it to construct the integer
        // arithmetically.

        // Small wrapper to return an array
        // from a lambda.
        struct arr_wrap {
            ::mp_limb_t arr[nlimbs];
        };

        constexpr auto limb_arr = [parse_limb]() {
            arr_wrap retval{};

            // Manually compute the first limb, which might
            // contain fewer digits than the others (in which
            // case rem is non-null).
            constexpr auto rem = ndigits % nd_limb;
            const auto idx_end1 = sizeof...(Chars) - ndigits + (rem == 0u ? nd_limb : rem);
            retval.arr[0] = parse_limb(sizeof...(Chars) - ndigits, idx_end1);

            for (std::size_t i = 1; i < nlimbs; ++i) {
                retval.arr[i] = parse_limb(idx_end1 + (i - 1u) * nd_limb, idx_end1 + i * nd_limb);
            }

            return retval;
        }();

        // Turn the limb array into an integer.
        // Start with the least significant limb.
        integer<SSize> retval{limb_arr.arr[nlimbs - 1u]}, tmp;

        // A couple of variables used only in base 10.
        [[maybe_unused]] const auto factor10 = []() {
            if constexpr (base == 10) {
                return mppp::pow_ui(integer<SSize>{10}, nd_limb);
            } else {
                return 0;
            }
        }();
        [[maybe_unused]] auto cur_factor10(factor10);

        if constexpr (base != 10) {
            // NOTE: for bases other than 10, we will use bit shifting below.
            // Make sure that we can represent the max shift amount
            // via std::size_t.
            static_assert(nlimbs <= std::numeric_limits<std::size_t>::max()
                                        / static_cast<unsigned>(std::numeric_limits<::mp_limb_t>::digits));
        }

        for (std::size_t i = nlimbs - 1u; i > 0u; --i) {
            tmp = limb_arr.arr[i - 1u];

            if constexpr (base == 2) {
                tmp <<= (nd_limb * (nlimbs - i));
            } else if constexpr (base == 8) {
                tmp <<= (nd_limb * 3u * (nlimbs - i));
            } else if constexpr (base == 16) {
                tmp <<= (nd_limb * 4u * (nlimbs - i));
            } else {
                tmp *= cur_factor10;
                cur_factor10 *= factor10;
            }

            retval += tmp;
        }

        return retval;
    }
#else
    // Run the checks on the char sequence, and determine the base.
    const auto base = integer_literal_check_str(arr);
    assert(base == 2 || base == 8 || base == 10 || base == 16);

    switch (base) {
        case 2:
            return integer<SSize>{arr + 2, arr + sizeof(arr) - 1, 2};
        case 8:
            return integer<SSize>{arr + 1, arr + sizeof(arr) - 1, 8};
        case 16:
            return integer<SSize>{arr + 2, arr + sizeof(arr) - 1, 16};
        default:
            return integer<SSize>{arr};
    }
#endif
}

} // namespace detail

inline namespace literals
{

#define MPPP_DECLARE_INTEGRAL_UDL(n)                                                                                   \
    template <char... Chars>                                                                                           \
    inline integer<n> operator"" _z##n()                                                                               \
    {                                                                                                                  \
        return detail::integer_literal_impl<n, Chars...>();                                                            \
    }

MPPP_DECLARE_INTEGRAL_UDL(1)
MPPP_DECLARE_INTEGRAL_UDL(2)
MPPP_DECLARE_INTEGRAL_UDL(3)

#undef MPPP_DECLARE_INTEGRAL_UDL

} // namespace literals

} // namespace mppp

#endif
