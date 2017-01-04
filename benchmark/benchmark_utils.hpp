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

#ifndef MPPP_BENCHMARK_UTILS_HPP
#define MPPP_BENCHMARK_UTILS_HPP

#include <algorithm>
#include <array>
#include <boost/multiprecision/cpp_int.hpp>
#include <cstdlib>
#include <gmp.h>
#include <limits>
#include <locale>
#include <mp++.hpp>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>

#if defined(MPPP_BENCHMARK_FLINT)
#include <flint/fmpz.h>
#endif

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

// Various test functions.

template <typename T, typename = void>
struct ctor_dist_type {
    using type = std::uniform_int_distribution<T>;
};

template <typename T>
struct ctor_dist_type<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
    using type = std::uniform_real_distribution<T>;
};

// Bench for construction/destruction from primitive types.
template <typename T, typename Integer>
inline void bench_ctor(nonius::chronometer meter, std::mt19937 &rng, T min = std::numeric_limits<T>::min(),
                       T max = std::numeric_limits<T>::max())
{
    constexpr unsigned vsize = 1000u;
    typename ctor_dist_type<T>::type dist(min, max);
    auto ptr = static_cast<Integer *>(std::malloc(sizeof(Integer) * vsize));
    std::array<T, vsize> randoms;
    std::generate(randoms.begin(), randoms.end(), [&dist, &rng]() { return dist(rng); });
    meter.measure([ptr, &randoms]() {
        for (auto i = 0u; i < vsize; ++i) {
            ::new (ptr + i) Integer(randoms[i]);
        }
        for (auto i = 0u; i < vsize; ++i) {
            (ptr + i)->~Integer();
        }
    });
    std::free(ptr);
}

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
            i.neg();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &sdist, M](Integer &i) {
        random_integer(i, M, rng, 2);
        if (sdist(rng)) {
            i.neg();
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
            i.neg();
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
            i.neg();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &sdist](Integer &i) {
        random_integer(i, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        if (sdist(rng)) {
            i.neg();
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
            i.neg();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2, &sdist](Integer &i) {
        random_integer(i, N2, rng);
        if (sdist(rng)) {
            i.neg();
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
            i.neg();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &sdist](Integer &i) {
        random_integer(i, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        if (sdist(rng)) {
            i.neg();
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
            i.neg();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &sdist](Integer &i) {
        random_integer(i, 1u, rng, ::mp_limb_t(1) << ((GMP_NUMB_BITS * 2) / 3));
        if (sdist(rng)) {
            i.neg();
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
            i.neg();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2, &sdist](Integer &i) {
        random_integer(i, N2, rng);
        if (sdist(rng)) {
            i.neg();
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
            i.neg();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2, &sdist](Integer &i) {
        random_integer(i, N2, rng);
        if (sdist(rng)) {
            i.neg();
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
inline void udiv_vec_mppp(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2, arr3, arr4;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N1](Integer &i) { random_integer(i, N1, rng); });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2](Integer &i) {
        do {
            random_integer(i, N2, rng);
        } while (i.sign() == 0);
    });
    meter.measure([&arr1, &arr2, &arr3, &arr4, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            tdiv_qr(arr3[j], arr4[j], arr1[j], arr2[j]);
        }
    });
}

template <typename Integer>
inline void ulshift_vec_mppp_half(nonius::chronometer meter, std::mt19937 &rng)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2;
    std::array<::mp_bitcnt_t, size> barr;
    std::for_each(arr1.begin(), arr1.end(),
                  [&rng](Integer &i) { random_integer(i, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2)); });
    std::for_each(barr.begin(), barr.end(), [&rng](::mp_bitcnt_t &s) {
        s = std::uniform_int_distribution<::mp_bitcnt_t>(0u, GMP_NUMB_BITS / 2 - 1u)(rng);
    });
    meter.measure([&arr1, &arr2, &barr, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            mul_2exp(arr2[j], arr1[j], barr[j]);
        }
    });
}

template <typename Integer>
inline void ulshift_vec_mppp(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2;
    std::array<::mp_bitcnt_t, size> barr;
    std::for_each(barr.begin(), barr.end(), [&rng](::mp_bitcnt_t &s) {
        s = std::uniform_int_distribution<::mp_bitcnt_t>(0u, GMP_NUMB_BITS)(rng);
    });
    std::for_each(arr1.begin(), arr1.end(), [&rng, N](Integer &i) { random_integer(i, N, rng); });
    meter.measure([&arr1, &arr2, &barr, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            mul_2exp(arr2[j], arr1[j], barr[j]);
        }
    });
}

template <typename Integer>
inline void urshift_vec_mppp_half(nonius::chronometer meter, std::mt19937 &rng)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2;
    std::array<::mp_bitcnt_t, size> barr;
    std::for_each(arr1.begin(), arr1.end(),
                  [&rng](Integer &i) { random_integer(i, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2)); });
    std::for_each(barr.begin(), barr.end(), [&rng](::mp_bitcnt_t &s) {
        s = std::uniform_int_distribution<::mp_bitcnt_t>(0u, GMP_NUMB_BITS / 2 - 1u)(rng);
    });
    meter.measure([&arr1, &arr2, &barr, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            tdiv_q_2exp(arr2[j], arr1[j], barr[j]);
        }
    });
}

