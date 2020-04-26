// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <cstddef>
#include <ostream>
#include <type_traits>

#include <mp++/complex.hpp>
#include <mp++/config.hpp>
#include <mp++/detail/mpc.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/utils.hpp>

namespace mppp
{

namespace detail
{

namespace
{

// Some misc tests to check that the mpc struct conforms to our expectations.
struct expected_mpc_struct_t {
    ::mpfr_t re;
    ::mpfr_t im;
};

static_assert(sizeof(expected_mpc_struct_t) == sizeof(mpc_struct_t) && offsetof(mpc_struct_t, re) == 0u
                  && offsetof(mpc_struct_t, im) == offsetof(expected_mpc_struct_t, im),
              "Invalid mpc_t struct layout and/or MPC types.");

#if MPPP_CPLUSPLUS >= 201703L

// If we have C++17, we can use structured bindings to test the layout of mpc_struct_t
// and its members' types.
constexpr bool test_mpc_struct_t()
{
    auto [re, im] = mpc_struct_t{};

    static_assert(std::is_same<decltype(re), ::mpfr_t>::value);
    static_assert(std::is_same<decltype(im), ::mpfr_t>::value);

    ignore(re, im);

    return true;
}

static_assert(test_mpc_struct_t());

#endif

} // namespace

} // namespace detail

complex::complex()
{
    ::mpc_init2(&m_mpc, real_prec_min());
    ::mpfr_set_zero(mpc_realref(&m_mpc), 1);
    ::mpfr_set_zero(mpc_imagref(&m_mpc), 1);
}

complex::complex(const complex &other) : complex(&other.m_mpc) {}

complex::~complex()
{
    if (mpc_realref(&m_mpc)->_mpfr_d) {
        // The object is not moved-from, destroy it.

        // Check that the imaginary part is also
        // valid.
        assert(mpc_imagref(&m_mpc)->_mpfr_d);
        // Check that the precision of the imaginary
        // part is equal to the precision of the real
        // part.
        assert(get_prec() == mpfr_get_prec(mpc_imagref(&m_mpc)));
        // Check that the precision value is valid.
        assert(detail::real_prec_check(get_prec()));

        ::mpc_clear(&m_mpc);
    }
}

complex::complex(const complex &other, ::mpfr_prec_t p)
{
    // Init with custom precision, and then set.
    ::mpc_init2(&m_mpc, check_init_prec(p));
    ::mpc_set(&m_mpc, &other.m_mpc, MPC_RNDNN);
}

complex::complex(const ::mpc_t c)
{
    // Init with the same precision as other, and then set.
    ::mpc_init2(&m_mpc, mpfr_get_prec(mpc_realref(c)));
    ::mpc_set(&m_mpc, c, MPC_RNDNN);
}

// Copy assignment operator.
complex &complex::operator=(const complex &other)
{
    if (mppp_likely(this != &other)) {
        if (is_valid()) {
            // this has not been moved-from.
            // Copy the precision. This will also reset the internal value.
            // No need for prec checking as we assume other has a valid prec.
            set_prec_impl<false>(other.get_prec());
        } else {
            // this has been moved-from: init before setting.
            ::mpc_init2(&m_mpc, other.get_prec());
        }
        // Perform the actual copy from other.
        ::mpc_set(&m_mpc, &other.m_mpc, MPC_RNDNN);
    }
    return *this;
}

// TODO implement on top of to_string().
std::ostream &operator<<(std::ostream &os, const complex &c)
{
    complex::const_re_extractor rex{c};
    complex::const_im_extractor iex{c};

    os << '(' << rex.get().to_string() << ',' << iex.get().to_string() << ')';

    return os;
}

} // namespace mppp
