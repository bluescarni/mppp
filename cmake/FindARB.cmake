# Originally copied from the KDE project repository:
# http://websvn.kde.org/trunk/KDE/kdeutils/cmake/modules/FindGMP.cmake?view=markup&pathrev=675218

# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
# Copyright (c) 2008-2016 Francesco Biscani, <bluescarni@gmail.com>

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

if(ARB_INCLUDE_DIR AND ARB_LIBRARY)
	# Already in cache, be silent
	set(ARB_FIND_QUIETLY TRUE)
endif()

find_path(ARB_INCLUDE_DIR NAMES arb.h)
find_library(ARB_LIBRARY NAMES arb)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(ARB DEFAULT_MSG ARB_INCLUDE_DIR ARB_LIBRARY)

mark_as_advanced(ARB_INCLUDE_DIR ARB_LIBRARY)

# NOTE: this has been adapted from CMake's FindPNG.cmake.
if(ARB_FOUND AND NOT TARGET ARB::ARB)
	add_library(ARB::ARB UNKNOWN IMPORTED)
    set_target_properties(ARB::ARB PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${ARB_INCLUDE_DIR}")
    set_target_properties(ARB::ARB PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        IMPORTED_LOCATION "${ARB_LIBRARY}")
endif()
