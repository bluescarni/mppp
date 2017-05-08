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

Additionally, `CMake <http://www.cmake.org/>`__ is the build system used by mp++ and it must also be installed when
installing from source (the minimum required version is 3.2).

Installation from source
------------------------

Source releases of mp++ can be downloaded from `github <https://github.com/bluescarni/mppp/releases>`__. Once in the source tree
of mp++, you can use ``cmake`` to configure the build to your liking (e.g., enabling optional features, customizing the installation
path, etc.). Since mp++ is a header-only library, there's no compilation step, and the installation of mp++ via ``make install`` or
similar will just copy the headers to your ``INSTALL_PREFIX``, in the ``include`` subdirectory.

After the installation of the headers, you can test the installation with the following simple ``main.cpp``:

.. code-block:: c++

   #include <iostream>
   #include <mp++/mp++.hpp>

   using int_t = mppp::integer<1>;

   int main()
   {
       int_t n{42};
       std::cout << n << '\n';
   }

If mp++ was installed in a standard prefix, on a typical GNU/Linux system you can compile this example with the following command:

.. code-block:: console

   $ g++ -std=c++11 main.cpp -lgmp

If you installed mp++ with the optional MPFR support turned on, you will need to link in the MPFR library as well:

.. code-block:: console

   $ g++ -std=c++11 main.cpp -lmpfr -lgmp
