set(APP_NAME example)

find_package(fmt REQUIRED)

add_executable(${APP_NAME}
        main.cpp
)

target_link_libraries(${APP_NAME} PRIVATE
        interface_informer
        fmt::fmt
)