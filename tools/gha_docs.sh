#!/usr/bin/env bash

# Build the Sphinx documentation. The HTML output is written to doc/_build/html.

# Echo each command and exit on error.
set -ex

# Install conda+deps.
#
# NOTE: the compilers and GMP are needed only for the CMake configure step below.
curl -fsSL -o miniforge.sh "https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-$(uname)-$(uname -m).sh"
export deps_dir=$HOME/local
export PATH="$HOME/miniforge/bin:$PATH"
bash miniforge.sh -b -p $HOME/miniforge
conda create -y -p $deps_dir c-compiler cxx-compiler ninja cmake gmp \
    'sphinx=9.*' 'sphinx-book-theme=1.*' 'myst-nb=1.*'
source activate $deps_dir

# Create the build dir and cd into it.
mkdir build
cd build

# Clear the compilation flags set up by conda.
unset CXXFLAGS
unset CFLAGS

# Configure.
#
# NOTE: we only need to run the configure step, which generates doc/conf.py from doc/conf.py.in.
# The library does not need to be built, as the notebooks in the docs are not executed.
cmake ../ -G Ninja -DCMAKE_PREFIX_PATH=$deps_dir

# Build the docs.
cd ../doc
make html
