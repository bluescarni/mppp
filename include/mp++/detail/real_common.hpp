// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_REAL_COMMON_HPP
#define MPPP_DETAIL_REAL_COMMON_HPP

#include <ostream>

#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/visibility.hpp>

namespace mppp
{

namespace detail
{

MPPP_DLL_PUBLIC std::ostream &mpfr_t_to_stream(std::ostream &, const ::mpfr_t);

}

} // namespace mppp

#endif
