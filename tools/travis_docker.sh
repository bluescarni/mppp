#!/usr/bin/env bash

# Echo each command.
set -x

# Exit on error.
set -e

docker pull ubuntu:focal
docker run --rm -e TRAVIS_TAG -v `pwd`:/mppp ubuntu:focal bash /mppp/tools/travis_ubuntu_ppc64.sh

set +e
set +x
