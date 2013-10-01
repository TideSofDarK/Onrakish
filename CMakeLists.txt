cmake_minimum_required(VERSION 2.8)

project(onrakish)

include_directories(onrakish-lib)
add_subdirectory(onrakish-lib)

include_directories(onrakish-client)
add_subdirectory(onrakish-client)

include_directories(onrakish-server)
add_subdirectory(onrakish-server)