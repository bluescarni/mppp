# mp++
[![Build Status](https://travis-ci.org/bluescarni/mppp.svg?branch=master)](https://travis-ci.org/bluescarni/mppp)
[![Build status](https://ci.appveyor.com/api/projects/status/github/bluescarni/mppp?branch=master&svg=true)](https://ci.appveyor.com/project/bluescarni/mppp)
[![codecov.io](https://codecov.io/github/bluescarni/mppp/coverage.svg?branch=master)](https://codecov.io/github/bluescarni/mppp?branch=master)
[![Docs](https://readthedocs.org/projects/pip/badge/?version=latest)](http://bluescarni.github.io/mppp/)

mp++ is a small C++11 header-only library for multiprecision integer arithmetic. Based
on the well-known GMP library, mp++ places a strong emphasis on optimising operations on small values.
When operating on small operands, mp++ will:

* avoid heap memory allocations as much as possible, and
* use optimised implementations of basic operations (instead of calling GMP functions).

The combination of these two techniques results in a performance increase, on small operands,
with respect to GMP (see the [benchmarks](https://bluescarni.github.io/mppp/benchmarks.html) section of the
documentation). The price to pay is a small overhead when operating on large operands.

mp++ was created to cater to the requirements of computer algebra systems, which typically need to be able
to manipulate arbitrarily-large integers but which, in practice, often end up storing many small integers
(e.g., as coefficients in a polynomial).

mp++ is written in modern C++, and it requires a C++11-capable compiler. Currently it is continuously tested
on the following setups:

* GCC 4.8 and later versions on GNU/Linux 32/64-bit,
* Clang 3.8 on GNU/Linux 64-bit,
* MSVC 2015 on Windows 32/64-bit,
* MinGW GCC 6 on Windows 64-bit.

The Intel compiler ICC 17 on GNU/Linux 64-bit is tested occasionally.

mp++ is [MPL2](https://www.mozilla.org/en-US/MPL/2.0/FAQ/)-licensed. The documentation is available
[here](https://bluescarni.github.io/mppp/).
