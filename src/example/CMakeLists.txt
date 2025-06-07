set(APP_NAME example)

find_package(fmt REQUIRED)
find_package(interface_informer REQUIRED)

add_executable(${APP_NAME}
        main.cpp
)

target_link_libraries(${APP_NAME} PRIVATE
        interface_informer::interface_informer
        fmt::fmt
)