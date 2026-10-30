#include "header/map.h"
#include "header/game.h"
#include "header/hero_stats.h"
#include "header/monsters.h"
#include "header/inventory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COLOR_EXIT      "\x1b[32m"        /* зелёный */
#define COLOR_PLAYER    "\x1b[34m"        /* синий */
#define COLOR_MONSTER   "\x1b[31m"        /* красный */
#define COLOR_GOLD      "\x1b[33m"        /* жёлтый */
#define COLOR_POTION    "\x1b[35m"        /* фиолетовый */
#define COLOR_RESET     "\x1b[0m"

static void putchar_colored(char c, const char *color) {
    if (color && *color) {
        fputs(color, stdout);
    }
    putchar(c);
    fputs(COLOR_RESET, stdout);
}

bool exit1 = false;
bool exit2 = false;

void free_map(Map *m) {
    free(m->data);
    m->data = NULL;
    m->X = m->Y = 0;
}

static inline char get_cell(const Map *m, int y, int x) {
    return m->data[y * m->Y + x];
}

void print_map(const Map *m, const HeroState *hero) {
    // for (int y = 0; y < m->X; ++y) {
    //     for (int x = 0; x < m->Y; ++x) {
    //         if (y == hero->hero_y && x == hero->hero_x) {
    //             putchar('@');
    //         } else {
    //             putchar(get_cell(m, y, x));
    //         }
    //     }
    //     putchar('\n');
    // }
    for (int y = 0; y < m->X; ++y) {
        for (int x = 0; x < m->Y; ++x) {
            const char *color = NULL;

            // Сначала проверяем, не стоит ли тут герой
            if (y == hero->hero_y && x == hero->hero_x) {
                color = COLOR_PLAYER;
                putchar_colored('@', color);
                continue;
            }

            char c = get_cell(m, y, x);

            switch (c) {
                case 'E':
                    color = COLOR_EXIT;
                    break;
                case 'M':
                    color = COLOR_MONSTER;
                    break;
                case 'G':
                    color = COLOR_GOLD;
                    break;
                case 'Z':
                    color = COLOR_POTION;
                    break;
                default:
                    /* Остальные символы (пол, выход и т.п.) — без цвета */
                    color = NULL;
                    break;
            }

            putchar_colored(c, color);
        }
        putchar('\n');
    }

}

bool get_move_result(const Map *m,
                     const HeroState *hero,
                     const char *cmd,
                     int  *out_next_y,
                     int  *out_next_x,
                     bool *out_in_bounds,
                     char *out_target,
                     int  *out_event_type)
{
    int next_y = hero->hero_y;
    int next_x = hero->hero_x;

    if (strcmp(cmd, "north") == 0)      --next_y;
    else if (strcmp(cmd, "south") == 0) ++next_y;
    else if (strcmp(cmd, "east") == 0)  ++next_x;
    else if (strcmp(cmd, "west") == 0)  --next_x;
    else {
        *out_event_type = -1;
        return false;
    }

    bool in_bounds = (next_y >= 0 && next_y < m->X && next_x >= 0 && next_x < m->Y);
    char target = '#';
    int event_type = EVENT_NONE;

    if (in_bounds) {
        target = get_cell(m, next_y, next_x);
    } else {
        event_type = EVENT_OUT_OF_BOUNDS;
    }

    *out_next_y     = next_y;
    *out_next_x     = next_x;
    *out_in_bounds  = in_bounds;
    *out_target     = target;
    *out_event_type = event_type;

    if (!in_bounds) return false;

    switch (target) {
        case '#':
            *out_event_type = EVENT_WALL;
            return false;
        case 'G':
            *out_event_type = EVENT_GOLD;
            break;
        case 'E':
            *out_event_type = EVENT_EXIT;
            break;
        case 'Z':
            *out_event_type = EVENT_ZELIE;
        default:
            *out_event_type = EVENT_NONE;
            break;
    }
    return true;
}

