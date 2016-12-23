#!/usr/bin/env bash

# Exit on error
set -e
# Echo each command
set -x

if [[ "${MPPP_BUILD}" == "ReleaseGCC48" ]]; then
    CXX=g++-4.8 CC=gcc-4.8 cmake -DCMAKE_BUILD_TYPE=Release -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_LONG_DOUBLE=yes ../;
    make -j2;
    ctest -V;
elif [[ "${MPPP_BUILD}" == "DebugGCC48" ]]; then
    CXX=g++-4.8 CC=gcc-4.8 cmake -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_LONG_DOUBLE=yes -DCMAKE_CXX_FLAGS="-fsanitize=address" ../;
    make -j2;
    ctest -V;
elif [[ "${MPPP_BUILD}" == "CoverageGCC5" ]]; then
    CXX=g++-5 CC=gcc-5 cmake -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_LONG_DOUBLE=yes -DCMAKE_CXX_FLAGS="--coverage" ../;
    make -j2;
    ctest -V;
    bash <(curl -s https://codecov.io/bash) -x gcov-5
elif [[ "${MPPP_BUILD}" == "DebugClang38" ]]; then
    CXX=clang++-3.8 CC=clang-3.8 cmake -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_LONG_DOUBLE=yes ../;
    make -j2;
    ctest -V;
elif [[ "${MPPP_BUILD}" == "ReleaseClang38" ]]; then
    CXX=clang++-3.8 CC=clang-3.8 cmake -DCMAKE_BUILD_TYPE=Release -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_LONG_DOUBLE=yes ../;
    make -j2;
    ctest -V;
fi
