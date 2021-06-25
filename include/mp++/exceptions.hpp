// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_EXCEPTIONS_HPP
#define MPPP_EXCEPTIONS_HPP

#include <stdexcept>

#include <mp++/detail/visibility.hpp>

namespace mppp
{

// Exception to signal division by zero.
class MPPP_DLL_PUBLIC_INLINE_CLASS zero_division_error final : public std::domain_error
{
public:
    using std::domain_error::domain_error;
};

} // namespace mppp

#endif
