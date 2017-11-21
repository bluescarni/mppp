Changelog
=========

0.6 (unreleased)
----------------

New
~~~

- Implement :ref:`integration with pybind11 <tutorial_pybind11>` (`#81 <https://github.com/bluescarni/mppp/pull/81>`__).

- Implement the ``frexp()`` primitive for :cpp:class:`~mppp::real128` (`#81 <https://github.com/bluescarni/mppp/pull/81>`__).

- Implement the ``get/set_z_2exp()`` primitives for :cpp:class:`~mppp::real` (`#77 <https://github.com/bluescarni/mppp/pull/77>`__).

- Implement construction with preallocated storage for :cpp:class:`~mppp::integer` (`#74 <https://github.com/bluescarni/mppp/pull/74>`__).

- Implement construction from an array of limbs for :cpp:class:`~mppp::integer` (`#73 <https://github.com/bluescarni/mppp/pull/73>`__).

Changes
~~~~~~~

- Split an :cpp:class:`~mppp::integer` test in two parts to curb memory usage during compilation (`#80 <https://github.com/bluescarni/mppp/pull/80>`__).

- Use bit counting intrinsics in MSVC (`#79 <https://github.com/bluescarni/mppp/pull/79>`__).

- Update the internal copy of Catch to the latest version, 2.0.1 (`#76 <https://github.com/bluescarni/mppp/pull/76>`__).

- Improve the performance of generic assignment for :cpp:class:`~mppp::integer` (`#74 <https://github.com/bluescarni/mppp/pull/74>`__).

- Improve construction from C++ integrals for :cpp:class:`~mppp::integer` (`#74 <https://github.com/bluescarni/mppp/pull/74>`__).

Fix
~~~

- Various internal cleanups in :cpp:class:`~mppp::integer` (`#80 <https://github.com/bluescarni/mppp/pull/80>`__).

- Small fixes regarding the use of GMP type aliases in :cpp:class:`~mppp::integer` (`#73 <https://github.com/bluescarni/mppp/pull/73>`__).

0.5 (2017-11-07)
----------------

New
~~~

- Implement the :cpp:class:`~mppp::real` class (`#40 <https://github.com/bluescarni/mppp/pull/40>`__).

- Add non-throwing GMP-style conversion functions (`#59 <https://github.com/bluescarni/mppp/pull/59>`__,
  `#61 <https://github.com/bluescarni/mppp/pull/61>`__).

- Implement move constructors and move assignment operators from ``mpz_t`` and ``mpq_t`` for :cpp:class:`~mppp::integer`
  and :cpp:class:`~mppp::rational` (`#57 <https://github.com/bluescarni/mppp/pull/57>`__).

- Implement a cache for the allocation of limbs arrays in small :cpp:class:`~mppp::integer` objects
  (`#55 <https://github.com/bluescarni/mppp/pull/55>`__).

- Implement the :cpp:class:`~mppp::real128` class (`#31 <https://github.com/bluescarni/mppp/pull/31>`__).

- Implement the ``sub_ui()`` primitive for :cpp:class:`~mppp::integer` (`#37 <https://github.com/bluescarni/mppp/pull/37>`__).

- Add a CI build testing against the latest unstable GMP branch (`#34 <https://github.com/bluescarni/mppp/pull/34>`__).

- Add assignment operators from ``std::string_view`` for :cpp:class:`~mppp::integer` and :cpp:class:`~mppp::rational`
  (`#32 <https://github.com/bluescarni/mppp/pull/32>`__).

- Add the possibility of constructing non-canonical :cpp:class:`~mppp::rational` objects from numerator/denominator pairs
  (`#28 <https://github.com/bluescarni/mppp/pull/28>`__).

Changes
~~~~~~~

- Use the sphinx bootstrap theme for the html documentation (`#71 <https://github.com/bluescarni/mppp/pull/71>`__).

