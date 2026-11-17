# mp++
[![Build Status](https://img.shields.io/github/actions/workflow/status/bluescarni/mppp/gh_actions_ci.yml?branch=master&style=for-the-badge)](https://github.com/bluescarni/mppp/actions?query=workflow%3A%22GitHub+CI%22)
![language](https://img.shields.io/badge/language-C%2B%2B20-red.svg?style=for-the-badge)
[![Code Coverage](https://img.shields.io/codecov/c/github/bluescarni/mppp.svg?style=for-the-badge)](https://codecov.io/github/bluescarni/mppp?branch=master)
[![Anaconda-Server Badge](https://img.shields.io/conda/vn/conda-forge/mppp.svg?style=for-the-badge)](https://anaconda.org/conda-forge/mppp)
<!-- [![DOI](https://zenodo.org/badge/66504757.svg)](https://doi.org/10.5281/zenodo.1043579) -->

<!-- PROJECT LOGO -->
<br />
<p align="center">
  <p align="center">
    Multiprecision for C++
    <br />
    <a href="https://bluescarni.github.io/mppp/index.html"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/bluescarni/mppp/issues/new/choose">Report bug</a>
    ·
    <a href="https://github.com/bluescarni/mppp/issues/new/choose">Request feature</a>
    ·
    <a href="https://github.com/bluescarni/mppp/discussions">Discuss</a>
  </p>
</p>

mp++ is a C++20 library for multiprecision arithmetic, featuring:

* arbitrary-precision integers,
* arbitrary-precision rationals,
* quadruple-precision floats,
* quadruple-precision complex numbers,
* arbitrary-precision floats,
* arbitrary-precision complex numbers.

Design goals include:

* excellent performance for small integer and rational
  operands,
* easy embeddability in computer algebra systems and
  generic C++ libraries,
* a large collection of arbitrary-precision special functions,
* an ergonomic API based on modern C++ idioms.

Design non-goals include:

* support for fixed-size wide integrals (i.e., no ``uint512_t``,
  try [Boost Multiprecision](https://www.boost.org/doc/libs/1_72_0/libs/multiprecision/doc/html/index.html)
  instead).

Built on top of the GNU multiprecision stack ([GMP](https://gmplib.org/),
[MPFR](https://www.mpfr.org), [MPC](http://www.multiprecision.org/mpc/)),
mp++ was initially conceived as a GMP wrapper with special focus on performance with
small operands. In particular, a small buffer optimisation and custom implementations of basic mathematical primitives are
instrumental in achieving a performance increase, with respect to GMP and other integer multiprecision libraries, which can be
substantial (see the [benchmarks](https://bluescarni.github.io/mppp/benchmarks.html) section of the documentation).

Over time, mp++ has accrued many additional features, including
multiprecision real and complex number types, and an expanding
library of special functions (built in part on top of the excellent
[Arb](https://arblib.org/) library).

mp++ is a spinoff of the [Piranha](https://github.com/bluescarni/piranha) library, released under the
[MPL2](https://www.mozilla.org/en-US/MPL/2.0/FAQ/) license.
If you are using mp++ as part of your research, teaching, or other activities, we would be grateful if you could star
the repository and/or cite our work. The DOI of the latest version and other citation resources are available
at [this link](https://doi.org/10.5281/zenodo.1043579).

The documentation is available [here](https://bluescarni.github.io/mppp/).

