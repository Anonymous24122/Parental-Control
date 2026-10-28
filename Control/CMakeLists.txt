cmake_minimum_required(VERSION 3.16)

project(Control LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Control main.cpp
    dns_filter.h dns_filter.cpp
    promise.h
    headers.h
    domains.h
    utils.h)

include(GNUInstallDirs)
install(TARGETS Control
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
target_link_options(${PROJECT_NAME} PRIVATE "-static-libgcc" "-static-libstdc++" "-static")