#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install wget curl

# Install conda+deps.
wget https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-x86_64.sh -O mambaforge.sh
export deps_dir=$HOME/local
export PATH="$HOME/mambaforge/bin:$PATH"
bash mambaforge.sh -b -p $HOME/mambaforge
mamba create -y -p $deps_dir c-compiler cxx-compiler ninja cmake gmp mpfr 'libflint<3' arb mpc fmt libboost-devel lcov
source activate $deps_dir

# Create the build dir and cd into it.
mkdir build
cd build

unset CFLAGS
unset CXXFLAGS

# GCC build.
cmake ../ -G Ninja \
    -DCMAKE_PREFIX_PATH=$deps_dir \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_BUILD_TYPE=Debug \
    -DMPPP_BUILD_TESTS=yes \
    -DMPPP_WITH_MPFR=yes \
    -DMPPP_WITH_MPC=yes \
    -DMPPP_WITH_ARB=yes \
    -DMPPP_WITH_QUADMATH=yes \
    -DMPPP_WITH_BOOST_S11N=yes \
    -DCMAKE_CXX_FLAGS="--coverage"
    
ninja -v -j4
# Run the tests.
ctest -V -j4

# Create lcov report
lcov --capture --directory . --output-file coverage.info

# Upload coverage data.
curl -Os https://uploader.codecov.io/latest/linux/codecov
chmod +x codecov
./codecov -f coverage.info -g --gx $deps_dir/bin/gcov

set +e
set +x