template <typename Integer>
inline void urshift_vec_mppp(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<Integer, size> arr1, arr2;
    std::array<::mp_bitcnt_t, size> barr;
    std::for_each(barr.begin(), barr.end(), [&rng](::mp_bitcnt_t &s) {
        s = std::uniform_int_distribution<::mp_bitcnt_t>(0u, GMP_NUMB_BITS)(rng);
    });
    std::for_each(arr1.begin(), arr1.end(), [&rng, N](Integer &i) { random_integer(i, N, rng); });
    meter.measure([&arr1, &arr2, &barr, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            tdiv_q_2exp(arr2[j], arr1[j], barr[j]);
        }
    });
}

template <typename Integer>
inline void cmp_vec_mppp(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::uniform_int_distribution<int> sdist(0, 1);
    std::array<Integer, size> arr1, arr2;
    std::array<volatile int, size> cmp_arr;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &sdist, N1](Integer &i) {
        random_integer(i, N1, rng);
        if (sdist(rng)) {
            i.neg();
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &sdist, N2](Integer &i) {
        random_integer(i, N2, rng);
        if (sdist(rng)) {
            i.neg();
        }
    });
    meter.measure([&arr1, &arr2, &cmp_arr, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            cmp_arr[j] = cmp(arr1[j], arr2[j]);
        }
    });
}

