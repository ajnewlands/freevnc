# CMakeList.txt : CMake project for freevnc, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (freevncsrv "freevncsrv.cpp" "freevncsrv.h")
target_link_libraries (freevncsrv LINK_PUBLIC freevnc)

# TODO: Add tests and install targets if needed.
