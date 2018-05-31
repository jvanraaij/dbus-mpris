cmake_minimum_required(VERSION 3.0)
project(mpris_dumptrack C)

set(CMAKE_C_STANDARD 99)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO gio-2.0 REQUIRED)

add_executable(mpris_dumptrack main.c)
target_include_directories(mpris_dumptrack PUBLIC ${GIO_INCLUDE_DIRS})
target_link_libraries(mpris_dumptrack PUBLIC ${GIO_LIBRARIES})
