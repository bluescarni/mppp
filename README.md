# mp++
[![Build Status](https://travis-ci.org/bluescarni/mppp.svg?branch=master)](https://travis-ci.org/bluescarni/mppp)
[![Build status](https://ci.appveyor.com/api/projects/status/github/bluescarni/mppp?branch=master&svg=true)](https://ci.appveyor.com/project/bluescarni/mppp)
[![codecov.io](https://codecov.io/github/bluescarni/mppp/coverage.svg?branch=master)](https://codecov.io/github/bluescarni/mppp?branch=master)
[![Docs](https://media.readthedocs.org/static/projects/badges/passing.svg)](http://bluescarni.github.io/mppp/)

mp++ is a C++11 header-only library for multiprecision arithmetic, currently supporting integers,
rationals and quadruple-precision floats. Based on well-known libraries such as [GMP](http://www.gmplib.org>),
[MPFR](http://www.mpfr.org), and others, mp++ has two main objectives:

* to maximise performance for small integers and rationals,
* to provide a modern, consistent and unified C++ interface to several lower-level multiprecision libraries.

mp++ adopts various techniques to improve performance on small multiprecision integers and rationals.
In particular, a small buffer optimisation and custom implementations of basic mathematical primitives are
instrumental in achieving a performance increase, with respect to GMP and other libraries, which can be
substantial (see the [benchmarks](https://bluescarni.github.io/mppp/benchmarks.html) section). The price to pay is a
small overhead when operating on large integers.

mp++ is a spinoff of the [Piranha](https://github.com/bluescarni/piranha) library. It was created to cater to
the requirements of computer algebra systems, which typically need to be able
to manipulate arbitrarily-large numbers but which, in practice, often end up storing many small values
(e.g., as coefficients in a polynomial or entries in a matrix).

mp++ is written in modern C++, and it requires a C++11-capable compiler. Currently it is continuously tested
on the following setups:

* GCC 4.8 and later versions on GNU/Linux 32/64-bit,
* Clang 3.8 and later versions on GNU/Linux 64-bit,
* MSVC 2015 on Windows 32/64-bit,
* Clang 4 on Windows 32/64-bit (with the ``clang-cl`` driver for MSVC),
* MinGW GCC 6 on Windows 64-bit,
* Clang on OSX 64-bit (Xcode 6.4 and later).

The Intel compiler ICC 17 on GNU/Linux 64-bit is tested occasionally.

mp++ is [MPL2](https://www.mozilla.org/en-US/MPL/2.0/FAQ/)-licensed. The documentation is available
[here](https://bluescarni.github.io/mppp/).
