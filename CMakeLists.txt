cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

set(name daqana)
set(CMAKE_MODULE_PATH "/opt/geant4/geant4.9.5.p01/cmake/Modules" )

project(${name})
# find ROOT
find_package(ROOT REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${ROOT_INCLUDE_DIR})
file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)

add_executable(${name} EXCLUDE_FROM_ALL ${name}.cc ${sources})
target_link_libraries(${name} ${ROOT_LIBRARIES})
