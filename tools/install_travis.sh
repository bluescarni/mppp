#!/usr/bin/env bash

# Echo each command
set -x

# Exit on error.
set -e

if [[ "${MPPP_BUILD}" != DebugGCC48DebugGMP && "${MPPP_BUILD}" != Coverage32GCC6 ]]; then
    export deps_dir=$HOME/local
    export PATH="$HOME/miniconda/bin:$PATH"
    export PATH="$deps_dir/bin:$PATH"
fi

if [[ "${MPPP_BUILD}" == "ReleaseGCC48" ]]; then
    CXX=g++-4.8 CC=gcc-4.8 cmake -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Release -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_MPFR=yes -DMPPP_WITH_QUADMATH=yes ../;
    make -j2 VERBOSE=1 install;
    ctest -V;

    # Test the CMake export installation.
    cd ../tools/sample_project;
    mkdir build;
    cd build;
    CXX=g++-4.8 CC=gcc-4.8 cmake ../ -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir;
    make;
    ./main;
elif [[ "${MPPP_BUILD}" == "DebugGCC48" ]]; then
    CXX=g++-4.8 CC=gcc-4.8 cmake -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_MPFR=yes -DMPPP_WITH_QUADMATH=yes -DCMAKE_CXX_FLAGS="-fsanitize=address" ../;
    make -j2 VERBOSE=1;
    ctest -V;
elif [[ "${MPPP_BUILD}" == "DebugGCC48DebugGMP" ]]; then
    # Download and compile locally GMP in debug mode.
    wget ftp://ftp.gnu.org/gnu/gmp/gmp-6.1.2.tar.bz2;
    tar xjvf gmp-6.1.2.tar.bz2;
    cd gmp-6.1.2;
    CXX=g++-4.8 CC=gcc-4.8 ./configure --disable-shared --enable-assert --enable-alloca=debug --disable-assembly CFLAGS="-g -fsanitize=address";
    make -j2;
    cd ..;
    CXX=g++-4.8 CC=gcc-4.8 cmake -DCMAKE_BUILD_TYPE=Debug -DMPPP_WITH_QUADMATH=yes -DMPPP_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-fsanitize=address" -DGMP_INCLUDE_DIR=$TRAVIS_BUILD_DIR/build/gmp-6.1.2 -DGMP_LIBRARY=$TRAVIS_BUILD_DIR/build/gmp-6.1.2/.libs/libgmp.a ../;
    make -j2 VERBOSE=1;
    ctest -V;
