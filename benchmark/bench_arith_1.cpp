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

#include <mp++.hpp>
#include <random>

#include "utils.hpp"

using namespace mppp;
using namespace mppp_bench;

using integer = mp_integer<1>;

std::mt19937 rng;

BENCHMARK("1-limb unsigned addition", [](benchpress::context *ctx) { uadd1_mppp<integer>(ctx, rng); })
BENCHMARK("piranha 1-limb unsigned addition", [](benchpress::context *ctx) { uadd1_piranha(ctx, rng); })
BENCHMARK("gmp 1-limb unsigned addition", [](benchpress::context *ctx) { uadd1_gmp(ctx, rng); })

BENCHMARK("1-limb unsigned vector addition", [](benchpress::context *ctx) { uadd1_vec_mppp<integer>(ctx, rng); })
BENCHMARK("piranha 1-limb unsigned vector addition", [](benchpress::context *ctx) { uadd1_vec_piranha(ctx, rng); })
BENCHMARK("gmp 1-limb unsigned vector addition", [](benchpress::context *ctx) { uadd1_vec_gmp(ctx, rng); })

BENCHMARK("1-limb signed addition", [](benchpress::context *ctx) { sadd1_mppp<integer>(ctx, rng); })
BENCHMARK("piranha 1-limb signed addition", [](benchpress::context *ctx) { sadd1_piranha(ctx, rng); })
BENCHMARK("gmp 1-limb signed addition", [](benchpress::context *ctx) { sadd1_gmp(ctx, rng); })

BENCHMARK("1-limb signed vector addition", [](benchpress::context *ctx) { sadd1_vec_mppp<integer>(ctx, rng); })
BENCHMARK("piranha 1-limb signed vector addition", [](benchpress::context *ctx) { sadd1_vec_piranha(ctx, rng); })
BENCHMARK("gmp 1-limb signed vector addition", [](benchpress::context *ctx) { sadd1_vec_gmp(ctx, rng); })

