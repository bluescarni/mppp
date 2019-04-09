#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

if [[ "${MPPP_BUILD}" != DebugGCC48DebugGMP* && "${MPPP_BUILD}" != Coverage32GCC6 ]]; then
    if [[ "${TRAVIS_OS_NAME}" == "osx" ]]; then
        wget https://repo.continuum.io/miniconda/Miniconda2-latest-MacOSX-x86_64.sh -O miniconda.sh;
    else
        wget https://repo.continuum.io/miniconda/Miniconda2-latest-Linux-x86_64.sh -O miniconda.sh;
    fi
    export deps_dir=$HOME/local
    export PATH="$HOME/miniconda/bin:$PATH"
    bash miniconda.sh -b -p $HOME/miniconda
    conda config --add channels conda-forge --force

    conda_pkgs="cmake>=3.3 gmp mpfr"

    if [[ "${MPPP_BUILD}" == CoverageGCC5 ]]; then
        conda_pkgs="$conda_pkgs python=2.7 mpmath pybind11"
    fi

    if [[ "${MPPP_BUILD}" == CoverageGCC7 ]]; then
        conda_pkgs="$conda_pkgs python=3.6 mpmath pybind11"
    fi

    if [[ "${MPPP_BUILD}" == Documentation ]]; then
        conda_pkgs="$conda_pkgs graphviz doxygen"
    fi

    if [[ "${MPPP_BUILD}" == ReleaseGCC48 ]]; then
        conda_pkgs="$conda_pkgs boost-cpp"
    fi

    if [[ "${TRAVIS_OS_NAME}" == "osx" ]]; then
        conda_pkgs="$conda_pkgs clangdev"
    fi

    conda create -q -p $deps_dir -y $conda_pkgs
    source activate $deps_dir
fi

set +e
set +x