bool move_hero(Map *m, HeroState *hero, const char *cmd, Monsters *monsters) {
    int  next_y, next_x;
    bool in_bounds;
    char target;
    int  event_type;

    if (!get_move_result(m, hero, cmd, &next_y, &next_x, &in_bounds, &target, &event_type)) {
        if (event_type == EVENT_OUT_OF_BOUNDS) {
            printf("Нельзя выйти за границу карты!\n");
            print_map(m, hero);
            return false;
        }
        if (event_type == EVENT_WALL) {
            printf("Через стены ходить нельзя!\n");
            print_map(m, hero);
            return false;
        }
        return false;
    }

    // --- ОБРАБОТКА МОНСТРА ---
    if (target == 'M') {
        int mon_idx = monsters_find_at(monsters, next_x, next_y);
        if (mon_idx == -1) {
            // Это редкий случай: символ 'M' есть, а в массиве нет — заменяем на проход
            m->data[idx(next_y, next_x, m->Y)] = '.';
            printf("Странная аномалия подземелья… монстр исчез!\n");
            hero->hero_y = next_y;
            hero->hero_x = next_x;
            return true;
        }

        Monster *mon = &monsters->data[mon_idx];
        printf("Вы столкнулись с монстром! HP монстра: %d, урон: %d\n", mon->hp, mon->damage);

        // Цикл боя
        while (hero->health > 0 && mon->hp > 0) {
            ++g_ctx.val;
            int hero_dam = g_hero.damage + g_hero.strength_bonus_damage;
            // Ход героя
            if (hero_dam >= mon->hp) {
                mon->hp = 0;
            } else {mon->hp -= hero_dam;}
            printf("Вы бьёте монстра на %d урона. У монстра осталось HP: %d\n", hero_dam, mon->hp);

            if (mon->hp <= 0) {
                printf("Монстр повержен!\n");
                // Убираем монстра из массива
                monsters_remove_at_fast(monsters, (size_t)mon_idx);
                // Освободившееся место на карте
                m->data[idx(next_y, next_x, m->Y)] = '.';
                // Награда: немного золота
                hero->gold += 2;
                printf("Вы получили 2 золота. Всего золота: %u\n", hero->gold);
                break;
            }

            // Ход монстра
            if (mon->damage >= g_hero.health) {
                g_hero.health = 0;
            } else {
                hero->health -= mon->damage;
                printf("Монстр бьёт вас на %d урона. Ваше HP: %d\n", mon->damage, hero->health);
            }

            if (hero->health == 0) {
                printf("Вы погибли… Конец игры.\n");
                exit1 = true; // если у тебя есть такой флаг
                return false;
            }
        }

        if (!exit1) {
            // Если герой выжил — он всё равно перемещается на эту клетку
            hero->hero_y = next_y;
            hero->hero_x = next_x;
        }
        return true;
    }

    hero->hero_y = next_y;
    hero->hero_x = next_x;

    if (event_type == EVENT_GOLD) {
        hero->gold += 1;
        printf("Вы нашли золото. Ваше золото: %u\n", hero->gold);
        m->data[idx(next_y, next_x, m->Y)] = '.';
    } else if (event_type == EVENT_EXIT) {
        exit2 = true;
    } else if (target == 'Z') {
        potion_type_t random_potion = get_random_useful_potion();

        if (add_potion(&g_inventory, random_potion, 1)) {
            printf("Вы нашли зелье: %s!\n", potion_names[random_potion]);
        } else {
            printf("Инвентарь полон! Вы нашли зелье, но не смогли его подобрать.\n");
        }

        // Опционально: можно убрать 'Z' с карты, чтобы не подбирать дважды
        m->data[idx(next_y, next_x, m->Y)] = '.';
    }


    ++g_ctx.val;
    return true;
}

static bool can_reach_exit(const Map *m, int start_x, int start_y) {
    // Используем простой DFS/BFS. Для простоты сделаем рекурсивный DFS,
    // но с локальной visited-матрицей (динамической), чтобы не зависеть от глобального visited[256][256]
    int rows = m->X;
    int cols = m->Y;

    // Выделяем плоский visited под размер карты
    bool *visited = calloc((size_t)rows * cols, sizeof(bool));
    if (!visited) return false;

    // Стек для DFS (максимально rows*cols элементов)
    int *stack_y = malloc((size_t)rows * cols * sizeof(int));
    int *stack_x = malloc((size_t)rows * cols * sizeof(int));
    if (!stack_y || !stack_x) {
        free(visited); free(stack_y); free(stack_x);
        return false;
    }

    int top = 0;
    stack_y[top] = start_y;
    stack_x[top] = start_x;
    top++;

    while (top > 0) {
        --top;
        int y = stack_y[top];
        int x = stack_x[top];

        if (y < 0 || y >= rows || x < 0 || x >= cols) continue;
        size_t i = idx(y, x, cols);
        if (visited[i]) continue;

        char cell = m->data[i];
        if (cell == '#') continue; // стена

        visited[i] = true;

        if (cell == 'E') {
            // Нашли выход
            free(visited); free(stack_y); free(stack_x);
            return true;
        }

        // Добавляем соседей
        const int dy[4] = {-1, 1, 0, 0};
        const int dx[4] = {0, 0, -1, 1};
        for (int k = 0; k < 4; ++k) {
            int ny = y + dy[k];
            int nx = x + dx[k];
            if (ny >= 0 && ny < rows && nx >= 0 && nx < cols) {
                size_t ni = idx(ny, nx, cols);
                if (!visited[ni] && m->data[ni] != '#') {
                    stack_y[top] = ny;
                    stack_x[top] = nx;
                    top++;
                }
            }
        }
    }

    free(visited); free(stack_y); free(stack_x);
    return false;
}

