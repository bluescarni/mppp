# Disable selection of the standard by pybind11.
set(PYBIND11_CPP_STANDARD "")

# Find pybind11.
find_package(pybind11 REQUIRED)

# NOTE: this is very similar to the function for creating C++ tests.
function(ADD_MPPP_PYBIND11_TESTCASE arg1)
    if(MPPP_TEST_NSPLIT)
        math(EXPR __MPPP_TEST_NUM "(${_MPPP_TEST_NUM} + 1) % ${MPPP_TEST_NSPLIT}")
        set(_MPPP_TEST_NUM ${__MPPP_TEST_NUM} PARENT_SCOPE)
    endif()
    if(MPPP_TEST_NSPLIT AND "${MPPP_TEST_SPLIT_NUM}" STREQUAL "${_MPPP_TEST_NUM}")
        return()
    endif()
    add_library(${arg1} MODULE ${arg1}.cpp)
    target_link_libraries(${arg1} PRIVATE mp++ pybind11::module)
    set_target_properties(${arg1} PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}"
        SUFFIX "${PYTHON_MODULE_EXTENSION}")
    target_compile_options(${arg1} PRIVATE
        "$<$<CONFIG:Debug>:${MPPP_CXX_FLAGS_DEBUG}>"
        "$<$<CONFIG:Release>:${MPPP_CXX_FLAGS_RELEASE}>"
        "$<$<CONFIG:RelWithDebInfo>:${MPPP_CXX_FLAGS_RELEASE}>"
        "$<$<CONFIG:MinSizeRel>:${MPPP_CXX_FLAGS_RELEASE}>"
    )
    target_compile_features(${arg1} PRIVATE cxx_std_11)
    set_property(TARGET ${arg1} PROPERTY CXX_EXTENSIONS NO)
    # Copy over the Python test runner.
    # NOTE: need to use file(GENERATE) because in multi-config build systems (e.g., MSVC) we need to copy
    # the runner in a directory depending on the config type, and we need to do it at generation time.
    # We can fetch the correct directory by reading the TARGET_FILE_DIR property of the python module.
    file(GENERATE OUTPUT "$<TARGET_FILE_DIR:${arg1}>/run_${arg1}.py" INPUT "${CMAKE_CURRENT_SOURCE_DIR}/run_${arg1}.py")
    # Add the actual test.
    add_test(NAME ${arg1} COMMAND "${PYTHON_EXECUTABLE}" run_${arg1}.py WORKING_DIRECTORY "$<TARGET_FILE_DIR:${arg1}>")
endfunction()

ADD_MPPP_PYBIND11_TESTCASE(pybind11_test_01)

# Change the split test number in the parent scope.
if(MPPP_TEST_NSPLIT)
    set(_MPPP_TEST_NUM ${_MPPP_TEST_NUM} PARENT_SCOPE)
endif()
