#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install build-essential cmake wget

# Create the build dir and cd into it.
mkdir build
cd build

# Download and compile locally GMP in debug mode.
GMP_VERSION="6.1.2"
wget https://gmplib.org/download/gmp/gmp-${GMP_VERSION}.tar.bz2 -O gmp.tar.bz2
tar xjvf gmp.tar.bz2
cd gmp-${GMP_VERSION}
./configure --enable-shared --disable-static --enable-assert --enable-alloca=debug --disable-assembly CFLAGS="-g -fsanitize=address" --prefix=/home/circleci/.local
make -j2
make install

# Compile mppp and run the tests.
cd ..
cmake ../ -DCMAKE_BUILD_TYPE=Debug -DMPPP_WITH_QUADMATH=yes -DMPPP_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-fsanitize=address" -DCMAKE_PREFIX_PATH=/home/circleci/.local
make -j2 VERBOSE=1
ctest -V


set +e
set +x