// GMP.
inline void uadd_vec_gmp(nonius::chronometer meter, std::mt19937 &rng, unsigned N, unsigned M)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mppp_impl::mpz_raii, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N](mppp::mppp_impl::mpz_raii &m) { random_mpz(m, N, rng, 2); });
    std::for_each(arr2.begin(), arr2.end(), [&rng, M](mppp::mppp_impl::mpz_raii &m) { random_mpz(m, M, rng, 2); });
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
    std::array<mppp::mppp_impl::mpz_raii, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&sdist, &rng, N](mppp::mppp_impl::mpz_raii &m) {
        random_mpz(m, N, rng, 2);
        if (sdist(rng)) {
            ::mpz_neg(&m.m_mpz, &m.m_mpz);
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&sdist, &rng, M](mppp::mppp_impl::mpz_raii &m) {
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
    std::array<mppp::mppp_impl::mpz_raii, size> arr;
    std::for_each(arr.begin(), arr.end(),
                  [&rng, N, size](mppp::mppp_impl::mpz_raii &m) { random_mpz(m, N, rng, size); });
    mppp::mppp_impl::mpz_raii c;
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
    std::array<mppp::mppp_impl::mpz_raii, size> arr;
    std::for_each(arr.begin(), arr.end(), [&sdist, &rng, N, size](mppp::mppp_impl::mpz_raii &m) {
        random_mpz(m, N, rng, size);
        if (sdist(rng)) {
            ::mpz_neg(&m.m_mpz, &m.m_mpz);
        }
    });
    mppp::mppp_impl::mpz_raii c;
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
    std::array<mppp::mppp_impl::mpz_raii, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng](mppp::mppp_impl::mpz_raii &m) {
        random_mpz(m, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng](mppp::mppp_impl::mpz_raii &m) {
        random_mpz(m, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
    });
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
    std::array<mppp::mppp_impl::mpz_raii, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &sdist](mppp::mppp_impl::mpz_raii &m) {
        random_mpz(m, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
        if (sdist(rng)) {
            ::mpz_neg(&m.m_mpz, &m.m_mpz);
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &sdist](mppp::mppp_impl::mpz_raii &m) {
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
    std::array<mppp::mppp_impl::mpz_raii, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N1](mppp::mppp_impl::mpz_raii &m) { random_mpz(m, N1, rng); });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2](mppp::mppp_impl::mpz_raii &m) { random_mpz(m, N2, rng); });
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
    std::array<mppp::mppp_impl::mpz_raii, size> arr1, arr2, arr3;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N1, &sdist](mppp::mppp_impl::mpz_raii &m) {
        random_mpz(m, N1, rng);
        if (sdist(rng)) {
            ::mpz_neg(&m.m_mpz, &m.m_mpz);
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2, &sdist](mppp::mppp_impl::mpz_raii &m) {
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

inline void udiv_vec_gmp(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mppp_impl::mpz_raii, size> arr1, arr2, arr3, arr4;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N1](mppp::mppp_impl::mpz_raii &m) { random_mpz(m, N1, rng); });
    std::for_each(arr2.begin(), arr2.end(), [&rng, N2](mppp::mppp_impl::mpz_raii &m) {
        do {
            random_mpz(m, N2, rng);
        } while (mpz_sgn(&m.m_mpz) == 0);
    });
    meter.measure([&arr1, &arr2, &arr3, &arr4, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            ::mpz_tdiv_qr(&arr3[j].m_mpz, &arr4[j].m_mpz, &arr1[j].m_mpz, &arr2[j].m_mpz);
        }
    });
}

inline void ulshift_vec_gmp_half(nonius::chronometer meter, std::mt19937 &rng)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mppp_impl::mpz_raii, size> arr1, arr2;
    std::array<::mp_bitcnt_t, size> barr;
    std::for_each(arr1.begin(), arr1.end(), [&rng](mppp::mppp_impl::mpz_raii &m) {
        random_mpz(m, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
    });
    std::for_each(barr.begin(), barr.end(), [&rng](::mp_bitcnt_t &s) {
        s = std::uniform_int_distribution<::mp_bitcnt_t>(0u, GMP_NUMB_BITS / 2 - 1u)(rng);
    });
    meter.measure([&arr1, &arr2, &barr, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            ::mpz_mul_2exp(&arr2[j].m_mpz, &arr1[j].m_mpz, barr[j]);
        }
    });
}

inline void ulshift_vec_gmp(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mppp_impl::mpz_raii, size> arr1, arr2;
    std::array<::mp_bitcnt_t, size> barr;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N](mppp::mppp_impl::mpz_raii &m) { random_mpz(m, N, rng); });
    std::for_each(barr.begin(), barr.end(), [&rng](::mp_bitcnt_t &s) {
        s = std::uniform_int_distribution<::mp_bitcnt_t>(0u, GMP_NUMB_BITS)(rng);
    });
    meter.measure([&arr1, &arr2, &barr, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            ::mpz_mul_2exp(&arr2[j].m_mpz, &arr1[j].m_mpz, barr[j]);
        }
    });
}

