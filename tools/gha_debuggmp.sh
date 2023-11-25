#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install wget

# Install conda+deps.
wget https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-x86_64.sh -O miniforge.sh
export deps_dir=$HOME/local
export PATH="$HOME/miniforge/bin:$PATH"
bash miniforge.sh -b -p $HOME/miniforge
mamba create -y -p $deps_dir c-compiler cxx-compiler ninja cmake make
source activate $deps_dir

# Create the build dir and cd into it.
mkdir build
cd build

unset CFLAGS
unset CXXFLAGS

# Download and compile locally GMP in debug mode.
GMP_VERSION="6.2.1"
wget https://github.com/esa/manylinux_x86_64_with_deps/raw/master/gmp-${GMP_VERSION}.tar.bz2 -O gmp.tar.bz2
tar xjvf gmp.tar.bz2
cd gmp-${GMP_VERSION}
./configure --enable-shared --disable-static --enable-assert --enable-alloca=debug --disable-assembly CFLAGS="-g -fsanitize=address" --prefix=$CONDA_PREFIX
make -j2
make install

# Compile mppp and run the tests.
cd ..
cmake ../ -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DMPPP_WITH_QUADMATH=yes \
    -DMPPP_BUILD_TESTS=yes \
    -DCMAKE_CXX_FLAGS="-fsanitize=address" \
    -DCMAKE_PREFIX_PATH=$CONDA_PREFIX \
    -DMPPP_ENABLE_IPO=yes

ninja -v
ctest -V -j2

set +e
set +x
