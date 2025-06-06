cmake_minimum_required(VERSION 3.31)
project(printer)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNL REQUIRED libnl-3.0 libnl-route-3.0)
find_package(fmt REQUIRED)

include_directories(${LIBNL_INCLUDE_DIRS})
link_directories(${LIBNL_LIBRARY_DIRS})

add_executable(${PROJECT_NAME}
        main.cpp
        switch_ns.cpp
        printer.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${LIBNL_LIBRARIES}
        fmt::fmt
)
