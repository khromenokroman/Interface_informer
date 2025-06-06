cmake_minimum_required(VERSION 3.31)
project(printer
        VERSION 0.0.1
        LANGUAGES CXX
        HOMEPAGE_URL "https://github.com/khromenokroman/printer_interface"
)

configure_file(Version.in Version @ONLY)

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
        boost_program_options
)

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
)

set(CPACK_GENERATOR DEB)
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEB_COMPONENT_INSTALL ON)

set(CPACK_DEBIAN_PACKAGE_NAME "${PROJECT_NAME}${PROJECT_VERSION_MAJOR}")
#set(CPACK_DEBIAN_PACKAGE_DEPENDS "libnl-3, libfmt11, libnl-route-3")
set(CPACK_DEBIAN_DESCRIPTION "Getting info interface")
set(CPACK_DEBIAN_MAINTAINER "roma55592@yandex.ru")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "roma55592@yandex.ru")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

include(CPack)