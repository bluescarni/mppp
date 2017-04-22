.. _configuration:

Configuration
=============

The library has a few configuration switches in the form of preprocessor definitions. These can be set
on the compiler command-line or in the source code **before** including the headers. The switches are:

* ``MPPP_WITH_LONG_DOUBLE``: if defined, mp++ will be able to interoperate with the ``long double`` C++ type (whereas
  normally floating-point interoperability is limited to the ``float`` and ``double`` types). This option requires
  the `GNU MPFR <http://www.mpfr.org>`__ multiprecision floating-point library.
