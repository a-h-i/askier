cmake_minimum_required(VERSION 4.0)
project(
        askier
        VERSION 0.0.1
        DESCRIPTION "ASCII art converter"
        LANGUAGES CXX
)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_subdirectory(apps)
add_subdirectory(libs)


install(TARGETS askier-gui)