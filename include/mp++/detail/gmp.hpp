// Copyright 2016-2017 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MPPP_DETAIL_GMP_HPP
#define MPPP_DETAIL_GMP_HPP

#include <cassert>
#include <gmp.h>
#include <type_traits>
#include <utility>

#include <mp++/detail/type_traits.hpp>

#if __GNU_MP_VERSION < 5

#error Minimum supported GMP version is 5.

#endif

namespace mppp
{

inline namespace detail
{

// mpz_t is an array of some struct.
using mpz_struct_t = remove_extent_t<::mpz_t>;
// Integral types used for allocation size and number of limbs.
using mpz_alloc_t = decltype(std::declval<mpz_struct_t>()._mp_alloc);
using mpz_size_t = decltype(std::declval<mpz_struct_t>()._mp_size);

// Simple RAII holder for GMP integers.
struct mpz_raii {
    mpz_raii()
    {
        ::mpz_init(&m_mpz);
        assert(m_mpz._mp_alloc >= 0);
    }
    mpz_raii(const mpz_raii &) = delete;
    mpz_raii(mpz_raii &&) = delete;
    mpz_raii &operator=(const mpz_raii &) = delete;
    mpz_raii &operator=(mpz_raii &&) = delete;
    ~mpz_raii()
    {
        // NOTE: even in recent GMP versions, with lazy allocation,
        // it seems like the pointer always points to something:
        // https://gmplib.org/repo/gmp/file/835f8974ff6e/mpz/init.c
        assert(m_mpz._mp_d != nullptr);
        ::mpz_clear(&m_mpz);
    }
    mpz_struct_t m_mpz;
};

// mpq_t is an array of some struct.
using mpq_struct_t = remove_extent_t<::mpq_t>;

// Simple RAII holder for GMP rationals.
struct mpq_raii {
    mpq_raii()
    {
        ::mpq_init(&m_mpq);
    }
    mpq_raii(const mpq_raii &) = delete;
    mpq_raii(mpq_raii &&) = delete;
    mpq_raii &operator=(const mpq_raii &) = delete;
    mpq_raii &operator=(mpq_raii &&) = delete;
    ~mpq_raii()
    {
        ::mpq_clear(&m_mpq);
    }
    mpq_struct_t m_mpq;
};

// mpf_t is an array of some struct.
using mpf_struct_t = remove_extent_t<::mpf_t>;

// Simple RAII holder for GMP floats.
struct mpf_raii {
    explicit mpf_raii(::mp_bitcnt_t prec)
    {
        ::mpf_init2(&m_mpf, prec);
    }
    mpf_raii(const mpf_raii &) = delete;
    mpf_raii(mpf_raii &&) = delete;
    mpf_raii &operator=(const mpf_raii &) = delete;
    mpf_raii &operator=(mpf_raii &&) = delete;
    ~mpf_raii()
    {
        ::mpf_clear(&m_mpf);
    }
    mpf_struct_t m_mpf;
};
}
}

#endif
