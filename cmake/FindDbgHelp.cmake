# Locate the DbgHelp library for MSVC.

if(DbgHelp_INCLUDE_DIR AND DbgHelp_LIBRARY)
    # Already in cache, be silent
    set(DbgHelp_FIND_QUIETLY TRUE)
endif()

find_path(DbgHelp_INCLUDE_DIR NAMES Dbghelp.h)
find_library(DbgHelp_LIBRARY NAMES dbghelp)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(DbgHelp DEFAULT_MSG DbgHelp_INCLUDE_DIR DbgHelp_LIBRARY)

mark_as_advanced(DbgHelp_INCLUDE_DIR DbgHelp_LIBRARY)

# NOTE: this has been adapted from CMake's FindPNG.cmake.
if(DbgHelp_FOUND AND NOT TARGET DbgHelp::DbgHelp)
    add_library(DbgHelp::DbgHelp UNKNOWN IMPORTED)
    set_target_properties(DbgHelp::DbgHelp PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${DbgHelp_INCLUDE_DIR}"
        IMPORTED_LINK_INTERFACE_LANGUAGES "C" IMPORTED_LOCATION "${DbgHelp_LIBRARY}")
endif()
