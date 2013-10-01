cmake_minimum_required(VERSION 2.8)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    SET(CMAKE_CXX_FLAGS "/EHsc")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(CMAKE_CXX_FLAGS "-std=c++11")
endif()

project(onrakish-client)

file(GLOB SOURCE
    "*.cpp"
)

file(GLOB HEADER
    "*.h"
    "*.hpp"
)

add_executable(onrakish-client ${SOURCE})

include_directories( ${LIBRARY_INCLUDE_DIRS} )

target_link_libraries(onrakish-client onrakish-lib)

#add tclap
set(TCLAP_INCLUDE_DIR "" CACHE FILEPATH "sets tclap include path")
include_directories(${TCLAP_INCLUDE_DIR})

#add MapLoader
set(MAPLOADER_INCLUDE_DIR "" CACHE FILEPATH "sets maploader include path")
include_directories(${MAPLOADER_INCLUDE_DIR})

#zlib
find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
  include_directories( ${ZLIB_INCLUDE_DIRS} )
  target_link_libraries(onrakish-client ${ZLIB_LIBRARIES} )
endif(ZLIB_FOUND)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

#Find SFML
find_package(SFML 2 COMPONENTS system window graphics audio network REQUIRED)
if(SFML_FOUND)
  include_directories(${SFML_INCLUDE_DIR})
  target_link_libraries(onrakish-client ${SFML_LIBRARIES})
endif()