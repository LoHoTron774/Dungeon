#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include "header/game.h"
#include "header/check_comm.h"
#include "header/map.h"
#include <time.h>
#include "header/hero_stats.h"
#include "header/inventory.h"
#include <stdlib.h>
#include "header/game_stats.h"

Gamestats g_ctx;
inventory_t g_inventory;
static Map g_map;
Monsters g_monsters;

void run_game() {
    SetConsoleOutputCP(65001);
    game_stats_init(&g_ctx);
    init_inventory(&g_inventory);
    monsters_init(&g_monsters);
    srand((unsigned int)time(NULL));

    if (generation_map(&g_map, g_ctx.row, g_ctx.col,&g_monsters) != 0) {
        fputs("[Ошибка] Не удалось сгенерировать карту подземелья!\n", stderr);
        return;
    }

    printf("Добро пожаловать в подземелье!\n");
    const char *dostup_com =
        "'#' - стена; '.' - пол; '@' - герой; 'G' - золото; 'M' - монстр; 'Z' - зелье; 'E' - выход на следующий этаж\n"
        "Доступные команды:\n"
        "   north, south, east, west - Перемещение\n"
        "   stats                    - Характеристики героя\n"
        "   inventory                - Показать инвентарь\n"
        "   use N                    - Использовать предмет номер N\n"
        "   on <state>               - Включить состояние\n"
        "   off <state>              - Выключить состояние\n"
        "   toggle <state>           - Переключить состояние\n"
        "   Состояния: poisoned, blinded, invisible, petrified, burning, sleep\n"
        "   look                     - Осмотреться\n"
        "   help                     - Показать команды\n"
        "   new                      - Новая игра\n"
        "   save                     - Сохранить прогресс\n"
        "   load                     - Загрузить последнее сохранение\n"
        "   quit                     - Выйти\n";
    printf("%s\n",dostup_com);
    print_map(&g_map, &g_hero);
    print_inventory(&g_inventory);

    bool is_running = true;
    while (is_running) {
        printf("\n[Ход %d] > ", g_ctx.val);
        char cmd[64]; // Увеличили буфер для безопасности
        if (!fgets(cmd, sizeof(cmd), stdin)) {
            break; // EOF или ошибка ввода
        }

        // Удаляем перевод строки (\r\n или \n)
        size_t len = strlen(cmd);
        while (len > 0 && (cmd[len - 1] == '\n' || cmd[len - 1] == '\r')) {
            cmd[--len] = '\0';
        }

        g_ctx.old_val = g_ctx.val;
        is_running = check_command(&g_map, &g_hero, cmd, &g_monsters);
        tick_strength_buff();
    }
    free_map(&g_map);
    monsters_free(&g_monsters);
}