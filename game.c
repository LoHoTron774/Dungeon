#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>
#include "game.h"
#include "map.h"

void run_game() {
    SetConsoleOutputCP(65001);
    const char *dostup_com = "Доступные команды: north, south, east, west, look, quit\n";
    printf("%s\n", dostup_com);
    print_map();

    int val = 0;

    bool is_running = true;
    while (is_running) {
        val++;
        printf("\n[Ход %d] > ", val);
        char cmd[32];
        if (scanf("%31s",cmd) !=1) {break;}
        if (strcmp(cmd,"north") == 0 ) {
            printf("Ты пошел на север\n");
        } else if (strcmp(cmd,"south")==0) {
            printf("Ты пошел на юг\n");
        } else if (strcmp(cmd,"east")==0) {
            printf("Ты пошел на восток\n");
        } else if (strcmp(cmd,"west")==0) {
            printf("Ты пошел на запад\n");
        } else if (strcmp(cmd,"look")==0) {
            printf("Ты осмотрелся\n");
            print_map();
        } else if (strcmp(cmd,"quit")==0) {
            printf("Игра окончена\n");
            is_running = false;
        } else {
            printf("Не известная команда\n");
            printf("%s", dostup_com);

        }
    }
}