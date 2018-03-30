Changelog
=========

0.10 (unreleased)
-----------------

New
~~~

- Extend the :cpp:func:`~mppp::add_ui()` and :cpp:func:`~mppp::sub_ui()` functions to work on all unsigned
  C++ integral types, and introduce corresponding :cpp:func:`~mppp::add_si()` and :cpp:func:`~mppp::sub_si()`
  functions for signed C++ integral types (`#131 <https://github.com/bluescarni/mppp/pull/131>`__).
- Work on the rational tutorial (`#130 <https://github.com/bluescarni/mppp/pull/130>`__).
- The demangler is now aware of cv-qualifiers and references (`#129 <https://github.com/bluescarni/mppp/pull/129>`__).

Fix
~~~

- Various small documentation fixes (`#130 <https://github.com/bluescarni/mppp/pull/130>`__).
- Fix demangling failures for 128-bit integers in OSX (`#128 <https://github.com/bluescarni/mppp/pull/128>`__).

0.9 (2018-02-25)
----------------

New
~~~

- Add a couple of benchmarks against hardware integer types (`#124 <https://github.com/bluescarni/mppp/pull/124>`__).

Changes
~~~~~~~

- The :cpp:concept:`mppp::StringType` concept is now satisfied by cv qualified types as well
  (`#127 <https://github.com/bluescarni/mppp/pull/127>`__).

- Add a leading ``mppp::`` to the names of mp++'s classes in the pybind11 custom type casters
  (`#120 <https://github.com/bluescarni/mppp/pull/120>`__). This should be only a cosmetic change.

- Update the internal copy of Catch to the latest version, 2.1.1 (`#120 <https://github.com/bluescarni/mppp/pull/120>`__).

- Small tweaks/improvements to the build system and to the docs (`#118 <https://github.com/bluescarni/mppp/pull/118>`__,
  `#120 <https://github.com/bluescarni/mppp/pull/120>`__, `#121 <https://github.com/bluescarni/mppp/pull/121>`__,
  `#124 <https://github.com/bluescarni/mppp/pull/124>`__, `#126 <https://github.com/bluescarni/mppp/pull/126>`__).

Fix
~~~

- Fix a potential bug in the :cpp:class:`~mppp::real` printing code (`#123 <https://github.com/bluescarni/mppp/pull/123>`__).

- Fix a potential name shadowing issue in the pybind11 integration utilities (`#125 <https://github.com/bluescarni/mppp/pull/125>`__).

0.8 (2018-01-26)
----------------

New
~~~

- Add a function to check if a :cpp:class:`~mppp::real` is equal to one
  (`#117 <https://github.com/bluescarni/mppp/pull/117>`__).

- The pybind11 integration utilities now automatically translate mp++ exceptions into appropriate
  Python exceptions (`#115 <https://github.com/bluescarni/mppp/pull/115>`__).

- Expose various internal type traits in the public API (`#114 <https://github.com/bluescarni/mppp/pull/114>`__).

- Add an implementation of the binomial coefficient for rational top arguments
  (`#113 <https://github.com/bluescarni/mppp/pull/113>`__).

Changes
~~~~~~~

- When C++ concepts are enabled, various functions now use automatically-deduced return types
  to simplify the implementation and improve the generated documentation
  (`#114 <https://github.com/bluescarni/mppp/pull/114>`__).

- In the CMake config-file package produced by the installation process, ensure
  that the installed package version is considered compatible with any other version with the same
  major version number (`#113 <https://github.com/bluescarni/mppp/pull/113>`__).

Fix
~~~

- Fix a compilation error in the in-place operators of :cpp:class:`~mppp::real` when using concepts
  (`#116 <https://github.com/bluescarni/mppp/pull/116>`__).

- Fix a compilation error in the pybind11 utilities when mp++ is configured with quadmath support
  but without MPFR (`#114 <https://github.com/bluescarni/mppp/pull/114>`__).

0.7 (2018-01-11)
----------------

New
~~~

- Implement the initial version of the binary serialisation API (`#110 <https://github.com/bluescarni/mppp/pull/110>`__).

- Add builds based on MSVC 2017 in Appveyor (`#110 <https://github.com/bluescarni/mppp/pull/110>`__).

- Extend the :cpp:concept:`~mppp::CppInteroperable` concept to include all C++ integral types
  (`#104 <https://github.com/bluescarni/mppp/pull/104>`__).

- Add left bit shift benchmarks for :cpp:class:`~mppp::integer` (`#103 <https://github.com/bluescarni/mppp/pull/103>`__).

- Implement division without remainder (``tdiv_q()``) and exact division with positive divisor (``divexact_gcd()``)
  for :cpp:class:`~mppp::integer` (`#103 <https://github.com/bluescarni/mppp/pull/103>`__).

- Implement the ``trunc()`` and  ``integer_p()`` primitives for :cpp:class:`~mppp::real`
  (`#102 <https://github.com/bluescarni/mppp/pull/102>`__).

