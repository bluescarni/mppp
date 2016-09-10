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
#include <gmp.h>
#include <limits>
#include <locale>
#include <mp++.hpp>
#include <piranha/piranha.hpp>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>

#define NONIUS_RUNNER
#include <nonius/main.h++>
#include <nonius/nonius.h++>

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

inline std::string lex_cast(const mppp::mpz_raii &m)
{
    return std::string(mppp::mpz_to_str(&m.m_mpz));
}

// Set mpz to random value with n limbs, with the most significant limb divided optionally by div.
inline void random_mpz(mppp::mpz_raii &m, unsigned n, std::mt19937 &rng, unsigned div = 1u)
{
    if (!n) {
        ::mpz_set_ui(&m.m_mpz, 0);
        return;
    }
    static thread_local mppp::mpz_raii tmp;
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
inline void random_integer(Integer &m, unsigned n, std::mt19937 &rng, unsigned div = 1u)
{
    static thread_local mppp::mpz_raii tmp;
    random_mpz(tmp, n, rng, div);
    m = Integer(lex_cast(tmp));
}

// Various test functions.

#define MPPP_BENCHMARK_VEC_SIZE 100u

// mppp::integer.
template <typename Integer>
inline void uadd_vec_mppp(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr;
    std::for_each(arr.begin(), arr.end(), [&rng, N](Integer &i) { random_integer(i, N, rng, size); });
    meter.measure([&arr, size, N]() {
        Integer c(arr[0u]);
        for (unsigned j = 1u; j < size; ++j) {
            add(c, c, arr[j]);
        }
    });
}

template <typename Integer>
inline void sadd_vec_mppp(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr;
    std::for_each(arr.begin(), arr.end(), [&rng, &sdist, N](Integer &i) {
        random_integer(i, N, rng, size);
        if (sdist(rng)) {
            i.negate();
        }
    });
    meter.measure([&arr, size]() {
        Integer c(arr[0u]);
        for (unsigned j = 1u; j < size; ++j) {
            add(c, c, arr[j]);
        }
    });
}

// piranha::integer.
inline void uadd_vec_piranha(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    static thread_local mppp::mpz_raii tmp;
    std::array<piranha::integer, size> arr;
    std::generate(arr.begin(), arr.end(), [&rng, N]() -> piranha::integer {
        random_mpz(tmp, N, rng, size);
        return piranha::integer(&tmp.m_mpz);
    });
    meter.measure([&arr, size, N]() {
        piranha::integer c(arr[0u]);
        for (unsigned j = 1u; j < size; ++j) {
            c.add(c, arr[j]);
        }
    });
}

inline void sadd_vec_piranha(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    static thread_local mppp::mpz_raii tmp;
    std::array<piranha::integer, size> arr;
    std::generate(arr.begin(), arr.end(), [&sdist, &rng, N]() -> piranha::integer {
        random_mpz(tmp, N, rng, size);
        piranha::integer retval(&tmp.m_mpz);
        if (sdist(rng)) {
            retval.negate();
        }
        return retval;
    });
    meter.measure([&arr, size]() {
        piranha::integer c(arr[0u]);
        for (unsigned j = 1u; j < size; ++j) {
            c.add(c, arr[j]);
        }
    });
}

// GMP.
inline void uadd_vec_gmp(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mpz_raii, size> arr;
    std::for_each(arr.begin(), arr.end(), [&rng, N](mppp::mpz_raii &m) { random_mpz(m, N, rng, size); });
    mppp::mpz_raii c;
    meter.measure([&arr, size, &c]() {
        ::mpz_set(&c.m_mpz, &arr[0].m_mpz);
        for (unsigned j = 1u; j < size; ++j) {
            ::mpz_add(&c.m_mpz, &c.m_mpz, &arr[j].m_mpz);
        }
    });
}

inline void sadd_vec_gmp(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mpz_raii, size> arr;
    std::for_each(arr.begin(), arr.end(), [&sdist, &rng, N](mppp::mpz_raii &m) {
        random_mpz(m, N, rng, size);
        if (sdist(rng)) {
            ::mpz_neg(&m.m_mpz, &m.m_mpz);
        }
    });
    mppp::mpz_raii c;
    meter.measure([&arr, size, &c]() {
        ::mpz_set(&c.m_mpz, &arr[0].m_mpz);
        for (unsigned j = 1u; j < size; ++j) {
            ::mpz_add(&c.m_mpz, &c.m_mpz, &arr[j].m_mpz);
        }
    });
}
}

#undef MPPP_BENCHMARK_VEC_SIZE

#endif