/*
BENCHMARK("1-limb signed addition", ([](benchpress::context *ctx) ))

BENCHMARK("piranha 1-limb signed addition", ([](benchpress::context *ctx) {
              std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                              std::numeric_limits<::mp_limb_t>::max());
              std::uniform_int_distribution<int> sdist(0, 1);
              auto a = piranha::integer((dist(rng) & GMP_NUMB_MASK) / 2u),
                   b = piranha::integer((dist(rng) & GMP_NUMB_MASK) / 2u);
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
          }))

BENCHMARK("mpz 1-limb signed addition", ([](benchpress::context *ctx) {
              std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                              std::numeric_limits<::mp_limb_t>::max());
              std::uniform_int_distribution<int> sdist(0, 1);
              mpz_raii a, b, c;
              const auto s1 = lex_cast((dist(rng) & GMP_NUMB_MASK) / 2u);
              const auto s2 = lex_cast((dist(rng) & GMP_NUMB_MASK) / 2u);
              ::mpz_set_str(&a.m_mpz, s1.data(), 10);
              ::mpz_set_str(&b.m_mpz, s2.data(), 10);
              if (sdist(rng)) {
                  ::mpz_neg(&a.m_mpz, &a.m_mpz);
              }
              if (sdist(rng)) {
                  ::mpz_neg(&b.m_mpz, &b.m_mpz);
              }
              ctx->reset_timer();
              for (std::size_t i = 0; i < ctx->num_iterations(); ++i) {
                  ::mpz_add(&c.m_mpz, &a.m_mpz, &b.m_mpz);
              }
          }))

BENCHMARK("2-limbs unsigned addition", ([](benchpress::context *ctx) {
              std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                              std::numeric_limits<::mp_limb_t>::max());
              auto a = integer(dist(rng) & GMP_NUMB_MASK), b = integer(dist(rng) & GMP_NUMB_MASK);
              integer c, d;
              add(c, a, integer{::mp_limb_t(-1) & GMP_NUMB_MASK});
              add(d, b, integer{::mp_limb_t(-1) & GMP_NUMB_MASK});
              integer e;
              ctx->reset_timer();
              for (std::size_t i = 0; i < ctx->num_iterations(); ++i) {
                  add(e, c, d);
              }
          }))

BENCHMARK("piranha 2-limbs unsigned addition", ([](benchpress::context *ctx) {
              std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                              std::numeric_limits<::mp_limb_t>::max());
              auto a = piranha::integer(dist(rng) & GMP_NUMB_MASK), b = piranha::integer(dist(rng) & GMP_NUMB_MASK);
              piranha::integer c, d;
              c.add(a, piranha::integer{::mp_limb_t(-1) & GMP_NUMB_MASK});
              d.add(b, piranha::integer{::mp_limb_t(-1) & GMP_NUMB_MASK});
              piranha::integer e;
              ctx->reset_timer();
              for (std::size_t i = 0; i < ctx->num_iterations(); ++i) {
                  e.add(c, d);
              }
          }))

BENCHMARK("mpz 2-limbs unsigned addition", ([](benchpress::context *ctx) {
              std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                              std::numeric_limits<::mp_limb_t>::max());
              mpz_raii a, b, c, d, e, f, g;
              const auto s1 = lex_cast(dist(rng) & GMP_NUMB_MASK);
              const auto s2 = lex_cast(dist(rng) & GMP_NUMB_MASK);
              ::mpz_set_str(&a.m_mpz, s1.data(), 10);
              ::mpz_set_str(&b.m_mpz, s2.data(), 10);
              const auto s3 = lex_cast(::mp_limb_t(-1) & GMP_NUMB_MASK);
              ::mpz_set_str(&c.m_mpz, s3.data(), 10);
              ::mpz_set_str(&d.m_mpz, s3.data(), 10);
              ::mpz_add(&e.m_mpz, &a.m_mpz, &c.m_mpz);
              ::mpz_add(&f.m_mpz, &b.m_mpz, &d.m_mpz);
              for (std::size_t i = 0; i < ctx->num_iterations(); ++i) {
                  ::mpz_add(&g.m_mpz, &e.m_mpz, &f.m_mpz);
              }
          }))

BENCHMARK("2-limbs signed addition", ([](benchpress::context *ctx) {
              std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                              std::numeric_limits<::mp_limb_t>::max());
              std::uniform_int_distribution<int> sdist(0, 1);
              auto a = integer(dist(rng) & GMP_NUMB_MASK), b = integer(dist(rng) & GMP_NUMB_MASK);
              integer c, d;
              add(c, a, integer{::mp_limb_t(-1) & GMP_NUMB_MASK});
              add(d, b, integer{::mp_limb_t(-1) & GMP_NUMB_MASK});
              if (sdist(rng)) {
                  c.negate();
              }
              if (sdist(rng)) {
                  d.negate();
              }
              integer e;
              ctx->reset_timer();
              for (std::size_t i = 0; i < ctx->num_iterations(); ++i) {
                  add(e, c, d);
              }
          }))

BENCHMARK("piranha 2-limbs signed addition", ([](benchpress::context *ctx) {
              std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                              std::numeric_limits<::mp_limb_t>::max());
              std::uniform_int_distribution<int> sdist(0, 1);
              auto a = piranha::integer(dist(rng) & GMP_NUMB_MASK), b = piranha::integer(dist(rng) & GMP_NUMB_MASK);
              piranha::integer c, d;
              c.add(a, piranha::integer{::mp_limb_t(-1) & GMP_NUMB_MASK});
              d.add(b, piranha::integer{::mp_limb_t(-1) & GMP_NUMB_MASK});
              if (sdist(rng)) {
                  c.negate();
              }
              if (sdist(rng)) {
                  d.negate();
              }
              piranha::integer e;
              ctx->reset_timer();
              for (std::size_t i = 0; i < ctx->num_iterations(); ++i) {
                  e.add(c, d);
              }
          }))

BENCHMARK("mpz 2-limbs signed addition", ([](benchpress::context *ctx) {
              std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                              std::numeric_limits<::mp_limb_t>::max());
              std::uniform_int_distribution<int> sdist(0, 1);
              mpz_raii a, b, c, d, e, f, g;
              const auto s1 = lex_cast(dist(rng) & GMP_NUMB_MASK);
              const auto s2 = lex_cast(dist(rng) & GMP_NUMB_MASK);
              ::mpz_set_str(&a.m_mpz, s1.data(), 10);
              ::mpz_set_str(&b.m_mpz, s2.data(), 10);
              const auto s3 = lex_cast(::mp_limb_t(-1) & GMP_NUMB_MASK);
              ::mpz_set_str(&c.m_mpz, s3.data(), 10);
              ::mpz_set_str(&d.m_mpz, s3.data(), 10);
              ::mpz_add(&e.m_mpz, &a.m_mpz, &c.m_mpz);
              ::mpz_add(&f.m_mpz, &b.m_mpz, &d.m_mpz);
              if (sdist(rng)) {
                  ::mpz_neg(&e.m_mpz, &e.m_mpz);
              }
              if (sdist(rng)) {
                  ::mpz_neg(&f.m_mpz, &f.m_mpz);
              }
              for (std::size_t i = 0; i < ctx->num_iterations(); ++i) {
                  ::mpz_add(&g.m_mpz, &e.m_mpz, &f.m_mpz);
              }
          }))
*/

