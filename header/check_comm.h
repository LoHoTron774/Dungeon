#ifndef CHECK_COMM_H
#define CHECK_COMM_H

#include <stdbool.h>
#include "map.h"
#include "hero_stats.h"
#include "monsters.h"
// Возвращает false, если нужно завершить игру (quit, смерть и т.д.)
bool check_command(Map *m, HeroState *hero, const char *cmd, Monsters *mon);

//Функции для ручного чтения команды
void to_lower_string(char *str);
int split_words(char *buffer, char **out_words, int max_words);
int parse_number(const char *str, int *out_value);

#endif

