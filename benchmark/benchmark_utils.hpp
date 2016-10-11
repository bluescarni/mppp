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
    return mppp::mpz_to_str(&m.m_mpz);
}

// Set mpz to random value with n limbs, with the most significant limb divided optionally by div.
inline void random_mpz(mppp::mpz_raii &m, unsigned n, std::mt19937 &rng, ::mp_limb_t div = 1u)
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
inline void random_integer(Integer &m, unsigned n, std::mt19937 &rng, ::mp_limb_t div = 1u)
{
    static thread_local mppp::mpz_raii tmp;
    random_mpz(tmp, n, rng, div);
    m = Integer(lex_cast(tmp));
}

// Various test functions.

#define MPPP_BENCHMARK_VEC_SIZE 100u

// mppp::integer.
template <typename Integer>
inline void uadd_vec_mppp(nonius::chronometer meter, std::mt19937 &rng, unsigned N, unsigned M)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N](Integer &i) { random_integer(i, N, rng, 2); });
    std::for_each(arr2.begin(), arr2.end(), [&rng, M](Integer &i) { random_integer(i, M, rng, 2); });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (auto j = 0u; j < size; ++j) {
            add(arr3[j], arr1[j], arr2[j]);
        }
    });
}

template <typename Integer>
inline void sadd_vec_mppp(nonius::chronometer meter, std::mt19937 &rng, unsigned N, unsigned M)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &sdist, N](Integer &i) {
        random_integer(i, N, rng, 2);
        if (sdist(rng)) {
            i.negate();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &sdist, M](Integer &i) {
        random_integer(i, M, rng, 2);
        if (sdist(rng)) {
            i.negate();
        }
    });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (auto j = 0u; j < size; ++j) {
            add(arr3[j], arr1[j], arr2[j]);
        }
    });
}

template <typename Integer>
inline void uacc_mppp(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr;
    std::for_each(arr.begin(), arr.end(), [&rng, N, size](Integer &i) { random_integer(i, N, rng, size); });
    meter.measure([&arr, size]() {
        Integer c(arr[0u]);
        for (unsigned j = 1u; j < size; ++j) {
            add(c, c, arr[j]);
        }
    });
}

template <typename Integer>
inline void sacc_mppp(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr;
    std::for_each(arr.begin(), arr.end(), [&rng, &sdist, N, size](Integer &i) {
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

// Special casing to force multiplication within a single limb.
template <typename Integer>
inline void umul_vec_mppp_half(nonius::chronometer meter, std::mt19937 &rng)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(),
                  [&rng](Integer &i) { random_integer(i, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2)); });
    std::for_each(arr2.begin(), arr2.end(),
                  [&rng](Integer &i) { random_integer(i, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2)); });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            mul(arr3[j], arr1[j], arr2[j]);
        }
    });
}

template <typename Integer>
inline void smul_vec_mppp_half(nonius::chronometer meter, std::mt19937 &rng)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &sdist](Integer &i) {
        random_integer(i, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        if (sdist(rng)) {
            i.negate();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &sdist](Integer &i) {
        random_integer(i, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        if (sdist(rng)) {
            i.negate();
        }
    });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            mul(arr3[j], arr1[j], arr2[j]);
        }
    });
}

template <typename Integer>
inline void umul_vec_mppp(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N1](Integer &i) { random_integer(i, N1, rng); });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2](Integer &i) { random_integer(i, N2, rng); });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            mul(arr3[j], arr1[j], arr2[j]);
        }
    });
}

template <typename Integer>
inline void smul_vec_mppp(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N1, &sdist](Integer &i) {
        random_integer(i, N1, rng);
        if (sdist(rng)) {
            i.negate();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2, &sdist](Integer &i) {
        random_integer(i, N2, rng);
        if (sdist(rng)) {
            i.negate();
        }
    });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            mul(arr3[j], arr1[j], arr2[j]);
        }
    });
}