- Various simplifications in the :cpp:class:`~mppp::rational` API (`#66 <https://github.com/bluescarni/mppp/pull/66>`__).

- Introduce a :cpp:concept:`~mppp::StringType` concept and use it to reduce the number of overloads in the
  constructors/assignment operators from string (`#63 <https://github.com/bluescarni/mppp/pull/63>`__,
  `#64 <https://github.com/bluescarni/mppp/pull/64>`__).

- The :cpp:class:`~mppp::integer` functions accepting the return value as a parameter will now
  demote a return value with dynamic storage to static storage if the other arguments all have static storage
  (`#58 <https://github.com/bluescarni/mppp/pull/58>`__).

- The free functions for :cpp:class:`~mppp::integer` and :cpp:class:`~mppp::rational` now return a reference
  to the return value, rather than ``void`` (`#56 <https://github.com/bluescarni/mppp/pull/56>`__).

- Performance improvements and code simplifications for :cpp:class:`~mppp::integer` division
  (`#55 <https://github.com/bluescarni/mppp/pull/55>`__).

- Minor improvements in the static checks for the expected layouts of ``mpz_t`` and ``mpq_t``
  (`#53 <https://github.com/bluescarni/mppp/pull/53>`__, `#42 <https://github.com/bluescarni/mppp/pull/42>`__).

- Enable additional compiler warning flags in debug builds for GCC (`#52 <https://github.com/bluescarni/mppp/pull/52>`__).

- **BREAKING**: various improvements/changes to the bit shifting functions for :cpp:class:`~mppp::integer`,
  and the exception raised by the bit shifting operators is not any more
  ``std::domain_error``, it is now ``std::overflow_error`` (`#48 <https://github.com/bluescarni/mppp/pull/48>`__).

- Various updates to the benchmarks (`#39 <https://github.com/bluescarni/mppp/pull/39>`__).

- Use various C++17 standard library bits if available, and improve general C++17 compatibility
  (`#31 <https://github.com/bluescarni/mppp/pull/31>`__, `#37 <https://github.com/bluescarni/mppp/pull/37>`__).

- Update the internal copy of Catch to the latest version, 1.9.7 (`#36 <https://github.com/bluescarni/mppp/pull/36>`__).

- Bump up the minimum required CMake version to 3.3 (`#31 <https://github.com/bluescarni/mppp/pull/31>`__).

- Performance improvements and simplifications in the :cpp:class:`~mppp::rational` constructors and assignment operators
  (`#28 <https://github.com/bluescarni/mppp/pull/28>`__, `#32 <https://github.com/bluescarni/mppp/pull/32>`__).

Fix
~~~

- Fixes/improvements in the support for ``long double`` (`#50 <https://github.com/bluescarni/mppp/pull/50>`__,
  `#54 <https://github.com/bluescarni/mppp/pull/54>`__).

- Fix the compilation of the tests on Clang 5 (`#43 <https://github.com/bluescarni/mppp/pull/43>`__).

- Fix too lax constraints in the implementation of in-place operators for :cpp:class:`~mppp::integer` and
  :cpp:class:`~mppp::rational` (`#41 <https://github.com/bluescarni/mppp/pull/41>`__).

- Fix the PDF build of the documentation (`#39 <https://github.com/bluescarni/mppp/pull/39>`__).

- Fix a few missing ``inline`` specifiers (`#38 <https://github.com/bluescarni/mppp/pull/38>`__, `#41 <https://github.com/bluescarni/mppp/pull/41>`__).

- Fix C++ version detection on MSVC (`#36 <https://github.com/bluescarni/mppp/pull/36>`__).

- Fix missing tests for :cpp:class:`~mppp::rational` hashing (`#29 <https://github.com/bluescarni/mppp/pull/29>`__).

- Fix some MSVC warnings when compiling the tests in release mode (`#28 <https://github.com/bluescarni/mppp/pull/28>`__).

- Various minor documentation fixes.

0.4 (2017-07-29)
----------------

