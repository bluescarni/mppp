# mp++ needs at least CMake 3.2.
cmake_minimum_required(VERSION 3.2.0)

# The name of our project.
project(sample_project)

# Look for an installation of mp++ in the system.
find_package(Mp++ REQUIRED)

# Create an executable, and link it to the mp++ headers.
add_executable(main main.cpp)
target_link_libraries(main Mp++::mp++)

# This line is necessary to instruct your compiler
# that C++11 is needed for the compilation.
set_property(TARGET main PROPERTY CXX_STANDARD 11)
