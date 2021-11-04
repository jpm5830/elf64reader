cmake_minimum_required(VERSION 3.20)
project(elf C)

set(CMAKE_C_STANDARD 99)

add_executable(elf64reader main.c elf64funcs.c)
