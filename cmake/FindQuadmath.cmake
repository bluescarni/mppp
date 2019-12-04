# Originally copied from the KDE project repository:
# http://websvn.kde.org/trunk/KDE/kdeutils/cmake/modules/FindGMP.cmake?view=markup&pathrev=675218

# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
# Copyright (c) 2008-2019 Francesco Biscani, <bluescarni@gmail.com>

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

message(STATUS "Requested Quadmath components: ${Quadmath_FIND_COMPONENTS}")

# Check the components that were passed to find_package().
set(_Quadmath_ALLOWED_COMPONENTS header libquadmath)
foreach(_Quadmath_CUR_COMPONENT ${Quadmath_FIND_COMPONENTS})
    if(NOT ${_Quadmath_CUR_COMPONENT} IN_LIST _Quadmath_ALLOWED_COMPONENTS)
        message(FATAL_ERROR "'${_Quadmath_CUR_COMPONENT}' is not a valid component for Quadmath.")
    endif()
endforeach()
unset(_Quadmath_ALLOWED_COMPONENTS)

# Setup the list of arguments to be passed to
# find_package_handle_standard_args().
set(_Quadmath_FPHSA_ARGS)

if ("header" IN_LIST Quadmath_FIND_COMPONENTS)
    # The header component was requested.
    # The associated variable is Quadmath_INCLUDE_DIR.
    list(APPEND _Quadmath_FPHSA_ARGS Quadmath_INCLUDE_DIR)

    find_path(Quadmath_INCLUDE_DIR NAMES quadmath.h)

    if(NOT Quadmath_INCLUDE_DIR)
        # quadmath.h was not found in standard
        # paths, let's see if it "just works".
        cmake_push_check_state(RESET)
        CHECK_CXX_SOURCE_COMPILES("
            #include <quadmath.h>
            int main(void){
                return 0;
            }"
            Quadmath_INCLUDE_DIRECTLY)
        cmake_pop_check_state()

        if(Quadmath_INCLUDE_DIRECTLY)
            # We can include quadmath.h directly,
            # set Quadmath_INCLUDE_DIR to a value
            # signalling that it has been detected
            # (even if this value will never be used).
            set(Quadmath_INCLUDE_DIR "unused" CACHE PATH "" FORCE)
        endif()
    endif()
endif()

if ("libquadmath" IN_LIST Quadmath_FIND_COMPONENTS)
    # The libquadmath component was requested.
    # The associated variable is Quadmath_LIBRARY.
    list(APPEND _Quadmath_FPHSA_ARGS Quadmath_LIBRARY)

    find_library(Quadmath_LIBRARY NAMES quadmath)

    if(NOT Quadmath_LIBRARY)
        # libquadmath was not found in standard
        # paths, let's see if it "just works".
        cmake_push_check_state(RESET)
        list(APPEND CMAKE_REQUIRED_LIBRARIES "quadmath")
        CHECK_CXX_SOURCE_COMPILES("
            int main(void){
                return 0;
            }"
            Quadmath_LINK_DIRECTLY)
        cmake_pop_check_state()

        if (Quadmath_LINK_DIRECTLY)
            # We can link libquadmath directly,
            # set Quadmath_LIBRARY to a value
            # signalling that it has been detected
            # (even if this value will never be used).
            set(Quadmath_LIBRARY "quadmath" CACHE FILEPATH "" FORCE)
        endif()
    endif()
endif()

# Run the standard find_package() machinery.
find_package_handle_standard_args(Quadmath DEFAULT_MSG ${_Quadmath_FPHSA_ARGS})
unset(_Quadmath_FPHSA_ARGS)

if("header" IN_LIST Quadmath_FIND_COMPONENTS)
    mark_as_advanced(Quadmath_INCLUDE_DIR)

    if(Quadmath_FOUND AND NOT TARGET Quadmath::header)
        message(STATUS "Creating the 'Quadmath::header' imported target.")
        add_library(Quadmath::header INTERFACE IMPORTED)
        if(Quadmath_INCLUDE_DIRECTLY)
            message(STATUS "quadmath.h will be included directly.")
        else()
            message(STATUS "Path to the quadmath.h header: ${Quadmath_INCLUDE_DIR}")
            set_target_properties(Quadmath::header PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${Quadmath_INCLUDE_DIR}")
        endif()
    endif()
endif()

if ("libquadmath" IN_LIST Quadmath_FIND_COMPONENTS)
    mark_as_advanced(Quadmath_LIBRARY)

    if(Quadmath_FOUND AND NOT TARGET Quadmath::libquadmath)
        message(STATUS "Creating the 'Quadmath::libquadmath' imported target.")
        if(Quadmath_LINK_DIRECTLY)
            message(STATUS "libquadmath will be linked directly.")
            # If we are using it directly, we must define an interface library,
            # as we do not have the full path to the shared library.
            add_library(Quadmath::libquadmath INTERFACE IMPORTED)
            set_target_properties(Quadmath::libquadmath PROPERTIES INTERFACE_LINK_LIBRARIES "${Quadmath_LIBRARY}")
        else()
            # Otherwise, we proceed as usual.
            message(STATUS "Path to libquadmath: ${Quadmath_LIBRARY}")
            add_library(Quadmath::libquadmath UNKNOWN IMPORTED)
            set_target_properties(Quadmath::libquadmath PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${Quadmath_LIBRARY}")
        endif()
    endif()
endif()
