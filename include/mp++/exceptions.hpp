// Copyright 2016-2018 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_EXCEPTIONS_HPP
#define MPPP_EXCEPTIONS_HPP

#include <stdexcept>

namespace mppp
{

/// Exception to signal division by zero.
/**
 * \rststar
 * This exception inherits all members (including constructors) from
 * `std::domain_error <https://en.cppreference.com/w/cpp/error/domain_error>`_. It will be thrown
 * when a division by zero involving a multiprecision class is attempted, and the type of the result cannot
 * represent infinities.
 * \endrststar
 */
struct zero_division_error final : std::domain_error {
    using std::domain_error::domain_error;
};
} // namespace mppp

#endif
