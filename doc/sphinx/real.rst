Multiprecision floats
=====================

.. note::

   The functionality described in this page is available only if mp++ was configured
   with the ``MPPP_WITH_MPFR`` option enabled (see the :ref:`installation instructions <installation>`).

.. versionadded:: 0.5

*#include <mp++/real.hpp>*

The ``real`` class
------------------

.. doxygenclass:: mppp::real
   :members:

Types
-----

.. cpp:type:: mpfr_prec_t

   An integral type defined by the MPFR library, used to represent the precision of ``mpfr_t``
   and (by extension) :cpp:class:`mppp::real` objects.

   .. seealso::

      http://www.mpfr.org/mpfr-current/mpfr.html#Nomenclature-and-Types

.. doxygenstruct:: mppp::real_prec
   :members:

Concepts
--------

.. _real_functions:

Functions
---------

.. _real_prec:

Precision handling
~~~~~~~~~~~~~~~~~~

.. doxygengroup:: real_prec
   :content-only:

.. _real_arithmetic:

Arithmetic
~~~~~~~~~~

.. doxygengroup:: real_arithmetic
   :content-only:

.. _real_comparison:

Comparison
~~~~~~~~~~

.. doxygengroup:: real_comparison
   :content-only:

.. _real_roots:

Roots
~~~~~

.. doxygengroup:: real_roots
   :content-only:

.. _real_exponentiation:

Exponentiation
~~~~~~~~~~~~~~

.. doxygengroup:: real_exponentiation
   :content-only:

.. _real_trig:

Trigonometry
~~~~~~~~~~~~

.. doxygengroup:: real_trig
   :content-only:

.. _real_logexp:

Logarithms and exponentials
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygengroup:: real_logexp
   :content-only:

.. _real_io:

Input/Output
~~~~~~~~~~~~

.. doxygengroup:: real_io
   :content-only:

.. _real_operators:

Operators
---------

.. doxygengroup:: real_operators
   :content-only:

.. _real_constants:

Constants
---------

.. doxygengroup:: real_constants
   :content-only:
