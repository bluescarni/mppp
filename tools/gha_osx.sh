
#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Install conda+deps.
wget https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-MacOSX-x86_64.sh -O miniforge.sh
export deps_dir=$HOME/local
export PATH="$HOME/miniforge/bin:$PATH"
bash miniforge.sh -b -p $HOME/miniforge
mamba create -y -p $deps_dir c-compiler cxx-compiler ninja cmake gmp mpfr 'libflint<3' arb mpc fmt libboost-devel
source activate $deps_dir

# Create the build dir and cd into it.
mkdir build
cd build

cmake ../ -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DMPPP_BUILD_TESTS=yes \
    -DBoost_NO_BOOST_CMAKE=yes \
    -DMPPP_WITH_BOOST_S11N=yes \
    -DMPPP_WITH_FMT=yes \
    -DMPPP_WITH_MPFR=yes \
    -DMPPP_WITH_ARB=yes \
    -DMPPP_WITH_MPC=yes \
    -DMPPP_ENABLE_IPO=yes

ninja -v
ctest -V -j2

set +e
set +x
