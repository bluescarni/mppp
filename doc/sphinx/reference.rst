Reference
=========

.. note::

   Generic functions and classes in mp++ support a mechanism called `concepts <https://en.wikipedia.org/wiki/Concepts_(C%2B%2B)>`_
   to constrain the types with which they can be used. C++ concepts are not (yet) part of the standard, and they are
   currently available only in GCC 6 and later (with the ``-fconcepts`` compilation flag). When used with compilers which do not
   support concepts natively, mp++ will employ a concept emulation layer in order to provide the same functionality as native
   C++ concepts.

   Since the syntax of true C++ concepts is clearer than that of the concept emulation layer, the mp++ documentation describes
   and refers to concepts in their native C++ form (see, e.g., the :cpp:concept:`mppp::CppInteroperable` concept).

.. toctree::
   :maxdepth: 2

   namespaces.rst
   exceptions.rst
   concepts.rst
   integer.rst