inline void urshift_vec_gmp_half(nonius::chronometer meter, std::mt19937 &rng)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mppp_impl::mpz_raii, size> arr1, arr2;
    std::array<::mp_bitcnt_t, size> barr;
    std::for_each(arr1.begin(), arr1.end(), [&rng](mppp::mppp_impl::mpz_raii &m) {
        random_mpz(m, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2));
    });
    std::for_each(barr.begin(), barr.end(), [&rng](::mp_bitcnt_t &s) {
        s = std::uniform_int_distribution<::mp_bitcnt_t>(0u, GMP_NUMB_BITS / 2 - 1u)(rng);
    });
    meter.measure([&arr1, &arr2, &barr, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            ::mpz_tdiv_q_2exp(&arr2[j].m_mpz, &arr1[j].m_mpz, barr[j]);
        }
    });
}

inline void urshift_vec_gmp(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<mppp::mppp_impl::mpz_raii, size> arr1, arr2;
    std::array<::mp_bitcnt_t, size> barr;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N](mppp::mppp_impl::mpz_raii &m) { random_mpz(m, N, rng); });
    std::for_each(barr.begin(), barr.end(), [&rng](::mp_bitcnt_t &s) {
        s = std::uniform_int_distribution<::mp_bitcnt_t>(0u, GMP_NUMB_BITS)(rng);
    });
    meter.measure([&arr1, &arr2, &barr, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            ::mpz_tdiv_q_2exp(&arr2[j].m_mpz, &arr1[j].m_mpz, barr[j]);
        }
    });
}

inline void cmp_vec_gmp(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::uniform_int_distribution<int> sdist(0, 1);
    std::array<mppp::mppp_impl::mpz_raii, size> arr1, arr2;
    std::array<volatile int, size> cmp_arr;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &sdist, N1](mppp::mppp_impl::mpz_raii &m) {
        random_mpz(m, N1, rng);
        if (sdist(rng)) {
            ::mpz_neg(&m.m_mpz, &m.m_mpz);
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &sdist, N2](mppp::mppp_impl::mpz_raii &m) {
        random_mpz(m, N2, rng);
        if (sdist(rng)) {
            ::mpz_neg(&m.m_mpz, &m.m_mpz);
        }
    });
    meter.measure([&arr1, &arr2, &cmp_arr, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            cmp_arr[j] = ::mpz_cmp(&arr1[j].m_mpz, &arr2[j].m_mpz);
        }
    });
}

#if defined(MPPP_BENCHMARK_FLINT)

// Flint.
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

inline void uadd_vec_fmpz(nonius::chronometer meter, std::mt19937 &rng, unsigned N, unsigned M)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<fmpz_raii, size> arr1, arr2, arr3;
    mppp::mppp_impl::mpz_raii m;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N, &m](fmpz_raii &f) {
        random_mpz(m, N, rng, 8);
        ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, M, &m](fmpz_raii &f) {
        random_mpz(m, M, rng, 8);
        ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
    });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (auto j = 0u; j < size; ++j) {
            ::fmpz_add(arr3[j].m_fmpz, arr1[j].m_fmpz, arr2[j].m_fmpz);
        }
    });
}

inline void uacc_fmpz(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<fmpz_raii, size> arr;
    mppp::mppp_impl::mpz_raii m;
    std::for_each(arr.begin(), arr.end(), [&rng, N, size, &m](fmpz_raii &f) {
        random_mpz(m, N, rng, size * 8u);
        ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
    });
    fmpz_raii c;
    meter.measure([&arr, size, &c]() {
        ::fmpz_set(c.m_fmpz, arr[0].m_fmpz);
        for (unsigned j = 1u; j < size; ++j) {
            ::fmpz_add(c.m_fmpz, c.m_fmpz, arr[j].m_fmpz);
        }
    });
}

