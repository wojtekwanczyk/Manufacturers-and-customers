cmake_minimum_required(VERSION 3.10)
project(CLion_zad1 C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -pthread")

add_executable(CLion_zad1 main.c)