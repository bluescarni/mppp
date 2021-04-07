#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install build-essential

# Install conda+deps.
wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh -O miniconda.sh
export deps_dir=$HOME/local
export PATH="$HOME/miniconda/bin:$PATH"
bash miniconda.sh -b -p $HOME/miniconda
conda config --add channels conda-forge
conda config --set channel_priority strict
conda_pkgs="cmake gmp mpfr libflint arb python=3.7 pybind11 mpc boost-cpp sphinx sphinx-book-theme myst-nb xeus-cling"
conda create -q -p $deps_dir -y
source activate $deps_dir
conda install $conda_pkgs -y

# Create the build dir and cd into it.
mkdir build
cd build

# Run the configuration step.
cmake ../ -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_CXX_STANDARD=17 -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=YES -DBoost_NO_BOOST_CMAKE=ON -DMPPP_WITH_BOOST_S11N=yes -DMPPP_WITH_MPFR=yes -DMPPP_WITH_MPC=yes -DMPPP_WITH_ARB=yes -DMPPP_WITH_QUADMATH=yes -DMPPP_TEST_PYBIND11=yes -DPYBIND11_PYTHON_VERSION=3.7 -DMPPP_ENABLE_IPO=yes

# Build the docs.
cd ../doc
export SPHINX_OUTPUT=`make html linkcheck 2>&1 >/dev/null`;

# Debug.
cat /home/circleci/project/doc/_build/html/reports/integer_arithmetic.log

if [[ "${SPHINX_OUTPUT}" != "" ]]; then
    echo "Sphinx encountered some problem:";
    echo "${SPHINX_OUTPUT}";
    exit 1;
fi
echo "Sphinx ran successfully";

if [[ ! -z "${CI_PULL_REQUEST}" ]]; then
    echo "Testing a pull request, the generated documentation will not be uploaded.";
    exit 0;
fi

if [[ "${CIRCLE_BRANCH}" != "master" ]]; then
    echo "Branch is not master, the generated documentation will not be uploaded.";
    exit 0;
fi

# Check out the gh_pages branch in a separate dir.
cd ../
git config --global push.default simple
git config --global user.name "CircleCI"
git config --global user.email "bluescarni@gmail.com"
set +x
git clone "https://${GH_TOKEN}@github.com/bluescarni/mppp.git" mppp_gh_pages -q
set -x
cd mppp_gh_pages
git checkout -b gh-pages --track origin/gh-pages;
git rm -fr *;
mv ../doc/_build/html/* .;
git add *;
# We assume here that a failure in commit means that there's nothing
# to commit.
git commit -m "Update Sphinx documentation, commit ${CIRCLE_SHA1} [skip ci]." || exit 0
PUSH_COUNTER=0
until git push -q
do
    git pull -q
    PUSH_COUNTER=$((PUSH_COUNTER + 1))
    if [ "$PUSH_COUNTER" -gt 3 ]; then
        echo "Push failed, aborting.";
        exit 1;
    fi
done

set +e
set +x
