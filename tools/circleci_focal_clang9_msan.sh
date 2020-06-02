#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install build-essential cmake wget clang ninja-build

# Create the build dir and cd into it.
mkdir build
cd build
MPPP_BUILD_DIR=`pwd`

# libc++ setup.
git clone --depth=1 https://github.com/llvm/llvm-project
cd llvm-project
mkdir build
cd build
LLVM_BUILD_DIR=`pwd`
cmake -GNinja ../llvm \
	-DCMAKE_BUILD_TYPE=Release \
	-DLLVM_ENABLE_PROJECTS="libcxx;libcxxabi" \
	-DCMAKE_C_COMPILER=clang \
	-DCMAKE_CXX_COMPILER=clang++ \
	-DLLVM_USE_SANITIZER=MemoryWithOrigins
cmake --build . -- cxx cxxabi
# Back to the MPPP build dir.
cd $MPPP_BUILD_DIR

# Download and compile locally GMP in debug mode.
GMP_VERSION="6.2.0"
wget https://gmplib.org/download/gmp/gmp-${GMP_VERSION}.tar.bz2 -O gmp.tar.bz2
tar xjvf gmp.tar.bz2
cd gmp-${GMP_VERSION}
CC=clang CXX=clang++ ./configure --enable-shared --disable-static --enable-assert --enable-alloca=debug --disable-assembly CFLAGS="-g -fsanitize=memory" --prefix=/home/circleci/.local
make -j2
make install

# Compile mppp and run the tests.
cd ..
MPPP_MSAN_FLAGS="-fsanitize=memory -stdlib=libc++ -nostdinc++ -isystem ${LLVM_BUILD_DIR}/include/c++/v1 -L${LLVM_BUILD_DIR}/lib -Wl,-rpath,${LLVM_BUILD_DIR}/lib -Wno-unused-command-line-argument"
CC=clang CXX=clang++ cmake ../ -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="${MPPP_MSAN_FLAGS}" -DCMAKE_C_FLAGS="${MPPP_MSAN_FLAGS}" -DCMAKE_PREFIX_PATH=/home/circleci/.local
make -j2 VERBOSE=1
ctest -V

set +e
set +x