template <typename Integer>
inline void uaddmul_vec_mppp_half(nonius::chronometer meter, std::mt19937 &rng)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(),
                  [&rng](Integer &i) { random_integer(i, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2)); });
    std::for_each(arr2.begin(), arr2.end(),
                  [&rng](Integer &i) { random_integer(i, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2)); });
    std::for_each(arr3.begin(), arr3.end(), [](Integer &i) { i = Integer(1); });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        auto arr3a(arr3);
        for (unsigned j = 0u; j < size; ++j) {
            addmul(arr3a[j], arr1[j], arr2[j]);
        }
    });
}

template <typename Integer>
inline void saddmul_vec_mppp_half(nonius::chronometer meter, std::mt19937 &rng)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &sdist](Integer &i) {
        random_integer(i, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        if (sdist(rng)) {
            i.negate();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &sdist](Integer &i) {
        random_integer(i, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        if (sdist(rng)) {
            i.negate();
        }
    });
    std::for_each(arr3.begin(), arr3.end(), [](Integer &i) { i = Integer(1); });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        auto arr3a(arr3);
        for (unsigned j = 0u; j < size; ++j) {
            addmul(arr3a[j], arr1[j], arr2[j]);
        }
    });
}

template <typename Integer>
inline void uaddmul_acc_mppp_half(nonius::chronometer meter, std::mt19937 &rng)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2;
    std::for_each(arr1.begin(), arr1.end(),
                  [&rng](Integer &i) { random_integer(i, 1u, rng, ::mp_limb_t(1) << ((GMP_NUMB_BITS * 2) / 3)); });
    std::for_each(arr2.begin(), arr2.end(),
                  [&rng](Integer &i) { random_integer(i, 1u, rng, ::mp_limb_t(1) << ((GMP_NUMB_BITS * 2) / 3)); });
    meter.measure([&arr1, &arr2, size]() {
        Integer acc;
        for (unsigned j = 0u; j < size; ++j) {
            addmul(acc, arr1[j], arr2[j]);
        }
    });
}

template <typename Integer>
inline void saddmul_acc_mppp_half(nonius::chronometer meter, std::mt19937 &rng)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &sdist](Integer &i) {
        random_integer(i, 1u, rng, ::mp_limb_t(1) << ((GMP_NUMB_BITS * 2) / 3));
        if (sdist(rng)) {
            i.negate();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &sdist](Integer &i) {
        random_integer(i, 1u, rng, ::mp_limb_t(1) << ((GMP_NUMB_BITS * 2) / 3));
        if (sdist(rng)) {
            i.negate();
        }
    });
    meter.measure([&arr1, &arr2, size]() {
        Integer acc;
        for (unsigned j = 0u; j < size; ++j) {
            addmul(acc, arr1[j], arr2[j]);
        }
    });
}

template <typename Integer>
inline void uaddmul_vec_mppp(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N1](Integer &i) { random_integer(i, N1, rng); });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2](Integer &i) { random_integer(i, N2, rng); });
    std::for_each(arr3.begin(), arr3.end(), [](Integer &i) { i = Integer(1); });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        auto arr3a(arr3);
        for (unsigned j = 0u; j < size; ++j) {
            addmul(arr3a[j], arr1[j], arr2[j]);
        }
    });
}

template <typename Integer>
inline void uaddmul_acc_mppp(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N1, size](Integer &i) { random_integer(i, N1, rng, size); });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2](Integer &i) { random_integer(i, N2, rng); });
    meter.measure([&arr1, &arr2, size]() {
        Integer acc;
        for (unsigned j = 0u; j < size; ++j) {
            addmul(acc, arr1[j], arr2[j]);
        }
    });
}

template <typename Integer>
inline void saddmul_vec_mppp(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N1, &sdist](Integer &i) {
        random_integer(i, N1, rng);
        if (sdist(rng)) {
            i.negate();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2, &sdist](Integer &i) {
        random_integer(i, N2, rng);
        if (sdist(rng)) {
            i.negate();
        }
    });
    std::for_each(arr3.begin(), arr3.end(), [](Integer &i) { i = Integer(1); });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        auto arr3a(arr3);
        for (unsigned j = 0u; j < size; ++j) {
            addmul(arr3a[j], arr1[j], arr2[j]);
        }
    });
}

