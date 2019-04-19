#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install build-essential cmake libgmp-dev libmpfr-dev curl python3-pybind11 python3-dev

# Create the build dir and cd into it.
mkdir build
cd build

# GCC build.
cmake ../ -DCMAKE_CXX_STANDARD=17 -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=YES -DMPPP_WITH_MPFR=yes -DMPPP_WITH_QUADMATH=yes -DCMAKE_CXX_FLAGS="--coverage -fconcepts -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC" -DMPPP_TEST_PYBIND11=yes -DPYBIND11_PYTHON_VERSION=3.6
make -j2 VERBOSE=1
# Run the tests.
ctest -V
# Upload coverage data.
bash <(curl -s https://codecov.io/bash) -x gcov-8 > /dev/null

set +e
set +x
