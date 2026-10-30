#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "header/game_stats.h"
#include "header/inventory.h"
#include "header/map.h"

bool save_game(const char *filename,
               Map *map,
               const HeroState *hero,
               const inventory_t *inv,
               const Gamestats *stats,
               const Monsters *monsters)
{
    if (!filename || !map || !hero || !inv || !stats) {
        fprintf(stderr, "[Ошибка] Один из указателей равен NULL.\n");
        return false;
    }

    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("[Ошибка] Не удалось открыть файл для сохранения");
        return false;
    }

    // 1. Карта: размеры и содержимое
    fprintf(f, "MAP_SIZE %d %d\n", map->X, map->Y);
    for (int y = 0; y < map->X; ++y) {
        for (int x = 0; x < map->Y; ++x) {
            fprintf(f, "%c", map->data[idx(y, x, map->Y)]);
        }
        fprintf(f, "\n");
    }

    // 2. Монстры
    fprintf(f, "MONSTERS_COUNT %zu\n", monsters->count);
    for (size_t i = 0; i < monsters->count; ++i) {
        const Monster *m = &monsters->data[i];
        // x y hp damage
        fprintf(f, "MONSTER %d %d %d %d\n", m->x, m->y, m->hp, m->damage);
    }

    // 3. Герой: координаты, здоровье, урон, этаж
    fprintf(f, "HERO_POS %d %d\n", hero->hero_y, hero->hero_x);
    fprintf(f, "HERO_STATS %hu %hu %hhu %hu %u %u\n",
            hero->health,
            hero->max_health,
            hero->damage,
            hero->floor_number,
            hero->gold,
            hero->hero_stats);

    // Бафф силы (если есть)
    fprintf(f, "HERO_BUFF %d %d\n",
            hero->strength_bonus_damage,
            hero->strength_bonus_turns);

    // 4. Инвентарь: количество слотов и каждый слот (тип, количество)
    fprintf(f, "INVENTORY_SLOTS %d\n", inv->slot_count);
    for (int i = 0; i < inv->slot_count; ++i) {
        // Используем enum potion_type_t как число
        fprintf(f, "ITEM %d %d\n", (int)inv->slots[i].type, inv->slots[i].count);
    }

    // 5. Состояние игры (флаги и тики)
    fprintf(f, "GAME_STATS %d %d %d %d\n",
            stats->val,
            stats->old_val,
            (int)stats->exit1,
            (int)stats->exit2);

    if (fclose(f) != 0) {
        fprintf(stderr, "[Ошибка] Не удалось корректно закрыть файл. Возможно, диск заполнен или файл заблокирован.\n");
        return false;
    }
    return true;
}

int read_token(FILE *f, char *buf, size_t buf_size) {
    int ch;
    size_t i = 0;

    // Пропускаем все пробельные символы (пробел, табуляция, \n, \r)
    while ((ch = fgetc(f)) != EOF && isspace((unsigned char)ch)) {
        continue;
    }
    if (ch == EOF) return 0;          // конец файла
    ungetc(ch, f);                   // возвращаем символ обратно

    while (i < buf_size - 1) {       // защита от переполнения
        ch = fgetc(f);
        if (ch == EOF || isspace((unsigned char)ch)) break;
        buf[i++] = (char)ch;
    }
    buf[i] = '\0';
    return i > 0;                    // успех, если прочитали хотя бы 1 символ
}

