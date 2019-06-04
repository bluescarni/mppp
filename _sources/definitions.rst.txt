Macros and definitions
======================

.. versionadded:: 0.6

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

.. c:macro:: MPPP_HAVE_GCC_INT128

   This name is defined if mp++ detects the presence of the GCC-style 128-bit integers
   ``__int128_t`` and ``__uint128_t``, available on some compiler/platform combinations.

.. c:macro:: MPPP_STATIC_BUILD

   .. versionadded:: 0.15

   This name is defined if mp++ was built as a static library, instead of a dynamic
   library (see the :ref:`installation instructions <installation>`).

.. c:macro:: MPPP_HAVE_STRING_VIEW

   .. versionadded:: 0.17

   This name is defined if mp++ detects the availability of the ``std::string_view``
   class.
