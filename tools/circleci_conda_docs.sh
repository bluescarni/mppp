#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

# Core deps.
sudo apt-get install wget

# Install conda+deps.
wget https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-x86_64.sh -O mambaforge.sh
export deps_dir=$HOME/local
export PATH="$HOME/mambaforge/bin:$PATH"
bash mambaforge.sh -b -p $HOME/mambaforge
mamba create -y -p $deps_dir c-compiler cxx-compiler ninja cmake gmp mpfr 'libflint<3' arb mpc fmt libboost-devel 'sphinx=6.*' 'sphinx-book-theme=1.*' myst-nb xeus-cling
source activate $deps_dir

# Create the build dir and cd into it.
mkdir build
cd build

unset CFLAGS
unset CXXFLAGS

# Build and install
cmake ../ -G Ninja \
    -DCMAKE_PREFIX_PATH=$deps_dir \
    -DCMAKE_INSTALL_PREFIX=$deps_dir \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBoost_NO_BOOST_CMAKE=yes \
    -DMPPP_WITH_BOOST_S11N=yes \
    -DMPPP_WITH_FMT=yes \
    -DMPPP_WITH_MPFR=yes \
    -DMPPP_WITH_MPC=yes \
    -DMPPP_WITH_ARB=yes \
    -DMPPP_WITH_QUADMATH=yes

ninja -v -j4 install

# Build the docs.
cd ../doc
make html linkcheck

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
