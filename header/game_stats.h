#ifndef GAME_STATS_H
#define GAME_STATS_H

#include <stdbool.h>

typedef struct {

    int row;
    int col;
    int val;        // счётчик ходов (тик)
    int old_val;    // предыдущий тик

    bool exit1;      // флаг смерти/конца игры
    bool exit2;      // флаг перехода на следующий этаж
} Gamestats;

void game_stats_init(Gamestats *ctx);
void game_stats_reset(Gamestats *ctx);

#endif