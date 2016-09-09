/* Copyright 2009-2016 Francesco Biscani (bluescarni@gmail.com)

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

#ifndef MPPP_BENCHMARK_UTILS_HPP
#define MPPP_BENCHMARK_UTILS_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <gmp.h>
#include <limits>
#include <locale>
#include <mp++.hpp>
#include <piranha/piranha.hpp>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>

#define BENCHPRESS_CONFIG_MAIN
#include <benchpress/benchpress.hpp>

namespace mppp_bench
{

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
    oss.imbue(std::locale::classic());
    oss << lex_cast_tr(x);
    return oss.str();
}

// Various test functions.

#define MPPP_BENCHMARK_VEC_SIZE 100u

// mppp::integer.
template <typename Integer>
inline void uadd1_mppp(benchpress::context *ctx, std::mt19937 &rng)
{
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    // NOTE: divide by 2 so we always stay in a single limb.
    auto a = Integer((dist(rng) & GMP_NUMB_MASK) / 2u), b = Integer((dist(rng) & GMP_NUMB_MASK) / 2u);
    Integer c;
    ctx->reset_timer();
    for (std::size_t i = 0; i < ctx->num_iterations(); ++i) {
        add(c, a, b);
    }
}

template <typename Integer>
inline void sadd1_mppp(benchpress::context *ctx, std::mt19937 &rng)
{
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    std::uniform_int_distribution<int> sdist(0, 1);
    auto a = Integer((dist(rng) & GMP_NUMB_MASK) / 2u), b = Integer((dist(rng) & GMP_NUMB_MASK) / 2u);
    if (sdist(rng)) {
        a.negate();
    }
    if (sdist(rng)) {
        b.negate();
    }
    Integer c;
    ctx->reset_timer();
    for (std::size_t i = 0; i < ctx->num_iterations(); ++i) {
        add(c, a, b);
    }
}

template <typename Integer>
inline void uadd1_vec_mppp(benchpress::context *ctx, std::mt19937 &rng)
{
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr;
    std::generate(arr.begin(), arr.end(), [&dist, &rng]() { return Integer((dist(rng) & GMP_NUMB_MASK) / size); });
    ctx->reset_timer();
    for (std::size_t i = 0; i < ctx->num_iterations(); ++i) {
        Integer c(arr[0u]);
        for (unsigned j = 1u; j < size; ++j) {
            add(c, c, arr[j]);
        }
    }
}

template <typename Integer>
inline void sadd1_vec_mppp(benchpress::context *ctx, std::mt19937 &rng)
{
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr;
    std::generate(arr.begin(), arr.end(), [&dist, &sdist, &rng]() -> Integer {
        Integer retval((dist(rng) & GMP_NUMB_MASK) / size);
        if (sdist(rng)) {
            retval.negate();
        }
        return retval;
    });
    ctx->reset_timer();
    for (std::size_t i = 0; i < ctx->num_iterations(); ++i) {
        Integer c(arr[0u]);
        for (unsigned j = 1u; j < size; ++j) {
            add(c, c, arr[j]);
        }
    }
}

// piranha::integer.
inline void uadd1_piranha(benchpress::context *ctx, std::mt19937 &rng)
{
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    auto a = piranha::integer((dist(rng) & GMP_NUMB_MASK) / 2u), b = piranha::integer((dist(rng) & GMP_NUMB_MASK) / 2u);
    piranha::integer c;
    ctx->reset_timer();
    for (std::size_t i = 0; i < ctx->num_iterations(); ++i) {
        c.add(a, b);
    }
}

inline void sadd1_piranha(benchpress::context *ctx, std::mt19937 &rng)
{
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    std::uniform_int_distribution<int> sdist(0, 1);
    auto a = piranha::integer((dist(rng) & GMP_NUMB_MASK) / 2u), b = piranha::integer((dist(rng) & GMP_NUMB_MASK) / 2u);
    if (sdist(rng)) {
        a.negate();
    }
    if (sdist(rng)) {
        b.negate();
    }
    piranha::integer c;
    ctx->reset_timer();
    for (std::size_t i = 0; i < ctx->num_iterations(); ++i) {
        c.add(a, b);
    }
}

inline void uadd1_vec_piranha(benchpress::context *ctx, std::mt19937 &rng)
{
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<piranha::integer, size> arr;
    std::generate(arr.begin(), arr.end(),
                  [&dist, &rng]() { return piranha::integer((dist(rng) & GMP_NUMB_MASK) / size); });
    ctx->reset_timer();
    for (std::size_t i = 0; i < ctx->num_iterations(); ++i) {
        piranha::integer c(arr[0u]);
        for (unsigned j = 1u; j < size; ++j) {
            c.add(c, arr[j]);
        }
    }
}

inline void sadd1_vec_piranha(benchpress::context *ctx, std::mt19937 &rng)
{
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<piranha::integer, size> arr;
    std::generate(arr.begin(), arr.end(), [&dist, &sdist, &rng]() -> piranha::integer {
        piranha::integer retval((dist(rng) & GMP_NUMB_MASK) / size);
        if (sdist(rng)) {
            retval.negate();
        }
        return retval;
    });
    ctx->reset_timer();
    for (std::size_t i = 0; i < ctx->num_iterations(); ++i) {
        piranha::integer c(arr[0u]);
        for (unsigned j = 1u; j < size; ++j) {
            c.add(c, arr[j]);
        }
    }
}

// GMP.
inline void uadd1_gmp(benchpress::context *ctx, std::mt19937 &rng)
{
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    mppp::mpz_raii a, b, c;
    const auto s1 = lex_cast((dist(rng) & GMP_NUMB_MASK) / 2u);
    const auto s2 = lex_cast((dist(rng) & GMP_NUMB_MASK) / 2u);
    ::mpz_set_str(&a.m_mpz, s1.data(), 10);
    ::mpz_set_str(&b.m_mpz, s2.data(), 10);
    ctx->reset_timer();
    for (size_t i = 0; i < ctx->num_iterations(); ++i) {
        ::mpz_add(&c.m_mpz, &a.m_mpz, &b.m_mpz);
    }
}

inline void uadd1_vec_gmp(benchpress::context *ctx, std::mt19937 &rng)
{
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mpz_raii, size> arr;
    std::for_each(arr.begin(), arr.end(), [&dist, &rng](mppp::mpz_raii &m) {
        const auto s = lex_cast((dist(rng) & GMP_NUMB_MASK) / size);
        ::mpz_set_str(&m.m_mpz, s.data(), 10);
    });
    mppp::mpz_raii c;
    ctx->reset_timer();
    for (size_t i = 0; i < ctx->num_iterations(); ++i) {
        ::mpz_set(&c.m_mpz, &arr[0].m_mpz);
        for (unsigned j = 1u; j < size; ++j) {
            ::mpz_add(&c.m_mpz, &c.m_mpz, &arr[j].m_mpz);
        }
    }
}

inline void sadd1_gmp(benchpress::context *ctx, std::mt19937 &rng)
{
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    std::uniform_int_distribution<int> sdist(0, 1);
    mppp::mpz_raii a, b, c;
    const auto s1 = lex_cast((dist(rng) & GMP_NUMB_MASK) / 2u);
    const auto s2 = lex_cast((dist(rng) & GMP_NUMB_MASK) / 2u);
    ::mpz_set_str(&a.m_mpz, s1.data(), 10);
    if (sdist(rng)) {
        ::mpz_neg(&a.m_mpz, &a.m_mpz);
    }
    ::mpz_set_str(&b.m_mpz, s2.data(), 10);
    if (sdist(rng)) {
        ::mpz_neg(&b.m_mpz, &b.m_mpz);
    }
    ctx->reset_timer();
    for (size_t i = 0; i < ctx->num_iterations(); ++i) {
        ::mpz_add(&c.m_mpz, &a.m_mpz, &b.m_mpz);
    }
}

inline void sadd1_vec_gmp(benchpress::context *ctx, std::mt19937 &rng)
{
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mpz_raii, size> arr;
    std::for_each(arr.begin(), arr.end(), [&dist, &sdist, &rng](mppp::mpz_raii &m) {
        const auto s = lex_cast((dist(rng) & GMP_NUMB_MASK) / size);
        ::mpz_set_str(&m.m_mpz, s.data(), 10);
        if (sdist(rng)) {
            ::mpz_neg(&m.m_mpz, &m.m_mpz);
        }
    });
    mppp::mpz_raii c;
    ctx->reset_timer();
    for (size_t i = 0; i < ctx->num_iterations(); ++i) {
        ::mpz_set(&c.m_mpz, &arr[0].m_mpz);
        for (unsigned j = 1u; j < size; ++j) {
            ::mpz_add(&c.m_mpz, &c.m_mpz, &arr[j].m_mpz);
        }
    }
}
}

#undef MPPP_BENCHMARK_VEC_SIZE

#endif
