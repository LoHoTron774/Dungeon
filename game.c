#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include "game.h"
#include "check_comm.h"
#include "map.h"

void run_game() {
    SetConsoleOutputCP(65001);
    const char *dostup_com = "Доступные команды: north, south, east, west, help, look, quit\n";
    printf("%s\n", dostup_com);
    print_map();

    int val = 0;

    bool is_running = true;
    while (is_running) {
        val++;
        printf("\n[Ход %d] > ", val);
        char cmd[32];
        if (scanf("%31s",cmd) !=1) {break;}
        is_running = check_command(cmd);
    }
}