Changelog
=========

0.17 (unreleased)
-----------------

New
~~~

- Expose the gamma functions from the MPFR API
  for :cpp:class:`~mppp::real`
  (`#190 <https://github.com/bluescarni/mppp/pull/190>`__).

0.16 (25-05-2019)
-----------------

Fix
~~~

- Properly set the version numbers for the mp++ dynamic library
  (`#187 <https://github.com/bluescarni/mppp/pull/187>`__).

0.15 (24-05-2019)
-----------------

New
~~~

- Expose the hyperbolic functions from the MPFR API
  for :cpp:class:`~mppp::real`
  (`#184 <https://github.com/bluescarni/mppp/pull/184>`__).
- Add the possibility of generating Unicode MSVC solutions
  (`#183 <https://github.com/bluescarni/mppp/pull/183>`__).
- Finish exposing all the trigonometric functions from the MPFR API
  for :cpp:class:`~mppp::real`
  (`#180 <https://github.com/bluescarni/mppp/pull/180>`__).
- Add the possibility to build mp++ as a static library
  (`#176 <https://github.com/bluescarni/mppp/pull/176>`__).
- Add CircleCI to the continuous integration pipeline
  (`#173 <https://github.com/bluescarni/mppp/pull/173>`__).
- Implement the logarithm/exponential functions for :cpp:class:`~mppp::real`
  (`#172 <https://github.com/bluescarni/mppp/pull/172>`__).

Changes
~~~~~~~

- When compiled with MPFR version 4 or later, mp++ now ensures that
  thread-local and global caches are freed separately at thread exit
  and program shutdown
  (`#182 <https://github.com/bluescarni/mppp/pull/182>`__).
- Update the internal copy of Catch to the latest version, 2.7.2
  (`#181 <https://github.com/bluescarni/mppp/pull/181>`__).
- The MPFR cleanup function ``mpfr_free_cache()`` is now called
  at the end of every thread which creates at least
  one :cpp:class:`~mppp::real` object
  (`#180 <https://github.com/bluescarni/mppp/pull/180>`__).
- Implement a specialised version of the ``swap()`` primitive
  for :cpp:class:`~mppp::integer` and
  :cpp:class:`~mppp::rational` (`#174 <https://github.com/bluescarni/mppp/pull/174>`__).
- Improve the implementation of the less than/greater than operators for
  :cpp:class:`~mppp::integer`. Together with the ``swap()`` improvements,
  this change leads to a ~9% decrease in runtime for the
  ``integer1_sort_signed``
  benchmark (`#174 <https://github.com/bluescarni/mppp/pull/174>`__).
- Continue moving code from the headers into the compiled library (`#170 <https://github.com/bluescarni/mppp/pull/170>`__,
  `#172 <https://github.com/bluescarni/mppp/pull/172>`__).

Fix
~~~

- Fix two race conditions in the testing code
  (`#181 <https://github.com/bluescarni/mppp/pull/181>`__).
- The :cpp:class:`~mppp::zero_division_error` exception is now correctly
  marked as visible
  (`#180 <https://github.com/bluescarni/mppp/pull/180>`__).
- Add a workaround for a ``clang-cl`` bug (`#179 <https://github.com/bluescarni/mppp/pull/179>`__).
- Various build system and documentation improvements (`#172 <https://github.com/bluescarni/mppp/pull/172>`__).
- Fix a warning when building mp++ with older MSVC versions (`#170 <https://github.com/bluescarni/mppp/pull/170>`__).

0.14 (2019-04-11)
-----------------

New
~~~

- The :cpp:func:`~mppp::type_name()` function is now part of the public API
  (`#169 <https://github.com/bluescarni/mppp/pull/169>`__).
- :cpp:class:`~mppp::integer` and :cpp:class:`~mppp::rational` now respect the format
  flags in output streams (`#161 <https://github.com/bluescarni/mppp/pull/161>`__).

Changes
~~~~~~~

- mp++ does not depend on the DbgHelp library on Windows any more
  (`#169 <https://github.com/bluescarni/mppp/pull/169>`__).
- **BREAKING**: mp++ has now a compiled component. In order to use mp++, you will now have to
  both include the mp++ headers **and** link to the mp++ library
  (`#169 <https://github.com/bluescarni/mppp/pull/169>`__).
- Various improvements to the benchmarks (`#166 <https://github.com/bluescarni/mppp/pull/166>`__).
- **BREAKING**: the input stream operators have been removed from all classes
  (`#161 <https://github.com/bluescarni/mppp/pull/161>`__).

Fix
~~~

- Fix an issue in the build system when compiling the unit tests in release mode with MSVC (`#164 <https://github.com/bluescarni/mppp/pull/164>`__).
- Fixes for the demangler on OSX when 128-bit integers are involved (`#163 <https://github.com/bluescarni/mppp/pull/163>`__).
- Fix a build issue on OSX when the compiler is not Xcode (`#161 <https://github.com/bluescarni/mppp/pull/161>`__).

0.13 (2019-03-13)
-----------------

Changes
~~~~~~~

- Update copyright date (`#162 <https://github.com/bluescarni/mppp/pull/162>`__).
- Add a tutorial for :cpp:class:`~mppp::real128` (`#160 <https://github.com/bluescarni/mppp/pull/160>`__).
- Various build system improvements (`#159 <https://github.com/bluescarni/mppp/pull/159>`__).
- Update the internal copy of Catch to the latest version, 2.5.0 (`#158 <https://github.com/bluescarni/mppp/pull/158>`__).

Fix
~~~

