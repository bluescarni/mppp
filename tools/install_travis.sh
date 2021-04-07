#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

if [[ "${MPPP_BUILD}" != Coverage32GCC6 ]]; then
    export deps_dir=$HOME/local
    export PATH="$HOME/miniconda/bin:$PATH"
    export PATH="$deps_dir/bin:$PATH"
fi

if [[ "${MPPP_BUILD}" == "ReleaseGCC48" ]]; then
    CXX=g++-4.8 CC=gcc-4.8 cmake -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Release -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_MPFR=yes -DMPPP_WITH_ARB=yes -DMPPP_WITH_MPC=yes -DMPPP_WITH_QUADMATH=yes ../;
    make -j2 VERBOSE=1 install;
    ctest -V;

    # Test the CMake export installation.
    cd ../tools/sample_project;
    mkdir build;
    cd build;
    CXX=g++-4.8 CC=gcc-4.8 cmake ../ -DCMAKE_PREFIX_PATH=$deps_dir;
    make;
    ./main;
elif [[ "${MPPP_BUILD}" == "DebugGCC48" ]]; then
    CXX=g++-4.8 CC=gcc-4.8 cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_MPFR=yes -DMPPP_WITH_ARB=yes -DMPPP_WITH_MPC=yes -DMPPP_WITH_QUADMATH=yes -DCMAKE_CXX_FLAGS="-fsanitize=address" ../;
    make -j2 VERBOSE=1;
    ctest -V;
elif [[ "${MPPP_BUILD}" == "DebugARM64" || "${MPPP_BUILD}" == "DebugPPC64" ]]; then
    cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DBoost_NO_BOOST_CMAKE=ON -DMPPP_WITH_BOOST_S11N=yes -DMPPP_WITH_MPFR=yes -DMPPP_WITH_ARB=yes -DMPPP_WITH_MPC=yes -DMPPP_ENABLE_IPO=yes ../;
    make -j2 VERBOSE=1;
    ctest -V;
elif [[ "${MPPP_BUILD}" == "Coverage32GCC6" ]]; then
    wget ftp://ftp.gnu.org/gnu/gmp/gmp-6.2.1.tar.bz2;
    tar xjvf gmp-6.2.1.tar.bz2;
    cd gmp-6.2.1;
    CXX=g++-6 CC=gcc-6 ABI=32 ./configure --disable-shared;
    make -j2;
    cd ..;
    CXX=g++-6 CC=gcc-6 cmake -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-m32 --coverage" -DMPPP_GMP_INCLUDE_DIR=$TRAVIS_BUILD_DIR/build/gmp-6.2.1 -DMPPP_GMP_LIBRARY=$TRAVIS_BUILD_DIR/build/gmp-6.2.1/.libs/libgmp.a ../;
    make -j2 VERBOSE=1;
    ctest -V;
    bash <(curl -s https://codecov.io/bash) -x gcov-6;
elif [[ "${MPPP_BUILD}" == "OSXDebug" ]]; then
    CXX=clang++ CC=clang cmake -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DBoost_NO_BOOST_CMAKE=ON -DMPPP_WITH_BOOST_S11N=yes -DMPPP_WITH_MPFR=yes -DMPPP_WITH_ARB=yes -DMPPP_WITH_MPC=yes -DMPPP_ENABLE_IPO=yes ../;
    make -j2 VERBOSE=1;
    ctest -V;
fi
