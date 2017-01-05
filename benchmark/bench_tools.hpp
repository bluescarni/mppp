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
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <gmp.h>
#include <iterator>
#include <limits>
#include <mp++.hpp>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(MPPP_BENCHMARK_FLINT)
#include <flint/fmpz.h>
#endif

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
        auto elapsed = static_cast<double>(
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start)
                .count());
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
         "    g = sns.barplot(data=df)\n"
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
        m_randoms.resize(vsize);
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
    std::vector<T> m_randoms;
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

inline namespace impl
{

template <typename T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, int>::type = 0>
inline long long lex_cast_tr(T n)
{
    return static_cast<long long>(n);
}

template <typename T, typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, int>::type = 0>
inline unsigned long long lex_cast_tr(T n)
{
    return static_cast<unsigned long long>(n);
}

template <typename T, typename std::enable_if<!std::is_integral<T>::value, int>::type = 0>
inline const T &lex_cast_tr(const T &x)
{
    return x;
}
}

// Lexical cast: retrieve the string representation of input object x.
template <typename T>
inline std::string lex_cast(const T &x)
{
    std::ostringstream oss;
    oss << lex_cast_tr(x);
    return oss.str();
}

inline std::string lex_cast(const mppp::mppp_impl::mpz_raii &m)
{
    return mppp::mppp_impl::mpz_to_str(&m.m_mpz);
}

// Set mpz to random value with n limbs, with the most significant limb divided optionally by div.
inline void random_mpz(mppp::mppp_impl::mpz_raii &m, unsigned n, std::mt19937 &rng, ::mp_limb_t div = 1u)
{
    if (!n) {
        ::mpz_set_ui(&m.m_mpz, 0);
        return;
    }
    static thread_local mppp::mppp_impl::mpz_raii tmp;
    std::uniform_int_distribution<::mp_limb_t> dist(0u, std::numeric_limits<::mp_limb_t>::max());
    // Set the first limb.
    ::mpz_set_str(&m.m_mpz, lex_cast((dist(rng) & GMP_NUMB_MASK) / div).c_str(), 10);
    for (unsigned i = 1u; i < n; ++i) {
        ::mpz_set_str(&tmp.m_mpz, lex_cast(dist(rng) & GMP_NUMB_MASK).c_str(), 10);
        ::mpz_mul_2exp(&m.m_mpz, &m.m_mpz, GMP_NUMB_BITS);
        ::mpz_add(&m.m_mpz, &m.m_mpz, &tmp.m_mpz);
    }
}

// Set integer to random value with n limbs, with the most significant limb divided optionally by div.
template <typename Integer>
inline void random_integer(Integer &m, unsigned n, std::mt19937 &rng, ::mp_limb_t div = 1u)
{
    static thread_local mppp::mppp_impl::mpz_raii tmp;
    random_mpz(tmp, n, rng, div);
    m = Integer(lex_cast(tmp));
}

#define MPPP_BENCHMARK_VEC_SIZE 10000u

// mp++ benchmarking.
template <typename Integer>
struct uadd_vec_mppp {
    static constexpr unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    uadd_vec_mppp(std::mt19937 &rng, unsigned N, unsigned M) : m_arr1(size), m_arr2(size), m_arr3(size)
    {
        std::for_each(m_arr1.begin(), m_arr1.end(), [&rng, N](Integer &i) { random_integer(i, N, rng, 2); });
        std::for_each(m_arr2.begin(), m_arr2.end(), [&rng, M](Integer &i) { random_integer(i, M, rng, 2); });
    }
    void operator()() const
    {
        for (auto j = 0u; j < size; ++j) {
            add(m_arr3[j], m_arr1[j], m_arr2[j]);
        }
    }
    std::vector<Integer> m_arr1;
    std::vector<Integer> m_arr2;
    mutable std::vector<Integer> m_arr3;
};

// GMP benchmarking.
struct uadd_vec_gmp {
    static constexpr unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    uadd_vec_gmp(std::mt19937 &rng, unsigned N, unsigned M) : m_arr1(size), m_arr2(size), m_arr3(size)
    {
        std::for_each(m_arr1.begin(), m_arr1.end(),
                      [&rng, N](mppp::mppp_impl::mpz_raii &m) { random_mpz(m, N, rng, 2); });
        std::for_each(m_arr2.begin(), m_arr2.end(),
                      [&rng, M](mppp::mppp_impl::mpz_raii &m) { random_mpz(m, M, rng, 2); });
    }
    void operator()() const
    {
        for (auto j = 0u; j < size; ++j) {
            ::mpz_add(&m_arr3[j].m_mpz, &m_arr1[j].m_mpz, &m_arr2[j].m_mpz);
        }
    }
    std::vector<mppp::mppp_impl::mpz_raii> m_arr1;
    std::vector<mppp::mppp_impl::mpz_raii> m_arr2;
    mutable std::vector<mppp::mppp_impl::mpz_raii> m_arr3;
};

#if defined(MPPP_BENCHMARK_FLINT)

// Flint benchmarking.
struct fmpz_raii {
    fmpz_raii()
    {
        ::fmpz_init(m_fmpz);
    }
    ~fmpz_raii()
    {
        ::fmpz_clear(m_fmpz);
    }
    ::fmpz_t m_fmpz;
};

struct uadd_vec_fmpz {
    static constexpr unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    uadd_vec_fmpz(std::mt19937 &rng, unsigned N, unsigned M) : m_arr1(size), m_arr2(size), m_arr3(size)
    {
        mppp::mppp_impl::mpz_raii m;
        std::for_each(m_arr1.begin(), m_arr1.end(), [&rng, N, &m](fmpz_raii &f) {
            random_mpz(m, N, rng, 8);
            ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
        });
        std::for_each(m_arr2.begin(), m_arr2.end(), [&rng, M, &m](fmpz_raii &f) {
            random_mpz(m, M, rng, 8);
            ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
        });
    }
    void operator()() const
    {
        for (auto j = 0u; j < size; ++j) {
            ::fmpz_add(m_arr3[j].m_fmpz, m_arr1[j].m_fmpz, m_arr2[j].m_fmpz);
        }
    }
    std::vector<fmpz_raii> m_arr1;
    std::vector<fmpz_raii> m_arr2;
    mutable std::vector<fmpz_raii> m_arr3;
};

#endif

#if defined(MPPP_BENCHMARK_BOOST)

// Boost benchmarking.
template <typename Integer>
struct uadd_vec_boost {
    static constexpr unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    uadd_vec_boost(std::mt19937 &rng, unsigned N, unsigned M) : m_arr1(size), m_arr2(size), m_arr3(size)
    {
        mppp::mppp_impl::mpz_raii m;
        std::for_each(m_arr1.begin(), m_arr1.end(), [&rng, N, &m](Integer &n) {
            random_mpz(m, N, rng, 8);
            n = Integer(lex_cast(m));
        });
        std::for_each(m_arr2.begin(), m_arr2.end(), [&rng, M, &m](Integer &n) {
            random_mpz(m, M, rng, 8);
            n = Integer(lex_cast(m));
        });
    }
    void operator()() const
    {
        for (auto j = 0u; j < size; ++j) {
            m_arr3[j] = m_arr1[j] + m_arr2[j];
        }
    }
    std::vector<Integer> m_arr1;
    std::vector<Integer> m_arr2;
    mutable std::vector<Integer> m_arr3;
};

#endif

#undef MPPP_BENCHMARK_VEC_SIZE
}

#endif
