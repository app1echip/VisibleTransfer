cmake_minimum_required(VERSION 3.0.0)
project(VisibleTransfer VERSION 0.1.0)

#include(CTest)
#enable_testing()

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_library(ImageCode ImageCode.cpp ImageCode.h CRC.h)

add_executable(encode encode.cpp)
add_executable(decode decode.cpp)

target_link_libraries(encode ${OpenCV_LIBS} ImageCode)
target_link_libraries(decode ${OpenCV_LIBS} ImageCode)

#set(CPACK_PROJECT_NAME ${PROJECT_NAME})
#set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
#include(CPack)
