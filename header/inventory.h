#ifndef INVENTORY_H
#define INVENTORY_H

#include <stdbool.h>

extern const char *potion_names[];
// Типы зелий
typedef enum {
    POT_NONE = 0,
    POT_HEAL_SMALL,   // 20 HP
    POT_HEAL_MEDIUM,  // 45 HP
    POT_HEAL_LARGE,   // 80 HP
    POT_STRENGTH      // +5 урона на 5 ходов
} potion_type_t;

// Один слот инвентаря: тип и количество
typedef struct {
    potion_type_t type;
    int count;
} inventory_slot_t;

#define INV_SIZE 10

// Сам инвентарь: массив слотов и счётчик занятых типов
typedef struct {
    inventory_slot_t slots[INV_SIZE];
    int slot_count;
} inventory_t;

void init_inventory(inventory_t *inv);
bool add_potion(inventory_t *inv, potion_type_t type, int amount);
int find_potion_index(const inventory_t *inv, potion_type_t type);
// Возвращает heal_amount > 0 если лечение, strength_bonus_turns > 0 если бафф силы
bool use_potion(inventory_t *inv, int index, int *out_heal_amount, int *out_strength_bonus_turns);
void print_inventory(const inventory_t *inv);
potion_type_t get_random_useful_potion();

#endif

