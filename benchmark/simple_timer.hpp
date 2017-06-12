// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
    double elapsed() const
    {
        return static_cast<double>(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_start)
                .count());
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
