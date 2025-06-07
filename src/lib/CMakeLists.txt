set(LIB_NAME interface_informer)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNL REQUIRED libnl-3.0 libnl-route-3.0)
find_package(fmt REQUIRED)

include_directories(${LIBNL_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${LIBNL_LIBRARY_DIRS})

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${LIB_NAME} SHARED
        printer.cpp
)

set_target_properties(${LIB_NAME} PROPERTIES
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_link_libraries(${LIB_NAME} PRIVATE
        ${LIBNL_LIBRARIES}
        fmt::fmt
)

install(DIRECTORY ${INCLUDE_DIR}/
        COMPONENT dev
        DESTINATION include
)

install(TARGETS ${LIB_NAME}
        COMPONENT lib
        LIBRARY DESTINATION lib
        NAMELINK_COMPONENT dev
)