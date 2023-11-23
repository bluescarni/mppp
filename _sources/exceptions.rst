.. _exceptions:

Exceptions
==========

*#include <mp++/exceptions.hpp>*

.. cpp:class:: mppp::zero_division_error final : public std::domain_error

   Exception to signal division by zero.

   This exception inherits all members (including constructors) from
   `std::domain_error <https://en.cppreference.com/w/cpp/error/domain_error>`_. It will be thrown
   when a division by zero involving a multiprecision class is attempted, and the type of the result cannot
   represent infinities.
