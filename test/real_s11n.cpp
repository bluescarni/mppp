// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/config.hpp>

#include <array>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(MPPP_WITH_BOOST_S11N)

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#endif

#include <mp++/detail/gmp.hpp>
#include <mp++/detail/mpfr.hpp>
#include <mp++/real.hpp>

#include "catch.hpp"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace mppp;

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size, readability-function-size)
TEST_CASE("real binary_save_load")
{
    using Catch::Matchers::Contains;
    using Catch::Matchers::Message;

    // Def cted, minimum precision.
    {
        // Raw buffer.
        real r;
        const auto orig_bs = r.binary_size();
        REQUIRE(orig_bs > 0u);
        REQUIRE(orig_bs == binary_size(r));
        std::vector<char> buffer;
        buffer.resize(orig_bs);
        REQUIRE(r.binary_save(buffer.data()) == orig_bs);
        r = 1.23_r256;
        REQUIRE(r.binary_load(buffer.data()) == orig_bs);
        REQUIRE(r == 0);
        REQUIRE(r.get_prec() == real_prec_min());

        // std::vector.
        buffer.clear();
        REQUIRE(r.binary_save(buffer) == orig_bs);
        REQUIRE(buffer.size() == r.binary_size());
        r = 1.23_r256;
        REQUIRE(r.binary_load(buffer) == orig_bs);
        REQUIRE(r == 0);
        REQUIRE(r.get_prec() == real_prec_min());

        // std::vector with more data than necessary.
        buffer.clear();
        buffer.resize(orig_bs * 2u);
        REQUIRE(r.binary_save(buffer) == orig_bs);
        REQUIRE(buffer.size() == orig_bs * 2u);
        r = 1.23_r256;
        REQUIRE(r.binary_load(buffer) == orig_bs);
        REQUIRE(r == 0);
        REQUIRE(r.get_prec() == real_prec_min());
        buffer.clear();

        // std::array.
        std::array<char, 512> abuff{};
        REQUIRE(r.binary_save(abuff) == orig_bs);
        r = 1.23_r256;
        REQUIRE(r.binary_load(abuff) == orig_bs);
        REQUIRE(r == 0);
        REQUIRE(r.get_prec() == real_prec_min());

        // stream.
        std::stringstream ss;
        REQUIRE(r.binary_save(ss) == orig_bs);
        r = 1.23_r256;
        REQUIRE(r.binary_load(ss) == orig_bs);
        REQUIRE(r == 0);
        REQUIRE(r.get_prec() == real_prec_min());

        // Test the free function interface.
        buffer.resize(orig_bs);
        REQUIRE(binary_save(r, buffer.data()) == orig_bs);
        r = 1.23_r256;
        REQUIRE(binary_load(r, buffer.data()) == orig_bs);
        REQUIRE(r == 0);
        REQUIRE(r.get_prec() == real_prec_min());

        // Error checking.
        std::array<char, 1> abuff2{};
        REQUIRE(r.binary_save(abuff2) == 0u);

        ss.str("");
        ss.setstate(std::ios_base::failbit);
        REQUIRE(r.binary_save(ss) == 0u);

        buffer.clear();
        REQUIRE_THROWS_MATCHES(r.binary_load(buffer), std::invalid_argument,
                               Message("Invalid size detected in the deserialisation of a real via a std::vector: the "
                                       "std::vector size must be at least "
                                       + std::to_string(sizeof(::mpfr_prec_t) + sizeof(::mpfr_sign_t)
                                                        + sizeof(::mpfr_exp_t) + sizeof(::mp_limb_t))
                                       + " bytes, but it is only 0 bytes"));
        auto r2 = 1.23_r512;
        r2.binary_save(buffer);
        buffer.resize(r2.binary_size() - 1u);
        REQUIRE_THROWS_MATCHES(r2.binary_load(buffer), std::invalid_argument,
                               Message("Invalid size detected in the deserialisation of a real via a std::vector: the "
                                       "std::vector size must be at least "
                                       + std::to_string(r2.binary_size()) + " bytes, but it is only "
                                       + std::to_string(r2.binary_size() - 1u) + " bytes"));
        REQUIRE_THROWS_MATCHES(r.binary_load(abuff2), std::invalid_argument,
                               Message("Invalid size detected in the deserialisation of a real via a std::array: the "
                                       "std::array size must be at least "
                                       + std::to_string(sizeof(::mpfr_prec_t) + sizeof(::mpfr_sign_t)
                                                        + sizeof(::mpfr_exp_t) + sizeof(::mp_limb_t))
                                       + " bytes, but it is only 1 bytes"));

#if MPPP_CPLUSPLUS >= 201703L
        std::stringstream ss2;
        REQUIRE(r.binary_load(ss2) == 0u);
        ss2 = std::stringstream{};

        ::mpfr_prec_t p{};
        ss2.write(reinterpret_cast<const char *>(&p), sizeof(::mpfr_prec_t));
        REQUIRE(r.binary_load(ss2) == 0u);
        ss2 = std::stringstream{};

        ss2.write(reinterpret_cast<const char *>(&p), sizeof(::mpfr_prec_t));
        ::mpfr_sign_t sb{};
        ss2.write(reinterpret_cast<const char *>(&sb), sizeof(::mpfr_sign_t));
        REQUIRE(r.binary_load(ss2) == 0u);
        ss2 = std::stringstream{};

        ss2.write(reinterpret_cast<const char *>(&p), sizeof(::mpfr_prec_t));
        ss2.write(reinterpret_cast<const char *>(&sb), sizeof(::mpfr_sign_t));
        ::mpfr_exp_t e{};
        ss2.write(reinterpret_cast<const char *>(&e), sizeof(::mpfr_exp_t));
        // NOTE: this will fail due to the precision being set to 0.
        REQUIRE_THROWS_AS(r.binary_load(ss2), std::invalid_argument);
        ss2 = std::stringstream{};

        p = 512;
        ss2.write(reinterpret_cast<const char *>(&p), sizeof(::mpfr_prec_t));
        ss2.write(reinterpret_cast<const char *>(&sb), sizeof(::mpfr_sign_t));
        ss2.write(reinterpret_cast<const char *>(&e), sizeof(::mpfr_exp_t));
        ::mp_limb_t l{};
        ss2.write(reinterpret_cast<const char *>(&l), sizeof(::mp_limb_t));
        REQUIRE(r.binary_load(ss2) == 0u);

        REQUIRE(r == 0);
        REQUIRE(r.get_prec() == real_prec_min());
#endif
    }

    // Prime number of bits of precision.
    {
        // Raw buffer.
        real r{"1.3", 419};
        const auto orig_bs = r.binary_size();
        REQUIRE(orig_bs > 0u);
        REQUIRE(orig_bs == binary_size(r));
        std::vector<char> buffer;
        buffer.resize(orig_bs);
        REQUIRE(r.binary_save(buffer.data()) == orig_bs);
        r = 1.23_r256;
        REQUIRE(r.binary_load(buffer.data()) == orig_bs);
        REQUIRE(r == real{"1.3", 419});
        REQUIRE(r.get_prec() == 419);

        // std::vector.
        buffer.clear();
        REQUIRE(r.binary_save(buffer) == orig_bs);
        REQUIRE(buffer.size() == r.binary_size());
        r = 1.23_r256;
        REQUIRE(r.binary_load(buffer) == orig_bs);
        REQUIRE(r == real{"1.3", 419});
        REQUIRE(r.get_prec() == 419);

        // std::vector with more data than necessary.
        buffer.clear();
        buffer.resize(orig_bs * 2u);
        REQUIRE(r.binary_save(buffer) == orig_bs);
        REQUIRE(buffer.size() == orig_bs * 2u);
        r = 1.23_r256;
        REQUIRE(r.binary_load(buffer) == orig_bs);
        REQUIRE(r == real{"1.3", 419});
        REQUIRE(r.get_prec() == 419);
        buffer.clear();

        // std::array.
        std::array<char, 512> abuff{};
        REQUIRE(r.binary_save(abuff) == orig_bs);
        r = 1.23_r256;
        REQUIRE(r.binary_load(abuff) == orig_bs);
        REQUIRE(r == real{"1.3", 419});
        REQUIRE(r.get_prec() == 419);

        // stream.
        std::stringstream ss;
        REQUIRE(r.binary_save(ss) == orig_bs);
        r = 1.23_r256;
        REQUIRE(r.binary_load(ss) == orig_bs);
        REQUIRE(r == real{"1.3", 419});
        REQUIRE(r.get_prec() == 419);

        // Test the free function interface.
        buffer.clear();
        REQUIRE(binary_save(r, buffer) == orig_bs);
        REQUIRE(buffer.size() == orig_bs);
        r = 1.23_r256;
        REQUIRE(binary_load(r, buffer) == orig_bs);
        REQUIRE(r == real{"1.3", 419});
        REQUIRE(r.get_prec() == 419);

        // Error checking.
        std::array<char, 1> abuff2{};
        REQUIRE(r.binary_save(abuff2) == 0u);

        ss.str("");
        ss.setstate(std::ios_base::failbit);
        REQUIRE(r.binary_save(ss) == 0u);

        buffer.clear();
        REQUIRE_THROWS_MATCHES(r.binary_load(buffer), std::invalid_argument,
                               Message("Invalid size detected in the deserialisation of a real via a std::vector: the "
                                       "std::vector size must be at least "
                                       + std::to_string(sizeof(::mpfr_prec_t) + sizeof(::mpfr_sign_t)
                                                        + sizeof(::mpfr_exp_t) + sizeof(::mp_limb_t))
                                       + " bytes, but it is only 0 bytes"));

#if MPPP_CPLUSPLUS >= 201703L
        std::stringstream ss2;
        REQUIRE(r.binary_load(ss2) == 0u);
        ss2 = std::stringstream{};

        ::mpfr_prec_t p{};
        ss2.write(reinterpret_cast<const char *>(&p), sizeof(::mpfr_prec_t));
        REQUIRE(r.binary_load(ss2) == 0u);
        ss2 = std::stringstream{};

        ss2.write(reinterpret_cast<const char *>(&p), sizeof(::mpfr_prec_t));
        ::mpfr_sign_t sb{};
        ss2.write(reinterpret_cast<const char *>(&sb), sizeof(::mpfr_sign_t));
        REQUIRE(r.binary_load(ss2) == 0u);
        ss2 = std::stringstream{};

        ss2.write(reinterpret_cast<const char *>(&p), sizeof(::mpfr_prec_t));
        ss2.write(reinterpret_cast<const char *>(&sb), sizeof(::mpfr_sign_t));
        ::mpfr_exp_t e{};
        ss2.write(reinterpret_cast<const char *>(&e), sizeof(::mpfr_exp_t));
        // NOTE: this will fail due to the precision being set to 0.
        REQUIRE_THROWS_AS(r.binary_load(ss2), std::invalid_argument);
        ss2 = std::stringstream{};

        p = 512;
        ss2.write(reinterpret_cast<const char *>(&p), sizeof(::mpfr_prec_t));
        ss2.write(reinterpret_cast<const char *>(&sb), sizeof(::mpfr_sign_t));
        ss2.write(reinterpret_cast<const char *>(&e), sizeof(::mpfr_exp_t));
        ::mp_limb_t l{};
        ss2.write(reinterpret_cast<const char *>(&l), sizeof(::mp_limb_t));
        REQUIRE(r.binary_load(ss2) == 0u);

        REQUIRE(r == real{"1.3", 419});
        REQUIRE(r.get_prec() == 419);
#endif
    }

    // Precision exactly dividing limb size on 64-bit archs.
    {
        // Raw buffer.
        real r{"1.3", 128};
        const auto orig_bs = r.binary_size();
        REQUIRE(orig_bs > 0u);
        REQUIRE(orig_bs == binary_size(r));
        std::vector<char> buffer;
        buffer.resize(orig_bs);
        REQUIRE(r.binary_save(buffer.data()) == orig_bs);
        r = 1.23_r256;
        REQUIRE(r.binary_load(buffer.data()) == orig_bs);
        REQUIRE(r == real{"1.3", 128});
        REQUIRE(r.get_prec() == 128);

        // std::vector.
        buffer.clear();
        REQUIRE(r.binary_save(buffer) == orig_bs);
        REQUIRE(buffer.size() == r.binary_size());
        r = 1.23_r256;
        REQUIRE(r.binary_load(buffer) == orig_bs);
        REQUIRE(r == real{"1.3", 128});
        REQUIRE(r.get_prec() == 128);

        // std::vector with more data than necessary.
        buffer.clear();
        buffer.resize(orig_bs * 2u);
        REQUIRE(r.binary_save(buffer) == orig_bs);
        REQUIRE(buffer.size() == orig_bs * 2u);
        r = 1.23_r256;
        REQUIRE(r.binary_load(buffer) == orig_bs);
        REQUIRE(r == real{"1.3", 128});
        REQUIRE(r.get_prec() == 128);
        buffer.clear();

        // std::array.
        std::array<char, 512> abuff{};
        REQUIRE(r.binary_save(abuff) == orig_bs);
        r = 1.23_r256;
        REQUIRE(r.binary_load(abuff) == orig_bs);
        REQUIRE(r == real{"1.3", 128});
        REQUIRE(r.get_prec() == 128);

        // stream.
        std::stringstream ss;
        REQUIRE(r.binary_save(ss) == orig_bs);
        r = 1.23_r256;
        REQUIRE(r.binary_load(ss) == orig_bs);
        REQUIRE(r == real{"1.3", 128});
        REQUIRE(r.get_prec() == 128);

        // Test the free function interface.
        ss.str("");
        REQUIRE(binary_save(r, ss) == orig_bs);
        r = 1.23_r256;
        REQUIRE(binary_load(r, ss) == orig_bs);
        REQUIRE(r == real{"1.3", 128});
        REQUIRE(r.get_prec() == 128);

        // Error checking.
        std::array<char, 1> abuff2{};
        REQUIRE(r.binary_save(abuff2) == 0u);

        ss.str("");
        ss.setstate(std::ios_base::failbit);
        REQUIRE(r.binary_save(ss) == 0u);

        buffer.clear();
        REQUIRE_THROWS_MATCHES(r.binary_load(buffer), std::invalid_argument,
                               Message("Invalid size detected in the deserialisation of a real via a std::vector: the "
                                       "std::vector size must be at least "
                                       + std::to_string(sizeof(::mpfr_prec_t) + sizeof(::mpfr_sign_t)
                                                        + sizeof(::mpfr_exp_t) + sizeof(::mp_limb_t))
                                       + " bytes, but it is only 0 bytes"));

#if MPPP_CPLUSPLUS >= 201703L
        std::stringstream ss2;
        REQUIRE(r.binary_load(ss2) == 0u);
        ss2 = std::stringstream{};

        ::mpfr_prec_t p{};
        ss2.write(reinterpret_cast<const char *>(&p), sizeof(::mpfr_prec_t));
        REQUIRE(r.binary_load(ss2) == 0u);
        ss2 = std::stringstream{};

        ss2.write(reinterpret_cast<const char *>(&p), sizeof(::mpfr_prec_t));
        ::mpfr_sign_t sb{};
        ss2.write(reinterpret_cast<const char *>(&sb), sizeof(::mpfr_sign_t));
        REQUIRE(r.binary_load(ss2) == 0u);
        ss2 = std::stringstream{};

        ss2.write(reinterpret_cast<const char *>(&p), sizeof(::mpfr_prec_t));
        ss2.write(reinterpret_cast<const char *>(&sb), sizeof(::mpfr_sign_t));
        ::mpfr_exp_t e{};
        ss2.write(reinterpret_cast<const char *>(&e), sizeof(::mpfr_exp_t));
        // NOTE: this will fail due to the precision being set to 0.
        REQUIRE_THROWS_AS(r.binary_load(ss2), std::invalid_argument);
        ss2 = std::stringstream{};

        p = 512;
        ss2.write(reinterpret_cast<const char *>(&p), sizeof(::mpfr_prec_t));
        ss2.write(reinterpret_cast<const char *>(&sb), sizeof(::mpfr_sign_t));
        ss2.write(reinterpret_cast<const char *>(&e), sizeof(::mpfr_exp_t));
        ::mp_limb_t l{};
        ss2.write(reinterpret_cast<const char *>(&l), sizeof(::mp_limb_t));
        REQUIRE(r.binary_load(ss2) == 0u);

        REQUIRE(r == real{"1.3", 128});
        REQUIRE(r.get_prec() == 128);
#endif
    }
}

#if defined(MPPP_WITH_BOOST_S11N)

template <typename OA, typename IA>
void test_s11n()
{
    std::stringstream ss;

    auto x = 1.1_r512;
    {
        OA oa(ss);
        oa << x;
    }

    x = real{};
    {
        IA ia(ss);
        ia >> x;
    }

    REQUIRE(x == 1.1_r512);
    REQUIRE(x.get_prec() == 512);
}

TEST_CASE("boost_s11n")
{
    test_s11n<boost::archive::text_oarchive, boost::archive::text_iarchive>();
    test_s11n<boost::archive::binary_oarchive, boost::archive::binary_iarchive>();
}

#endif
