cmake_minimum_required(VERSION 3.10)
project(dungeon)

set(CMAKE_C_STANDARD 99)

add_executable(dungeon
        main.c
        game.c
        map.c
        hero_stats.c
        check_comm.c
        inventory.c
        header/monsters.h
        monsters.c
        game_stats.c
        header/game_stats.h
        save.c
        header/save.h
)
