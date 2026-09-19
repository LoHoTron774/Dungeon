cmake_minimum_required(VERSION 3.10)
project(dungeon)

set(CMAKE_C_STANDARD 99)

add_executable(dungeon main.c
        map.h
        map.c
        game.h
        game.c
