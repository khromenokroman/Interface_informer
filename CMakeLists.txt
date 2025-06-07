cmake_minimum_required(VERSION 3.25)
project(interface_informer
        VERSION 0.0.1
        LANGUAGES CXX
        HOMEPAGE_URL "https://github.com/khromenokroman/printer_interface"
)

configure_file(Version.in Version @ONLY)

set(CMAKE_CXX_STANDARD 20)

option(BUILD_SHARED_LIBS "Собирать динамическую библиотеку вместо статической" ON)
option(BUILD_EXAMPLE "Собирать приложение (пример)" ON)

if (BUILD_EXAMPLE)
    add_subdirectory(src/example)
endif ()
add_subdirectory(src/lib)

set(CPACK_GENERATOR DEB)
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEB_COMPONENT_INSTALL ON)

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "roma55592@yandex.ru")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

if (BUILD_SHARED_LIBS)
    # Настройки для динамической библиотеки
    set(CPACK_DEBIAN_LIB_PACKAGE_NAME "lib${PROJECT_NAME}${PROJECT_VERSION_MAJOR}")
    set(CPACK_DEBIAN_LIB_DESCRIPTION "Getting info interface")
    set(CPACK_DEBIAN_LIB_MAINTAINER "roma55592@yandex.ru")
    set(CPACK_DEBIAN_LIB_PACKAGE_SHLIBDEPS ON)

    # Настройки для dev-пакета при динамической библиотеке
    set(CPACK_DEBIAN_DEV_PACKAGE_NAME "lib${PROJECT_NAME}${PROJECT_VERSION_MAJOR}-dev")
    set(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS "lib${PROJECT_NAME}${PROJECT_VERSION_MAJOR} (= ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})")
    set(CPACK_DEBIAN_DEV_DESCRIPTION "Getting info interface - development files")
    set(CPACK_DEBIAN_DEV_MAINTAINER "roma55592@yandex.ru")
    set(CPACK_DEBIAN_DEV_PACKAGE_SHLIBDEPS ON)

    # Указываем компоненты для CPack
    set(CPACK_COMPONENTS_ALL lib dev)
else ()
    # Настройки для статической библиотеки - создаём только dev-пакет
    set(CPACK_DEBIAN_DEV_PACKAGE_NAME "lib${PROJECT_NAME}${PROJECT_VERSION_MAJOR}-dev")
    set(CPACK_DEBIAN_DEV_DESCRIPTION "Getting info interface - static library")
    set(CPACK_DEBIAN_DEV_MAINTAINER "roma55592@yandex.ru")
    set(CPACK_DEBIAN_DEV_PACKAGE_SHLIBDEPS ON)

    # Указываем только компонент dev для CPack
    set(CPACK_COMPONENTS_ALL dev)

    # Указываем, что это единственный компонент (важно!)
    set(CPACK_COMPONENT_DEV_REQUIRED ON)
endif ()

include(CPack)