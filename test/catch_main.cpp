// Copyright 2016-2021 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the mp++ library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Minimal main file to reduce catch compile times:
// https://github.com/catchorg/Catch2/blob/master/docs/slow-compiles.md

#define CATCH_CONFIG_MAIN

// NOTE: the unicode MSVC builds lead to catch
// defining a wmain() function rather than the usual
// main, and this leads to undefined references
// in the appveyor builds. Luckily, there is
// this definition one can set to use the good
// ole main() instead.
#define DO_NOT_USE_WMAIN

#include "catch.hpp"
