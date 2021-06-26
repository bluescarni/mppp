// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_BENCHMARK_UTILS_HPP
#define MPPP_BENCHMARK_UTILS_HPP

#include <chrono>
#include <string>
#include <utility>
#include <vector>

namespace mppp_benchmark
{

// A simple RAII timer class, using std::chrono. It will print, upon destruction,
// the time elapsed since construction (in ms).
class simple_timer
{
public:
    simple_timer();
    double elapsed() const;

private:
    const std::chrono::high_resolution_clock::time_point m_start;
};

// Warmup function.
void warmup();

namespace detail
{

std::string filename_from_abs_path(const std::string &);

}

// Small macro to get the file name of the current translation
// unit, stripping away the path and the extension. This
// will be used as the benchmark name.
#define mppp_benchmark_name() mppp_benchmark::detail::filename_from_abs_path(__FILE__)

// Format strings.
extern const char *res_print_format;

// Benchmark result data type.
using data_t = std::vector<std::pair<std::string, double>>;

// Write out the Python plotting file.
void write_out(data_t, const std::string &);

} // namespace mppp_benchmark

#endif
