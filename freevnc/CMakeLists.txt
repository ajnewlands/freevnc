﻿# CMakeList.txt : CMake project for freevnc, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

#build the underlying library
add_library( freevnc SHARED freevnc.cpp vncscreen.cpp server.cpp freevnc.h)
set_property(TARGET freevnc PROPERTY CXX_STANDARD 17)
set_property(TARGET freevnc PROPERTY CXX_STANDARD_REQUIRED ON)

if(WIN32)
  target_link_libraries(freevnc wsock32 ws2_32)
  target_link_libraries(freevnc bcrypt)

endif()

# make library headers available to other targets
target_include_directories (freevnc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# TODO: Add tests and install targets if needed.
