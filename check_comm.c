#include "header/check_comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "header/game.h"
#include "header/map.h"
#include "header/hero_stats.h"
#include "header/inventory.h"
#include "header/save.h"


// --- Вспомогательные функции ---

// Приводим строку к нижнему регистру на месте
void to_lower_string(char *str) {
    for (int i = 0; str[i] != '\0'; ++i) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] - 'A' + 'a';
        }
    }
}

// Разбивает строку buffer на слова.
// Заменяет пробелы на '\0' и возвращает массив указателей на начала слов.
// Возвращает количество слов (не более max_words).
int split_words(char *buffer, char **out_words, int max_words) {
    int count = 0;
    char *p = buffer;

    while (*p != '\0' && count < max_words) {
        // Пропускаем пробелы
        while (*p == ' ') {
            ++p;
        }
        if (*p == '\0') break;

        // Начало слова
        out_words[count] = p;
        ++count;

        // Идём до следующего пробела или конца строки
        while (*p != '\0' && *p != ' ') {
            ++p;
        }

        // Если не конец строки — заменяем пробел на '\0', чтобы слово было нуль-терминированным
        if (*p != '\0') {
            *p = '\0';
            ++p; // переходим к следующему символу
        }
    }
    return count;
}

// Парсит число из строки вручную.
// Возвращает:
//   1 — успешно, out_value содержит число
//   0 — нет цифр
//  -1 — есть нецифровые символы
//  -2 — переполнение относительно int
int parse_number(const char *str, int *out_value) {
    if (!str || *str == '\0') return 0;

    const char *p = str;
    long long value = 0;
    int has_digit = 0;


    while (*p != '\0') {
        if (*p >= '0' && *p <= '9') {
            has_digit = 1;
            int digit = *p - '0';

            // Проверка на переполнение перед умножением и сложением
            if (value > (LLONG_MAX - digit) / 10) {
                return -2; // переполнение
            }
            value = value * 10 + digit;

            if (value > INT_MAX) {
                return -2;
            }
        } else {
            // Нецифровой символ
            return -1;
        }
        ++p;
    }

    if (!has_digit) return 0;

    *out_value = (int)value;
    return 1;
}

// --- Сравнение команд с поддержкой сокращений ---
static int is_direction_command(const char *word, const char **out_full) {
    // Поддерживаем полные и сокращённые команды
    static const struct {
        const char *full;
        const char *short1;
    } dirs[] = {
        {"north", "n"},
        {"south", "s"},
        {"east",  "e"},
        {"west",  "w"}
    };

    for (size_t i = 0; i < sizeof(dirs) / sizeof(dirs[0]); ++i) {
        if (strcmp(word, dirs[i].full) == 0 || strcmp(word, dirs[i].short1) == 0) {
            *out_full = dirs[i].full;
            return 1;
        }
    }
    return 0;
}

