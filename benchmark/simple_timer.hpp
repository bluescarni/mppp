/* Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)

This file is part of the mp++ library.

The mp++ library is free software; you can redistribute it and/or modify
it under the terms of either:

  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.

or

  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.

or both in parallel, as here.

The mp++ library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the mp++ library.  If not,
see https://www.gnu.org/licenses/. */

#ifndef MPPP_SIMPLE_TIMER_HPP
#define MPPP_SIMPLE_TIMER_HPP

#include <chrono>
#include <iostream>

namespace mppp_bench
{

// A simple RAII timer class, using std::chrono. It will print, upon destruction,
// the time elapsed since construction (in ms).
class simple_timer
{
public:
    simple_timer() : m_start(std::chrono::high_resolution_clock::now())
    {
    }
    ~simple_timer()
    {
        std::cout << "Elapsed time: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()
                                                                           - m_start)
                         .count()
                  << "ms\n";
    }

private:
    const std::chrono::high_resolution_clock::time_point m_start;
};
}

#endif
