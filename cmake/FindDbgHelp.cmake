# Create an imported target for the DbgHelp library in MSVC.
# NOTE: the library is supposed to be always available in Windows:
# https://stackoverflow.com/questions/1619754/is-dbghelp-dll-built-in-to-windows-can-i-rely-on-it-being-there
# So we don't bother about locating the include path or the library, just link
# it directly.

add_library(DbgHelp::DbgHelp UNKNOWN IMPORTED)
set_target_properties(DbgHelp::DbgHelp PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES "C" IMPORTED_LOCATION "dbghelp.lib")
