#ifndef SAVE_H
#define SAVE_H
#include "game_stats.h"
#include "inventory.h"
#include "map.h"

bool save_game(const char *filename,
               Map *map,
               const HeroState *hero,
               const inventory_t *inv,
               const Gamestats *stats,
               const Monsters *monsters);

bool load_game(const char *filename,
               Map *map,
               HeroState *hero,
               inventory_t *inv,
               Gamestats *stats,
               Monsters *monsters);
bool new_game(Map *map,
               HeroState *hero,
               inventory_t *inv,
               Gamestats *stats,
               Monsters *monsters);
#endif //SAVE_H
