#include "check_comm.h"
#include <stdio.h>
#include <string.h>
#include "map.h"

bool check_command(const char *cmd) {
    const char *dostup_com = "Доступные команды: north, south, east, west, help, look, quit\n";
    if (strcmp(cmd, "north") == 0) {
        printf("Ты пошёл на север\n");
    } else if (strcmp(cmd, "south") == 0) {
        printf("Ты пошёл на юг\n");
    } else if (strcmp(cmd, "east") == 0) {
        printf("Ты пошёл на восток\n");
    } else if (strcmp(cmd, "west") == 0) {
        printf("Ты пошёл на запад\n");
    } else if (strcmp(cmd, "look") == 0) {
        printf("Ты осмотрелся\n");
        print_map();
    } else if (strcmp(cmd, "help") == 0) {
        printf("%s", dostup_com);
    } else if (strcmp(cmd, "quit") == 0) {
        printf("Игра окончена\n");
        return false;
    } else {
        printf("Неизвестная команда\n%s", dostup_com);
    }
    return true;
}