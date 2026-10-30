#include "header/inventory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int heal_amounts[4] = {
    0,
    20, // POT_HEAL_SMALL
    45, // POT_HEAL_MEDIUM
    80  // POT_HEAL_LARGE
};

const char *potion_names[] = {
    "пусто",
    "Зелье лечения малое +20 HP",
    "Зелье лечения среднее +45 HP",
    "Зелье лечения большое +80 HP",
    "Зелье силы +5 к урону на 5 ходов"
};

void init_inventory(inventory_t *inv) {
    memset(inv->slots, 0, sizeof(inv->slots));
    inv->slot_count = 0;
}

bool add_potion(inventory_t *inv, potion_type_t type, int amount) {
    if (type == POT_NONE || amount <= 0) return false;

    // Ищем, есть ли уже такой тип в инвентаре
    for (int i = 0; i < inv->slot_count; ++i) {
        if (inv->slots[i].type == type) {
            inv->slots[i].count += amount;
            return true;
        }
    }

    // Если нет — добавляем в первый свободный слот
    if (inv->slot_count >= INV_SIZE) {
        return false; // инвентарь полон
    }

    int idx = inv->slot_count++;
    inv->slots[idx].type = type;
    inv->slots[idx].count = amount;
    return true;
}

int find_potion_index(const inventory_t *inv, potion_type_t type) {
    for (int i = 0; i < inv->slot_count; ++i) {
        if (inv->slots[i].type == type) {
            return i;
        }
    }
    return -1;
}

bool use_potion(inventory_t *inv, int index, int *out_heal_amount, int *out_strength_bonus_turns) {
    *out_heal_amount = 0;
    *out_strength_bonus_turns = 0;

    if (index < 0 || index >= inv->slot_count) {
        return false;
    }

    inventory_slot_t *slot = &inv->slots[index];
    if (slot->count <= 0) {
        return false;
    }

    slot->count--;

    potion_type_t t = slot->type;
    if (t >= POT_HEAL_SMALL && t <= POT_HEAL_LARGE) {
        *out_heal_amount = heal_amounts[t];
        return true;
    }if (t == POT_STRENGTH) {
        // Бафф силы: +5 урона на 5 хода (можно менять)
        *out_strength_bonus_turns = 5;
        return true;
    }

    return false;
}

void print_inventory(const inventory_t *inv) {
    printf("\n--- Инвентарь ---\n");
    if (inv->slot_count == 0) {
        printf("Инвентарь пуст.\n");
        printf("-----------------\n");
        printf("\n");
        return;
    }

    for (int i = 0; i < inv->slot_count; ++i) {
        const inventory_slot_t *s = &inv->slots[i];
        if (s->count <= 0) continue;

        printf("%d. %s — %d шт.\n", i + 1, potion_names[s->type], s->count);
    }
    printf("-----------------\n");
    printf("\n");
}

potion_type_t get_random_useful_potion() {
    // rand() % 4 даёт 0,1,2,3. Прибавляем 1 → получаем 1,2,3,4 (все полезные)
    int r = rand() % 4;
    return (potion_type_t)(r + 1);
}