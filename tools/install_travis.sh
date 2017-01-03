#!/usr/bin/env bash

# Exit on error
set -e
# Echo each command
set -x

if [[ "${MPPP_BUILD}" == "ReleaseGCC48" ]]; then
    CXX=g++-4.8 CC=gcc-4.8 cmake -DCMAKE_BUILD_TYPE=Release -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_LONG_DOUBLE=yes ../;
    make -j2 VERBOSE=1;
    ctest -V;
elif [[ "${MPPP_BUILD}" == "DebugGCC48" ]]; then
    CXX=g++-4.8 CC=gcc-4.8 cmake -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_LONG_DOUBLE=yes -DCMAKE_CXX_FLAGS="-fsanitize=address" ../;
    make -j2 VERBOSE=1;
    ctest -V;
elif [[ "${MPPP_BUILD}" == "DebugGCC48DebugGMP" ]]; then
    # Download and compile locally GMP in debug mode.
    wget https://gmplib.org/download/gmp/gmp-6.1.2.tar.bz2;
    tar xjvf gmp-6.1.2.tar.bz2;
    cd gmp-6.1.2;
    CXX=g++-4.8 CC=gcc-4.8 ./configure --disable-shared --enable-assert --enable-alloca=debug --disable-assembly CFLAGS=-g;
    make -j2;
    cd ..;
    CXX=g++-4.8 CC=gcc-4.8 cmake -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-fsanitize=address" -DGMP_INCLUDE_DIR=$TRAVIS_BUILD_DIR/build/gmp-6.1.2 -DGMP_LIBRARY=$TRAVIS_BUILD_DIR/build/gmp-6.1.2/.libs/libgmp.a ../;
    make -j2 VERBOSE=1;
    ctest -V;
elif [[ "${MPPP_BUILD}" == "CoverageGCC5" ]]; then
    CXX=g++-5 CC=gcc-5 cmake -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_LONG_DOUBLE=yes -DCMAKE_CXX_FLAGS="--coverage" ../;
    make -j2 VERBOSE=1;
    ctest -V;
    bash <(curl -s https://codecov.io/bash) -x gcov-5
elif [[ "${MPPP_BUILD}" == "Coverage32GCC6" ]]; then
    wget https://gmplib.org/download/gmp/gmp-6.1.2.tar.bz2;
    tar xjvf gmp-6.1.2.tar.bz2;
    cd gmp-6.1.2;
    CXX=g++-6 CC=gcc-6 ABI=32 ./configure --disable-shared;
    make -j2;
    cd ..;
    CXX=g++-6 CC=gcc-6 cmake -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-m32 --coverage -D_GLIBCXX_CONCEPT_CHECKS" -DGMP_INCLUDE_DIR=$TRAVIS_BUILD_DIR/build/gmp-6.1.2 -DGMP_LIBRARY=$TRAVIS_BUILD_DIR/build/gmp-6.1.2/.libs/libgmp.a ../;
    make -j2 VERBOSE=1;
    ctest -V;
    bash <(curl -s https://codecov.io/bash) -x gcov-6
elif [[ "${MPPP_BUILD}" == "DebugClang38" ]]; then
    CXX=clang++-3.8 CC=clang-3.8 cmake -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_LONG_DOUBLE=yes ../;
    make -j2 VERBOSE=1;
    ctest -V;
elif [[ "${MPPP_BUILD}" == "ReleaseClang38" ]]; then
    CXX=clang++-3.8 CC=clang-3.8 cmake -DCMAKE_BUILD_TYPE=Release -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_LONG_DOUBLE=yes ../;
    make -j2 VERBOSE=1;
    ctest -V;
elif [[ "${MPPP_BUILD}" == "ICC" ]]; then
    set +x;
    docker login -u=bluescarni -p=${DOCKER_CLOUD_PWD};
    set -x;
    docker pull ${DOCKER_IMAGE};
fi
