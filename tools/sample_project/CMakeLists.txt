# mp++ needs at least CMake 3.2.
cmake_minimum_required(VERSION 3.2.0)

# The name of our project.
project(sample_project)

# Look for an installation of mp++ in the system.
find_package(Mp++ REQUIRED)

# Create an executable, and link it to the Mp++::mp++ imported target.
# This ensures that, in the compilation of 'main', mp++'s include
# dirs are added to the include path of the compiler and that mp++'s
# dependencies (e.g., GMP) are transitively linked to 'main'.
add_executable(main main.cpp)
target_link_libraries(main Mp++::mp++)

# This line is necessary to instruct your compiler
# that C++11 is needed for the compilation.
# Not strictly necessary with a recent-enough compiler.
set_property(TARGET main PROPERTY CXX_STANDARD 11)
