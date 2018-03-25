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

The :cpp:class:`~mppp::rational` class adopts the usual canonical representation of rational numbers:

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
  :cpp:func:`mppp::rational::_get_den()` member functions:

  .. code-block:: c++

     rat_t q1{2, 5};
     assert(q1.get_num() == 2); // Const access to numerator and denominator.
     assert(q1.get_den() == 5);

     q1._get_num() = 1;         // Mutable access to numerator and denominator.
     q1._get_den() = 3;
     assert(q1 == rat_t{1, 3});

* the :cpp:func:`mppp::rational::canonicalise()` member function (and its free function counterpart)
  can be used to canonicalise a :cpp:class:`~mppp::rational`. This is normally not necessary,
  unless the numerator/denominator have been changed manually via the mutable getters:

  .. code-block:: c++

     rat_t q1{1, 6};
     q1._get_num() = 9;         // Change the numerator using the mutable getter.
                                // WARNING: q1 is not in canonical form any more!

     q1.canonicalise();         // Canonicalise q1.
     assert(q1 == rat_t{3, 2});

* a few additional :ref:`arithmetic <rational_arithmetic>` and :ref:`comparison <rational_comparison>`
  functions are available;
* a few :ref:`exponentiation <rational_exponentiation>` and :ref:`number-theoretic <rational_ntheory>`
  functions are available;
* like :cpp:class:`~mppp::integer`, :cpp:class:`~mppp::rational` also provides a specialisation
  of ``std::hash`` for use in the standard unordered containers.

Many of these features, which are documented in detail in the :ref:`rational reference <rational_reference>`, are available
in multiple overloads, often both as free and member functions.
