cmake_minimum_required(VERSION 4.0)
project(LFTC C)

set(CMAKE_C_STANDARD 11)

add_executable(LFTC files/lexer.c files/utils.c main.c)