template <typename Integer>
inline void saddmul_acc_mppp(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N1, size, &sdist](Integer &i) {
        random_integer(i, N1, rng, size);
        if (sdist(rng)) {
            i.negate();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2, &sdist](Integer &i) {
        random_integer(i, N2, rng);
        if (sdist(rng)) {
            i.negate();
        }
    });
    meter.measure([&arr1, &arr2, size]() {
        Integer acc;
        for (unsigned j = 0u; j < size; ++j) {
            addmul(acc, arr1[j], arr2[j]);
        }
    });
}

// piranha::integer.
inline void uadd_vec_piranha(nonius::chronometer meter, std::mt19937 &rng, unsigned N, unsigned M)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    mppp::mpz_raii tmp;
    std::array<piranha::integer, size> arr1, arr2, arr3;
    std::generate(arr1.begin(), arr1.end(), [&rng, N, &tmp]() -> piranha::integer {
        random_mpz(tmp, N, rng, 2);
        return piranha::integer(&tmp.m_mpz);
    });
    std::generate(arr2.begin(), arr2.end(), [&rng, M, &tmp]() -> piranha::integer {
        random_mpz(tmp, M, rng, 2);
        return piranha::integer(&tmp.m_mpz);
    });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (auto j = 0u; j < size; ++j) {
            arr3[j].add(arr1[j], arr2[j]);
        }
    });
}

inline void sadd_vec_piranha(nonius::chronometer meter, std::mt19937 &rng, unsigned N, unsigned M)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    mppp::mpz_raii tmp;
    std::array<piranha::integer, size> arr1, arr2, arr3;
    std::generate(arr1.begin(), arr1.end(), [&sdist, &rng, N, &tmp]() -> piranha::integer {
        random_mpz(tmp, N, rng, 2);
        piranha::integer retval(&tmp.m_mpz);
        if (sdist(rng)) {
            retval.negate();
        }
        return retval;
    });
    std::generate(arr2.begin(), arr2.end(), [&sdist, &rng, M, &tmp]() -> piranha::integer {
        random_mpz(tmp, M, rng, 2);
        piranha::integer retval(&tmp.m_mpz);
        if (sdist(rng)) {
            retval.negate();
        }
        return retval;
    });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (auto j = 0u; j < size; ++j) {
            arr3[j].add(arr1[j], arr2[j]);
        }
    });
}

inline void uacc_piranha(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    mppp::mpz_raii tmp;
    std::array<piranha::integer, size> arr;
    std::generate(arr.begin(), arr.end(), [&rng, N, size, &tmp]() -> piranha::integer {
        random_mpz(tmp, N, rng, size);
        return piranha::integer(&tmp.m_mpz);
    });
    meter.measure([&arr, size]() {
        piranha::integer c(arr[0u]);
        for (unsigned j = 1u; j < size; ++j) {
            c.add(c, arr[j]);
        }
    });
}

inline void sacc_piranha(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    mppp::mpz_raii tmp;
    std::array<piranha::integer, size> arr;
    std::generate(arr.begin(), arr.end(), [&sdist, &rng, N, size, &tmp]() -> piranha::integer {
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

inline void umul_vec_piranha_half(nonius::chronometer meter, std::mt19937 &rng)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<piranha::integer, size> arr1, arr2, arr3;
    mppp::mpz_raii tmp;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &tmp](piranha::integer &i) {
        random_mpz(tmp, 1, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        i = piranha::integer(&tmp.m_mpz);
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &tmp](piranha::integer &i) {
        random_mpz(tmp, 1, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        i = piranha::integer(&tmp.m_mpz);
    });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            arr3[j].mul(arr1[j], arr2[j]);
        }
    });
}

inline void smul_vec_piranha_half(nonius::chronometer meter, std::mt19937 &rng)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<piranha::integer, size> arr1, arr2, arr3;
    mppp::mpz_raii tmp;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &tmp, &sdist](piranha::integer &i) {
        random_mpz(tmp, 1, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        i = piranha::integer(&tmp.m_mpz);
        if (sdist(rng)) {
            i.negate();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &tmp, &sdist](piranha::integer &i) {
        random_mpz(tmp, 1, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        i = piranha::integer(&tmp.m_mpz);
        if (sdist(rng)) {
            i.negate();
        }
    });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            arr3[j].mul(arr1[j], arr2[j]);
        }
    });
}

