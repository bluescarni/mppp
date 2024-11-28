if(MPPP_FLINT_INCLUDE_DIR AND MPPP_FLINT_LIBRARY)
    # Already in cache, be silent
    set(mp++_FLINT_FIND_QUIETLY TRUE)
endif()

find_path(MPPP_FLINT_INCLUDE_DIR NAMES flint/flint.h)
find_library(MPPP_FLINT_LIBRARY NAMES flint)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(mp++_FLINT DEFAULT_MSG MPPP_FLINT_INCLUDE_DIR MPPP_FLINT_LIBRARY)

mark_as_advanced(MPPP_FLINT_INCLUDE_DIR MPPP_FLINT_LIBRARY)

# NOTE: this has been adapted from CMake's FindPNG.cmake.
if(mp++_FLINT_FOUND AND NOT TARGET mp++::FLINT)
    add_library(mp++::FLINT UNKNOWN IMPORTED)
    set_target_properties(mp++::FLINT PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${MPPP_FLINT_INCLUDE_DIR}"
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        IMPORTED_LOCATION "${MPPP_FLINT_LIBRARY}")
endif()
