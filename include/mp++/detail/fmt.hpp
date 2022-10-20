// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
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

namespace mppp
{

namespace detail
{

struct to_string_formatter {
    template <typename ParseContext>
    auto parse(ParseContext &ctx) -> decltype(ctx.begin())
    {
        if (ctx.begin() != ctx.end()) {
            throw std::invalid_argument("No format strings are currently supported for mp++'s classes");
        }

        return ctx.begin();
    }

    template <typename T, typename FormatContext>
    auto format(const T &x, FormatContext &ctx) -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", x.to_string());
    }
};

} // namespace detail

} // namespace mppp

#endif