// --- Основная функция проверки команд ---
bool check_command(Map *m, HeroState *hero, const char *cmd, Monsters *mon) {
    const char *dostup_com =
        "Доступные команды:\n"
        "   north, south, east, west - Перемещение\n"
        "   stats                    - Характеристики героя\n"
        "   inventory                - Показать инвентарь\n"
        "   use N                    - Использовать предмет номер N\n"
        "   on <state>               - Включить состояние\n"
        "   off <state>              - Выключить состояние\n"
        "   toggle <state>           - Переключить состояние\n"
        "   Состояния: poisoned, blinded, invisible, petrified, burning, sleep\n"
        "   look                     - Осмотреться\n"
        "   help                     - Показать команды\n"
        "   new                      - Новая игра\n"
        "   save                     - Сохранить прогресс\n"
        "   load                     - Загрузить последнее сохранение\n"
        "   quit                     - Выйти\n";

    // Копируем в локальный буфер, чтобы можно было модифицировать (ставить '\0' вместо пробелов)
    char buffer[64];
    size_t len = strlen(cmd);
    if (len >= sizeof(buffer)) {
        len = sizeof(buffer) - 1;
    }
    memcpy(buffer, cmd, len);
    buffer[len] = '\0';

    to_lower_string(buffer);

    char *words[16];
    int word_count = split_words(buffer, words, 16);

    if (word_count == 0) {
        printf("[Ошибка] Пустая команда.\n");
        return true;
    }

    const char *first = words[0];

    // --- quit ---
    if (strcmp(first, "quit") == 0) {
        const char *fname = "saves/save.txt";
        if (save_game(fname, m, &g_hero, &g_inventory, &g_ctx,mon)) {
            printf("Игра сохранена в %s\n", fname);
        } else {
            printf("[Ошибка] Ошибка сохранения! Проверьте права на запись или диск.\n");
        }
        printf("Игра окончена\n");
        return false;
    }

    // --- help ---
    if (strcmp(first, "help") == 0) {
        printf("%s", dostup_com);
        return true;
    }

    // --- look ---
    if (strcmp(first, "look") == 0) {
        printf("Ты осмотрелся\n");
        ++g_ctx.val;
        print_map(m, hero);
        return true;
    }

    // --- stats ---
    if (strcmp(first, "stats") == 0) {
        cmd_stats();
        return true;
    }

    // --- inventory ---
    if (strcmp(first, "inventory") == 0) {
        print_inventory(&g_inventory);
        return true;
    }

    // --- new ---
    if (strcmp(first, "new") == 0) {
        if (new_game(m,hero,&g_inventory,&g_ctx,mon)) {
            printf("[Загрузка] Новая игра загружена.\n");
            printf("%s\n",dostup_com);
            print_map(m, hero);
            print_inventory(&g_inventory);
        } else {printf("[Ошибка] Не удалось загрузить новую игру.\n");}
        return true;
    }

    // --- load ---
    if (strcmp(first, "load") == 0) {
        const char *path = "saves/save.txt";
        if (load_game(path, m, &g_hero, &g_inventory, &g_ctx,mon)) {
            printf("[Загрузка] Сохранённый прогресс восстановлен!\n");
            printf("Вы на %hu этаже подземелья\n", g_hero.floor_number);
            // Опционально: перерисовать карту, чтобы видеть монстров/двери
            print_map(m, &g_hero);
            cmd_stats();
            print_inventory(&g_inventory);
        } else {
            printf("[Ошибка] Не удалось загрузить сохранение. Проверьте файл.\n");
        }
        return true;
    }

    // --- save ---
    if (strcmp(first, "save") == 0) {
        const char *fname = "saves/save.txt";
        if (save_game(fname, m, &g_hero, &g_inventory, &g_ctx,mon)) {
            printf("Игра сохранена в %s\n", fname);
        } else {
            printf("[Ошибка] Ошибка сохранения! Проверьте права на запись или диск.\n");
        }
        return true;
    }

    // --- heroexit ---
    if (strcmp(first, "heroexit") == 0) {
        int exit_y, exit_x;

        if (find_exit_position(m, &exit_y, &exit_x)) {
            printf("чит активирован\n");
            // Телепортируем героя
            hero->hero_y = exit_y;
            hero->hero_x = exit_x;

            exit2 = true;

            if (exit2) {
                exit2 = false;// сразу сбрасываем
                ++g_hero.floor_number;
                game_stats_reset(&g_ctx);

                // Увеличиваем размер карты для нового этажа
                int new_rows = m->X + 2;
                int new_cols = m->Y + 2;

                monsters_free(mon);
                monsters_init(mon);

                if (generation_map(m, new_rows, new_cols, mon)!= 0) {
                    printf("\n!!! ПОДЗЕМЕЛЬЕ РАСШИРИЛОСЬ НАСТОЛЬКО, ЧТО ДАЖЕ ПАМЯТЬ НЕ ВМЕЩАЕТ ЕГО !!!\n");
                    printf("Вы прошли все доступные этажи и выбрались на поверхность!\n");
                    printf("Поздравляем, вы победили!\n\n");
                    cmd_stats();
                    print_inventory(&g_inventory);

                    return false;
                }

                // Сбрасываем позицию героя на стартовую (1,1)
                hero->hero_y = 1;
                hero->hero_x = 1;

                printf("Вы спустились на этаж %d!\n", g_hero.floor_number);
                print_map(m, hero);
                return true; // не печатаем сообщение «Ты пошёл на …», потому что это был переход
            }
        }
        return true;
    }

    // --- hesoyam ---

    if (strcmp(first, "hesoyam") == 0) {
        printf("чит активирован\n");
        hero->health = hero->max_health;
        double procent = (double)hero->health / hero->max_health * 100;
        printf("Здоровье: %hu/%hu (%.1f%%)", hero->health, hero->max_health, procent);
        return true;
    }

    // --- damage ---
    if (strcmp(first, "damage") == 0) {
        printf("чит активирован\n");
        add_potion(&g_inventory,POT_STRENGTH,1);
        printf("Вы нашли зелье: Зелье силы +5 к урону на 5 ходов\n");
        return true;
    }

    // --- use N ---
    if (strcmp(first, "use") == 0) {
        if (word_count < 2) {
            printf("[Ошибка] команда use требует номер предмета.\nИспользуйте: use N (N - номер предмета)\n");
            print_inventory(&g_inventory);
            return true;
        }
        int n;
        int res = parse_number(words[1], &n);
        if (res == 0) {
            printf("[Ошибка] отсутствует числовой аргумент.\n");
            return true;
        } if (res == -1) {
            printf("[Ошибка] аргумент должен содержать только цифры.\n");
            return true;
        } if (res == -2) {
            printf("[Ошибка] число слишком большое.\n");
            return true;
        }

        int index = n - 1; // пользователь считает с 1, а массив с 0
        int heal_amount = 0;
        int strength_turns = 0;

        if (!use_potion(&g_inventory, index, &heal_amount, &strength_turns)) {
            printf("[Ошибка] Неверный номер предмета или предмет закончился.\n");
            print_inventory(&g_inventory);
            return true;
        }

        if (heal_amount > 0) {
            g_hero.health += heal_amount;
            if (g_hero.health > g_hero.max_health) {
                g_hero.health = g_hero.max_health;
            }
            double procent = (double)g_hero.health / g_hero.max_health * 100;
            printf("Вы использовали зелье лечения. Восстановлено %d HP. Здоровье: %d/%d (%.1f%%)\n",
                   heal_amount, g_hero.health, g_hero.max_health, procent);
            ++g_ctx.val;
        } else if (strength_turns > 0) {
            ++g_ctx.val;
            apply_strength_buff(strength_turns, 5); // +5 урона
        }

        print_inventory(&g_inventory);
        return true;
    }

    // --- on/off/toggle <state> ---
    if (strcmp(first, "on") == 0 || strcmp(first, "off") == 0 || strcmp(first, "toggle") == 0) {
        if (word_count < 2) {
            printf("[Ошибка] требуется название состояния.\n");
            printf("Состояния: poisoned, blinded, invisible, petrified, burning, sleep\n");
            return true;
        }
        const char *sname = words[1];
        unsigned int mask = 0;
        if      (strcmp(sname, "poisoned")  == 0) mask = st_poisoned;
        else if (strcmp(sname, "blinded")   == 0) mask = st_blinded;
        else if (strcmp(sname, "invisible") == 0) mask = st_invisible;
        else if (strcmp(sname, "petrified") == 0) mask = st_petrified;
        else if (strcmp(sname, "burning")   == 0) mask = st_burning;
        else if (strcmp(sname, "sleep")     == 0) mask = st_sleep;
        else {
            printf("[Ошибка] Неизвестное состояние: %s\n", sname);
            printf("Доступные состояния: poisoned, blinded, invisible, petrified, burning, sleep\n");
            return true;
        }

        if (strcmp(first, "on") == 0) {
            states_on(mask);
            printf("Состояние \"%s\" включено.\n", sname);
        } else if (strcmp(first, "off") == 0) {
            states_off(mask);
            printf("Состояние \"%s\" выключено.\n", sname);
        } else if (strcmp(first, "toggle") == 0) {
            g_hero.hero_stats ^= mask;
            printf("Состояние \"%s\" переключено (%s).\n",
                   sname, states_is(mask) ? "вкл" : "выкл");
        }
        return true;
    }

    // --- Команды перемещения (с поддержкой сокращений) ---
    const char *dir_full = NULL;
    if (is_direction_command(first, &dir_full)) {
        if (move_hero(m, hero, dir_full, mon)) {
            if (exit2) {
                exit2 = false;// сразу сбрасываем
                ++g_hero.floor_number;
                game_stats_reset(&g_ctx);

                // Увеличиваем размер карты для нового этажа
                int new_rows = m->X + 2;
                int new_cols = m->Y + 2;

                monsters_free(mon);
                monsters_init(mon);

                if (generation_map(m, new_rows, new_cols, mon)!= 0) {
                    printf("\n!!! ПОДЗЕМЕЛЬЕ РАСШИРИЛОСЬ НАСТОЛЬКО, ЧТО ДАЖЕ ПАМЯТЬ НЕ ВМЕЩАЕТ ЕГО !!!\n");
                    printf("Вы прошли все доступные этажи и выбрались на поверхность!\n");
                    printf("Поздравляем, вы победили!\n\n");
                    cmd_stats();
                    print_inventory(&g_inventory);

                    return false;
                }

                // Сбрасываем позицию героя на стартовую (1,1)
                hero->hero_y = 1;
                hero->hero_x = 1;

                printf("Вы спустились на этаж %d!\n", g_hero.floor_number);
                print_map(m, hero);
                return true; // не печатаем сообщение «Ты пошёл на …», потому что это был переход
            }
            if (exit1) return false;
            const char *dir_msg = "";
            if      (strcmp(dir_full, "north") == 0) dir_msg = "север";
            else if (strcmp(dir_full, "south") == 0) dir_msg = "юг";
            else if (strcmp(dir_full, "east")  == 0) dir_msg = "восток";
            else if (strcmp(dir_full, "west")  == 0) dir_msg = "запад";
            printf("Ты пошёл на %s\n", dir_msg);
        }
        return true;
    }

    // Неизвестная команда
    printf("[Ошибка] Неизвестная команда\n%s", dostup_com);
    return true;
}