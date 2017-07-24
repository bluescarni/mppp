Changelog
=========

0.4 (unreleased)
----------------

New
~~~

- Implement the constructors from a range of characters and from ``std::string_view`` for :cpp:class:`~mppp::integer`
  and :cpp:class:`~mppp::rational` (`#23 <https://github.com/bluescarni/mppp/pull/23>`__).

- Implement the assignment operator and the constructor from ``mpz_t`` in :cpp:class:`~mppp::rational`
  (`#19 <https://github.com/bluescarni/mppp/pull/19>`__).

Changes
~~~~~~~

- Performance improvements for :cpp:func:`mppp::integer::size()` (`#23 <https://github.com/bluescarni/mppp/pull/23>`__).

- Performance improvements for the construction/conversion of :cpp:class:`~mppp::integer` from/to C++ integrals
  (`#23 <https://github.com/bluescarni/mppp/pull/23>`__).

- Make sure the MPFR cleanup routine is automatically called on shutdown (`#22 <https://github.com/bluescarni/mppp/pull/22>`__).

- Performance improvements for :cpp:func:`mppp::integer::nbits()` on GCC and clang (`#17 <https://github.com/bluescarni/mppp/pull/17>`__).

Fix
~~~

- Various small documentation and build system fixes.

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
