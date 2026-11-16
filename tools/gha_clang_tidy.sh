#!/usr/bin/env bash

# clang-tidy build of the library and of the full test suite. Any clang-tidy diagnostic
# fails the build (see WarningsAsErrors in .clang-tidy).
#
# NOTE: the clang version is deliberately not pinned. If a new clang-tidy release breaks
# the build, either fix the new diagnostics or pin the version here.
#
# NOTE: this script is meant to run on Linux x86_64 only (quadmath support is enabled unconditionally).

# Echo each command and exit on error.
set -ex

# Install conda+deps.
curl -fsSL -o miniforge.sh "https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-$(uname)-$(uname -m).sh"
export deps_dir=$HOME/local
export PATH="$HOME/miniforge/bin:$PATH"
bash miniforge.sh -b -p $HOME/miniforge
conda create -y -p $deps_dir c-compiler cxx-compiler clang clangxx clang-tools ninja cmake \
    gmp mpfr libflint mpc fmt libboost-devel
source activate $deps_dir

# Print the clang-tidy version, for reference.
clang-tidy --version

# Create the build dir and cd into it.
mkdir build
cd build

# Clear the compilation flags set up by conda.
unset CXXFLAGS
unset CFLAGS

# Configure.
cmake ../ -G Ninja \
    -DCMAKE_PREFIX_PATH=$deps_dir \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_CXX_CLANG_TIDY=$(which clang-tidy) \
    -DMPPP_BUILD_TESTS=yes \
    -DMPPP_WITH_MPFR=yes \
    -DMPPP_WITH_MPC=yes \
    -DMPPP_WITH_FLINT=yes \
    -DMPPP_WITH_QUADMATH=yes \
    -DMPPP_WITH_BOOST_S11N=yes \
    -DMPPP_WITH_FMT=yes

# Build.
#
# NOTE: -k 0 keeps the build going after the first failure, so that a single
# CI run reports all clang-tidy diagnostics.
ninja -v -k 0

