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

The only mandatory dependency is the `GMP <http://www.gmplib.org>`__ library (GMP 5 and later versions are supported).
The `MPIR <http://mpir.org/>`__ fork of GMP can also be used as a drop-in replacement for GMP.
In order to interoperate with the ``long double`` C++ type, mp++ optionally depends
on the `GNU MPFR <http://www.mpfr.org>`__ multiprecision floating-point library (see the
:ref:`configuration <configuration>` section - MPFR 3 or a later version is required).
The benchmark suite depends optionally on `Boost <http://www.boost.org/>`__ and `FLINT <http://flintlib.org/>`__.

Releases of mp++ can be downloaded from `github <https://github.com/bluescarni/mppp/releases>`__.
In order to use mp++, you only have to include the ``include/mp++.hpp`` header in your project and link to the GMP
(or MPIR) library when compiling. E.g., on a Unix-like operating system with GCC, you would compile your executable as:

.. code-block:: console

   $ g++ -std=c++11 my_executable.cpp -lgmp

If you require interoperability with ``long double``, link in the MPFR library as well and activate the ``MPPP_WITH_LONG_DOUBLE``
preprocessor definition:

.. code-block:: console

   $ g++ -std=c++11 -DMPPP_WITH_LONG_DOUBLE my_executable.cpp -lmpfr -lgmp
