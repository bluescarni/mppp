#!/usr/bin/env bash

# Coverage build. The lcov report is written to build/coverage.info, from where
# it is uploaded to codecov by the CI workflow.
#
# NOTE: this script is meant to run on Linux x86_64 only (quadmath support is enabled unconditionally).

# Echo each command and exit on error.
set -ex

# Install conda+deps.
curl -fsSL -o miniforge.sh "https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-$(uname)-$(uname -m).sh"
export deps_dir=$HOME/local
export PATH="$HOME/miniforge/bin:$PATH"
bash miniforge.sh -b -p $HOME/miniforge
conda create -y -p $deps_dir c-compiler cxx-compiler ninja cmake \
    gmp mpfr libflint mpc fmt libboost-devel lcov
source activate $deps_dir

# Create the build dir and cd into it.
mkdir build
cd build

# Clear the compilation flags set up by conda.
unset CXXFLAGS
unset CFLAGS

# Configure.
#
# NOTE: -fprofile-update=atomic is needed because some tests run instrumented code concurrently
# from multiple threads (e.g., integer_caches). With the default non-atomic counter updates,
# the counters can get corrupted and lcov may error out.
cmake ../ -G Ninja \
    -DCMAKE_PREFIX_PATH=$deps_dir \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="--coverage -fprofile-update=atomic" \
    -DMPPP_BUILD_TESTS=yes \
    -DMPPP_WITH_MPFR=yes \
    -DMPPP_WITH_MPC=yes \
    -DMPPP_WITH_FLINT=yes \
    -DMPPP_WITH_QUADMATH=yes \
    -DMPPP_WITH_BOOST_S11N=yes \
    -DMPPP_WITH_FMT=yes

# Build.
ninja -v

# Run the tests.
ctest --output-on-failure -j4

# Create the lcov report.
lcov --capture --directory . --output-file coverage.info --ignore-errors inconsistent,inconsistent
