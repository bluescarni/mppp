
#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Install conda+deps.
wget https://repo.anaconda.com/miniconda/Miniconda3-latest-MacOSX-x86_64.sh -O miniconda.sh
export deps_dir=$HOME/local
export PATH="$HOME/miniconda/bin:$PATH"
bash miniconda.sh -b -p $HOME/miniconda
conda config --add channels conda-forge
conda config --set channel_priority strict
conda create -y -q -p $deps_dir c-compiler cxx-compiler libcxx cmake gmp mpfr arb libflint mpc boost-cpp
source activate $deps_dir

# Create the build dir and cd into it.
mkdir build
cd build

# GCC build.
CXX=clang++ CC=clang cmake -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DBoost_NO_BOOST_CMAKE=ON -DMPPP_WITH_BOOST_S11N=yes -DMPPP_WITH_MPFR=yes -DMPPP_WITH_ARB=yes -DMPPP_WITH_MPC=yes -DMPPP_ENABLE_IPO=yes ../
make -j2 VERBOSE=1
ctest -V -j2

set +e
set +x
