// Copyright 2016-2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_FMT_HPP
#define MPPP_DETAIL_FMT_HPP

#include <fmt/base.h>

#include <mp++/config.hpp>

MPPP_BEGIN_NAMESPACE

namespace detail
{

struct to_string_formatter {
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx) -> decltype(ctx.begin())
    {
        auto it = ctx.begin();
        const auto end = ctx.end();

        // Handle the special case for the '{}' format string.
        if (it == end) {
            return it;
        }

        // Parse until we get to the end or we find
        // the closing bracket '}', ignoring the format
        // string.
        for (; it != end; ++it) {
            if (*it == '}') {
                // NOTE: according to the docs, we must return
                // an iterator pointing to '}'.
                return it;
            }
        }

        // The format spec was not terminated by a closing bracket. report_error() throws fmt::format_error at runtime,
        // and it results in a compilation error when the format string is checked at compile time.
        fmt::report_error("Invalid format");

        // NOTE: unreachable, as report_error() does not return.
        return it; // LCOV_EXCL_LINE
    }

    template <typename T, typename FormatContext>
    auto format(const T &x, FormatContext &ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", x.to_string());
    }
};

} // namespace detail

MPPP_END_NAMESPACE

#endif
