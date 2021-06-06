cmake_minimum_required(VERSION 3.13)
project(ft_ping)

set(CMAKE_C_STANDARD 99)

include_directories(/ft_printf/includes)

add_executable(ft_ping
        main.c
        )

target_link_libraries(ft_ping
        ${PROJECT_SOURCE_DIR}/ft_printf/libftprintf.a)