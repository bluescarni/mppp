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
conda config --add channels conda-forge --force
conda_pkgs="cmake gmp mpfr libflint arb"
conda create -q -p $deps_dir -y
source activate $deps_dir
conda install $conda_pkgs -y

# Create the build dir and cd into it.
mkdir build
cd build

# GCC build.
cmake ../ -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_CXX_STANDARD=17 -DCMAKE_BUILD_TYPE=Release -DMPPP_BUILD_TESTS=YES -DMPPP_WITH_MPFR=yes -DMPPP_WITH_ARB=yes -DMPPP_WITH_QUADMATH=yes -DCMAKE_CXX_FLAGS="-fconcepts"
make -j2 VERBOSE=1
# Run the tests.
ctest -j2 -E integer_hash -V

set +e
set +x
