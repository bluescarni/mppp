.. _tutorial_numtower:

The numerical tower and type coercion
=====================================

Before proceeding to show how mp++'s classes can be used to perform arithmetic computations,
it is useful to introduce first the hierarchy on top of which mp++'s type coercion
is built.

In a broad sense, mp++ aims to extend C++'s type hierarchy with multiprecision
numerical types. In C++, when numerical operands of different types are involved
in an arithmetic operation, all operands are converted (or *coerced*) into a
common type determined from the types of the operands. The converted operands
are then used in place of the original operands to perform the operation,
and the type of the result will be the common type. For instance:

.. code-block:: c++

   auto a = 4 + 5l;   // '4' is int, '5l' is long: a will be long.
   auto b = 4.f + 5l; // '4.f' is float, '5l' is long: b will be float.
   auto c = 4.f + 5.; // '4.f' is float, '5.' is double: c will be double.

In order to determine the common type, C++ assigns a rank to each fundamental type.
In an operation involving operands of different types, the type of the result will be
the type with the highest rank among the types of the operands. Although there are
a few complications and caveats, the general rule in C++ is that integral types have
a lower rank than floating-point types, and that, within the integral and floating-point
types, a higher range or bit width translates to a higher rank. The underlying
idea is that automatic type coercion should not change the value of an operand [#f1]_.

For real-valued types, mp++ extends C++'s type hierarchy in a (hopefully) natural way:

* :cpp:class:`~mppp::integer` has a rank higher than any integral C++ type, but lower
  than any floating-point C++ type;
* :cpp:class:`~mppp::rational` has a rank higher than :cpp:class:`~mppp::integer`, but lower
  than any floating-point C++ type;
* :cpp:class:`~mppp::real128` has a rank higher than any floating-point C++ type;
* :cpp:class:`~mppp::real` has a rank higher than :cpp:class:`~mppp::real128`.

In other words, mp++'s real-valued type hierarchy (or *numerical tower*)
from the lowest rank to the highest is the following:

* integral C++ types,
* :cpp:class:`~mppp::integer`,
* :cpp:class:`~mppp::rational`,
* floating-point C++ types,
* :cpp:class:`~mppp::real128`,
* :cpp:class:`~mppp::real`.

Note that up to and including :cpp:class:`~mppp::rational`, types with higher rank can represent exactly all values
of any type with a lower rank. The floating-point types, however, cannot represent exactly all values representable
by :cpp:class:`~mppp::rational`, :cpp:class:`~mppp::integer` or even the integral C++ types. It should also be noted
that :cpp:class:`~mppp::real`'s precision is set at runtime, and it is thus possible to create :cpp:class:`~mppp::real`
objects with a precision lower than :cpp:class:`~mppp::real128` or any of the floating-point C++ types. Regardless,
when it comes to type coercion, :cpp:class:`~mppp::real` is always assigned a rank higher than any other type.

For complex-valued types, the type hierarchy is easily extended. In a binary operation involving
two different complex-valued types, the real-valued coercion rules can be directly applied:

.. code-block:: c++

   std::complex<double> c1{1, 2}; // The complex counterpart of double.
   complex128 c2{3, 4};           // The complex counterpart of real128.

   auto res = c1 + c2;            // 'res' will be complex128, because
                                  // in the type hierarchy real128 > double.

If one of the operands is complex-valued and the other one is real-valued, then
first the real-valued type is formally promoted to its complex counterpart, and
then the real-valued coercion rules can be applied again:

.. code-block:: c++

   std::complex<double> c{1, 2};
   real128 r{4};

   auto res = c + r; // 'res' will be complex128: r is first
                     // promoted to its complex counterpart,
                     // complex128, and the rules from the
                     // previous example are then applied.

mp++'s type coercion rules extend beyond arithmetic operators (e.g., they also apply to binary
special functions).

.. rubric:: Footnotes

.. [#f1] Strictly speaking, this is of course not true. On modern architectures, a large enough
   64-bit ``long long`` will be subject to a lossy conversion to, e.g., ``float``
   during type coercion.