inline void umul_vec_piranha(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<piranha::integer, size> arr1, arr2, arr3;
    mppp::mpz_raii tmp;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N1, &tmp](piranha::integer &i) {
        random_mpz(tmp, N1, rng);
        i = piranha::integer(&tmp.m_mpz);
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2, &tmp](piranha::integer &i) {
        random_mpz(tmp, N2, rng);
        i = piranha::integer(&tmp.m_mpz);
    });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            arr3[j].mul(arr1[j], arr2[j]);
        }
    });
}

inline void smul_vec_piranha(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<piranha::integer, size> arr1, arr2, arr3;
    mppp::mpz_raii tmp;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N1, &tmp, &sdist](piranha::integer &i) {
        random_mpz(tmp, N1, rng);
        i = piranha::integer(&tmp.m_mpz);
        if (sdist(rng)) {
            i.negate();
        }

    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2, &tmp, &sdist](piranha::integer &i) {
        random_mpz(tmp, N2, rng);
        i = piranha::integer(&tmp.m_mpz);
        if (sdist(rng)) {
            i.negate();
        }

    });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            arr3[j].mul(arr1[j], arr2[j]);
        }
    });
}

inline void uaddmul_vec_piranha_half(nonius::chronometer meter, std::mt19937 &rng)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<piranha::integer, size> arr1, arr2, arr3;
    mppp::mpz_raii tmp;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &tmp](piranha::integer &i) {
        random_mpz(tmp, 1, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        i = piranha::integer(&tmp.m_mpz);
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &tmp](piranha::integer &i) {
        random_mpz(tmp, 1, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        i = piranha::integer(&tmp.m_mpz);
    });
    std::for_each(arr3.begin(), arr3.end(), [](piranha::integer &i) { i = piranha::integer(1); });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        auto arr3a(arr3);
        for (unsigned j = 0u; j < size; ++j) {
            arr3a[j].multiply_accumulate(arr1[j], arr2[j]);
        }
    });
}

inline void saddmul_vec_piranha_half(nonius::chronometer meter, std::mt19937 &rng)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<piranha::integer, size> arr1, arr2, arr3;
    mppp::mpz_raii tmp;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &tmp, &sdist](piranha::integer &i) {
        random_mpz(tmp, 1, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        i = piranha::integer(&tmp.m_mpz);
        if (sdist(rng)) {
            i.negate();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &tmp, &sdist](piranha::integer &i) {
        random_mpz(tmp, 1, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        i = piranha::integer(&tmp.m_mpz);
        if (sdist(rng)) {
            i.negate();
        }
    });
    std::for_each(arr3.begin(), arr3.end(), [](piranha::integer &i) { i = piranha::integer(1); });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        auto arr3a(arr3);
        for (unsigned j = 0u; j < size; ++j) {
            arr3a[j].multiply_accumulate(arr1[j], arr2[j]);
        }
    });
}

inline void uaddmul_acc_piranha_half(nonius::chronometer meter, std::mt19937 &rng)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<piranha::integer, size> arr1, arr2;
    mppp::mpz_raii tmp;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &tmp](piranha::integer &i) {
        random_mpz(tmp, 1, rng, ::mp_limb_t(1) << ((GMP_NUMB_BITS * 2) / 3));
        i = piranha::integer(&tmp.m_mpz);
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &tmp](piranha::integer &i) {
        random_mpz(tmp, 1, rng, ::mp_limb_t(1) << ((GMP_NUMB_BITS * 2) / 3));
        i = piranha::integer(&tmp.m_mpz);
    });
    meter.measure([&arr1, &arr2, size]() {
        piranha::integer acc;
        for (unsigned j = 0u; j < size; ++j) {
            acc.multiply_accumulate(arr1[j], arr2[j]);
        }
    });
}

