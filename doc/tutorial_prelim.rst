Preliminaries
=============

In this tutorial, we will assume that the global mp++ header has been included:

.. code-block:: c++

   #include <mp++/mp++.hpp>

The ``mp++.hpp`` header will pull in the public mp++ API in its entirety.
It is also possible to include individual mp++ headers instead of ``mp++.hpp``, e.g.:

.. code-block:: c++

   #include <mp++/integer.hpp>  // For the integer multiprecision class
                                // and associated functions.
   #include <mp++/rational.hpp> // For the rational multiprecision class
                                // and associated functions.

Including individual headers rather than the global ``mp++.hpp`` header is in general a good idea
if you don't need all the features provided by mp++, and it may improve compilation times.

.. warning::

   Do **not** include headers from the ``mp++/detail`` subdirectory! They contain
   implementation details which may change from version to version in incompatible ways.

We will also assume, for convenience, that mp++'s multiprecision classes have been lifted into the root namespace,
and we will introduce a couple of handy aliases for the :cpp:class:`~mppp::integer` and :cpp:class:`~mppp::rational`
classes:

.. code-block:: c++

   using int_t = mppp::integer<1>;
   using rat_t = mppp::rational<1>;

:cpp:class:`~mppp::integer` and :cpp:class:`~mppp::rational` are class templates parametrised over a positive
integral value (referred to as *static size*) which indicates how many limbs will be stored in static storage
before resorting to dynamic memory allocation. In the definitions above, ``int_t`` and ``rat_t`` will store
values of size up to 1 limb without resorting to heap memory allocation. On modern 64-bit architectures,
this means in practice that ``int_t`` will be able to represent in static storage integers whose absolute
value is less than :math:`2^{64}`. A static size of 1 is a good default choice.

In this tutorial, we will **not** assume that mp++'s functions have been pulled into the global namespace: the mp++
API is designed around `argument-dependent lookup <https://en.wikipedia.org/wiki/Argument-dependent_name_lookup>`__,
which means it is usually possible to call mp++'s functions without prepending ``mppp::`` or employing ``using``
directives.