inline void umul_vec_fmpz_half(nonius::chronometer meter, std::mt19937 &rng)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<fmpz_raii, size> arr1, arr2, arr3;
    mppp::mppp_impl::mpz_raii m;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &m](fmpz_raii &f) {
        random_mpz(m, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2 - 2));
        ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &m](fmpz_raii &f) {
        random_mpz(m, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2 - 2));
        ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
    });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            ::fmpz_mul(arr3[j].m_fmpz, arr1[j].m_fmpz, arr2[j].m_fmpz);
        }
    });
}

inline void smul_vec_fmpz_half(nonius::chronometer meter, std::mt19937 &rng)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::uniform_int_distribution<int> sdist(0, 1);
    std::array<fmpz_raii, size> arr1, arr2, arr3;
    mppp::mppp_impl::mpz_raii m;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &m, &sdist](fmpz_raii &f) {
        random_mpz(m, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2 - 2));
        if (sdist(rng)) {
            ::mpz_neg(&m.m_mpz, &m.m_mpz);
        }
        ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &m, &sdist](fmpz_raii &f) {
        random_mpz(m, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2 - 2));
        if (sdist(rng)) {
            ::mpz_neg(&m.m_mpz, &m.m_mpz);
        }
        ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
    });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            ::fmpz_mul(arr3[j].m_fmpz, arr1[j].m_fmpz, arr2[j].m_fmpz);
        }
    });
}

inline void udiv_vec_fmpz(nonius::chronometer meter, std::mt19937 &rng, unsigned N, unsigned M)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<fmpz_raii, size> arr1, arr2, arr3, arr4;
    mppp::mppp_impl::mpz_raii m;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N, &m](fmpz_raii &f) {
        random_mpz(m, N, rng, 8);
        ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, M, &m](fmpz_raii &f) {
        do {
            random_mpz(m, M, rng, 8);
        } while (mpz_sgn(&m.m_mpz) == 0);
        ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
    });
    meter.measure([&arr1, &arr2, &arr3, &arr4, size]() {
        for (auto j = 0u; j < size; ++j) {
            ::fmpz_tdiv_qr(arr3[j].m_fmpz, arr4[j].m_fmpz, arr1[j].m_fmpz, arr2[j].m_fmpz);
        }
    });
}

inline void ulshift_vec_fmpz_half(nonius::chronometer meter, std::mt19937 &rng)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<fmpz_raii, size> arr1, arr2;
    std::array<::mp_bitcnt_t, size> barr;
    mppp::mppp_impl::mpz_raii m;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &m](fmpz_raii &f) {
        random_mpz(m, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2 - 2));
        ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
    });
    std::for_each(barr.begin(), barr.end(), [&rng](::mp_bitcnt_t &s) {
        s = std::uniform_int_distribution<::mp_bitcnt_t>(0u, GMP_NUMB_BITS / 2 - 1u)(rng);
    });
    meter.measure([&arr1, &arr2, &barr, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            ::fmpz_mul_2exp(arr2[j].m_fmpz, arr1[j].m_fmpz, barr[j]);
        }
    });
}

inline void ulshift_vec_fmpz(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<fmpz_raii, size> arr1, arr2;
    std::array<::mp_bitcnt_t, size> barr;
    mppp::mppp_impl::mpz_raii m;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &m, N](fmpz_raii &f) {
        random_mpz(m, N, rng);
        ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
    });
    std::for_each(barr.begin(), barr.end(), [&rng](::mp_bitcnt_t &s) {
        s = std::uniform_int_distribution<::mp_bitcnt_t>(0u, GMP_NUMB_BITS)(rng);
    });
    meter.measure([&arr1, &arr2, &barr, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            ::fmpz_mul_2exp(arr2[j].m_fmpz, arr1[j].m_fmpz, barr[j]);
        }
    });
}