inline void saddmul_acc_piranha_half(nonius::chronometer meter, std::mt19937 &rng)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<piranha::integer, size> arr1, arr2;
    mppp::mpz_raii tmp;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &tmp, &sdist](piranha::integer &i) {
        random_mpz(tmp, 1, rng, ::mp_limb_t(1) << ((GMP_NUMB_BITS * 2) / 3));
        i = piranha::integer(&tmp.m_mpz);
        if (sdist(rng)) {
            i.negate();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &tmp, &sdist](piranha::integer &i) {
        random_mpz(tmp, 1, rng, ::mp_limb_t(1) << ((GMP_NUMB_BITS * 2) / 3));
        i = piranha::integer(&tmp.m_mpz);
        if (sdist(rng)) {
            i.negate();
        }
    });
    meter.measure([&arr1, &arr2, size]() {
        piranha::integer acc;
        for (unsigned j = 0u; j < size; ++j) {
            acc.multiply_accumulate(arr1[j], arr2[j]);
        }
    });
}

inline void uaddmul_vec_piranha(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<piranha::integer, size> arr1, arr2, arr3;
    mppp::mpz_raii tmp;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &tmp, N1](piranha::integer &i) {
        random_mpz(tmp, N1, rng);
        i = piranha::integer(&tmp.m_mpz);
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &tmp, N2](piranha::integer &i) {
        random_mpz(tmp, N2, rng);
        i = piranha::integer(&tmp.m_mpz);
    });
    std::for_each(arr3.begin(), arr3.end(), [](piranha::integer &i) { i = piranha::integer(1); });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        auto arr3a(arr3);
        for (unsigned j = 0u; j < size; ++j) {
            arr3a[j].multiply_accumulate(arr1[j], arr2[j]);
        }
    });
}

inline void uaddmul_acc_piranha(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<piranha::integer, size> arr1, arr2;
    mppp::mpz_raii tmp;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &tmp, N1, size](piranha::integer &i) {
        random_mpz(tmp, N1, rng, size);
        i = piranha::integer(&tmp.m_mpz);
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &tmp, N2](piranha::integer &i) {
        random_mpz(tmp, N2, rng);
        i = piranha::integer(&tmp.m_mpz);
    });
    meter.measure([&arr1, &arr2, size]() {
        piranha::integer acc;
        for (unsigned j = 0u; j < size; ++j) {
            acc.multiply_accumulate(arr1[j], arr2[j]);
        }
    });
}

inline void saddmul_vec_piranha(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<piranha::integer, size> arr1, arr2, arr3;
    mppp::mpz_raii tmp;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &tmp, N1, &sdist](piranha::integer &i) {
        random_mpz(tmp, N1, rng);
        i = piranha::integer(&tmp.m_mpz);
        if (sdist(rng)) {
            i.negate();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &tmp, N2, &sdist](piranha::integer &i) {
        random_mpz(tmp, N2, rng);
        i = piranha::integer(&tmp.m_mpz);
        if (sdist(rng)) {
            i.negate();
        }
    });
    std::for_each(arr3.begin(), arr3.end(), [](piranha::integer &i) { i = piranha::integer(1); });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        auto arr3a(arr3);
        for (unsigned j = 0u; j < size; ++j) {
            arr3a[j].multiply_accumulate(arr1[j], arr2[j]);
        }
    });
}

inline void saddmul_acc_piranha(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<piranha::integer, size> arr1, arr2;
    mppp::mpz_raii tmp;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &tmp, N1, size, &sdist](piranha::integer &i) {
        random_mpz(tmp, N1, rng, size);
        i = piranha::integer(&tmp.m_mpz);
        if (sdist(rng)) {
            i.negate();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &tmp, N2, &sdist](piranha::integer &i) {
        random_mpz(tmp, N2, rng);
        i = piranha::integer(&tmp.m_mpz);
        if (sdist(rng)) {
            i.negate();
        }
    });
    meter.measure([&arr1, &arr2, size]() {
        piranha::integer acc;
        for (unsigned j = 0u; j < size; ++j) {
            acc.multiply_accumulate(arr1[j], arr2[j]);
        }
    });
}

