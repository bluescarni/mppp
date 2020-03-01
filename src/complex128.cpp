// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <ostream>

// NOTE: extern "C" is already included in quadmath.h since GCC 4.8:
// https://stackoverflow.com/questions/13780219/link-libquadmath-with-c-on-linux
#include <quadmath.h>

#include <mp++/complex128.hpp>
#include <mp++/real128.hpp>

namespace mppp
{

std::ostream &operator<<(std::ostream &os, const complex128 &c)
{
    // NOTE: use the same printing format as std::complex.
    os << '(';
    detail::float128_stream(os, ::crealq(c.m_value));
    os << ',';
    detail::float128_stream(os, ::cimagq(c.m_value));
    os << ')';

    return os;
}

} // namespace mppp
