Changelog
=========

0.2 (2017-05-09)
----------------

New
~~~

- Provide a CMake config-file package as part of the install process.
  [Francesco Biscani]

- Implement the missing in-place modulo operator with C++ integrals
  on the left. [Francesco Biscani]

- Experimental support for C++ concepts. [Francesco Biscani]

- Support the ``clang-cl`` compiler on Windows. [Francesco Biscani]

- Add input stream operator. [Francesco Biscani]

- Add in-place arithmetic operators with interoperable types on the
  left-hand side. [Francesco Biscani]

- Add convenience overloads for the computation of the binomial
  coefficient. [Francesco Biscani]

- Add convenience overloads for ``pow()``. [Francesco Biscani]

- Add functions to test if an integer is equal to -1. [Francesco
  Biscani]

- Add a static member to ``integer`` storing the static size. [Francesco
  Biscani]

Changes
~~~~~~~

- Split out the library in multiple files. [Francesco Biscani]

- Rename the ``mp_integer`` class to ``integer``. [Francesco Biscani]

- Various improvements to the documentation. [Francesco Biscani]

- Rework the library interface to use regular functions rather than
  ``inline friend`` functions. [Francesco Biscani]

- Change the license to MPL2. [Francesco Biscani]

- Remove the allocation cache. [Francesco Biscani]

- Remove the custom namespace option. [Francesco Biscani]

Fix
~~~

- Fix operators example in the documentation. [Francesco Biscani]
