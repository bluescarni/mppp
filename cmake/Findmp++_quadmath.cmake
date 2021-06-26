# Originally copied from the KDE project repository:
# http://websvn.kde.org/trunk/KDE/kdeutils/cmake/modules/FindGMP.cmake?view=markup&pathrev=675218

# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
# Copyright (c) 2008-2021 Francesco Biscani, <bluescarni@gmail.com>

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# ------------------------------------------------------------------------------------------

include(FindPackageHandleStandardArgs)
include(CMakePushCheckState)
include(CheckCXXSourceCompiles)

if(MPPP_QUADMATH_INCLUDE_DIR AND MPPP_QUADMATH_LIBRARY)
	# Already in cache, be silent
	set(mp++_quadmath_FIND_QUIETLY TRUE)
endif()

find_path(MPPP_QUADMATH_INCLUDE_DIR NAMES quadmath.h)
find_library(MPPP_QUADMATH_LIBRARY NAMES quadmath)

if(NOT MPPP_QUADMATH_INCLUDE_DIR OR NOT MPPP_QUADMATH_LIBRARY)
    cmake_push_check_state(RESET)
    list(APPEND CMAKE_REQUIRED_LIBRARIES "quadmath")
    CHECK_CXX_SOURCE_COMPILES("
        #include <quadmath.h>
        int main(void){
            __float128 foo = ::sqrtq(123.456);
        }"
        MPPP_QUADMATH_USE_DIRECTLY
    )
    cmake_pop_check_state()
    if (MPPP_QUADMATH_USE_DIRECTLY)
        set(MPPP_QUADMATH_INCLUDE_DIR "unused" CACHE PATH "" FORCE)
        set(MPPP_QUADMATH_LIBRARY "quadmath" CACHE FILEPATH "" FORCE)
    endif()
endif()

find_package_handle_standard_args(mp++_quadmath DEFAULT_MSG MPPP_QUADMATH_LIBRARY MPPP_QUADMATH_INCLUDE_DIR)

mark_as_advanced(MPPP_QUADMATH_INCLUDE_DIR MPPP_QUADMATH_LIBRARY)

# NOTE: this has been adapted from CMake's FindPNG.cmake.
if(mp++_quadmath_FOUND AND NOT TARGET mp++::quadmath)
    message(STATUS "Creating the 'mp++::quadmath' imported target.")
    if(MPPP_QUADMATH_USE_DIRECTLY)
        message(STATUS "libquadmath will be included and linked directly.")
        # If we are using it directly, we must define an interface library,
        # as we do not have the full path to the shared library.
        add_library(mp++::quadmath INTERFACE IMPORTED)
        set_target_properties(mp++::quadmath PROPERTIES INTERFACE_LINK_LIBRARIES "${MPPP_QUADMATH_LIBRARY}")
    else()
        # Otherwise, we proceed as usual.
        add_library(mp++::quadmath UNKNOWN IMPORTED)
        set_target_properties(mp++::quadmath PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${MPPP_QUADMATH_INCLUDE_DIR}"
            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
            IMPORTED_LOCATION "${MPPP_QUADMATH_LIBRARY}")
    endif()
endif()
