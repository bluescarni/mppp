Macros and definitions
======================

*#include <mp++/config.hpp>*

.. c:macro:: MPPP_VERSION_STRING

   This definition expands to a string literal containing the full version of the mp++ library
   (e.g., for version 0.6 this macro expands to ``"0.6"``).

.. c:macro:: MPPP_VERSION_MAJOR

   This definition expands to an integral literal corresponding to the major mp++ version (e.g.,
   for version 0.6, this macro expands to ``0``).

.. c:macro:: MPPP_VERSION_MINOR

   This definition expands to an integral literal corresponding to the minor mp++ version (e.g.,
   for version 0.6, this macro expands to ``6``).

.. c:macro:: MPPP_WITH_QUADMATH

   This name is defined if mp++ was configured with support for the quadmath library
   (see the :ref:`installation instructions <installation>`).

.. c:macro:: MPPP_WITH_MPFR

   This name is defined if mp++ was configured with support for the MPFR library
   (see the :ref:`installation instructions <installation>`).
