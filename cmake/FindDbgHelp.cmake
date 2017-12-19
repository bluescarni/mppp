# This is similar to FindQuadmath.cmake, only we don't care about the include dir.

include(FindPackageHandleStandardArgs)

if(DbgHelp_LIBRARY)
	# Already in cache, be silent
	set(DbgHelp_FIND_QUIETLY TRUE)
endif()

find_library(DbgHelp_LIBRARY NAMES "dbghelp")

if(NOT DbgHelp_LIBRARY)
    set(DbgHelp_USE_DIRECTLY TRUE)
    set(DbgHelp_LIBRARY "dbghelp.lib" CACHE FILEPATH "" FORCE)
endif()

if(DbgHelp_USE_DIRECTLY)
    message(STATUS "dbghelp.lib will be included and linked directly.")
endif()

find_package_handle_standard_args(DbgHelp DEFAULT_MSG DbgHelp_LIBRARY)

mark_as_advanced(DbgHelp_LIBRARY)

if(DbgHelp_FOUND AND NOT TARGET DbgHelp::DbgHelp)
    message(STATUS "Creating the 'DbgHelp::DbgHelp' imported target.")
    if(DbgHelp_USE_DIRECTLY)
        # If we are using it directly, we must define an interface library,
        # as we do not have the full path to the shared library.
        add_library(DbgHelp::DbgHelp INTERFACE IMPORTED)
        set_target_properties(DbgHelp::DbgHelp PROPERTIES INTERFACE_LINK_LIBRARIES "${DbgHelp_LIBRARY}")
    else()
        # Otherwise, we proceed as usual.
        add_library(DbgHelp::DbgHelp UNKNOWN IMPORTED)
        set_target_properties(DbgHelp::DbgHelp PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${DbgHelp_INCLUDE_DIR}"
            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
            IMPORTED_LOCATION "${DbgHelp_LIBRARY}")
    endif()
endif()
