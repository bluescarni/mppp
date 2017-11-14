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

Construction, conversion and assignment
---------------------------------------

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

All of mp++'s multiprecision classes support a uniform style of initialisastion and conversion based
on curly brackets. Using the same syntax, it is possible to:

* initialise multiprecision objects from objects of (most of) C++'s numerical types (see the
  :cpp:concept:`~mppp::CppInteroperable` concept for a full list),
* initialise multiprecision objects from multiprecision objects of a different type,
* initialise C++ numerical objects from multiprecision objects.

Let's see a few examples:

.. code-block:: c++

   int_t n{42};
   assert(n == 42);
   int m{n};
   assert(m == 42);

In the code above, we are creating a multiprecision integer ``n`` from the C++ ``int`` literal ``42``. We are then converting
``n`` back to ``int``, and checking that the converted value is the original one. As a general rule, mp++ will strive
to preserve the exact input value during construction and conversion. If this is not possible, what happens next depends
on the types and values involved. For instance, initialising an :cpp:class:`~mppp::integer`
with a floating-point value results in truncation:

.. code-block:: c++

   int_t n{1.23};
   assert(n == 1);

In a similar fashion, initialising an :cpp:class:`~mppp::integer` with a :cpp:class:`~mppp::rational` also
results in truncation:

.. code-block:: c++

   int_t n{rat_t{-7, 3}};
   assert(n == -2);

:cpp:class:`~mppp::integer` and :cpp:class:`~mppp::rational` cannot represent non-finite values, thus construction
from such values will raise an exception:

.. code-block:: c++

   int_t n{std::numeric_limits<double>::infinity()};  // Raises std::domain_error.
   rat_t q{std::numeric_limits<double>::quiet_NaN()}; // Raises std::domain_error.

Construction of C++ integrals from :cpp:class:`~mppp::integer` and :cpp:class:`~mppp::rational` might fail
in case of overflow, and it will produce the truncated value when constructing from :cpp:class:`~mppp::rational`:

.. code-block:: c++

   int n{int_t{1} << 1024};       // int construction from very large value, raises std::overflow_error.
   assert(int{rat_t{4, 3}} == 1); // int construction from rational truncates.

On the other hand, conversion of :cpp:class:`~mppp::integer` objects to C++ floating-point types does not raise any error
even if it does not preserve the exact value:

.. code-block:: c++

   float f{int_t{"32327737199221993919239912"}}; // Constructs a single-precision approximation
                                                 // of the original integer.

The documentation of the multiprecision classes explains in detail the behaviour during construction and conversion.

All of mp++'s multiprecision classes can also be initialised from string-like entities (see the
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
   assert(real{"7B.1", 32, 100} == 235.03125); // Base 32, 100 bits of precision.
