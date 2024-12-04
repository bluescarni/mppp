#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install wget

# Install conda+deps.
wget https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-aarch64.sh -O mambaforge.sh
export deps_dir=$HOME/local
export PATH="$HOME/mambaforge/bin:$PATH"
bash mambaforge.sh -b -p $HOME/mambaforge
mamba create -y -p $deps_dir c-compiler cxx-compiler ninja cmake gmp mpfr libflint mpc fmt libboost-devel
source activate $deps_dir

# Create the build dir and cd into it.
mkdir build
cd build

unset CFLAGS
unset CXXFLAGS

# GCC build.
cmake ../ -G Ninja \
    -DCMAKE_PREFIX_PATH=$deps_dir \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_BUILD_TYPE=Debug \
    -DMPPP_BUILD_TESTS=yes \
    -DMPPP_WITH_BOOST_S11N=yes \
    -DMPPP_WITH_FMT=yes \
    -DMPPP_WITH_MPFR=yes \
    -DMPPP_WITH_MPC=yes \
    -DMPPP_WITH_FLINT=yes \
    -DMPPP_ENABLE_IPO=yes

ninja -v -j4
# Run the tests.
ctest -V -j4

set +e
set +x
