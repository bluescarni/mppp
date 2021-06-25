// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stdexcept>
#include <string>
#include <utility>

#include <mp++/real.hpp>

#include "catch.hpp"
#include "test_utils.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("real hyper")
{
    real r0{0};
    r0.sinh();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0.zero_p());
    real rop;
    REQUIRE(sinh(rop, r0).zero_p());
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(sinh(r0).zero_p());
    REQUIRE(sinh(std::move(r0)).zero_p());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    r0 = real{0};
    r0.cosh();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    rop = real{};
    r0 = real{0};
    REQUIRE(cosh(rop, r0) == 1);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(cosh(r0) == 1);
    REQUIRE(cosh(std::move(r0)) == 1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    r0 = real{0};
    r0.tanh();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    rop = real{1};
    r0 = real{0};
    REQUIRE(tanh(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(tanh(r0) == 0);
    REQUIRE(tanh(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    r0 = real{0};
    r0.sech();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    r0 = real{0};
    rop = real{1};
    REQUIRE(sech(rop, r0) == 1);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(sech(r0) == 1);
    REQUIRE(sech(std::move(r0)) == 1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    r0 = real{0};
    r0.csch();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0.inf_p());
    r0 = real{0};
    rop = real{1};
    REQUIRE(csch(rop, r0).inf_p());
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(csch(r0).inf_p());
    REQUIRE(csch(std::move(r0)).inf_p());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    r0 = real{0};
    r0.coth();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0.inf_p());
    r0 = real{0};
    rop = real{1};
    REQUIRE(coth(rop, r0).inf_p());
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(coth(r0).inf_p());
    REQUIRE(coth(std::move(r0)).inf_p());
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    r0 = real{0};
    r0.asinh();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    rop = real{1};
    r0 = real{0};
    REQUIRE(asinh(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(asinh(r0) == 0);
    REQUIRE(asinh(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    r0 = real{1};
    r0.acosh();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    rop = real{0};
    r0 = real{1};
    REQUIRE(acosh(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(acosh(r0) == 0);
    REQUIRE(acosh(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    r0 = real{0};
    r0.atanh();
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    rop = real{1};
    r0 = real{0};
    REQUIRE(atanh(rop, r0) == 0);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(atanh(r0) == 0);
    REQUIRE(atanh(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.get_mpfr_t()->_mpfr_d);

    // sinh_cosh.
    real sop{1, detail::real_deduce_precision(0) * 2}, cop{2, detail::real_deduce_precision(0) * 3};
    REQUIRE(sop.get_prec() != detail::real_deduce_precision(0));
    REQUIRE(cop.get_prec() != detail::real_deduce_precision(0));
    sinh_cosh(sop, cop, real{32});
    REQUIRE(sop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(cop.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(sop == sinh(real{32}));
    REQUIRE(cop == cosh(real{32}));

    REQUIRE_THROWS_PREDICATE(sinh_cosh(sop, sop, real{32}), std::invalid_argument, [](const std::invalid_argument &ex) {
        return ex.what()
               == std::string{
                   "In the real sinh_cosh() function, the return values 'sop' and 'cop' must be distinct objects"};
    });

    // Try with overlapping op/sop and op/cop.
    sop = real{1, detail::real_deduce_precision(0) * 2};
    cop = real{2, detail::real_deduce_precision(0) * 3};
    sinh_cosh(sop, cop, sop);
    REQUIRE(sop.get_prec() == detail::real_deduce_precision(0) * 2);
    REQUIRE(cop.get_prec() == detail::real_deduce_precision(0) * 2);
    REQUIRE(sop == sinh(real{1, detail::real_deduce_precision(0) * 2}));
    REQUIRE(cop == cosh(real{1, detail::real_deduce_precision(0) * 2}));

    sop = real{1, detail::real_deduce_precision(0) * 2};
    cop = real{2, detail::real_deduce_precision(0) * 3};
    sinh_cosh(sop, cop, cop);
    REQUIRE(sop.get_prec() == detail::real_deduce_precision(0) * 3);
    REQUIRE(cop.get_prec() == detail::real_deduce_precision(0) * 3);
    REQUIRE(sop == sinh(real{2, detail::real_deduce_precision(0) * 3}));
    REQUIRE(cop == cosh(real{2, detail::real_deduce_precision(0) * 3}));
}
