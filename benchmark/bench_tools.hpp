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

#ifndef MPPP_BENCH_TOOLS_HPP
#define MPPP_BENCH_TOOLS_HPP

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace mppp_bench
{

inline namespace impl
{

inline std::string bs_impl()
{
    return "";
}

template <typename Str, typename F, typename... Args>
inline std::string bs_impl(Str &&s, F &&f, unsigned ntrials, Args &&... args)
{
    auto retval = std::string("('") + s + "',[";
    for (auto i = 0u; i < ntrials; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        f();
        double elapsed
            = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start)
                  .count();
        retval += std::to_string(elapsed);
        if (i != ntrials - 1u) {
            retval += ",";
        }
    }
    retval += "])";
    if (sizeof...(args)) {
        retval += ",";
    }
    retval += bs_impl(std::forward<Args>(args)...);
    return retval;
}
}

template <typename... Args>
inline void benchmark_suite(const std::string &name, Args &&... args)
{
    volatile int c = 0;
    auto counter = 0ull;
    while (!c && counter < 1000000000ull) {
        ++counter;
    }
    std::string s;
    s += "# -*- coding: utf-8 -*-\n"
         "def get_data():\n"
         "    import pandas\n"
         "    data = [";
    s += bs_impl(std::forward<Args>(args)...) + "]\n";
    s += "    return pandas.DataFrame(dict(data))\n\n";
    s += u8"if __name__ == '__main__':\n"
         "    import matplotlib as mpl\n"
         "    mpl.use('Agg')\n"
         "    import seaborn as sns\n"
         "    df = get_data()\n"
         "    g = sns.barplot(data=df,palette='muted')\n"
         "    for l in g.get_xticklabels():\n"
         "        l.set_rotation(45)\n"
         "    g.set_title('"
         + name + "')\n"
                  "    g.set_ylabel('Total runtime (μs)')\n"
                  "    g.get_figure().set_size_inches(10,7.5)\n"
                  "    g.get_figure().tight_layout()\n"
                  "    g.get_figure().savefig('"
         + name + ".svg')\n";
    std::ofstream of(name + ".py", std::ios_base::trunc);
    of << s;
}

// Bench for construction/destruction from primitive types.
template <typename T, typename = void>
struct ctor_dist_type {
    using type = std::uniform_int_distribution<T>;
};

template <typename T>
struct ctor_dist_type<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
    using type = std::uniform_real_distribution<T>;
};

template <typename T, typename Integer>
struct bench_ctor {
    static constexpr unsigned vsize = 10000u;
    bench_ctor(std::mt19937 &rng, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
    {
        typename ctor_dist_type<T>::type dist(min, max);
        std::generate(m_randoms.begin(), m_randoms.end(), [&dist, &rng]() { return dist(rng); });
        m_ptr = static_cast<Integer *>(std::malloc(sizeof(Integer) * vsize));
        if (!m_ptr) {
            throw;
        }
    }
    ~bench_ctor()
    {
        std::free(m_ptr);
    }
    void operator()() const
    {
        for (auto i = 0u; i < vsize; ++i) {
            ::new (m_ptr + i) Integer(m_randoms[i]);
        }
        for (auto i = 0u; i < vsize; ++i) {
            (m_ptr + i)->~Integer();
        }
    }
    Integer *m_ptr;
    std::array<T, vsize> m_randoms;
};

// Conversion benchmark.
template <typename T, typename Integer>
struct bench_conv {
    static constexpr unsigned vsize = 10000u;
    bench_conv(std::mt19937 &rng, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
    {
        typename ctor_dist_type<T>::type dist(min, max);
        m_randoms.resize(vsize);
        std::generate(m_randoms.begin(), m_randoms.end(), [&dist, &rng]() { return Integer{dist(rng)}; });
    }
    void operator()() const
    {
        for (const auto &n : m_randoms) {
            volatile T tmp = static_cast<T>(n);
        }
    }
    std::vector<Integer> m_randoms;
};
}

#endif