New
~~~

- Implement the constructors from a range of characters and from ``std::string_view`` for :cpp:class:`~mppp::integer`
  and :cpp:class:`~mppp::rational` (`#23 <https://github.com/bluescarni/mppp/pull/23>`__).

- Implement the assignment operator and the constructor from ``mpz_t`` in :cpp:class:`~mppp::rational`
  (`#19 <https://github.com/bluescarni/mppp/pull/19>`__).

Changes
~~~~~~~

- Expand CI to include GCC 7 in C++17 mode (`#27 <https://github.com/bluescarni/mppp/pull/27>`__).

- Improve testing coverage (`#25 <https://github.com/bluescarni/mppp/pull/25>`__).

- Various extensions to the benchmark suite (`#25 <https://github.com/bluescarni/mppp/pull/25>`__).

- Various performance improvements in :cpp:class:`~mppp::integer` thanks to the reduction of the number of branches
  in the implementation of basic arithmetic for the 1/2-limb specialisations (`#25 <https://github.com/bluescarni/mppp/pull/25>`__).

- Update the internal copy of Catch to the latest version, 1.9.6 (`#24 <https://github.com/bluescarni/mppp/pull/24>`__).

- Performance improvements for :cpp:func:`mppp::integer::size()` (`#23 <https://github.com/bluescarni/mppp/pull/23>`__).

- Performance improvements for the construction/conversion of :cpp:class:`~mppp::integer` from/to C++ integrals
  (`#23 <https://github.com/bluescarni/mppp/pull/23>`__).

- Make sure the MPFR cleanup routine is automatically called on shutdown (`#22 <https://github.com/bluescarni/mppp/pull/22>`__).

- Performance improvements for :cpp:func:`mppp::integer::nbits()` on GCC and clang (`#17 <https://github.com/bluescarni/mppp/pull/17>`__).

Fix
~~~

- Fix a build failure on older GMP versions (`#25 <https://github.com/bluescarni/mppp/pull/25>`__).

- Fix a build system bug when building the benchmarks with older CMake versions (`#25 <https://github.com/bluescarni/mppp/pull/25>`__).

- Various minor fixes.

0.3 (2017-06-12)
----------------

New
~~~

- Implement the multiprecision :cpp:class:`~mppp::rational` class (`#14 <https://github.com/bluescarni/mppp/pull/14>`__).

- Implement fast assignment functions to zero and plus/minus one for :cpp:class:`~mppp::integer`.

- Add assignment operators from string for :cpp:class:`~mppp::integer`.

- Implement the ``submul()`` primitive for :cpp:class:`~mppp::integer`.

- Implement the assignment operator from ``mpz_t`` in :cpp:class:`~mppp::integer`, and use it in various function
  in order to avoid the creation of a temporary.

Changes
~~~~~~~

- Performance improvements for the copy/move assignment operators of :cpp:class:`~mppp::integer`.

Fix
~~~

- Various small documentation fixes.

0.2 (2017-05-09)
----------------

New
~~~

- Provide a CMake config-file package as part of the install process.

- Implement the missing in-place modulo operator with C++ integrals
  on the left.

- Experimental support for C++ concepts.

- Support the ``clang-cl`` compiler on Windows.

- Add input stream operator.

- Add in-place arithmetic operators with interoperable types on the
  left-hand side.

- Add convenience overloads for the computation of the binomial
  coefficient.

- Add convenience overloads for ``pow()``.

- Add functions to test if an integer is equal to -1.

- Add a static member to ``integer`` storing the static size.

Changes
~~~~~~~

- Split out the library in multiple files.

- Rename the ``mp_integer`` class to ``integer``.

- Various improvements to the documentation.

- Rework the library interface to use regular functions rather than
  ``inline friend`` functions.

- Change the license to MPL2.

- Remove the allocation cache.

- Remove the custom namespace option.

Fix
~~~

- Fix operators example in the documentation.