/*NONIUS_BENCHMARK("1-limb unsigned mul", [](nonius::chronometer meter) {
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    auto a = integer((dist(rng) & GMP_NUMB_MASK)), b = integer((dist(rng) & GMP_NUMB_MASK));
    integer c;
    meter.measure([&a, &b, &c] { mul(c, a, b); });
});*/

/*
NONIUS_BENCHMARK("piranha 1-limb unsigned mult", [](nonius::chronometer meter) {
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    auto a = piranha::integer((dist(rng) & GMP_NUMB_MASK)), b = piranha::integer((dist(rng) & GMP_NUMB_MASK));
    piranha::integer c;
    meter.measure([&a, &b, &c] { c.mul(a, b); });
});

NONIUS_BENCHMARK("2-limbs unsigned addition", [](nonius::chronometer meter) {
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    auto a = integer(dist(rng) & GMP_NUMB_MASK), b = integer(dist(rng) & GMP_NUMB_MASK);
    integer c, d;
    add(c, a, integer{::mp_limb_t(-1) & GMP_NUMB_MASK});
    add(d, b, integer{::mp_limb_t(-1) & GMP_NUMB_MASK});
    integer e;
    meter.measure([&e, &c, &d] { add(e, c, d); });
});

NONIUS_BENCHMARK("piranha 2-limbs unsigned addition", [](nonius::chronometer meter) {
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    auto a = piranha::integer(dist(rng) & GMP_NUMB_MASK), b = piranha::integer(dist(rng) & GMP_NUMB_MASK);
    piranha::integer c, d;
    c.add(a, piranha::integer{::mp_limb_t(-1) & GMP_NUMB_MASK});
    d.add(b, piranha::integer{::mp_limb_t(-1) & GMP_NUMB_MASK});
    piranha::integer e;
    meter.measure([&e, &c, &d] { e.add(c, d); });
});

NONIUS_BENCHMARK("piranha 2-limbs unsigned mult", [](nonius::chronometer meter) {
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    auto a = piranha::integer(dist(rng) & GMP_NUMB_MASK), b = piranha::integer(dist(rng) & GMP_NUMB_MASK);
    piranha::integer c, d;
    c.add(a, piranha::integer{::mp_limb_t(-1) & GMP_NUMB_MASK});
    d.add(b, piranha::integer{::mp_limb_t(-1) & GMP_NUMB_MASK});
    piranha::integer e;
    meter.measure([&e, &c, &d] { e.mul(c, d); });
});*/

/*
NONIUS_BENCHMARK("mpz 2-limbs unsigned addition", [](nonius::chronometer meter) {
    std::uniform_int_distribution<::mp_limb_t> dist(std::numeric_limits<::mp_limb_t>::min(),
                                                    std::numeric_limits<::mp_limb_t>::max());
    mpz_raii a, b, c, d, e, f, g;
    const auto s1 = lex_cast(dist(rng) & GMP_NUMB_MASK);
    const auto s2 = lex_cast(dist(rng) & GMP_NUMB_MASK);
    ::mpz_set_str(&a.m_mpz, s1.data(), 10);
    ::mpz_set_str(&b.m_mpz, s2.data(), 10);
    const auto s3 = lex_cast(::mp_limb_t(-1) & GMP_NUMB_MASK);
    ::mpz_set_str(&c.m_mpz, s3.data(), 10);
    ::mpz_set_str(&d.m_mpz, s3.data(), 10);
    ::mpz_add(&e.m_mpz, &a.m_mpz, &c.m_mpz);
    ::mpz_add(&f.m_mpz, &b.m_mpz, &d.m_mpz);
    meter.measure([&g, &e, &f] { ::mpz_add(&g.m_mpz, &e.m_mpz, &f.m_mpz); });
});*/
