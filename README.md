# mp++
[![Build Status](https://travis-ci.org/bluescarni/mppp.svg?branch=master)](https://travis-ci.org/bluescarni/mppp)
[![Build status](https://ci.appveyor.com/api/projects/status/github/bluescarni/mppp?branch=master&svg=true)](https://ci.appveyor.com/project/bluescarni/mppp)

mp++ is a small C++11 header-only library for doing multiprecision integer arithmetic. Based
on the well-known GMP library, mp++ places a strong emphasis on optimising operations on small values.
When operating on small operands, mp++ will:

* avoid heap memory allocations as much as possible, and
* use optimised implementations of basic operations (instead of calling the GMP functions).

The combination of these two techniques results typically in a x2-x3 performance increase, on small operands,
with respect to GMP. The price to pay is a small overhead when operating on large operands.

mp++ was created to cater to the requirements of computer algebra systems, which typically need to be able
to manipulate arbitrarily-large integers but which, in practice, often end up storing many small integers
(e.g., as coefficients in a polynomial).
