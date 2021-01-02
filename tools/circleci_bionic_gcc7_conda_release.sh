#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install build-essential wget

# Install conda+deps.
wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh -O miniconda.sh
export deps_dir=$HOME/local
export PATH="$HOME/miniconda/bin:$PATH"
bash miniconda.sh -b -p $HOME/miniconda
conda config --add channels conda-forge
conda config --set channel_priority strict
conda_pkgs="cmake gmp mpfr libflint arb mpc boost-cpp fmt"
conda create -q -p $deps_dir -y
source activate $deps_dir
conda install $conda_pkgs -y

# Create the build dir and cd into it.
mkdir build
cd build

# GCC build.
cmake ../ -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_CXX_STANDARD=17 -DCMAKE_BUILD_TYPE=Release -DMPPP_BUILD_TESTS=YES -DBoost_NO_BOOST_CMAKE=ON -DMPPP_WITH_BOOST_S11N=yes -DMPPP_WITH_MPFR=yes -DMPPP_WITH_MPC=yes -DMPPP_WITH_ARB=yes -DMPPP_WITH_QUADMATH=yes -DCMAKE_CXX_FLAGS="-fconcepts" -DMPPP_ENABLE_IPO=yes -DMPPP_BUILD_BENCHMARKS=yes
make -j2 VERBOSE=1
# Run the tests.
ctest -j2 -E integer_hash -V

# Run the benchmarks as well.
make -j2 VERBOSE=1 benchmark

set +e
set +x
