.. _installation:

Installation
============

mp++ is written in modern C++, and it requires a C++11-capable compiler. It is known to run
on the following setups:

* GCC 4.8 and later versions on GNU/Linux 32/64-bit,
* Clang 3.8 on GNU/Linux 64-bit,
* MSVC 2015 on Windows 32/64-bit,
* Clang 3.9 on Windows 32/64-bit (with the ``clang-cl`` driver for MSVC),
* MinGW GCC 6 on Windows 64-bit,
* Intel compiler ICC 17 on GNU/Linux 64-bit (tested only occasionally).

mp++ has been written, tested and benchmarked on x86 processors, but it does not contain any architecture-specific code
and thus it should run on any architecture supported by GMP (as long as a decent C++11 compiler is available).
mp++ does use compiler-specific functionality (e.g., 128-bit `integers <https://gcc.gnu.org/onlinedocs/gcc/_005f_005fint128.html>`__
or `intrinsics <https://msdn.microsoft.com/en-us/library/windows/desktop/hh802933(v=vs.85).aspx>`__ on 64-bit
architectures) for the implementation of fast basic arithmetics. If such functionality is not available, mp++
will fall back to GMP's ``mpn_`` functions.

mp++ has the following dependencies:

* the `GMP <http://www.gmplib.org>`__ library, **mandatory** (GMP 5 and later versions are supported,
  the `MPIR <http://mpir.org/>`__ fork of GMP can also be used);
* the `GNU MPFR <http://www.mpfr.org>`__ multiprecision floating-point library, *optional*, currently used only for
  supporting the ``long double`` type (MPFR 3 or a later version is required);
* the `Boost <http://www.boost.org/>`__ and `FLINT <http://flintlib.org/>`__ libraries, *optional*, currently used
  only in the benchmarking suite.

Additionally, `CMake <http://www.cmake.org/>`__ is the build system used by mp++ and it must also be available when
installing from source (the minimum required version is 3.2).

Installation from source
------------------------

Source releases of mp++ can be downloaded from `github <https://github.com/bluescarni/mppp/releases>`__. Once in the source tree
of mp++, you can use ``cmake`` to configure the build to your liking (e.g., enabling optional features, customizing the installation
path, etc.). The available configuration options are:

* ``MPPP_WITH_MPFR``: enable features relying on the GNU MPFR library (off by default),
* ``MPPP_BUILD_TESTS``: build the test suite (off by default),
* ``MPPP_BUILD_BENCHMARKS``: build the benchmarking suite (off by default).

Since mp++ is a header-only library, there's no compilation step (unless the tests or the benchmarks are being built),
and the installation of mp++ via ``make install`` or similar will just copy the headers to your ``CMAKE_INSTALL_PREFIX``,
in the ``include`` subdirectory.

After the installation of the headers, you can test the installation with the following simple ``main.cpp`` program:

.. code-block:: c++

   #include <iostream>
   #include <mp++/mp++.hpp>

   using int_t = mppp::integer<1>;

   int main()
   {
       int_t n{42};
       std::cout << n << '\n';
   }

If mp++ is installed in a standard prefix, on a typical GNU/Linux system you can compile this example with the following command:

.. code-block:: console

   $ g++ -std=c++11 main.cpp -lgmp

If you installed mp++ with the optional MPFR support turned on, you will need to link in the MPFR library as well:

.. code-block:: console

   $ g++ -std=c++11 main.cpp -lmpfr -lgmp

Including mp++ in your project via CMake
----------------------------------------

As part of the mp++ installation, a group of CMake files is installed into ``CMAKE_INSTALL_PREFIX/lib/cmake/mp++``.
This bundle, which is known in the CMake lingo as a `config-file package <https://cmake.org/cmake/help/v3.2/manual/cmake-packages.7.html>`__,
facilitates the detection and use of mp++ from other CMake-based projects. mp++'s config-file package, once loaded, provides
an imported target called ``Mp++::mp++`` which encapsulate all the information necessary to use mp++. That is, linking to
``Mp++::mp++`` ensures that mp++'s include directories are added to the include path of the compiler, and that the libraries
on which mp++ depends (e.g., GMP) are brought into the link chain.

For instance, a ``CMakeLists.txt`` file for the simple ``main.cpp`` program presented earlier may look like this:

.. code-block:: cmake

   # mp++ needs at least CMake 3.2.
   cmake_minimum_required(VERSION 3.2.0)

   # The name of our project.
   project(sample_project)

   # Look for an installation of mp++ in the system.
   find_package(Mp++ REQUIRED)

   # Create an executable, and link it to the Mp++::mp++ imported target.
   # This ensures that, in the compilation of 'main', mp++'s include
   # dirs are added to the include path of the compiler and that mp++'s
   # dependencies (e.g., GMP) are transitively linked to 'main'.
   add_executable(main main.cpp)
   target_link_libraries(main Mp++::mp++)

   # This line indicates to your compiler
   # that C++11 is needed for the compilation.
   # Not strictly necessary with a recent-enough compiler.
   set_property(TARGET main PROPERTY CXX_STANDARD 11)
