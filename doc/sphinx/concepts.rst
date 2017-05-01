General concepts
================

.. cpp:concept:: template <typename T> mppp::CppInteroperable

   This concept is satisfied by any C++ fundamental type with which the multiprecision classes (such as :cpp:class:`~mppp::integer`)
   can interoperate. The full list of types satisfying this concept is:

   * ``bool``,
   * ``char``, ``signed char`` and ``unsigned char``,
   * ``short`` and ``unsigned short``,
   * ``int`` and ``unsigned``,
   * ``long`` and ``unsigned long``,
   * ``long long`` and ``unsigned long long``,
   * ``float`` and ``double``.

   ``long double`` is also supported, but only if the ``MPPP_WITH_LONG_DOUBLE`` definition is activated
   (see the :ref:`configuration instructions <configuration>`).
