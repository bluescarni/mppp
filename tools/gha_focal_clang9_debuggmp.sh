#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install build-essential cmake wget clang

# Create the build dir and cd into it.
mkdir build
cd build

# Download and compile locally GMP in debug mode.
GMP_VERSION="6.2.1"
wget https://gmplib.org/download/gmp/gmp-${GMP_VERSION}.tar.bz2 -O gmp.tar.bz2
tar xjvf gmp.tar.bz2
cd gmp-${GMP_VERSION}
CC=clang CXX=clang++ ./configure --enable-shared --disable-static --enable-assert --enable-alloca=debug --disable-assembly CFLAGS="-g -fsanitize=address" --prefix=/home/circleci/.local
make -j2
make install

# Compile mppp and run the tests.
cd ..
CC=clang CXX=clang++ cmake ../ -DCMAKE_BUILD_TYPE=Debug -DMPPP_WITH_QUADMATH=yes -DMPPP_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-fsanitize=address" -DCMAKE_PREFIX_PATH=/home/circleci/.local -DMPPP_QUADMATH_INCLUDE_DIR=/usr/lib/gcc/x86_64-linux-gnu/9/include/ -DMPPP_QUADMATH_LIBRARY=/usr/lib/gcc/x86_64-linux-gnu/9/libquadmath.so -DMPPP_ENABLE_IPO=yes
make -j2 VERBOSE=1
ctest -V

set +e
set +x
