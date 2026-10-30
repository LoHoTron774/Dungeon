#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include "hero_stats.h"
#include "monsters.h"

#define EVENT_NONE          0
#define EVENT_WALL          1
#define EVENT_OUT_OF_BOUNDS 2
#define EVENT_GOLD          3
#define EVENT_EXIT          4
#define EVENT_ZELIE         5
#define idx(y, x, cols) ((size_t)(y) * (cols) + (x))

typedef struct {
    int X;
    int Y;
    char *data;  // плоский массив: data[y * cols + x]
} Map;

extern bool exit1;
extern bool exit2;


void free_map(Map *m);

void print_map(const Map *m, const HeroState *hero);

bool get_move_result(const Map *m,
                     const HeroState *hero,
                     const char *cmd,
                     int  *out_next_y,
                     int  *out_next_x,
                     bool *out_in_bounds,
                     char *out_target,
                     int  *out_event_type);

bool move_hero(Map *m, HeroState *hero, const char *cmd, Monsters *monsters);
int generation_map(Map *m, int rows, int cols, Monsters *monsters);
bool find_exit_position(const Map *m, int *out_y, int *out_x);
#endif
