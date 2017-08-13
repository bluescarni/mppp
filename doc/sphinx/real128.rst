Quadruple-precision floating-point
==================================

.. note::

   The functionality described in this page is available only if mp++ was configured
   with the ``MPPP_WITH_QUADMATH`` option enabled (see the :ref:`installation instructions <installation>`).

.. versionadded:: 0.5

*#include <mp++/real128.hpp>*

The ``real128`` class
---------------------

.. doxygenclass:: mppp::real128
   :members:

Types
-----

.. cpp:type:: __float128

   A quadruple-precision floating-point type available in the GCC compiler on most contemporary platforms.
   This is the type wrapped by the :cpp:class:`~mppp::real128` class.

   .. seealso::

      https://gcc.gnu.org/onlinedocs/gcc/Floating-Types.html

.. _real128_functions:

Functions
---------

.. _real128_io:

Input/Output
~~~~~~~~~~~~

.. doxygengroup:: real128_io
   :content-only:
