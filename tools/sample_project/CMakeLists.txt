# mp++ requires at least CMake 3.8.
cmake_minimum_required(VERSION 3.8.0)

# The name of our project.
project(sample_project)

# Look for an installation of mp++ in the system.
find_package(mp++ REQUIRED)

# Create an executable, and link it to the mp++::mp++ imported target.
# This ensures that, in the compilation of 'main', mp++'s include
# dirs are added to the include path of the compiler and that mp++'s
# dependencies (e.g., GMP) are transitively linked to 'main'.
add_executable(main main.cpp)
target_link_libraries(main mp++::mp++)
