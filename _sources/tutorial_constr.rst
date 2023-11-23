.. _tutorial_constr:

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

* initialise multiprecision objects from objects of C++'s numerical types,
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

   int n{int_t{1} << 1024};         // int construction from very large value,
                                    // raises std::overflow_error.
   assert((int{rat_t{4, 3}} == 1)); // int construction from rational truncates.

On the other hand, conversion of :cpp:class:`~mppp::integer` objects to floating-point C++ types does not raise any error
even if it does not preserve the exact value:

.. code-block:: c++

   float f{int_t{"32327737199221993919239912"}}; // Constructs a single-precision approximation
                                                 // of the original integer.

The documentation of the multiprecision classes explains in detail the behaviour during construction and conversion.

Note that, as a general rule, the constructors of mp++'s multiprecision classes are ``implicit``
when constructing from numerical types lower in the :ref:`numerical hierarchy <tutorial_numtower>`, ``explicit`` otherwise.
For instance, implicit construction of an :cpp:class:`~mppp::integer` from a C++ integral value is allowed, but implicit construction
from a C++ floating-point value is not:

.. code-block:: c++

   int_t n1 = 5;   // Valid.
   int_t n2 = 1.12 // Will NOT compile.

On the other hand, all the conversion operators in mp++'s multiprecision classes are currently ``explicit``. In particular,
converting an mp++ multiprecision object to a fundamental C++ type always requires an explicit cast. This behaviour may be
changed in the future so that conversions to fundamental C++ types higher in the hierarchy are ``implicit`` (e.g.,
:cpp:class:`~mppp::integer` to ``double`` conversions).

All of mp++'s multiprecision classes can also be initialised from string-like entities (see the
:cpp:concept:`~mppp::string_type` concept for a full list). By default, string input is interpreted as the base-10 representation
of the desired value, and parsing follows (hopefully) intuitive rules:

.. code-block:: c++

   assert(int_t{"-42"} == -42)
   assert(rat_t{"3/2"} == 1.5)
   assert(real128{"2.5"} == 2.5);
   assert((real{"-3.125E-2", 100} == -0.03125));

Note that for :cpp:class:`~mppp::real` we need to provide the precision as an additional parameter when constructing from string (in
this specific example, 100 bits of precision are used). Depending on the multiprecision class, additional string constructors are available
which allow to specify a different base for the representation of the value:

.. code-block:: c++

   assert((int_t{"-101010", 2} == -42))          // Base 2.
   assert((rat_t{"2a/1c", 16} == 1.5))           // Base 16.
   assert((real{"7B.1", 32, 100} == 235.03125)); // Base 32, 100 bits of precision.

Starting from mp++ 0.19, all multiprecision
classes support also initialisastion
via user-defined literals, implemented in the
``mppp::literals`` inline namespace:

.. code-block:: c++

   using namespace mppp::literals;

   auto n = 123_z1;   // n is an integer with 1 limb of static
                      // storage and initialised with the value 123.
   auto q = 456_q2;   // q is a rational with 2 limbs of static
                      // storage and initialised with the value 456.
   auto x = 0.1_rq;   // x is a real128 initialised with the
                      // quadruple-precision approximation of
                      // the value 0.1.
   auto y = 1.3_r256; // y is a real initialised with the 256-bit
                      // approximation of the value 1.3.

It is of course also possible to assign values to already-constructed multiprecision objects. In general, the behaviour
of the assignment operators mirrors the behaviour of the corresponding constructors. For instance:

.. code-block:: c++

   int_t n{1};
   n = 42;
   assert(n == 42);
   n = -3.7;
   assert(n == -3);
   n = "-128";
   assert(n == -128);
   n = std::numeric_limits<double>::quiet_NaN(); // Raises std::domain_error.

   rat_t q{3, 4};
   q = 1.5;
   assert((q == rat_t{3, 2}));
   q = int_t{10};
   assert(q == 10);
   q = "-5/6";
   assert((q == rat_t{-5, 6}));
   q = std::numeric_limits<double>::infinity();  // Raises std::domain_error.
