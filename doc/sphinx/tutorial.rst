Tutorial
========

Preliminaries
-------------

In this tutorial, we will assume that the global mp++ header has been included:

.. code-block:: c++

   #include <mp++/mp++.hpp>

The ``mp++.hpp`` header will pull in the public mp++ API in its entirety. It is also possible
to include individual mp++ headers instead of ``mp++.hpp``, e.g.:

.. code-block:: c++

   #include <mp++/integer.hpp>  // For the integer multiprecision class
                                // and associated functions.
   #include <mp++/rational.hpp> // For the rational multiprecision class
                                // and associated functions.

Including individual headers rather than the global ``mp++.hpp`` header is in general a good idea
if you don't need all the features provided by mp++, and it may improve compilation times.

.. note::

   The headers within the ``mp++/detail`` subdirectory are implementation details, and they should not be relied
   upon as they change in breaking ways from version to version.

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
which essentially means that you can call mp++'s functions without prepending ``mppp::`` or employing ``using``
directives.

Constructing values
-------------------

All of mp++'s multiprecision classes default-construct to zero:

.. code-block:: c++

   int_t n;
   assert(n == 0);

   rat_t q;
   assert(q == 0);

   real128 r128;
   assert(r128 == 0);

   real r;
   assert(r == 0);

All of mp++'s multiprecision classes can be initialised from (most of) C++'s numerical types (see the
:cpp:concept:`~mppp::CppInteroperable` concept for a full list):

.. code-block:: c++

   assert(int_t{-42} == -42)
   assert(rat_t{33u} == 33)
   assert(real128{1.5f} == 1.5f);
   assert(real{3.5} == 3.5);

An important feature of mp++'s multiprecision classes is that all constructors (apart from the default, copy
and move constructors) are ``explicit``. As a design choice, mp++'s API purposely eschews implicit conversions,
and, consequently, code like this will not work:

.. code-block:: c++

   int_t n = 42;

Direct initialisastion must be used instead:

.. code-block:: c++

   int_t n{42};

A typical pitfall when using multiprecision classes in C++ is the interaction with floating-point types.
The following code, for instance,

.. code-block:: c++

   rat_t q{1.1};

could be naively expected to initialise ``q`` to the rational value :math:`\frac{11}{10}`. In reality, on
modern architectures, ``q`` will be initialised to :math:`\frac{2476979795053773}{2251799813685248}`. This happens
because the literal ``1.1`` is first converted to a double-precision value by the compiler before being used
to construct ``q``. Since :math:`1.1` cannot be represented in finite terms in binary, the double-precision value
that will be passed to construct ``q`` will be the closest double-precision approximation to :math:`\frac{11}{10}` representable
in binary. In order to initialise ``q`` exactly to :math:`\frac{11}{10}`, :cpp:class:`~mppp::rational`'s constructor
from numerator and denominator can be used:

.. code-block:: c++

   rat_t q{11, 10};

A similar problem arises when using multiprecision floating-point classes. The following code, for instance,

.. code-block:: c++

   real128 r{1.1};

initialises ``r`` to circa :math:`1.10000000000000008881784197001252323`, which is *not* the closest quadruple-precision approximation
of :math:`1.1`. Again, this happens because :math:`1.1` is first converted to its double-precision approximation by the compiler,
and only afterwards it is passed to the :cpp:class:`~mppp::real128` constructor. In this case, there are two ways to rectify
the situation and initialise ``r`` to the quadruple-precision approximation of :math:`1.1`. The first approach is to use
a quadruple-precision literal to initialise ``r``:

.. code-block:: c++

   real128 r{1.1q};

This approach might require specific flags to be passed to the compiler. The second approach is to use the :cpp:class:`~mppp::real128`
constructor from string:

.. code-block:: c++

   real128 r{"1.1"};

This second option is slower than the quadruple-precision literal, but it does not require special flags to be passed to the compiler.

All of mp++'s multiprecision classes can be initialised from string-like entities (see the
:cpp:concept:`~mppp::StringType` concept for a full list). By default, string input is interpreted as the base-10 representation
of the desired value, and parsing follows (hopefully) intuitive rules:

.. code-block:: c++

   assert(int_t{"-42"} == -42)
   assert(rat_t{"3/2"} == 1.5)
   assert(real128{"2.5"} == 2.5);
   assert(real{"-3.125E-2", 100} == -0.03125);

Note that for :cpp:class:`~mppp::real` we need to provide the precision as an additional parameter when constructing from string (in
this specific example, 100 bits of precision are used). Depending on the multiprecision class, additional string constructors are available
which allow to specify a different base for the representation of the value:

.. code-block:: c++

   assert(int_t{"-101010", 2} == -42)          // Base 2.
   assert(rat_t{"2a/1c", 16} == 1.5)           // Base 16.
   assert(real{"7B.1", 32, 100} == 235.03125); // Base 32, 100 digits of precision.