elif [[ "${MPPP_BUILD}" == "CoverageGCC5" ]]; then
    CXX=g++-5 CC=gcc-5 cmake -DCMAKE_CXX_STANDARD=14 -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_MPFR=yes -DMPPP_WITH_QUADMATH=yes -DCMAKE_CXX_FLAGS="--coverage" ../;
    make -j2 VERBOSE=1;
    ctest -V;
    bash <(curl -s https://codecov.io/bash) -x gcov-5;
elif [[ "${MPPP_BUILD}" == "CoverageGCC7" ]]; then
    CXX=g++-7 CC=gcc-7 cmake -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_MPFR=yes -DMPPP_WITH_QUADMATH=yes -DCMAKE_CXX_FLAGS="--coverage -fconcepts" ../;
    make -j2 VERBOSE=1;
    ctest -V;
    bash <(curl -s https://codecov.io/bash) -x gcov-7;
elif [[ "${MPPP_BUILD}" == "Coverage32GCC6" ]]; then
    wget ftp://ftp.gnu.org/gnu/gmp/gmp-6.1.2.tar.bz2;
    tar xjvf gmp-6.1.2.tar.bz2;
    cd gmp-6.1.2;
    CXX=g++-6 CC=gcc-6 ABI=32 ./configure --disable-shared;
    make -j2;
    cd ..;
    CXX=g++-6 CC=gcc-6 cmake -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DCMAKE_CXX_FLAGS="-m32 --coverage -D_GLIBCXX_CONCEPT_CHECKS -fconcepts" -DGMP_INCLUDE_DIR=$TRAVIS_BUILD_DIR/build/gmp-6.1.2 -DGMP_LIBRARY=$TRAVIS_BUILD_DIR/build/gmp-6.1.2/.libs/libgmp.a ../;
    make -j2 VERBOSE=1;
    ctest -V;
    bash <(curl -s https://codecov.io/bash) -x gcov-6;
elif [[ "${MPPP_BUILD}" == "DebugClang38" ]]; then
    CXX=clang++-3.8 CC=clang-3.8 cmake -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_MPFR=yes ../;
    make -j2 VERBOSE=1;
    ctest -V;
elif [[ "${MPPP_BUILD}" == "ReleaseClang38" ]]; then
    CXX=clang++-3.8 CC=clang-3.8 cmake -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Release -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_MPFR=yes ../;
    make -j2 VERBOSE=1;
    ctest -V;
elif [[ "${MPPP_BUILD}" == "OSXDebug" ]]; then
    CXX=clang++ CC=clang cmake -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DCMAKE_BUILD_TYPE=Debug -DMPPP_BUILD_TESTS=yes -DMPPP_WITH_MPFR=yes ../;
    make -j2 VERBOSE=1;
    ctest -V;
# elif [[ "${MPPP_BUILD}" == "ICC" ]]; then
#     set +x;
#     docker login -u=bluescarni -p=${DOCKER_CLOUD_PWD};
#     set -x;
#     docker pull ${DOCKER_IMAGE};
elif [[ "${MPPP_BUILD}" == "Documentation" ]]; then
    # Run the configure step to create the doc config files.
    CXX=g++-5 CC=gcc-5 cmake -DCMAKE_INSTALL_PREFIX=$deps_dir -DCMAKE_PREFIX_PATH=$deps_dir -DMPPP_WITH_MPFR=yes ../;

    cd ..;
    cd doc/doxygen;
    export DOXYGEN_OUTPUT=`doxygen 2>&1 >/dev/null`;
    if [[ "${DOXYGEN_OUTPUT}" != "" ]]; then
        echo "Doxygen encountered some problem:";
        echo "${DOXYGEN_OUTPUT}";
        exit 1;
    fi
    cd ../sphinx;
    pip install --user 'sphinx<1.6' breathe requests[security]
    export SPHINX_OUTPUT=`make html 2>&1 >/dev/null`;
    if [[ "${SPHINX_OUTPUT}" != "" ]]; then
        echo "Sphinx encountered some problem:";
        echo "${SPHINX_OUTPUT}";
        exit 1;
    fi
    echo "Sphinx ran successfully";
    if [[ "${TRAVIS_PULL_REQUEST}" != "false" ]]; then
        echo "Testing a pull request, the generated documentation will not be uploaded.";
        exit 0;
    fi
    if [[ "${TRAVIS_BRANCH}" != "master" ]]; then
        echo "Branch is not master, the generated documentation will not be uploaded.";
        exit 0;
    fi
    # Move out the resulting documentation.
    mv _build/html /home/travis/sphinx;
    # Checkout a new copy of the repo, for pushing to gh-pages.
    cd ../../../;
    git config --global push.default simple
    git config --global user.name "Travis CI"
    git config --global user.email "bluescarni@gmail.com"
    set +x
    git clone "https://${GH_TOKEN}@github.com/bluescarni/mppp.git" mppp_gh_pages -q
    set -x
    cd mppp_gh_pages
    git checkout -b gh-pages --track origin/gh-pages;
    git rm -fr *;
    mv /home/travis/sphinx/* .;
    git add *;
    # We assume here that a failure in commit means that there's nothing
    # to commit.
    git commit -m "Update Sphinx documentation, commit ${TRAVIS_COMMIT} [skip ci]." || exit 0
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
fi
