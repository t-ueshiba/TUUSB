cmake_minimum_required(VERSION 2.4)
project(TUUSB)

enable_language(CXX)
set(CMAKE_CXX_STANDARD		14)
set(CMAKE_CXX_STANDARD_REQUIRED	ON)
set(CMAKE_CXX_EXTENSIONS	OFF)
set(CMAKE_CXX_FLAGS_RELEASE	"-Wall -O3")
set(CMAKE_CXX_FLAGS_DEBUG	"-g")
set(CMAKE_BUILD_TYPE		Release)
add_definitions(-DNDEBUG)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB libusb)
if(NOT LIBUSB_FOUND)
    return()
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${LIBUSB_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS})

link_directories(${CMAKE_INSTALL_PREFIX}/lib)

file(GLOB sources *.cc)
add_library(${PROJECT_NAME} SHARED ${sources})
target_link_libraries(${PROJECT_NAME} ${LIBUSB_LIBRARIES})

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)

add_subdirectory(TU)