- Implement the :cpp:func:`~mppp::free_integer_caches()` function to manually free the caches used internally by
  :cpp:class:`~mppp::integer` (`#98 <https://github.com/bluescarni/mppp/pull/98>`__).

Changes
~~~~~~~

- Update copyright date (`#110 <https://github.com/bluescarni/mppp/pull/110>`__).

- Various updates to the documentation and to the benchmarks (`#107 <https://github.com/bluescarni/mppp/pull/107>`__,
  `#108 <https://github.com/bluescarni/mppp/pull/108>`__).

- Add an internal demangling utility to improve the quality of the error messages (`#105 <https://github.com/bluescarni/mppp/pull/105>`__).

- Various performance improvements for :cpp:class:`~mppp::integer` division, fused multiply-add, left bit shift,
  addition and multiplication
  (`#103 <https://github.com/bluescarni/mppp/pull/103>`__, `#106 <https://github.com/bluescarni/mppp/pull/106>`__,
  `#108 <https://github.com/bluescarni/mppp/pull/108>`__).

- Improve the detection of the availability of the ``thread_local`` keyword on recent Xcode versions 
  (`#99 <https://github.com/bluescarni/mppp/pull/99>`__).

0.6 (2017-12-05)
----------------

New
~~~

- Implement additional ``get()`` conversion functions for :cpp:class:`~mppp::real128` (`#96 <https://github.com/bluescarni/mppp/pull/96>`__).

- Implement the increment and decrement operators for :cpp:class:`~mppp::rational` (`#95 <https://github.com/bluescarni/mppp/pull/95>`__).

- Implement support for ``__int128_t`` and ``__uint128_t`` (`#90 <https://github.com/bluescarni/mppp/pull/90>`__).

- Implement the bitwise logic operators for :cpp:class:`~mppp::integer` (`#86 <https://github.com/bluescarni/mppp/pull/86>`__).

- Initial implementation of the :ref:`pybind11 integration utilities <tutorial_pybind11>` (`#81 <https://github.com/bluescarni/mppp/pull/81>`__).

- Implement the ``frexp()`` primitive for :cpp:class:`~mppp::real128` (`#81 <https://github.com/bluescarni/mppp/pull/81>`__).

- Implement the ``get/set_z_2exp()`` primitives for :cpp:class:`~mppp::real` (`#77 <https://github.com/bluescarni/mppp/pull/77>`__).

- Implement construction with preallocated storage for :cpp:class:`~mppp::integer` (`#74 <https://github.com/bluescarni/mppp/pull/74>`__).

- Implement construction from an array of limbs for :cpp:class:`~mppp::integer` (`#73 <https://github.com/bluescarni/mppp/pull/73>`__).

Changes
~~~~~~~

- Various additions to the tutorial (`#97 <https://github.com/bluescarni/mppp/pull/97>`__).

- **BREAKING**: the imported target created by the installation process has been renamed from ``Mp++`` to ``mp++``
  (`#94 <https://github.com/bluescarni/mppp/pull/94>`__).

- Take advantage of ``std::gcd()`` on C++17 (`#93 <https://github.com/bluescarni/mppp/pull/93>`__).

- Update the benchmark results for :cpp:class:`~mppp::integer` (`#91 <https://github.com/bluescarni/mppp/pull/91>`__).

- Add division benchmarks for :cpp:class:`~mppp::integer` (`#91 <https://github.com/bluescarni/mppp/pull/91>`__).

- A few performance tweaks for :cpp:class:`~mppp::integer` (`#91 <https://github.com/bluescarni/mppp/pull/91>`__).

- Simplifications in the bit shifting primitives for :cpp:class:`~mppp::integer` (`#85 <https://github.com/bluescarni/mppp/pull/85>`__).

- Split an :cpp:class:`~mppp::integer` test in two parts to curb memory usage during compilation (`#80 <https://github.com/bluescarni/mppp/pull/80>`__).

- Use bit counting intrinsics in MSVC (`#79 <https://github.com/bluescarni/mppp/pull/79>`__).

- Update the internal copy of Catch to the latest version, 2.0.1 (`#76 <https://github.com/bluescarni/mppp/pull/76>`__).

- Improve the performance of generic assignment for :cpp:class:`~mppp::integer` (`#74 <https://github.com/bluescarni/mppp/pull/74>`__).

- Improve construction from C++ integrals for :cpp:class:`~mppp::integer` (`#74 <https://github.com/bluescarni/mppp/pull/74>`__).

Fix
~~~

- Fix :cpp:class:`~mppp::integer` warnings in release mode (`#97 <https://github.com/bluescarni/mppp/pull/97>`__).

- Various internal cleanups in :cpp:class:`~mppp::integer` (`#80 <https://github.com/bluescarni/mppp/pull/80>`__,
  `#85 <https://github.com/bluescarni/mppp/pull/85>`__, `#86 <https://github.com/bluescarni/mppp/pull/86>`__).

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
