#!/usr/bin/env bash

# Build & test mp++ against a debug build of GMP (with assertions and address sanitizer enabled).
#
# NOTE: this script is meant to run on Linux x86_64 only (quadmath support is enabled unconditionally).

# Echo each command and exit on error.
set -ex

# Install conda+deps.
curl -fsSL -o miniforge.sh "https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-$(uname)-$(uname -m).sh"
export deps_dir=$HOME/local
export PATH="$HOME/miniforge/bin:$PATH"
bash miniforge.sh -b -p $HOME/miniforge
conda create -y -p $deps_dir c-compiler cxx-compiler ninja cmake make
source activate $deps_dir

# Create the build dir and cd into it.
mkdir build
cd build

# Clear the compilation flags set up by conda.
unset CXXFLAGS
unset CFLAGS

# Download and compile locally GMP in debug mode.
#
# NOTE: -std=gnu17 is needed because the configure script of GMP 6.3.0 fails with C23 compilers
# (e.g., GCC >= 15, which defaults to -std=gnu23). See:
#
# https://gmplib.org/list-archives/gmp-bugs/2024-November/005550.html
#
# This can be removed once a fixed GMP version is released.
GMP_VERSION="6.3.0"
curl -fsSL -o gmp.tar.bz2 "https://github.com/esa/manylinux_x86_64_with_deps/raw/master/gmp-${GMP_VERSION}.tar.bz2"
tar xjf gmp.tar.bz2
cd gmp-${GMP_VERSION}
./configure --enable-shared --disable-static --enable-assert --enable-alloca=debug --disable-assembly \
    CFLAGS="-g -fsanitize=address -std=gnu17" --prefix=$deps_dir
make -j4
make install

# Configure mp++.
cd ..
cmake ../ -G Ninja \
    -DCMAKE_PREFIX_PATH=$deps_dir \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-fsanitize=address" \
    -DMPPP_BUILD_TESTS=yes \
    -DMPPP_WITH_QUADMATH=yes \
    -DMPPP_ENABLE_IPO=yes

# Build.
ninja -v

# Run the tests.
ctest --output-on-failure -j4
