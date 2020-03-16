#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install build-essential cmake clang libgmp-dev libmpfr-dev gcc-5

# Create the build dir and cd into it.
mkdir build
cd build

# Compile mppp and run the tests.
cd ..
CC=clang CXX=clang++ cmake ../ -DCMAKE_BUILD_TYPE=Debug -DMPPP_WITH_QUADMATH=yes -DMPPP_WITH_MPFR=yes -DMPPP_BUILD_TESTS=yes -DQuadmath_INCLUDE_DIR=/usr/lib/gcc/x86_64-linux-gnu/5/include/ -DQuadmath_LIBRARY=/usr/lib/gcc/x86_64-linux-gnu/5/libquadmath.so
make -j2 VERBOSE=1
ctest -V

CC=clang CXX=clang++ cmake ../ -DCMAKE_BUILD_TYPE=Debug -DMPPP_WITH_QUADMATH=yes -DMPPP_WITH_MPFR=no -DMPPP_BUILD_TESTS=yes -DQuadmath_INCLUDE_DIR=/usr/lib/gcc/x86_64-linux-gnu/5/include/ -DQuadmath_LIBRARY=/usr/lib/gcc/x86_64-linux-gnu/5/libquadmath.so
make -j2 VERBOSE=1
ctest -V

set +e
set +x
