cmake_minimum_required(VERSION 2.8)

project(onrakish-lib)

file(GLOB LIBSOURCE
    "*.cpp"
)

file(GLOB HEADER
    "*.h"
    "*.hpp"
)

add_library(onrakish-lib STATIC ${LIBSOURCE})

set(LIBRARY_INCLUDE_DIRS ${LIBSOURCE} CACHE STRING "library dir" FORCE)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

#add MapLoader
set(MAPLOADER_INCLUDE_DIR "" CACHE FILEPATH "sets maploader include path")
include_directories(${MAPLOADER_INCLUDE_DIR})

#zlib
find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
  include_directories( ${ZLIB_INCLUDE_DIRS} )
  target_link_libraries(onrakish-lib ${ZLIB_LIBRARIES} )
endif(ZLIB_FOUND)

#Find SFML
find_package(SFML 2 COMPONENTS system window graphics audio network REQUIRED)
if(SFML_FOUND)
  include_directories(${SFML_INCLUDE_DIR})
  target_link_libraries(onrakish-lib ${SFML_LIBRARIES})
endif()