inline void urshift_vec_fmpz_half(nonius::chronometer meter, std::mt19937 &rng)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<fmpz_raii, size> arr1, arr2;
    std::array<::mp_bitcnt_t, size> barr;
    mppp::mppp_impl::mpz_raii m;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &m](fmpz_raii &f) {
        random_mpz(m, 1u, rng, ::mp_limb_t(1) << (GMP_NUMB_BITS / 2 - 2));
        ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
    });
    std::for_each(barr.begin(), barr.end(), [&rng](::mp_bitcnt_t &s) {
        s = std::uniform_int_distribution<::mp_bitcnt_t>(0u, GMP_NUMB_BITS / 2 - 1u)(rng);
    });
    meter.measure([&arr1, &arr2, &barr, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            ::fmpz_tdiv_q_2exp(arr2[j].m_fmpz, arr1[j].m_fmpz, barr[j]);
        }
    });
}

inline void urshift_vec_fmpz(nonius::chronometer meter, std::mt19937 &rng, unsigned N)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<fmpz_raii, size> arr1, arr2;
    std::array<::mp_bitcnt_t, size> barr;
    mppp::mppp_impl::mpz_raii m;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &m, N](fmpz_raii &f) {
        random_mpz(m, N, rng);
        ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
    });
    std::for_each(barr.begin(), barr.end(), [&rng](::mp_bitcnt_t &s) {
        s = std::uniform_int_distribution<::mp_bitcnt_t>(0u, GMP_NUMB_BITS)(rng);
    });
    meter.measure([&arr1, &arr2, &barr, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            ::fmpz_tdiv_q_2exp(arr2[j].m_fmpz, arr1[j].m_fmpz, barr[j]);
        }
    });
}

inline void cmp_vec_fmpz(nonius::chronometer meter, std::mt19937 &rng, unsigned N1, unsigned N2)
{
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::uniform_int_distribution<int> sdist(0, 1);
    std::array<fmpz_raii, size> arr1, arr2;
    std::array<volatile int, size> cmp_arr;
    mppp::mppp_impl::mpz_raii m;
    std::for_each(arr1.begin(), arr1.end(), [&rng, &sdist, &m, N1](fmpz_raii &f) {
        random_mpz(m, N1, rng);
        ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
        if (sdist(rng)) {
            ::fmpz_neg(f.m_fmpz, f.m_fmpz);
        }
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, &sdist, &m, N2](fmpz_raii &f) {
        random_mpz(m, N2, rng);
        ::fmpz_set_str(f.m_fmpz, lex_cast(m).c_str(), 10);
        if (sdist(rng)) {
            ::fmpz_neg(f.m_fmpz, f.m_fmpz);
        }
    });
    meter.measure([&arr1, &arr2, &cmp_arr, size]() {
        for (unsigned j = 0u; j < size; ++j) {
            cmp_arr[j] = ::fmpz_cmp(arr1[j].m_fmpz, arr2[j].m_fmpz);
        }
    });
}

#endif

// BMP.
inline void uadd_vec_cpp_int(nonius::chronometer meter, std::mt19937 &rng, unsigned N, unsigned M)
{
    using boost::multiprecision::cpp_int;
    const unsigned size = MPPP_BENCHMARK_VEC_SIZE;
    std::array<cpp_int, size> arr1, arr2, arr3;
    mppp::mppp_impl::mpz_raii m;
    std::for_each(arr1.begin(), arr1.end(), [&rng, N, &m](cpp_int &n) {
        random_mpz(m, N, rng, 2);
        n = cpp_int{lex_cast(m)};
    });
    std::for_each(arr2.begin(), arr2.end(), [&rng, M, &m](cpp_int &n) {
        random_mpz(m, M, rng, 2);
        n = cpp_int{lex_cast(m)};
    });
    meter.measure([&arr1, &arr2, &arr3, size]() {
        for (auto j = 0u; j < size; ++j) {
            arr3[j] = arr1[j] + arr2[j];
        }
    });
}
}

#undef MPPP_BENCHMARK_VEC_SIZE

#endif