int generation_map(Map *m, int rows, int cols, Monsters *monsters) {
    if (rows < 5 || cols < 5 || cols > 119) {
        return -1;
    }

    free_map(m);

    m->X = rows;
    m->Y = cols;

    const size_t total = (size_t)m->X * m->Y;
    m->data = malloc(total);
    if (!m->data) {
        m->X = m->Y = 0;
        return -1;
    }
    memset(m->data, '.', total);

    // 1. Рамка
    for (int y = 0; y < m->X; ++y) {
        for (int x = 0; x < m->Y; ++x) {
            if (y == 0 || y == m->X - 1 || x == 0 || x == m->Y - 1) {
                m->data[idx(y, x, m->Y)] = '#';
            }
        }
    }

    // 2. Внутренние стены (примерно 35%)
    int wall_count = (int)((rows - 2) * (cols - 2) * 0.35);
    for (int i = 0; i < wall_count; ++i) {
        int w_y = 1 + rand() % (m->X - 2);
        int w_x = 1 + rand() % (m->Y - 2);
        if (m->data[idx(w_y, w_x, m->Y)] == '.') {
            m->data[idx(w_y, w_x, m->Y)] = '#';
        } else {
            --i;
        }
    }

    // 3. Выход 'E'
    int exit_y, exit_x;
    do {
        exit_y = 1 + rand() % (m->X - 2);
        exit_x = 1 + rand() % (m->Y - 2);
    } while (m->data[idx(exit_y, exit_x, m->Y)] != '.' || (exit_x == 1 && exit_y== 1));
    m->data[idx(exit_y, exit_x, m->Y)] = 'E';

    // 4. Золото 'G'
    int gold_count = 6 + 2*g_hero.floor_number;
    for (int i = 0; i < gold_count; ++i) {
        int g_y = 1 + rand() % (m->X - 2);
        int g_x = 1 + rand() % (m->Y - 2);
        if (m->data[idx(g_y, g_x, m->Y)] == '.') {
            m->data[idx(g_y, g_x, m->Y)] = 'G';
        } else {
            --i;
        }
    }

    // 5. Монстры 'M'
    // Ставим монстров на свободные клетки (не на стены, не на E, не на G)
    int monster_count = 4 + 2*g_hero.floor_number + rand() % 3; // от 4 до 6 монстров
    for (int i = 0; i < monster_count; ++i) {
        int m_y, m_x;
        do {
            m_y = 1 + rand() % (m->X - 2);
            m_x = 1 + rand() % (m->Y - 2);
        } while (m->data[idx(m_y, m_x, m->Y)] != '.' || (m_y == 1 && m_x == 1));

        // Ставим символ на карту
        m->data[idx(m_y, m_x, m->Y)] = 'M';

        // Добавляем в массив монстров
        if (monsters) {
            // Можно варьировать HP/урон, пока сделаем одинаково
            int hp = 10 + rand() % 21;      // 10–30
            int damage = 3 + rand() % 18;   // 3–20
            monsters_add(monsters, m_x, m_y, hp, damage);
        }
    }

    int z_count = 1+2*g_hero.floor_number;
    for (int i = 0; i < z_count; ++i) {
        int z_y, z_x;
        // Ищем свободную клетку ('.'), не занятую стенами, выходом, золотом или монстрами
        do {
            z_y = 1 + rand() % (m->X - 2);
            z_x = 1 + rand() % (m->Y - 2);
        } while (m->data[idx(z_y, z_x, m->Y)] != '.');

        m->data[idx(z_y, z_x, m->Y)] = 'Z';
    }

    // 6. Проверка связности: из (1,1) должен быть путь к 'E'
    // Если нет — убираем случайные стены, пока путь не появится
    int attempts = 0;
    while (!can_reach_exit(m, 1, 1) && attempts < 1000) {
        int wy = 1 + rand() % (m->X - 2);
        int wx = 1 + rand() % (m->Y - 2);
        if (m->data[idx(wy, wx, m->Y)] == '#') {
            m->data[idx(wy, wx, m->Y)] = '.';
        }
        ++attempts;
    }

    return 0;
}

bool find_exit_position(const Map *m, int *out_y, int *out_x) {
    for (int y = 0; y < m->X; ++y) {
        for (int x = 0; x < m->Y; ++x) {
            if (m->data[idx(y, x, m->Y)] == 'E') {
                *out_y = y;
                *out_x = x;
                return true;
            }
        }
    }
    return false; // Выход не найден (теоретически невозможно при корректной генерации)
}