bool load_game(const char *filename,
               Map *map,
               HeroState *hero,
               inventory_t *inv,
               Gamestats *stats,
               Monsters *monsters)
{
    if (!filename || !map || !hero || !inv || !stats) return false;

    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("[Ошибка] Не удалось открыть файл для загрузки");
        return false;
    }

    char token[1024];
    int val1, val2, val3, val4;

    // 1. Читаем MAP_SIZE
    if (!read_token(f, token, sizeof(token)) || strcmp(token, "MAP_SIZE") != 0) {
        fprintf(stderr, "[Ошибка] Неверный формат файла (нет MAP_SIZE)\n");
        fclose(f);
        return false;
    }
    if (fscanf(f, "%d %d", &val1, &val2) != 2) {
        fprintf(stderr, "[Ошибка] Ошибка чтения размеров карты\n");
        fclose(f);
        return false;
    }

    if (val1 <= 0 || val2 <= 0 || (size_t)val1 > 1000 || (size_t)val2 > 119) {
        fprintf(stderr, "[Ошибка] Недопустимые размеры карты: %d x %d\n", val1, val2);
        goto bad_format;
    }
    free_map(map);
    map->X = val1;
    map->Y = val2;
    const size_t total = (size_t)map->X * map->Y;
    map->data = malloc(total);


    // Читаем саму карту построчно
    for (int y = 0; y < map->X; ++y) {
        for (int x = 0; x < map->Y; ++x) {
            int ch = fgetc(f);
            // Пропускаем пробельные символы перед каждым символом карты
            while (ch != EOF && (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')) {
                ch = fgetc(f);
            }
            if (ch == EOF) {
                fprintf(stderr, "[Ошибка] Неожиданный конец файла при чтении карты\n");
                goto bad_format;
            }
            map->data[idx(y, x, map->Y)] = (char)ch;
        }
    }

    // Синхронизация после чтения карты перед следующим маркером
    int ch;
    while ((ch = fgetc(f)) != EOF && (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')) {
        continue;
    }
    if (ch != EOF) {
        ungetc(ch, f);
    }

    monsters_free(monsters);

    if (!read_token(f, token, sizeof(token))) {
        monsters->count = 0;
        monsters->capacity = 0;
        monsters->data = NULL;
        // Можно считать это корректным состоянием (нет монстров)
    } else if (strcmp(token, "MONSTERS_COUNT") != 0) {
        // Если маркер не тот, но файл не пустой — считаем формат повреждённым
        fprintf(stderr, "[Ошибка] Ожидался маркер 'MONSTERS_COUNT', но прочитано: '%s'\n", token);
        goto bad_format;
    } else {
        size_t count;
        if (fscanf(f, "%zu", &count) != 1) goto bad_format;

        if (count > 10000) {  // защита от огромного выделения
            fprintf(stderr, "[Ошибка] Слишком много монстров: %zu\n", count);
            goto bad_format;
        }

        monsters->capacity = (count > 0) ? count : 4;
        monsters->data = malloc(monsters->capacity * sizeof(Monster));
        if (!monsters->data) {
            fprintf(stderr, "[Ошибка] Не удалось выделить память под монстров.\n");
            goto bad_format;
        }
        monsters->count = 0;

        for (size_t i = 0; i < count; ++i) {
            if (!read_token(f, token, sizeof(token))) goto bad_format;

            if (strcmp(token, "MONSTER") != 0) goto bad_format;

            Monster *m = &monsters->data[monsters->count];
            if (fscanf(f, "%d %d %d %d", &m->x, &m->y, &m->hp, &m->damage) != 4) goto bad_format;

            // Проверка границ карты
            if (m->x < 0 || m->x >= map->Y || m->y < 0 || m->y >= map->X) {
                fprintf(stderr, "[Ошибка] Монстр за пределами карты: (%d, %d), карта (%d, %d)\n",
                        m->x, m->y, map->X, map->Y);
                goto bad_format;
            }

            monsters->count++;
        }
    }
    // 2. HERO_POS
    if (!read_token(f, token, sizeof(token)) || strcmp(token, "HERO_POS") != 0) goto bad_format;
    if (fscanf(f, "%d %d", &hero->hero_y, &hero->hero_x) != 2) goto bad_format;

    // 3. HERO_STATS
    if (!read_token(f, token, sizeof(token)) || strcmp(token, "HERO_STATS") != 0) goto bad_format;
    if (fscanf(f, "%hu %hu %hhu %hu %u %u",
               &hero->health, &hero->max_health,
               &hero->damage, &hero->floor_number,
               &hero->gold, &hero->hero_stats) != 6) goto bad_format;

    // 4. HERO_BUFF
    if (!read_token(f, token, sizeof(token)) || strcmp(token, "HERO_BUFF") != 0) goto bad_format;
    if (fscanf(f, "%d %d",
               &hero->strength_bonus_damage, &hero->strength_bonus_turns) != 2) goto bad_format;

    // 5. INVENTORY_SLOTS
    if (!read_token(f, token, sizeof(token)) || strcmp(token, "INVENTORY_SLOTS") != 0) goto bad_format;
    if (fscanf(f, "%d", &val1) != 1) goto bad_format;
    inv->slot_count = val1; // Важно: убедись, что массив slots уже выделен!

    for (int i = 0; i < inv->slot_count; ++i) {
        if (!read_token(f, token, sizeof(token)) || strcmp(token, "ITEM") != 0) goto bad_format;
        if (fscanf(f, "%d %d", &val1, &val2) != 2) goto bad_format;
        inv->slots[i].type = (potion_type_t)val1;
        inv->slots[i].count = val2;
    }

    // 6. GAME_STATS
    if (!read_token(f, token, sizeof(token)) || strcmp(token, "GAME_STATS") != 0) goto bad_format;
    if (fscanf(f, "%d %d %d %d",
               &stats->val, &stats->old_val, &val3, &val4) != 4) goto bad_format;
    stats->exit1 = (bool)val3;
    stats->exit2 = (bool)val4;

    fclose(f);
    return true;

bad_format:
    fclose(f);
    fprintf(stderr, "[Ошибка] Повреждённый файл сохранения или неверный формат.\n");
    return false;
}

bool new_game(Map *map,
               HeroState *hero,
               inventory_t *inv,
               Gamestats *stats,
               Monsters *monsters) {
    free_map(map);
    monsters_free(monsters);
    game_stats_reset(stats);

    monsters_init(monsters);
    init_inventory(inv);

    hero->floor_number = 0;
    hero->gold = 0;
    hero->health = 100;
    hero->strength_bonus_damage = 0;
    hero->strength_bonus_turns = 0;
    hero->hero_stats = 0;

    if (generation_map(map, stats->row, stats->col, monsters)!=0) {
        fputs("[Ошибка] Не удалось сгенерировать карту подземелья!\n", stderr);
        return false;
    }

    hero->hero_x = 1;
    hero->hero_y = 1;

    return true;

}