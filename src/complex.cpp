// Copyright 2016-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <cassert>
#include <cstddef>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#if defined(MPPP_HAVE_STRING_VIEW)
#include <string_view>
#endif

#include <mp++/complex.hpp>
#include <mp++/detail/mpc.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/detail/parse_complex.hpp>
#include <mp++/detail/utils.hpp>
#include <mp++/real.hpp>

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

// Double check that complex is a standard layout class.
static_assert(std::is_standard_layout<complex>::value, "complex is not a standard layout class.");

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

complex::complex(const complex &other, ::mpfr_prec_t p)
{
    // Init with custom precision, and then set.
    ::mpc_init2(&m_mpc, check_init_prec(p));
    ::mpc_set(&m_mpc, &other.m_mpc, MPC_RNDNN);
}

complex::complex(complex &&other, ::mpfr_prec_t p)
{
    // Check the precision first of all.
    check_init_prec(p);

    // Shallow copy other.
    m_mpc = other.m_mpc;
    // Mark the other as moved-from.
    other.m_mpc.re->_mpfr_d = nullptr;

    // Apply the precision.
    prec_round_impl<false>(p);
}

// Various helpers and constructors from string-like entities.
void complex::construct_from_c_string(const char *s, int base, ::mpfr_prec_t p)
{
    if (mppp_unlikely(base && (base < 2 || base > 62))) {
        throw std::invalid_argument("Cannot construct a complex from a string in base " + detail::to_string(base)
                                    + ": the base must either be zero or in the [2,62] range");
    }

    // Try parsing the real and imaginary parts.
    const auto res = detail::parse_complex(s);

    // Construct the real part.
    real re{res[0], res[1], base, p};

    // The imaginary part might not be present.
    auto im = (res[2] == nullptr) ? real{real_kind::zero, 1, p} : real{res[2], res[3], base, p};

    // Shallow-copy into this.
    m_mpc.re[0] = *re.get_mpfr_t();
    m_mpc.im[0] = *im.get_mpfr_t();

    // Deactivate the temporaries.
    re._get_mpfr_t()->_mpfr_d = nullptr;
    im._get_mpfr_t()->_mpfr_d = nullptr;
}

complex::complex(const stag &, const char *s, int base, ::mpfr_prec_t p)
{
    construct_from_c_string(s, base, p);
}

complex::complex(const stag &, const std::string &s, int base, ::mpfr_prec_t p) : complex(s.c_str(), base, p) {}

#if defined(MPPP_HAVE_STRING_VIEW)
complex::complex(const stag &, const std::string_view &s, int base, ::mpfr_prec_t p)
    : complex(s.data(), s.data() + s.size(), base, p)
{
}
#endif

// Constructor from range of characters, base and precision.
complex::complex(const char *begin, const char *end, int base, ::mpfr_prec_t p)
{
    MPPP_MAYBE_TLS std::vector<char> buffer;
    buffer.assign(begin, end);
    buffer.emplace_back('\0');
    construct_from_c_string(buffer.data(), base, p);
}

// Constructor from range of characters and precision.
complex::complex(const char *begin, const char *end, ::mpfr_prec_t p) : complex(begin, end, 10, p) {}

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

// Copy assignment from mpc_t.
complex &complex::operator=(const ::mpc_t c)
{
    // Set the precision, assuming the prec of c is valid.
    assert(mpfr_get_prec(mpc_realref(c)) == mpfr_get_prec(mpc_imagref(c)));
    set_prec_impl<false>(mpfr_get_prec(mpc_realref(c)));
    // Set the value.
    ::mpc_set(&m_mpc, c, MPC_RNDNN);
    return *this;
}

#if !defined(_MSC_VER) || defined(__clang__)

// Move assignment from mpc_t.
complex &complex::operator=(::mpc_t &&c)
{
    // Clear this.
    ::mpc_clear(&m_mpc);
    // Shallow copy c.
    m_mpc = *c;
    return *this;
}

#endif

complex &complex::set(const complex &c)
{
    return set(&c.m_mpc);
}

complex &complex::set(const ::mpc_t c)
{
    ::mpc_set(&m_mpc, c, MPC_RNDNN);
    return *this;
}

// TODO implement on top of to_string().
std::ostream &operator<<(std::ostream &os, const complex &c)
{
    complex::re_cref rex{c};
    complex::im_cref iex{c};

    os << '(' << rex->to_string() << ',' << iex->to_string() << ')';

    return os;
}

namespace detail
{

bool dispatch_complex_equality(const complex &c1, const complex &c2)
{
    complex::re_cref re1{c1}, re2{c2};
    complex::im_cref im1{c1}, im2{c2};

    return *re1 == *re2 && *im1 == *im2;
}

} // namespace detail

} // namespace mppp
