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
#include <utility>
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

// Init a complex with precision p, setting its value to (nan,nan). No precision
// checking is performed.
complex::complex(const ptag &, ::mpfr_prec_t p, bool ignore_prec)
{
    assert(ignore_prec);
    assert(detail::real_prec_check(p));
    detail::ignore(ignore_prec);
    ::mpc_init2(&m_mpc, p);
}

complex::complex(const complex &other) : complex(&other.m_mpc) {}

complex::complex(const complex &other, complex_prec_t p)
{
    // Init with custom precision, and then set.
    ::mpc_init2(&m_mpc, check_init_prec(static_cast<::mpfr_prec_t>(p)));
    ::mpc_set(&m_mpc, &other.m_mpc, MPC_RNDNN);
}

complex::complex(complex &&other, complex_prec_t p_)
{
    const auto p = static_cast<::mpfr_prec_t>(p_);

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

complex::complex(const stag &, const std::string &s, int base, ::mpfr_prec_t p)
    : complex(s.c_str(), base, complex_prec_t(p))
{
}

#if defined(MPPP_HAVE_STRING_VIEW)
complex::complex(const stag &, const std::string_view &s, int base, ::mpfr_prec_t p)
    : complex(s.data(), s.data() + s.size(), base, complex_prec_t(p))
{
}
#endif

// Constructor from range of characters, base and precision.
complex::complex(const char *begin, const char *end, int base, complex_prec_t p)
{
    MPPP_MAYBE_TLS std::vector<char> buffer;
    buffer.assign(begin, end);
    buffer.emplace_back('\0');
    construct_from_c_string(buffer.data(), base, static_cast<::mpfr_prec_t>(p));
}

// Constructor from range of characters and precision.
complex::complex(const char *begin, const char *end, complex_prec_t p) : complex(begin, end, 10, p) {}

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

complex &complex::set_nan()
{
    ::mpc_set_nan(&m_mpc);

    return *this;
}

// Implementation of the assignment from string.
void complex::string_assignment_impl(const char *s, int base)
{
    if (mppp_unlikely(base && (base < 2 || base > 62))) {
        throw std::invalid_argument("Cannot assign a complex from a string in base " + detail::to_string(base)
                                    + ": the base must either be zero or in the [2,62] range");
    }

    re_ref re{*this};
    im_ref im{*this};

    try {
        // Try parsing the real and imaginary parts.
        const auto res = detail::parse_complex(s);

        // Set the real part.
        re->set(res[0], res[1], base);

        // Set the imaginary part, if present.
        if (res[2] == nullptr) {
            im->set_zero(1);
        } else {
            im->set(res[2], res[3], base);
        }
    } catch (...) {
        // In case of errors, make sure we set this to (nan,nan)
        // before re-throwing.
        re->set_nan();
        im->set_nan();

        throw;
    }
}

// Implementation of string setters.
complex &complex::set_impl(const char *s, int base)
{
    string_assignment_impl(s, base);
    return *this;
}

complex &complex::set_impl(const std::string &s, int base)
{
    return set(s.c_str(), base);
}

#if defined(MPPP_HAVE_STRING_VIEW)
complex &complex::set_impl(const std::string_view &s, int base)
{
    return set(s.data(), s.data() + s.size(), base);
}
#endif

// Set to character range.
complex &complex::set(const char *begin, const char *end, int base)
{
    MPPP_MAYBE_TLS std::vector<char> buffer;
    buffer.assign(begin, end);
    buffer.emplace_back('\0');
    return set(buffer.data(), base);
}

// Detect one.
bool complex::is_one() const
{
    re_cref re{*this};
    im_cref im{*this};

    return im->zero_p() && re->is_one();
}

// Destructively set the precision.
complex &complex::set_prec(::mpfr_prec_t p)
{
    set_prec_impl<true>(p);
    return *this;
}

// Set the precision maintaining the current value.
complex &complex::prec_round(::mpfr_prec_t p)
{
    prec_round_impl<true>(p);
    return *this;
}

std::string complex::to_string(int base) const
{
    re_cref re{*this};
    im_cref im{*this};

    return '(' + re->to_string(base) + ',' + im->to_string(base) + ')';
}

int cmp_abs(const complex &c1, const complex &c2)
{
    ::mpfr_clear_erangeflag();
    auto retval = ::mpc_cmp_abs(c1.get_mpc_t(), c2.get_mpc_t());
    if (mppp_unlikely(::mpfr_erangeflag_p())) {
        ::mpfr_clear_erangeflag();
        throw std::domain_error(
            "Cannot compare the absolute values of two complex numbers if there are NaNs in the real/imaginary parts");
    }
    return retval;
}

std::ostream &operator<<(std::ostream &os, const complex &c)
{
    return os << c.to_string();
}

complex &operator++(complex &c)
{
    if (mppp_unlikely(c.get_prec() < detail::real_deduce_precision(1))) {
        c.prec_round(detail::real_deduce_precision(1));
    }
    ::mpc_add_ui(c._get_mpc_t(), c.get_mpc_t(), 1ul, MPC_RNDNN);
    return c;
}

complex operator++(complex &c, int)
{
    auto retval(c);
    ++c;
    return retval;
}

complex &operator--(complex &c)
{
    if (mppp_unlikely(c.get_prec() < detail::real_deduce_precision(1))) {
        c.prec_round(detail::real_deduce_precision(1));
    }
    ::mpc_sub_ui(c._get_mpc_t(), c.get_mpc_t(), 1ul, MPC_RNDNN);
    return c;
}

complex operator--(complex &c, int)
{
    auto retval(c);
    --c;
    return retval;
}

namespace detail
{

void dispatch_complex_in_place_add(complex &a, const real &x)
{
    auto wrapper = [&x](::mpc_t c, const ::mpc_t o) { ::mpc_add_fr(c, o, x.get_mpfr_t(), MPC_RNDNN); };

    mpc_nary_op_impl<false>(x.get_prec(), wrapper, a, a);
}

void dispatch_complex_in_place_add(complex &a, bool n)
{
    auto wrapper = [n](::mpc_t c, const ::mpc_t o) { ::mpc_add_ui(c, o, static_cast<unsigned long>(n), MPC_RNDNN); };

    mpc_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
}

void dispatch_complex_in_place_sub(complex &a, const real &x)
{
    auto wrapper = [&x](::mpc_t c, const ::mpc_t o) { ::mpc_sub_fr(c, o, x.get_mpfr_t(), MPC_RNDNN); };

    mpc_nary_op_impl<false>(x.get_prec(), wrapper, a, a);
}

void dispatch_complex_in_place_sub(complex &a, bool n)
{
    auto wrapper = [n](::mpc_t c, const ::mpc_t o) { ::mpc_sub_ui(c, o, static_cast<unsigned long>(n), MPC_RNDNN); };

    mpc_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
}

void dispatch_complex_in_place_mul(complex &a, const real &x)
{
    auto wrapper = [&x](::mpc_t c, const ::mpc_t o) { ::mpc_mul_fr(c, o, x.get_mpfr_t(), MPC_RNDNN); };

    mpc_nary_op_impl<false>(x.get_prec(), wrapper, a, a);
}

void dispatch_complex_in_place_mul(complex &a, bool n)
{
    auto wrapper = [n](::mpc_t c, const ::mpc_t o) { ::mpc_mul_ui(c, o, static_cast<unsigned long>(n), MPC_RNDNN); };

    mpc_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
}

void dispatch_complex_in_place_div(complex &a, const real &x)
{
    auto wrapper = [&x](::mpc_t c, const ::mpc_t o) { ::mpc_div_fr(c, o, x.get_mpfr_t(), MPC_RNDNN); };

    mpc_nary_op_impl<false>(x.get_prec(), wrapper, a, a);
}

void dispatch_complex_in_place_div(complex &a, bool n)
{
    auto wrapper = [n](::mpc_t c, const ::mpc_t o) { ::mpc_div_ui(c, o, static_cast<unsigned long>(n), MPC_RNDNN); };

    mpc_nary_op_impl<false>(real_deduce_precision(n), wrapper, a, a);
}

bool dispatch_complex_equality(const complex &c1, const complex &c2)
{
    // NOTE: don't use mpc_cmp() as it does not handle NaNs properly.
    complex::re_cref re1{c1}, re2{c2};
    complex::im_cref im1{c1}, im2{c2};

    return *re1 == *re2 && *im1 == *im2;
}

} // namespace detail

// Wrapper to apply the input unary MPC function to this with
// MPC_RNDNN rounding mode. Returns a reference to this.
template <typename T>
complex &complex::self_mpc_unary(T &&f)
{
    std::forward<T>(f)(&m_mpc, &m_mpc, MPC_RNDNN);
    return *this;
}

// In-place negation.
complex &complex::neg()
{
    return self_mpc_unary(::mpc_neg);
}

// In-place conjugate.
complex &complex::conj()
{
    return self_mpc_unary(::mpc_conj);
}

// In-place absolute value.
complex &complex::abs()
{
    MPPP_MAYBE_TLS real tmp;

    tmp.set_prec(get_prec());
    ::mpc_abs(tmp._get_mpfr_t(), &m_mpc, MPFR_RNDN);

    ::mpfr_set(mpc_realref(&m_mpc), tmp.get_mpfr_t(), MPFR_RNDN);
    ::mpfr_set_zero(mpc_imagref(&m_mpc), 1);

    return *this;
}

// In-place norm.
complex &complex::norm()
{
    MPPP_MAYBE_TLS real tmp;

    tmp.set_prec(get_prec());
    ::mpc_norm(tmp._get_mpfr_t(), &m_mpc, MPFR_RNDN);

    ::mpfr_set(mpc_realref(&m_mpc), tmp.get_mpfr_t(), MPFR_RNDN);
    ::mpfr_set_zero(mpc_imagref(&m_mpc), 1);

    return *this;
}

// In-place arg.
complex &complex::arg()
{
    MPPP_MAYBE_TLS real tmp;

    tmp.set_prec(get_prec());
    ::mpc_arg(tmp._get_mpfr_t(), &m_mpc, MPFR_RNDN);

    ::mpfr_set(mpc_realref(&m_mpc), tmp.get_mpfr_t(), MPFR_RNDN);
    ::mpfr_set_zero(mpc_imagref(&m_mpc), 1);

    return *this;
}

// In-place proj.
complex &complex::proj()
{
    return self_mpc_unary(::mpc_proj);
}

// In-place squaring.
complex &complex::sqr()
{
    return self_mpc_unary(::mpc_sqr);
}

// In-place mul_i.
complex &complex::mul_i(int sgn)
{
    ::mpc_mul_i(&m_mpc, &m_mpc, sgn, MPC_RNDNN);
    return *this;
}

// Free-function abs.
real &abs(real &rop, const complex &c)
{
    rop.set_prec(c.get_prec());
    ::mpc_abs(rop._get_mpfr_t(), c.get_mpc_t(), MPFR_RNDN);
    return rop;
}

real abs(const complex &c)
{
    real ret{real_kind::nan, c.get_prec()};
    ::mpc_abs(ret._get_mpfr_t(), c.get_mpc_t(), MPFR_RNDN);
    return ret;
}

// Free-function norm.
real &norm(real &rop, const complex &c)
{
    rop.set_prec(c.get_prec());
    ::mpc_norm(rop._get_mpfr_t(), c.get_mpc_t(), MPFR_RNDN);
    return rop;
}

real norm(const complex &c)
{
    real ret{real_kind::nan, c.get_prec()};
    ::mpc_norm(ret._get_mpfr_t(), c.get_mpc_t(), MPFR_RNDN);
    return ret;
}

// Free-function arg.
real &arg(real &rop, const complex &c)
{
    rop.set_prec(c.get_prec());
    ::mpc_arg(rop._get_mpfr_t(), c.get_mpc_t(), MPFR_RNDN);
    return rop;
}

real arg(const complex &c)
{
    real ret{real_kind::nan, c.get_prec()};
    ::mpc_arg(ret._get_mpfr_t(), c.get_mpc_t(), MPFR_RNDN);
    return ret;
}

} // namespace mppp
