#ifndef MONSTERS_H
#define MONSTERS_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int x;
    int y;
    int hp;
    int damage;
} Monster;

typedef struct {
    Monster *data;
    size_t count;
    size_t capacity;
} Monsters;

void monsters_init(Monsters *m);
bool monsters_add(Monsters *m, int x, int y, int hp, int damage);
bool monsters_remove_at_fast(Monsters *m, size_t idx);
int monsters_find_at(const Monsters *m, int x, int y);
void monsters_free(Monsters *m);

#endif
