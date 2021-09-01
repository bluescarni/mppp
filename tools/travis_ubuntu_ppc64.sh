#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Install wget.
apt-get update
apt-get -y install wget

# Install conda+deps.
wget https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-ppc64le.sh -O miniconda.sh
export deps_dir=$HOME/local
export PATH="$HOME/miniconda/bin:$PATH"
bash miniconda.sh -b -p $HOME/miniconda
conda create -y -q -p $deps_dir cmake gmp mpfr libflint arb python=3.8 pybind11 mpc boost-cpp c-compiler cxx-compiler make
source activate $deps_dir

# Create the build dir and cd into it.
cd /mppp
mkdir build
cd build

# GCC build.
cmake ../ -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_CXX_STANDARD=17 -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=YES -DBoost_NO_BOOST_CMAKE=ON -DMPPP_WITH_BOOST_S11N=yes -DMPPP_WITH_QUADMATH=yes -DMPPP_WITH_MPFR=yes -DMPPP_WITH_MPC=yes -DMPPP_WITH_ARB=yes -DMPPP_TEST_PYBIND11=yes -DPYBIND11_PYTHON_VERSION=3.8
make -j2 VERBOSE=1
# Run the tests.
ctest -V -j2

set +e
set +x
