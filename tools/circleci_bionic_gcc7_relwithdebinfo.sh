#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install build-essential cmake libgmp-dev libmpfr-dev

# Create the build dir and cd into it.
mkdir build
cd build

# GCC build.
cmake ../ -DCMAKE_CXX_STANDARD=17 -DCMAKE_BUILD_TYPE=RelWithDebInfo -DMPPP_BUILD_TESTS=YES -DMPPP_WITH_MPFR=yes -DMPPP_WITH_QUADMATH=yes -DCMAKE_CXX_FLAGS="-fconcepts"
make -j2 VERBOSE=1
# Run the tests.
ctest -E integer_hash -V

set +e
set +x
