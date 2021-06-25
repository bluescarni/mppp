// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <type_traits>
#include <utility>

#include <mp++/complex.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

TEST_CASE("sinh")
{
    complex r0{0};
    r0.sinh();
    REQUIRE(std::is_same<complex &, decltype(r0.sinh())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    r0 = 0;
    complex rop;
    REQUIRE(sinh(rop, r0) == 0);
    REQUIRE(std::is_same<complex &, decltype(sinh(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    r0 = 0;
    REQUIRE(sinh(r0) == 0);
    REQUIRE(std::is_same<complex, decltype(sinh(r0))>::value);
    REQUIRE(sinh(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    r0 = complex{16, 17, complex_prec_t(128)};
    const auto cmp = -1222565.916559164338912132229897167466665493455533226087690163237624417714978845_r128
                     - 4271542.183490460216231861008945167361582137849043217176434365579733004100250288_icr128;
    REQUIRE(abs(sinh(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(sinh(r0).get_prec() == 128);
    rop = complex{16, 17, complex_prec_t(40)};
    sinh(rop, r0);
    REQUIRE(abs(sinh(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(rop.get_prec() == 128);
    r0.sinh();
    REQUIRE(abs(r0 - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(r0.get_prec() == 128);
}

TEST_CASE("cosh")
{
    complex r0{0};
    r0.cosh();
    REQUIRE(std::is_same<complex &, decltype(r0.cosh())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 1);
    r0 = 0;
    complex rop;
    REQUIRE(cosh(rop, r0) == 1);
    REQUIRE(std::is_same<complex &, decltype(cosh(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    r0 = 0;
    REQUIRE(cosh(r0) == 1);
    REQUIRE(std::is_same<complex, decltype(cosh(r0))>::value);
    REQUIRE(cosh(std::move(r0)) == 1);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    r0 = complex{16, 17, complex_prec_t(128)};
    const auto cmp = -1222565.916559195304466456200916661043574527713755180111165540565160767449106639_r128
                     - 4271542.183490352025197137685518487475676617164197269201725926254946599193334409_icr128;
    REQUIRE(abs(cosh(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(cosh(r0).get_prec() == 128);
    rop = complex{16, 17, complex_prec_t(40)};
    cosh(rop, r0);
    REQUIRE(abs(cosh(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(rop.get_prec() == 128);
    r0.cosh();
    REQUIRE(abs(r0 - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(r0.get_prec() == 128);
}

TEST_CASE("tanh")
{
    complex r0{0};
    r0.tanh();
    REQUIRE(std::is_same<complex &, decltype(r0.tanh())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    r0 = 0;
    complex rop;
    REQUIRE(tanh(rop, r0) == 0);
    REQUIRE(std::is_same<complex &, decltype(tanh(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    r0 = 0;
    REQUIRE(tanh(r0) == 0);
    REQUIRE(std::is_same<complex, decltype(tanh(r0))>::value);
    REQUIRE(tanh(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    r0 = complex{16, 17, complex_prec_t(128)};
    const auto cmp
        = 1.000000000000021492868879825291419140230614697671889745594860101563353711786641_r128
          + 0.00000000000001340078145236711381204311425007077736496544843291887077864703178402413202739849_icr128;
    REQUIRE(abs(tanh(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(tanh(r0).get_prec() == 128);
    rop = complex{16, 17, complex_prec_t(40)};
    tanh(rop, r0);
    REQUIRE(abs(tanh(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(rop.get_prec() == 128);
    r0.tanh();
    REQUIRE(abs(r0 - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(r0.get_prec() == 128);
}

TEST_CASE("asinh")
{
    complex r0{0};
    r0.asinh();
    REQUIRE(std::is_same<complex &, decltype(r0.asinh())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    r0 = 0;
    complex rop;
    REQUIRE(asinh(rop, r0) == 0);
    REQUIRE(std::is_same<complex &, decltype(asinh(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    r0 = 0;
    REQUIRE(asinh(r0) == 0);
    REQUIRE(std::is_same<complex, decltype(asinh(r0))>::value);
    REQUIRE(asinh(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    r0 = complex{16, 17, complex_prec_t(128)};
    const auto cmp = 3.843512615825144982941941050107557235025371460726574538461989270725752510373571_r128
                     + 0.8152340115634736396198642310691627141843234347042991560624496416118863353977925_icr128;
    REQUIRE(abs(asinh(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(asinh(r0).get_prec() == 128);
    rop = complex{16, 17, complex_prec_t(40)};
    asinh(rop, r0);
    REQUIRE(abs(asinh(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(rop.get_prec() == 128);
    r0.asinh();
    REQUIRE(abs(r0 - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(r0.get_prec() == 128);
}

TEST_CASE("acosh")
{
    complex r0{1};
    r0.acosh();
    REQUIRE(std::is_same<complex &, decltype(r0.acosh())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    r0 = 1;
    complex rop;
    REQUIRE(acosh(rop, r0) == 0);
    REQUIRE(std::is_same<complex &, decltype(acosh(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    r0 = 1;
    REQUIRE(acosh(r0) == 0);
    REQUIRE(std::is_same<complex, decltype(acosh(r0))>::value);
    REQUIRE(acosh(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    r0 = complex{16, 17, complex_prec_t(128)};
    const auto cmp = 3.843568166588382136361882162833819833941936267903054343013316986770713537294589_r128
                     + 0.8161497587632627974129588763539161566033507196889074988230840343246890895775313_icr128;
    REQUIRE(abs(acosh(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(acosh(r0).get_prec() == 128);
    rop = complex{16, 17, complex_prec_t(40)};
    acosh(rop, r0);
    REQUIRE(abs(acosh(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(rop.get_prec() == 128);
    r0.acosh();
    REQUIRE(abs(r0 - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(r0.get_prec() == 128);
}

TEST_CASE("atanh")
{
    complex r0{0};
    r0.atanh();
    REQUIRE(std::is_same<complex &, decltype(r0.atanh())>::value);
    REQUIRE(r0.get_prec() == detail::real_deduce_precision(0));
    REQUIRE(r0 == 0);
    r0 = 0;
    complex rop;
    REQUIRE(atanh(rop, r0) == 0);
    REQUIRE(std::is_same<complex &, decltype(atanh(rop, r0))>::value);
    REQUIRE(rop.get_prec() == detail::real_deduce_precision(0));
    r0 = 0;
    REQUIRE(atanh(r0) == 0);
    REQUIRE(std::is_same<complex, decltype(atanh(r0))>::value);
    REQUIRE(atanh(std::move(r0)) == 0);
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move, hicpp-invalid-access-moved)
    REQUIRE(!r0.is_valid());
    r0 = complex{16, 17, complex_prec_t(128)};
    const auto cmp = 0.02933765080430309053681873024624940489042029904613618413783891238686224174512657_r128
                     + 1.539586921796917944994332135146998885295447755788803313183708387704563069534703_icr128;
    REQUIRE(abs(atanh(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(atanh(r0).get_prec() == 128);
    rop = complex{16, 17, complex_prec_t(40)};
    atanh(rop, r0);
    REQUIRE(abs(atanh(r0) - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(rop.get_prec() == 128);
    r0.atanh();
    REQUIRE(abs(r0 - cmp) / abs(cmp) < pow(2_r128, -120));
    REQUIRE(r0.get_prec() == 128);
}