- Fix a compilation error when using booleans as second arguments in the ``pow()`` and ``binomial()`` overloads of :cpp:class:`~mppp::integer`
  (`#162 <https://github.com/bluescarni/mppp/pull/162>`__).
- Work around a compilation error on MSVC when using C++17 (`#162 <https://github.com/bluescarni/mppp/pull/162>`__).
- Various documentation fixes (`#160 <https://github.com/bluescarni/mppp/pull/160>`__).

0.12 (2018-10-11)
-----------------

New
~~~

- Add a hash function for :cpp:class:`~mppp::real128` (`#157 <https://github.com/bluescarni/mppp/pull/157>`__).
- Add all the root functions from the GMP API to the :cpp:class:`~mppp::integer` API
  (`#156 <https://github.com/bluescarni/mppp/pull/156>`__).
- Add all the root functions from the MPFR API to the :cpp:class:`~mppp::real` API
  (`#154 <https://github.com/bluescarni/mppp/pull/154>`__).
- Add a specialisation of ``std::numeric_limits`` for :cpp:class:`~mppp::real128`
  (`#144 <https://github.com/bluescarni/mppp/pull/144>`__).

Changes
~~~~~~~

- Initialising a :cpp:class:`~mppp::real` with an invalid :cpp:type:`~mppp::real_kind` enum value now raises an
  exception, rather than initialising to NaN (`#153 <https://github.com/bluescarni/mppp/pull/153>`__).
- Switch to the sphinx material design theme for the documentation (`#153 <https://github.com/bluescarni/mppp/pull/153>`__).
- Update the internal copy of Catch to the latest version, 2.4.0 (`#152 <https://github.com/bluescarni/mppp/pull/152>`__).
- Various improvements to the GCD implementation for :cpp:class:`~mppp::integer`
  (`#150 <https://github.com/bluescarni/mppp/pull/150>`__).
- The addition/subtraction operators of :cpp:class:`~mppp::integer` now use the low-level :cpp:func:`~mppp::add_ui()`,
  :cpp:func:`~mppp::add_si()`, :cpp:func:`~mppp::sub_ui()` and :cpp:func:`~mppp::sub_si()` primitives when the other argument is a
  C++ integral (`#147 <https://github.com/bluescarni/mppp/pull/147>`__).
- Various documentation additions, improvements and fixes (`#146 <https://github.com/bluescarni/mppp/pull/146>`__,
  `#148 <https://github.com/bluescarni/mppp/pull/148>`__, `#149 <https://github.com/bluescarni/mppp/pull/149>`__,
  `#153 <https://github.com/bluescarni/mppp/pull/153>`__, `#155 <https://github.com/bluescarni/mppp/pull/155>`__).
- **BREAKING**: replace the ``integer_nbits_init`` tag structure with the strongly-typed :cpp:type:`mppp::integer_bitcnt_t` enum
  (`#145 <https://github.com/bluescarni/mppp/pull/145>`__).
- Ensure that :cpp:class:`~mppp::real128` is trivially copyable (`#144 <https://github.com/bluescarni/mppp/pull/144>`__).

Fix
~~~

- Fix various warnings issued by Clang 7 in the unit tests (`#157 <https://github.com/bluescarni/mppp/pull/157>`__).

0.11 (2018-05-22)
-----------------

New
~~~

- Wrap more functions from the quadmath API in :cpp:class:`~mppp::real128` (`#140 <https://github.com/bluescarni/mppp/pull/140>`__).

Changes
~~~~~~~

- The build system should now detect MPIR installations when looking for GMP (`#139 <https://github.com/bluescarni/mppp/pull/139>`__).
- Update the internal copy of Catch to the latest version, 2.2.2 (`#137 <https://github.com/bluescarni/mppp/pull/137>`__).

Fix
~~~

- Fix a couple of missing ``inline`` specifiers in the tests (`#143 <https://github.com/bluescarni/mppp/pull/143>`__).
- Fix a missing ``noexcept`` in the move constructor of :cpp:class:`~mppp::real128` (`#138 <https://github.com/bluescarni/mppp/pull/138>`__).

0.10 (2018-04-06)
-----------------

New
~~~

- Add a target in the build system to compile and run the benchmarks (`#135 <https://github.com/bluescarni/mppp/pull/135>`__).
- Extend the :cpp:func:`~mppp::add_ui()` and :cpp:func:`~mppp::sub_ui()` functions to work on all unsigned
  C++ integral types, and introduce corresponding :cpp:func:`~mppp::add_si()` and :cpp:func:`~mppp::sub_si()`
  functions for signed C++ integral types (`#131 <https://github.com/bluescarni/mppp/pull/131>`__).
- Initial version of the rational tutorial (`#130 <https://github.com/bluescarni/mppp/pull/130>`__).
- The demangler is now aware of cv-qualifiers and references (`#129 <https://github.com/bluescarni/mppp/pull/129>`__).

Changes
~~~~~~~

- **BREAKING**: the :cpp:func:`~mppp::add_ui()` function now **requires** an unsigned integral as the third argument
  (previously, the function could be invoked with a signed integral argument thanks to C++'s conversion rules).

Fix
~~~

- Fix a test failure on FreeBSD (`#134 <https://github.com/bluescarni/mppp/pull/134>`__).
- Various small documentation fixes (`#130 <https://github.com/bluescarni/mppp/pull/130>`__,
  `#135 <https://github.com/bluescarni/mppp/pull/135>`__).
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

- Performance improvements for :cpp:func:`mppp::integer::nbits()` on GCC and Clang (`#17 <https://github.com/bluescarni/mppp/pull/17>`__).

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