// GMP.
inline void uadd_vec_gmp(nonius::chronometer meter, std::mt19937 &rng, unsigned N, unsigned M)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mpz_raii, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N](mppp::mpz_raii &m) { random_mpz(m, N, rng, 2); });
    std::for_each(arr2.begin(), arr2.end(), [&rng, M](mppp::mpz_raii &m) { random_mpz(m, M, rng, 2); });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (auto j = 0u; j < size; ++j) {
            ::mpz_add(&arr3[j].m_mpz, &arr1[j].m_mpz, &arr2[j].m_mpz);
        }
    });
}

inline void sadd_vec_gmp(nonius::chronometer meter, std::mt19937 &rng, unsigned N, unsigned M)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mpz_raii, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&sdist, &rng, N](mppp::mpz_raii &m) {
        random_mpz(m, N, rng, 2);
        if (sdist(rng)) {
            ::mpz_neg(&m.m_mpz, &m.m_mpz);
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&sdist, &rng, M](mppp::mpz_raii &m) {
        random_mpz(m, M, rng, 2);
        if (sdist(rng)) {
            ::mpz_neg(&m.m_mpz, &m.m_mpz);
        }
    });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (auto j = 0u; j < size; ++j) {
            ::mpz_add(&arr3[j].m_mpz, &arr1[j].m_mpz, &arr2[j].m_mpz);
        }
    });
}

inline void uacc_gmp(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mpz_raii, size> arr;
    std::for_each(arr.begin(), arr.end(), [&rng, N, size](mppp::mpz_raii &m) { random_mpz(m, N, rng, size); });
    mppp::mpz_raii c;
    meter.measure([&arr, size, &c]() {
        ::mpz_set(&c.m_mpz, &arr[0].m_mpz);
        for (unsigned j = 1u; j < size; ++j) {
            ::mpz_add(&c.m_mpz, &c.m_mpz, &arr[j].m_mpz);
        }
    });
}

inline void sacc_gmp(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mpz_raii, size> arr;
    std::for_each(arr.begin(), arr.end(), [&sdist, &rng, N, size](mppp::mpz_raii &m) {
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

inline void umul_vec_gmp_half(nonius::chronometer meter, std::mt19937 &rng)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mpz_raii, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(),
                  [&rng](mppp::mpz_raii &m) { random_mpz(m, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2)); });
    std::for_each(arr2.begin(), arr2.end(),
                  [&rng](mppp::mpz_raii &m) { random_mpz(m, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2)); });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            ::mpz_mul(&arr3[j].m_mpz, &arr1[j].m_mpz, &arr2[j].m_mpz);
        }
    });
}

inline void smul_vec_gmp_half(nonius::chronometer meter, std::mt19937 &rng)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mpz_raii, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &sdist](mppp::mpz_raii &m) {
        random_mpz(m, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        if (sdist(rng)) {
            ::mpz_neg(&m.m_mpz, &m.m_mpz);
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &sdist](mppp::mpz_raii &m) {
        random_mpz(m, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        if (sdist(rng)) {
            ::mpz_neg(&m.m_mpz, &m.m_mpz);
        }
    });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            ::mpz_mul(&arr3[j].m_mpz, &arr1[j].m_mpz, &arr2[j].m_mpz);
        }
    });
}

inline void umul_vec_gmp(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mpz_raii, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N1](mppp::mpz_raii &m) { random_mpz(m, N1, rng); });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2](mppp::mpz_raii &m) { random_mpz(m, N2, rng); });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            ::mpz_mul(&arr3[j].m_mpz, &arr1[j].m_mpz, &arr2[j].m_mpz);
        }
    });
}

inline void smul_vec_gmp(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    std::uniform_int_distribution<int> sdist(0, 1);
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mpz_raii, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N1, &sdist](mppp::mpz_raii &m) {
        random_mpz(m, N1, rng);
        if (sdist(rng)) {
            ::mpz_neg(&m.m_mpz, &m.m_mpz);
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2, &sdist](mppp::mpz_raii &m) {
        random_mpz(m, N2, rng);
        if (sdist(rng)) {
            ::mpz_neg(&m.m_mpz, &m.m_mpz);
        }
    });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            ::mpz_mul(&arr3[j].m_mpz, &arr1[j].m_mpz, &arr2[j].m_mpz);
        }
    });
}
}

#undef MPPP_BENCHMARK_VEC_SIZE

#endif
