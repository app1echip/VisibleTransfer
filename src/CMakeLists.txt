cmake_minimum_required(VERSION 3.0.0)
project(VisibleTransfer)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_library(vistran crc.hpp mat.hpp codedef.h)
set_target_properties(vistran PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(vistran ${OpenCV_LIBS})

add_executable(encode encode.cpp)
add_executable(decode decode.cpp)

target_link_libraries(encode vistran)
target_link_libraries(decode vistran)