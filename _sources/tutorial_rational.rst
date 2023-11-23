.. _tutorial_rational:

Rational tutorial
=================

:cpp:class:`~mppp::rational` is a class that builds upon :cpp:class:`~mppp::integer` to provide
an implementation of rational numbers represented as a numerator-denominator
:cpp:class:`~mppp::integer` pairs.

Like :cpp:class:`~mppp::integer`, :cpp:class:`~mppp::rational` is parametrised over
an integral value ``SSize`` which represents the number of limbs that will be
stored directly within both the numerator and the denominator before resorting to dynamic memory allocation
(see the :ref:`integer tutorial <tutorial_integer>` for a detailed explanation of the meaning
of the ``SSize`` parameter).

The :cpp:class:`~mppp::rational` class adopts the usual *canonical* representation of rational numbers:

* numerator and denominator are coprime,
* the denominator is always strictly positive.

This representation is usually enforced automatically by the :cpp:class:`~mppp::rational` API (however,
a couple of low-level functions are available to access and mutate directly the numerator and the denominator
in performance-critical scenarios).

In addition to the features common to all mp++ classes, the :cpp:class:`~mppp::rational` API provides
a few additional capabilities:

* it is possible to construct a :cpp:class:`~mppp::rational` from an integral pair
  of arguments representing the numerator and the denominator:

  .. code-block:: c++

     rat_t q1{3, 4};                // Construction from int numerator and denominator.

     rat_t q2{long(-6), short(-8)}; // Construction from mixed numerator and denominator types.

     assert(q1 == q2);              // Construction canonicalises the rational.

* it is possible to get const and mutable references to the :cpp:class:`~mppp::integer` objects
  representing the numerator and the denominator via the :cpp:func:`mppp::rational::get_num()`,
  :cpp:func:`mppp::rational::get_den()`, :cpp:func:`mppp::rational::_get_num()` and
  :cpp:func:`mppp::rational::_get_den()` member functions. These accessors allow to use
  any function from the :cpp:class:`~mppp::integer` API on the numerator and on the denominator:

  .. code-block:: c++

     rat_t q1{2, 5};
     assert(q1.get_num() == 2); // Const access to numerator and denominator.
     assert(q1.get_den() == 5);

     q1._get_num() = 1;         // Mutable access to numerator and denominator.
     q1._get_den() = 3;
     assert(q1 == rat_t{1, 3});

* the :cpp:func:`mppp::rational::canonicalise()` member function (and its free function counterpart)
  can be used to canonicalise a :cpp:class:`~mppp::rational`. This is normally not necessary,
  unless the numerator or the denominator have been changed manually via the mutable getters:

  .. code-block:: c++

     rat_t q1{1, 6};
     q1._get_num() = 9;         // Change the numerator using the mutable getter.
                                // WARNING: q1 = 9/6, not in canonical form any more!

     q1.canonicalise();         // Canonicalise q1.
     assert(q1 == rat_t{3, 2});

* a few additional :ref:`arithmetic <rational_arithmetic>` and :ref:`comparison <rational_comparison>`
  functions are available;
* a few :ref:`exponentiation <rational_exponentiation>` and :ref:`number-theoretic <rational_ntheory>`
  functions are available;
* like :cpp:class:`~mppp::integer`, :cpp:class:`~mppp::rational` also provides a :ref:`specialisation <rational_std_specialisations>`
  of ``std::hash`` for use in the standard unordered containers.

Many of these features, which are documented in detail in the :ref:`rational reference <rational_reference>`, are available
in multiple overloads, often both as free and member functions.

Interacting with the GMP API
----------------------------

Like :cpp:class:`~mppp::integer`, :cpp:class:`~mppp::rational` provides facilities for interfacing
with the `GMP <https://gmplib.org/>`__ library. Currently, the interoperability is limited to
:cpp:class:`~mppp::rational` objects being able to be constructed and assigned from both :cpp:type:`mpz_t`
and ``mpq_t`` objects:

.. code-block:: c++

   mpz_t m;
   mpz_init_set_si(m, -4);      // Init an mpz_t with the value -4.

   rat_t q1{m};                 // Init a rat_t from the mpz_t.
   assert(q1 == -4);            // Verify that the value is correct.

   mpq_t q;
   mpq_init(q);
   mpq_set_si(q, 3, 4);         // Init an mpq_t with the value 3/4.

   rat_t q2{q};                 // Init a rat_t from the mpq_t.
   assert((q2 == rat_t{3, 4})); // Verify that the value is correct.

   rat_t q3;
   q3 = m;                      // Assign the mpz_t to another rat_t.
   assert(q3 == -4);            // Verify that the value is correct.

   rat_t q4;
   q4 = q;                      // Assign the mpq_t to another rat_t.
   assert((q4 == rat_t{3, 4})); // Verify that the value is correct.

   mpz_clear(m);                // Clear the mpz_t.
   mpq_clear(q);                // Clear the mpq_t.

User-defined literals
---------------------

.. versionadded:: 0.19

User-defined literals are available to construct
:cpp:class:`mppp::rational` instances with 1, 2 and 3
limbs of static storage. The :ref:`literals <rational_literals>`
are defined within
the inline namespace ``mppp::literals``, and they support
binary, octal, decimal and hexadecimal representations:

.. code-block:: c++

   using namespace mppp::literals;

   auto q1 = 123_q1;      // q1 has 1 limb of static storage,
                          // and it contains the value 123.

   auto q2 = -0b10011_q2; // q2 has 2 limbs of static storage,
                          // and it contains the value -19
                          // (-10011 in base 2).

   auto q3 = 0146_q1;     // q3 has 1 limb of static storage,
                          // and it contains the value 102
                          // (146 in base 8).

   auto q4 = 0xfe45_q3;   // q4 has 3 limbs of static storage,
                          // and it contains the value 65093
                          // (fe45 in base 16).

Note that, due to language limitations, only values with unitary denominator
can be constructed via these literals.

.. seealso::

   https://en.cppreference.com/w/cpp/language/integer_literal
