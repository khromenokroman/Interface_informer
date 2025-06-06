set(APP_NAME printer)

find_package(fmt REQUIRED)

add_executable(${APP_NAME}
        main.cpp
)

target_link_libraries(${APP_NAME} PRIVATE
        interface_informer
        fmt::fmt
)

install(TARGETS ${APP_NAME}
        COMPONENT bin
        RUNTIME DESTINATION bin
)