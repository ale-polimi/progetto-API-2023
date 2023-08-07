cmake_minimum_required(VERSION 3.25)
project(Progetto_API_2023 C)

set(CMAKE_C_STANDARD 11)
set(GCC_COVERAGE_COMPILE_FLAGS "-DEVAL -Wall -Werror -std=gnu11 -O2 -pipe -static")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

add_executable(Progetto_API_2023 main.c)
