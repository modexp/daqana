# $Id: CMakeLists.txt 68058 2013-03-13 14:47:43Z gcosmo $
# xx

#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(daqana)

#----------------------------------------------------------------------------
find_package(ROOT REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ROOT_DIR}/Modules)
message(STATUS " module path " ${CMAKE_MODULE_PATH})
message(STATUS " ROOT include dirs " ${ROOT_INCLUDE_DIR})
message(STATUS " ROOT libraries " ${ROOT_LIBRARIES})

# Johns addition to make it work for ROOT6
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

#----------------------------------------------------------------------------
# Setup ROOT include directories and compile definitions
# Setup include directory for this project
#
include_directories(${PROJECT_SOURCE_DIR}/include ${ROOT_INCLUDE_DIRS}) 

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
message(STATUS "source files " ${PROJECT_SOURCE_DIR})
# daqana
file(GLOB daqana_sources ${PROJECT_SOURCE_DIR}/src/daq.cc ${PROJECT_SOURCE_DIR}/src/driver.cc ${PROJECT_SOURCE_DIR}/src/event.cc ${PROJECT_SOURCE_DIR}/src/rootdriver.cc)
file(GLOB daqana_headers ${PROJECT_SOURCE_DIR}/include/daq.hh ${PROJECT_SOURCE_DIR}/include/driver.hh ${PROJECT_SOURCE_DIR}/include/event.hh ${PROJECT_SOURCE_DIR}/include/rootdriver.hh)
# slowdaq
file(GLOB slowdaq_sources ${PROJECT_SOURCE_DIR}/src/s*.cc ${PROJECT_SOURCE_DIR}/src/driver.cc)
file(GLOB slowdaq_headers ${PROJECT_SOURCE_DIR}/include/s*.hh ${PROJECT_SOURCE_DIR}/include/driver.hh)
#----------------------------------------------------------------------------
# Add the executable, and link it to the ROOT libraries
#
#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wno-shadow -std=gnu++0x -std=c++0x" )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-shadow " )
#set(CMAKE_CXX_FLAGS " -Wno-shadow " )
add_executable(daqana daqana.cc ${daqana_sources} ${daqana_headers})
add_executable(slowdaq slowdaq.cc ${slowdaq_sources} ${slowdaq_headers})
message(STATUS "root libs " ${ROOT_LIBRARIES})
target_link_libraries(daqana ${ROOT_LIBRARIES} ${ROOT_LIBRARIES} -std=c++0x)
target_link_libraries(slowdaq ${ROOT_LIBRARIES} ${ROOT_LIBRARIES} -std=c++0x)
# target_link_libraries(daqana ${ROOT_LIBRARIES})

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS daqana slowdaq DESTINATION bin)
#install(TARGETS slowdaq DESTINATION bin)

