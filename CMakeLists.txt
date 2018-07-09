cmake_minimum_required(VERSION 2.4)

project(TUUSB++)

enable_language(CXX)
set(CMAKE_CXX_STANDARD		14)
set(CMAKE_CXX_STANDARD_REQUIRED	ON)
set(CMAKE_CXX_EXTENSIONS	OFF)
set(CMAKE_CXX_FLAGS_RELEASE	"-Wall -O3")
set(CMAKE_CXX_FLAGS_DEBUG	"-g")
set(CMAKE_BUILD_TYPE		Release)
add_definitions(-DNDEBUG)

include_directories(. ${Boost_INCLUDE_DIRS})

file(GLOB sources *.cc)
file(GLOB headers TU/*.h)

add_library(TUUSB++ SHARED ${sources} ${headers})
target_link_libraries(TUUSB++ usb)

install(TARGETS TUUSB++ LIBRARY DESTINATION lib)

add_subdirectory(TU)

