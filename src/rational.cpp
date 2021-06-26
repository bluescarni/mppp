// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <array>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <ios>
#include <iostream>
#include <limits>
#include <locale>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <mp++/config.hpp>
#include <mp++/detail/gmp.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>

// NOLINTNEXTLINE(modernize-concat-nested-namespaces)
namespace mppp
{

namespace detail
{

namespace
{

// Some misc tests to check that the mpq struct conforms to our expectations.
struct expected_mpq_struct_t {
    mpz_struct_t _mp_num;
    mpz_struct_t _mp_den;
};

static_assert(sizeof(expected_mpq_struct_t) == sizeof(mpq_struct_t)
                  && offsetof(mpq_struct_t, _mp_num) == offsetof(expected_mpq_struct_t, _mp_num)
                  && offsetof(mpq_struct_t, _mp_den) == offsetof(expected_mpq_struct_t, _mp_den),
              "Invalid mpq_t struct layout.");

#if MPPP_CPLUSPLUS >= 201703L

constexpr bool test_mpq_struct_t()
{
    auto [num, den] = mpq_struct_t{};
    ignore(num, den);
    // NOLINTNEXTLINE(misc-redundant-expression)
    return std::is_same<decltype(num), mpz_struct_t>::value && std::is_same<decltype(den), mpz_struct_t>::value;
}

static_assert(test_mpq_struct_t(), "The mpq_struct_t does not have the expected layout.");

#endif

} // namespace

std::ostream &rational_stream_operator_impl(std::ostream &os, const mpz_struct_t *num, const mpz_struct_t *den,
                                            int q_sgn, bool den_unitary)
{
    // Get the stream width.
    const auto width = os.width();

    // Fetch the stream's flags.
    const auto flags = os.flags();

    // Start by figuring out the base.
    const auto base = stream_flags_to_base(flags);

    // Should we prefix the base? Do it only if:
    // - the number is nonzero,
    // - the showbase flag is set,
    // - the base is not 10.
    const bool with_base_prefix = q_sgn != 0 && (flags & std::ios_base::showbase) != 0 && base != 10;

    // Uppercase?
    const bool uppercase = (flags & std::ios_base::uppercase) != 0;

    // Write out to temporary vectors in the required base. This will produce
    // a representation in the required base, with no base prefix and no
    // extra '+' for nonnegative integers.
    MPPP_MAYBE_TLS std::vector<char> tmp_num, tmp_den;
    mpz_to_str(tmp_num, num, base);
    if (!den_unitary) {
        mpz_to_str(tmp_den, den, base);
    }
    // NOTE: the tmp vectors contain the terminator, and they might be
    // larger than needed. Make sure to shrink them so that
    // the last element is the terminator.
    tmp_num.resize(static_cast<decltype(tmp_num.size())>(std::strlen(tmp_num.data())) + 1u);
    if (!den_unitary) {
        tmp_den.resize(static_cast<decltype(tmp_den.size())>(std::strlen(tmp_den.data())) + 1u);
    }
    constexpr std::array<char, 2> hex_prefix = {{'0', 'x'}};

    // Formatting for the numerator.
    if (q_sgn == -1) {
        // Negative number.
        if (with_base_prefix) {
            // If we need the base prefix, we will have to add the base after the minus sign
            // in the numerator.
            assert(tmp_num[0] == '-');
            if (base == 16) {
                tmp_num.insert(tmp_num.begin() + 1, hex_prefix.begin(), hex_prefix.end());
            } else {
                tmp_num.insert(tmp_num.begin() + 1, '0');
            }
        }
    } else {
        // Nonnegative number. We will be prepending up to 3 characters to the number
        // representation in the numerator:
        // - 1 or 2 for the base prefix ('0' for octal, '0x'/'0X' for hex),
        // - the '+' sign, if requested.
        const bool with_plus = (flags & std::ios_base::showpos) != 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
        std::array<char, 3> prep_buffer;
        const auto prep_n = [&prep_buffer, with_plus, with_base_prefix, base]() -> std::size_t {
            std::size_t ret = 0;
            if (with_plus) {
                prep_buffer[ret++] = '+';
            }
            if (with_base_prefix) {
                prep_buffer[ret++] = '0';
                if (base == 16) {
                    prep_buffer[ret++] = 'x';
                }
            }
            return ret;
        }();
        // Prepend the additional characters.
        tmp_num.insert(tmp_num.begin(), prep_buffer.data(), prep_buffer.data() + prep_n);
    }

    // Formatting for the denominator: it cannot be negative, and it will ignore
    // the showpos flag (which will affect only the numerator). The only thing
    // to do is prepending the base prefix at the very beginning if requested.
    if (!den_unitary && with_base_prefix) {
        if (base == 16) {
            tmp_den.insert(tmp_den.begin(), hex_prefix.begin(), hex_prefix.end());
        } else {
            tmp_den.insert(tmp_den.begin(), '0');
        }
    }

    // Apply a final toupper() transformation in base 16, if needed,
    // but do it before doing the filling in order to avoid
    // uppercasing the fill character.
    if (base == 16 && uppercase) {
        const auto &cloc = std::locale::classic();
        for (decltype(tmp_num.size()) i = 0; i < tmp_num.size() - 1u; ++i) {
            if (std::isalpha(tmp_num[i], cloc)) {
                tmp_num[i] = std::toupper(tmp_num[i], cloc);
            }
        }
        if (!den_unitary) {
            for (decltype(tmp_den.size()) i = 0; i < tmp_den.size() - 1u; ++i) {
                if (std::isalpha(tmp_den[i], cloc)) {
                    tmp_den[i] = std::toupper(tmp_den[i], cloc);
                }
            }
        }
    }

    // Compute the total size of the number
    // representation (i.e., without fill characters).
    // Check for overflow.
    // LCOV_EXCL_START
    if (mppp_unlikely(!den_unitary
                      && tmp_num.size() > std::numeric_limits<decltype(tmp_num.size())>::max() - tmp_den.size())) {
        throw std::overflow_error("Overflow in rational's stream insertion operator");
    }
    // LCOV_EXCL_STOP
    // NOTE: -1 because of the terminator, tmp_den.size() stays as is because in that
    // case we'll need the divisor symbol as well.
    const auto final_size = (tmp_num.size() - 1u) + (den_unitary ? 0u : tmp_den.size());

    // We are going to do the filling
    // only if the stream width is larger
    // than the total size of the number.
    if (width >= 0 && make_unsigned(width) > final_size) {
        // Determine the fill type.
        const auto fill = stream_flags_to_fill(flags);

        // Compute how much fill we need.
        const auto fill_size = safe_cast<decltype(tmp_num.size())>(make_unsigned(width) - final_size);
        // Get the fill character.
        const auto fill_char = os.fill();

        switch (fill) {
            case 1:
                // Left fill: fill characters at the end.
                // NOTE: the end is the numerator's end, if den is
                // unitary, otherwise it is the denominator's end.
                if (den_unitary) {
                    tmp_num.insert(tmp_num.end() - 1, fill_size, fill_char);
                } else {
                    tmp_den.insert(tmp_den.end() - 1, fill_size, fill_char);
                }
                break;
            case 2:
                // Right fill: fill characters at the beginning (that is, always
                // before the numerator starts).
                tmp_num.insert(tmp_num.begin(), fill_size, fill_char);
                break;
            default: {
                assert(fill == 3);

                // Internal fill: the fill characters are always after the sign (if present).
                // NOTE: contrary to integer, the internal fill does not take into account
                // the base prefix, and it happens only if a sign is present.
                const auto delta = static_cast<int>(tmp_num[0] == '+' || tmp_num[0] == '-');
                tmp_num.insert(tmp_num.begin() + delta, fill_size, fill_char);
                break;
            }
        }
    }

    // Write out the unformatted data.
    assert(!tmp_num.empty());
    os.write(tmp_num.data(), safe_cast<std::streamsize>(tmp_num.size() - 1u));
    if (!den_unitary) {
        constexpr char div_sep = '/';
        os.write(&div_sep, 1);
        assert(!tmp_den.empty());
        os.write(tmp_den.data(), safe_cast<std::streamsize>(tmp_den.size() - 1u));
    }

    // Reset the stream width to zero, like the operator<<() does for builtin types.
    // https://en.cppreference.com/w/cpp/io/manip/setw
    // Do it here so we ensure we don't alter the state of the stream until the very end.
    os.width(0);

    return os;
}

} // namespace detail

} // namespace mppp
