#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install build-essential curl

# Install conda+deps.
wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh -O miniconda.sh
export deps_dir=$HOME/local
export PATH="$HOME/miniconda/bin:$PATH"
bash miniconda.sh -b -p $HOME/miniconda
conda config --add channels conda-forge --force
conda_pkgs="cmake gmp mpfr libflint arb python=3.7 pybind11"
conda create -q -p $deps_dir -y
source activate $deps_dir
conda install $conda_pkgs -y

# Create the build dir and cd into it.
mkdir build
cd build

# GCC build.
cmake ../ -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_CXX_STANDARD=17 -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=YES -DMPPP_WITH_MPFR=yes -DMPPP_WITH_ARB=yes -DMPPP_WITH_QUADMATH=yes -DCMAKE_CXX_FLAGS="--coverage -fconcepts -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC" -DMPPP_TEST_PYBIND11=yes -DPYBIND11_PYTHON_VERSION=3.7
make -j2 VERBOSE=1
# Run the tests.
ctest -V -j2
# Upload coverage data.
bash <(curl -s https://codecov.io/bash) -x gcov-8 > /dev/null

set +e
set +x
