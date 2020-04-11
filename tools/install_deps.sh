#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

if [[ "${MPPP_BUILD}" != Coverage32GCC6 ]]; then
    if [[ "${TRAVIS_OS_NAME}" == "osx" ]]; then
        wget https://repo.continuum.io/miniconda/Miniconda2-latest-MacOSX-x86_64.sh -O miniconda.sh;
    else
        if [[ "${TRAVIS_CPU_ARCH}" == "arm64" ]]; then
            wget https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-aarch64.sh -O miniconda.sh;
        elif [[ "${TRAVIS_CPU_ARCH}" == "ppc64le" ]]; then
            wget https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-ppc64le.sh -O miniconda.sh;
        else
            wget https://repo.continuum.io/miniconda/Miniconda2-latest-Linux-x86_64.sh -O miniconda.sh;
        fi
    fi
    export deps_dir=$HOME/local
    export PATH="$HOME/miniconda/bin:$PATH"
    bash miniconda.sh -b -p $HOME/miniconda
    if [[ "${TRAVIS_CPU_ARCH}" == "amd64" ]]; then
        conda config --add channels conda-forge
        conda config --set channel_priority strict
    fi

    conda_pkgs="cmake<3.16 gmp mpfr arb libflint"

    if [[ "${MPPP_BUILD}" == Documentation ]]; then
        conda_pkgs="$conda_pkgs pip"
    fi

    if [[ "${MPPP_BUILD}" == ReleaseGCC48 ]]; then
        conda_pkgs="$conda_pkgs boost-cpp"
    fi

    conda create -q -p $deps_dir -y $conda_pkgs
    source activate $deps_dir
fi

set +e
set +x
