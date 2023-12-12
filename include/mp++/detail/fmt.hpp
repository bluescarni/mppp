// Copyright 2016-2023 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_FMT_HPP
#define MPPP_DETAIL_FMT_HPP

#include <stdexcept>

#include <fmt/core.h>

#include <mp++/config.hpp>

MPPP_BEGIN_NAMESPACE

namespace detail
{

struct to_string_formatter {
    template <typename ParseContext>
    MPPP_CONSTEXPR_20 auto parse(ParseContext &ctx) -> decltype(ctx.begin())
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

        // LCOV_EXCL_START
#if FMT_VERSION < 100000
        throw std::invalid_argument("Invalid format");
#else
        fmt::throw_format_error("Invalid format");
#endif
        // LCOV_EXCL_STOP
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
