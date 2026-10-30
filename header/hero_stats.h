#ifndef HERO_STATS_H
#define HERO_STATS_H

#include <stdint.h>
#include <stdbool.h>

#define st_poisoned  (1u<<0)
#define st_blinded   (1u<<1)
#define st_invisible (1u<<2)
#define st_petrified (1u<<3)
#define st_burning   (1u<<4)
#define st_sleep     (1u<<5)

// Единая структура состояния героя — вместо набора разрозненных глобальных переменных.
// Так удобно передавать указатель на неё в функции, которые меняют состояние.
typedef struct {
    unsigned short max_health;
    unsigned short health;
    unsigned char  damage;
    unsigned int   gold;
    unsigned short floor_number;
    uint32_t       hero_stats;
    int            hero_y;
    int            hero_x;
    int strength_bonus_turns;      // сколько ходов ещё действует бафф
    int strength_bonus_damage;     // сколько бонусного урона
} HeroState;

extern HeroState g_hero;

void states_on (uint32_t mask);
void states_off(uint32_t mask);
int  states_is (uint32_t mask);

void cmd_stats(void);

void apply_strength_buff(int turns, int bonus_damage);
void tick_strength_buff();
int get_current_damage();
#endif // HERO_STATS_H

