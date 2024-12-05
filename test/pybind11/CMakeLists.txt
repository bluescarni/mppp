# Setup the list of Python components we need to locate.
set(_REQUIRED_PYTHON_COMPONENTS Interpreter Development.Module)

# NOTE: strictly speaking, we should need only the Interpreter and Development.Module
# components. However, in certain setups (e.g., skbuild), it looks
# like if we specify only Development.Module CMake is unable to correctly locate
# the Python installation. Hence, as a workaround, if the following line fails
# and Python3 is *not* found, we try again, this time with the full Development
# component (rather than only Development.Module). This seems to work around the
# issue, for now at least.
find_package(Python3 QUIET COMPONENTS ${_REQUIRED_PYTHON_COMPONENTS})

if(NOT Python3_FOUND)
    list(POP_BACK _REQUIRED_PYTHON_COMPONENTS)
    list(APPEND _REQUIRED_PYTHON_COMPONENTS Development)
endif()

find_package(Python3 QUIET REQUIRED COMPONENTS ${_REQUIRED_PYTHON_COMPONENTS})

message(STATUS "Python3 interpreter: ${Python3_EXECUTABLE}")
message(STATUS "Python3 installation directory: ${Python3_SITEARCH}")
unset(_REQUIRED_PYTHON_COMPONENTS)

# Find pybind11.
find_package(pybind11 REQUIRED CONFIG)

# NOTE: this is very similar to the function for creating C++ tests.
function(ADD_MPPP_PYBIND11_TESTCASE arg1)
    if(MPPP_TEST_NSPLIT)
        math(EXPR __MPPP_TEST_NUM "(${_MPPP_TEST_NUM} + 1) % ${MPPP_TEST_NSPLIT}")
        set(_MPPP_TEST_NUM ${__MPPP_TEST_NUM} PARENT_SCOPE)
    endif()
    if(MPPP_TEST_NSPLIT AND "${MPPP_TEST_SPLIT_NUM}" STREQUAL "${_MPPP_TEST_NUM}")
        return()
    endif()
    Python3_add_library(${arg1} MODULE WITH_SOABI ${arg1}.cpp)
    target_link_libraries(${arg1} PRIVATE mp++ "${pybind11_LIBRARIES}")
    target_include_directories(${arg1} SYSTEM PRIVATE "${pybind11_INCLUDE_DIR}" "${Python3_INCLUDE_DIRS}")
    target_compile_definitions(${arg1} PRIVATE "${pybind11_DEFINITIONS}")
    target_compile_options(${arg1} PRIVATE
        "$<$<CONFIG:Debug>:${MPPP_CXX_FLAGS_DEBUG}>"
        "$<$<CONFIG:Release>:${MPPP_CXX_FLAGS_RELEASE}>"
        "$<$<CONFIG:RelWithDebInfo>:${MPPP_CXX_FLAGS_RELEASE}>"
        "$<$<CONFIG:MinSizeRel>:${MPPP_CXX_FLAGS_RELEASE}>"
    )
    set_target_properties(${arg1} PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(${arg1} PROPERTIES VISIBILITY_INLINES_HIDDEN TRUE)
    target_compile_features(${arg1} PRIVATE cxx_std_11)
    set_property(TARGET ${arg1} PROPERTY CXX_EXTENSIONS NO)
    # Copy over the Python test runner.
    # NOTE: need to use file(GENERATE) because in multi-config build systems (e.g., MSVC) we need to copy
    # the runner in a directory depending on the config type, and we need to do it at generation time.
    # We can fetch the correct directory by reading the TARGET_FILE_DIR property of the python module.
    file(GENERATE OUTPUT "$<TARGET_FILE_DIR:${arg1}>/run_${arg1}.py" INPUT "${CMAKE_CURRENT_SOURCE_DIR}/run_${arg1}.py")
    # Add the actual test.
    add_test(NAME ${arg1} COMMAND "${Python3_EXECUTABLE}" run_${arg1}.py WORKING_DIRECTORY "$<TARGET_FILE_DIR:${arg1}>")
endfunction()

ADD_MPPP_PYBIND11_TESTCASE(pybind11_test_01)

# Change the split test number in the parent scope.
if(MPPP_TEST_NSPLIT)
    set(_MPPP_TEST_NUM ${_MPPP_TEST_NUM} PARENT_SCOPE)
endif()
