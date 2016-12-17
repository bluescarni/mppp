#!/usr/bin/env bash

# Exit on error
set -e
# Echo each command
set -x

if [[ "${BUILD_TYPE}" == "Debug" ]]; then
    if [[ "${MPPP_COMPILER}" == "gcc" ]]; then
        cmake -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_LONG_DOUBLE=yes -DCMAKE_CXX_FLAGS="-fsanitize=address" ../;
        make;
        ctest -V;
    elif [[ "${MPPP_COMPILER}" == "clang" ]]; then
        cmake -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_LONG_DOUBLE=yes ../;
        make;
        ctest -V;
    fi
elif [[ "${BUILD_TYPE}" == "Coverage" ]]; then
    cmake -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_LONG_DOUBLE=yes -DCMAKE_CXX_FLAGS="--coverage" ../;
    make;
    ctest -V;
    bash <(curl -s https://codecov.io/bash) -x $GCOV_EXECUTABLE
elif [[ "${BUILD_TYPE}" == "Release" ]]; then
    cmake -DCMAKE_BUILD_TYPE=Release -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_LONG_DOUBLE=yes ../;
    make;
    ctest -V;
fi
