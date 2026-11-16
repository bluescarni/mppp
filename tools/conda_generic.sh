#!/usr/bin/env bash

# Generic conda-based build & test script for Linux and OSX (x86_64 and arm64).
#
# Configuration via environment variables (all optional):
#
# - MPPP_BUILD_TYPE: CMake build type (default: Debug).
# - MPPP_CXX_FLAGS: extra compiler flags, e.g., "-fsanitize=address" (default: empty).
# - MPPP_CXX_STANDARD: C++ standard to build with, e.g., 23 (default: unset, i.e., the
#   minimum standard required by mp++).
# - MPPP_CMAKE_EXTRA: extra arguments passed verbatim to the CMake configure step (default: empty).
#
# Quadmath support is enabled only on Linux x86_64.

# Echo each command and exit on error.
set -ex

# Install conda+deps. The installer is selected based on the current OS and architecture
# (e.g., Linux-x86_64, Linux-aarch64, MacOSX-arm64, MacOSX-x86_64).
curl -fsSL -o miniforge.sh "https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-$(uname)-$(uname -m).sh"
export deps_dir=$HOME/local
export PATH="$HOME/miniforge/bin:$PATH"
bash miniforge.sh -b -p $HOME/miniforge
conda create -y -p $deps_dir c-compiler cxx-compiler ninja cmake \
    gmp mpfr libflint mpc fmt libboost-devel
source activate $deps_dir

# Quadmath is available only on Linux x86_64.
if [[ "$(uname)-$(uname -m)" == "Linux-x86_64" ]]; then
    WITH_QUADMATH=yes
else
    WITH_QUADMATH=no
fi

# Pass the C++ standard to CMake only if explicitly requested.
CXX_STANDARD_ARG=""
if [[ -n "${MPPP_CXX_STANDARD:-}" ]]; then
    CXX_STANDARD_ARG="-DCMAKE_CXX_STANDARD=${MPPP_CXX_STANDARD}"
fi

# Create the build dir and cd into it.
mkdir build
cd build

# Clear the compilation flags set up by conda.
unset CXXFLAGS
unset CFLAGS

# Configure.
# NOTE: CXX_STANDARD_ARG and MPPP_CMAKE_EXTRA are intentionally unquoted, so that they
# expand to nothing when empty (and MPPP_CMAKE_EXTRA can contain multiple arguments).
cmake ../ -G Ninja \
    -DCMAKE_PREFIX_PATH=$deps_dir \
    -DCMAKE_BUILD_TYPE=${MPPP_BUILD_TYPE:-Debug} \
    $CXX_STANDARD_ARG \
    -DCMAKE_CXX_FLAGS="${MPPP_CXX_FLAGS:-}" \
    -DCMAKE_CXX_FLAGS_DEBUG="-g -Og" \
    -DMPPP_BUILD_TESTS=yes \
    -DMPPP_WITH_MPFR=yes \
    -DMPPP_WITH_MPC=yes \
    -DMPPP_WITH_FLINT=yes \
    -DMPPP_WITH_QUADMATH=$WITH_QUADMATH \
    -DMPPP_WITH_BOOST_S11N=yes \
    -DMPPP_WITH_FMT=yes \
    ${MPPP_CMAKE_EXTRA:-}

# Build.
ninja -v

# Run the tests.
ctest --output-on-failure -j4
