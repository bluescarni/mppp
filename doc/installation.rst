.. _installation:

Installation
============

Introduction
------------

mp++ is written in modern C++, and it requires a compiler able to understand
at least C++11 (mp++ will also use features from C++14, C++17 and C++20,
if supported by the compiler). The library is regularly tested on
a comprehensive continuous integration pipeline, which includes:

* various versions of the three major compilers (GCC, Clang and MSVC [#f1]_),
* various versions of the three major operating systems
  (Linux, Windows and OSX),
* x86, ARM and PowerPC processors.

mp++ has the following dependencies:

* the `GMP <https://gmplib.org/>`__ library, **mandatory** (GMP 5 and later versions are supported,
  the `MPIR <http://mpir.org/>`__ fork of GMP can also be used);
* the `GNU MPFR <https://www.mpfr.org>`__ multiprecision floating-point library, *optional*, used in the implementation
  of the :cpp:class:`~mppp::real` class and for providing support
  for the ``long double`` type in :cpp:class:`~mppp::integer` and :cpp:class:`~mppp::rational`
  (MPFR 3 or a later version is required);
* the `GNU MPC <http://www.multiprecision.org/mpc/>`__ multiprecision complex library, *optional*, used in the implementation
  of the :cpp:class:`~mppp::complex` class;
* the `Arb <https://arblib.org/>`__ and `FLINT <http://flintlib.org/>`__ libraries, *optional*,
  used in the implementation of additional special functions for the
  :cpp:class:`~mppp::real` and :cpp:class:`~mppp::complex` classes, and in the benchmarking
  suite;
* the `quadmath library <https://gcc.gnu.org/onlinedocs/libquadmath/>`__ from GCC, *optional*, used
  in the implementation of the :cpp:class:`~mppp::real128` and :cpp:class:`~mppp::complex128` classes
  (typically, the quadmath library is part of GCC and it does not need to
  be installed separately);
* the `Boost <https://www.boost.org/>`__ libraries, *optional*, currently used
  for implementing (de)serialisation and in the benchmarking suite;
* the `{fmt} <https://fmt.dev/latest/index.html>`__ library (at least version 6.2), *optional*, currently used
  only in the benchmarking suite.

Additionally, `CMake <https://cmake.org/>`__ is the build system used by mp++ and it must also be available when
installing from source (the minimum required version is 3.8).

.. rubric:: Footnotes

.. [#f1] The Intel compiler is also occasionally tested, but it is not part of the continuous
     integration setup.

Installation from source
------------------------

Source releases of mp++ can be downloaded from
`github <https://github.com/bluescarni/mppp/releases>`__.
Once in the source tree
of mp++, you can use ``cmake`` to configure the build to your liking
(e.g., enabling optional features, customizing the installation
path, etc.). The available configuration options are:

* ``MPPP_WITH_MPFR``: enable features relying on the GNU
  MPFR library (off by default),
* ``MPPP_WITH_MPC``: enable features relying on the GNU
  MPC library (off by default, requires the ``MPPP_WITH_MPFR``
  option to be active),
* ``MPPP_WITH_ARB``: enable features relying on the Arb library
  (off by default, requires the ``MPPP_WITH_MPFR`` option to be active),
* ``MPPP_WITH_QUADMATH``: enable features relying on the
  quadmath library (off by default),
* ``MPPP_WITH_BOOST_S11N``: enable support for serialisation
  via the Boost.serialization library (off by default),
* ``MPPP_BUILD_TESTS``: build the test suite (off by default),
* ``MPPP_BUILD_BENCHMARKS``: build the benchmarking suite (off by default),
* ``MPPP_BUILD_STATIC_LIBRARY``: build mp++ as a static library, instead
  of a dynamic library (off by default),
* ``MPPP_MSVC_UNICODE``: enable Unicode solutions for MSVC (available only
  when using MSVC, off by default),
* ``MPPP_ENABLE_IPO``: enable link-time optimisations when building
  the mp++ library (requires CMake >= 3.9 and compiler support,
  off by default).

.. versionadded:: 0.5

   The ``MPPP_WITH_MPFR`` and ``MPPP_WITH_QUADMATH`` build options.

.. versionadded:: 0.15

   The ``MPPP_BUILD_STATIC_LIBRARY`` and ``MPPP_MSVC_UNICODE`` build options.

.. versionadded:: 0.19

   The ``MPPP_WITH_ARB`` build option.

.. versionadded:: 0.20

   The ``MPPP_WITH_MPC`` and ``MPPP_ENABLE_IPO`` build options.

.. versionadded:: 0.22

   The ``MPPP_WITH_BOOST_S11N`` build option.

Note that the ``MPPP_WITH_QUADMATH`` option, at this time, is available only
using GCC (all the supported versions), Clang
(since version 3.9) and the Intel compiler. When this option is active,
mp++ needs access at build time to both the quadmath header
``quadmath.h`` and the quadmath library
``libquadmath.so``, which may be installed in
non-standard locations. While GCC is typically
able to resolve the correct paths automatically, the other compilers
might need assistance
in order to identify the correct locations of these files.

To build mp++, you can run the following CMake command from the
build directory:

.. code-block:: console

   $ cmake --build .

To install mp++, you can use the following CMake command:

.. code-block:: console

   $ cmake  --build . --target install

The installation command will copy the mp++ headers and library to the
``CMAKE_INSTALL_PREFIX`` directory.

If you enabled the ``MPPP_BUILD_TESTS`` option, you can run the test suite
with the following command:

.. code-block:: console

   $ cmake  --build . --target test

If you enabled the ``MPPP_BUILD_BENCHMARKS`` option, you can run the benchmark
suite with the following command:

.. code-block:: console

   $ cmake  --build . --target benchmark

.. note::

   On Windows, and if mp++ is built as a shared library (the default),
   in order to execute the test or the benchmark suite you have to ensure that the
   ``PATH`` variable includes the directory that contains the mp++
   DLL (otherwise the tests will fail to run).


Packages
--------

mp++ is also available from a variety of package managers on various platforms.

Conda
^^^^^

.. versionadded:: 0.2

mp++ is available in the `conda <https://conda.io/en/latest/>`__ package manager from the
`conda-forge <https://conda-forge.org/>`__ channel. Packages for Linux, Windows
and OSX are available. In order to install mp++ via conda, you just need
to add ``conda-forge`` to the channels:

.. code-block:: console

   $ conda config --add channels conda-forge
   $ conda config --set channel_priority strict
   $ conda install mppp

(note that the `conda package <https://anaconda.org/conda-forge/mppp>`__ for mp++ is named ``mppp`` rather than ``mp++``)

Please refer to the `conda documentation <https://conda.io/en/latest/>`__ for instructions on how to setup and manage
your conda installation.

FreeBSD
^^^^^^^

A FreeBSD port via `pkg <https://www.freebsd.org/ports/>`__ has been created for
mp++. In order to install mp++ using pkg, execute the following command:

.. code-block:: console

   $ pkg install mppp


Checking the installation
-------------------------

You can test the installation of mp++ with the following
simple ``main.cpp`` program:

.. code-block:: c++

   #include <iostream>
   #include <mp++/mp++.hpp>

   using int_t = mppp::integer<1>;

   int main()
   {
       int_t n{42};
       std::cout << n << '\n';
   }

If mp++ is installed in a standard prefix, on a typical GNU/Linux
system you can compile this example with the following command:

.. code-block:: console

   $ g++ -std=c++11 main.cpp -lmp++ -lgmp

.. note::

   The ``-std=c++11`` flag is not necessary if your GCC version is recent enough (i.e., for GCC 6 and later).

Because parts of mp++ are implemented using templates,
users of the library will have to explicitly link to GMP
and (if enabled) MPFR, MPC and Boost.serialization.
Explicit linking to the other optional
dependencies is not necessary, as their use is confined within
the mp++ compiled library.

If you are using CMake, it is highly recommended to make use of the config-file
package provided with mp++ rather
than locating and linking manually the required dependencies
(see the next section).

.. note::

   Unless the definition ``NDEBUG`` is activated at compile time, mp++ runs extensive
   internal debug checks at runtime which carry a large performance penalty. Users are advised
   to always define ``NDEBUG`` when compiling code using mp++ in ``Release`` builds.

Including mp++ in your project via CMake
----------------------------------------

.. versionadded:: 0.2

As a part of the mp++ installation, a group of CMake files is installed into
``CMAKE_INSTALL_PREFIX/lib/cmake/mp++``.
This bundle, which is known in the CMake lingo as a
`config-file package <https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html>`__,
facilitates the detection and use of mp++ from other CMake-based projects.
mp++'s config-file package, once loaded, provides
an imported target called ``mp++::mp++`` which encapsulates all the information
necessary to use mp++. That is, linking to
``mp++::mp++`` ensures that mp++'s include directories are added to the include
path of the compiler, and that the libraries
on which mp++ depends (e.g., GMP) are brought into the link chain.

For instance, a ``CMakeLists.txt`` file for the simple ``main.cpp``
program presented earlier may look like this:

.. code-block:: cmake

  # mp++ requires at least CMake 3.8.
  cmake_minimum_required(VERSION 3.8.0)

   # The name of our project.
   project(sample_project)

   # Look for an installation of mp++ in the system.
   find_package(mp++ REQUIRED)

   # Create an executable, and link it to the mp++::mp++ imported target.
   # This ensures that, in the compilation of 'main', mp++'s include
   # dirs are added to the include path of the compiler and that mp++'s
   # dependencies (e.g., GMP) are transitively linked to 'main'.
   add_executable(main main.cpp)
   target_link_libraries(main mp++::mp++)

.. versionadded:: 0.22

mp++'s config-file package also exports the following boolean
variables to signal with which optional dependencies mp++ was compiled:

* ``mp++_WITH_MPFR`` if MPFR support was enabled,
* ``mp++_WITH_MPC`` if MPC support was enabled,
* ``mp++_WITH_ARB`` if Arb support was enabled,
* ``mp++_WITH_QUADMATH`` if quadmath support was enabled,
* ``mp++_WITH_BOOST_S11N`` if Boost.serialization support was enabled.

.. _inst_plat_specific:

Compiler and platform specific notes
------------------------------------

Visual Studio:

* The mp++ library is compiled with the ``NOMINMAX`` and
  ``WIN32_LEAN_AND_MEAN`` definitions, and,
  if supported, with the ``/permissive-`` compiler flag.
* If the ``MPPP_MSVC_UNICODE`` CMake option is enabled, the mp++ library
  is compiled with the ``UNICODE`` and ``_UNICODE`` definitions.
* When building mp++ as a static library, MSVC's static runtime will
  be used (instead of the dynamic runtime). One can force the use
  of the dynamic runtime when building mp++ as a static library by
  turning on the ``MPPP_BUILD_STATIC_LIBRARY_WITH_DYNAMIC_MSVC_RUNTIME``
  advanced CMake option.

Clang:

* On Clang<7, :cpp:type:`__float128` cannot be used in mixed-mode
  operations with ``long double``. Accordingly,
  :cpp:class:`~mppp::real128` will disable interoperability with
  ``long double`` if Clang<7 is being used.

Intel compiler:

* The Intel compiler does not implement certain :cpp:type:`__float128`
  floating-point primitives
  as constant expressions. As a result, a few :cpp:class:`~mppp::real128`
  functions which are ``constexpr`` on GCC and Clang are not ``constexpr``
  when using the Intel compiler. These occurrences are marked in the API
  reference. Also, the Intel compiler seems to be prone to internal
  errors when performing ``constexpr`` computations with
  :cpp:class:`~mppp::real128` and :cpp:class:`~mppp::complex128`.

MinGW:

* Due to a compiler bug in the implementation of ``thread_local``
  storage [#mingw_tls]_,
  certain performance optimisations are disabled
  when compiling with MinGW.

OSX:

* When using older versions of Xcode, performance in multi-threading
  scenarios might be reduced due to lack of support for the C++11
  ``thread_local`` feature.

FreeBSD:

* The ``long double`` overloads of some mathematical functions
  (such as ``std::pow()``) may be implemented in
  ``double`` precision. Additionally, if the arguments to such mathematical
  functions are compile-time constants, the compiler
  *may* decide (depending on the optimisation level) to actually compute the
  result at compile time using full ``long double`` precision.
  This behaviour can lead to subtle inconsistencies, and it results in one
  test case from the mp++ test suite failing on FreeBSD [#freebsd_mppp_bug]_.

.. rubric:: Footnotes

.. [#mingw_tls] https://sourceforge.net/p/mingw-w64/bugs/445/
.. [#freebsd_mppp_bug] https://github.com/bluescarni/mppp/issues/132
