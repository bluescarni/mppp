.. _tutorial_boost_s11n:

Boost.serialization support
===========================

Starting from version 0.22, all of mp++'s multiprecision classes support (de)serialisation
via the `Boost.serialization <https://www.boost.org/doc/libs/1_75_0/libs/serialization/doc/index.html>`_
library, provided that mp++ was compiled with the ``MPPP_WITH_BOOST_S11N`` option enabled
(see the :ref:`installation instructions <installation>`). We refer to the documentation
of Boost.serialization (particularly the
`tutorial <https://www.boost.org/doc/libs/1_75_0/libs/serialization/doc/tutorial.html>`_)
for usage examples. Note that, as detailed in the previous sections,
certain classes (such as :cpp:class:`~mppp::integer` and
:cpp:class:`~mppp::real`) also provide a separate, low-level binary serialisation API
which does not depend on Boost.serialization.

There is an important **caveat** to keep in mind when using mp++'s Boost.serialization support.
The serialisation to/from binary archives is optimised for speed, and no checks are performed
on the validity of the data that is loaded from a binary archive. In other words, a
maliciously-crafted binary archive could lead to the creation of an invalid mp++ object
whose use could then lead to undefined and/or erratic runtime behaviour. Users are thus
advised not to load data from untrusted binary archives. Non-binary archives do not suffer from
these issues.
