cmake_minimum_required(VERSION 3.31)
project(
        Askier
        VERSION 0.0.3
        DESCRIPTION "ASCII art converter"
        LANGUAGES CXX
)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_AUTOMOC ON)
add_compile_definitions(
        CL_HPP_TARGET_OPENCL_VERSION=200
)

configure_file(
        include/askier/version.hpp.in
        include/askier/version.hpp
)

add_subdirectory(apps)
add_subdirectory(libs)


install(TARGETS askier-gui askier-cli)