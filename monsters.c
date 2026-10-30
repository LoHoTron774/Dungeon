#include "header/monsters.h"
#include <stdlib.h>
#include <string.h>

void monsters_init(Monsters *m) {
    m->data = NULL;
    m->count = 0;
    m->capacity = 0;
}

static bool monsters_ensure_capacity(Monsters *m) {
    if (m->count < m->capacity) return true;

    size_t new_cap = (m->capacity == 0) ? 4 : m->capacity * 2;
    Monster *new_data = realloc(m->data, new_cap * sizeof(Monster));
    if (!new_data) return false;

    m->data = new_data;
    m->capacity = new_cap;
    return true;
}

bool monsters_add(Monsters *m, int x, int y, int hp, int damage) {
    if (!monsters_ensure_capacity(m)) return false;

    Monster *mon = &m->data[m->count];
    mon->x = x;
    mon->y = y;
    mon->hp = hp;
    mon->damage = damage;
    m->count++;
    return true;
}

bool monsters_remove_at_fast(Monsters *m, size_t idx) {
    if (idx >= m->count) return false;
    m->data[idx] = m->data[m->count - 1];  // последний на место удаляемого
    m->count--;
    return true;
}

int monsters_find_at(const Monsters *m, int x, int y) {
    for (size_t i = 0; i < m->count; ++i) {
        if (m->data[i].x == x && m->data[i].y == y) {
            return (int)i;
        }
    }
    return -1;
}

void monsters_free(Monsters *m) {
    free(m->data);
    m->data = NULL;
    m->count = 0;
    m->capacity = 0;
}
