#include "header/game_stats.h"
#include <string.h>

#include "header/hero_stats.h"

void game_stats_init(Gamestats *ctx) {
    ctx->row = 6 + 2*g_hero.floor_number;
    ctx->col = 12+2*g_hero.floor_number;
    ctx->val = 1;
    ctx->old_val = 0;
    ctx->exit1 = false;
    ctx->exit2 = false;
}

void game_stats_reset(Gamestats *ctx) {
    ctx->val = 1;
    ctx->old_val = 0;
    ctx->exit1 = false;
    ctx->exit2 = false;
}
