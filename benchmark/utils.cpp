// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <chrono>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>

#include <boost/filesystem/path.hpp>

#include <fmt/core.h>

#include "utils.hpp"

namespace mppp_benchmark
{

simple_timer::simple_timer() : m_start(std::chrono::high_resolution_clock::now()) {}

double simple_timer::elapsed() const
{
    return static_cast<double>(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_start)
            .count());
}

void warmup()
{
    fmt::print("Warming up");
    for (auto volatile counter = 0ull; counter < 1000000000ull; ++counter) {
        if (counter % 100000000ull == 0u) {
            fmt::print(".");
            std::cout.flush();
        }
    }
    fmt::print(" Done\n");
}

namespace detail
{

std::string filename_from_abs_path(const std::string &s)
{
    return boost::filesystem::path(s).stem().string();
}

} // namespace detail

const char *res_print_format = "{:16}: {:>5.0f}ms, res = {}\n";

namespace detail
{

namespace
{

constexpr auto py_script = R"(data = {0}

if __name__ == '__main__':
    import matplotlib as mpl
    import matplotlib.pyplot as plt

    mpl.use('Agg')

    plt.style.use('seaborn')

    x_pos = list(range(len(data)))
    labels, height = map(list, zip(*data))

    plt.barh(x_pos, height)
    plt.yticks(x_pos, labels)
    plt.title('{1}')
    plt.xlabel('Time (ms)')
    for i, v in enumerate(height):
        plt.text(v, i, " "+str(v), va='center', fontweight='bold')
    plt.tight_layout()

    plt.savefig('{1}.png', bbox_inches='tight', dpi=250)
)";

}

} // namespace detail

void write_out(data_t bdata, const std::string &benchmark_name)
{
    // Order the benchmark data according to performance.
    using rec_t = data_t::value_type;
    std::sort(bdata.begin(), bdata.end(), [](const rec_t &p1, const rec_t &p2) { return p1.second > p2.second; });

    // Create the Python list with the result data.
    std::string py_data_list = "[";
    for (const auto &p : bdata) {
        py_data_list += fmt::format("('{}', {:.0f}), ", p.first, p.second);
    }
    py_data_list += "]";

    // Write out the Python script.
    std::ofstream of(benchmark_name + ".py", std::ios_base::trunc);
    of.exceptions(std::ios_base::failbit);
    of << fmt::format(detail::py_script, py_data_list, benchmark_name);
}

} // namespace mppp_benchmark
