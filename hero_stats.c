#include <stdio.h>
#include "header/hero_stats.h"

#include "header/game.h"

// Единственный глобальный экземпляр состояния героя.
HeroState g_hero = {
    .max_health   = 100,
    .health       = 100,
    .damage       = 10,
    .gold         = 0,
    .floor_number = 0,
    .hero_stats   = 0,
    .hero_y       = 1,
    .hero_x       = 1,
    .strength_bonus_turns = 0,
    .strength_bonus_damage = 0
};

void states_on(uint32_t mask) {
    g_hero.hero_stats |= mask;
}

void states_off(uint32_t mask) {
    g_hero.hero_stats &= ~mask;
}

int states_is(uint32_t mask) {
    return (g_hero.hero_stats & mask) != 0;
}

void print_stats() {
    printf("Состояния:\n");
    int any = 0;
    if (states_is(st_poisoned)) { printf("Отравлен\n"); any = 1; }
    if (states_is(st_blinded))   { printf("Ослеплен\n"); any = 1; }
    if (states_is(st_invisible)) { printf("Невидим\n"); any = 1; }
    if (states_is(st_petrified)) { printf("Окаменел\n"); any = 1; }
    if (states_is(st_burning))   { printf("Горит\n");   any = 1; }
    if (states_is(st_sleep))     { printf("Спит\n");    any = 1; }
    if (!any) printf("Нет активных состояний\n");
    printf("\n");
}

void cmd_stats() {
    printf("____Характеристики героя____\n");
    double procent = (double)g_hero.health / g_hero.max_health * 100;
    printf("Здоровье: %hu / %hu (%.1f%%)\n", g_hero.health, g_hero.max_health, procent);
    printf("Урон: %hhu\n", g_hero.damage);
    printf("Золото: %u\n", g_hero.gold);
    printf("Этаж подземелья: %hu\n", g_hero.floor_number);
    if (g_hero.strength_bonus_turns > 0) {
        printf("Бонус силы: +%d к урону, действует ещё %d ходов\n",
               g_hero.strength_bonus_damage,
               g_hero.strength_bonus_turns);
    } else {
        printf("Бонус силы: отсутствует\n");
    }
    print_stats();
    printf("\n");
}


void apply_strength_buff(int turns, int bonus_damage) {
    g_hero.strength_bonus_turns = turns;
    g_hero.strength_bonus_damage = bonus_damage;
    printf("Вы выпили зелье силы! +%d к урону на %d ходов.\n", bonus_damage, turns);
}

void tick_strength_buff() {
    if (g_hero.strength_bonus_turns > 0 && g_ctx.val!=g_ctx.old_val) {
        g_hero.strength_bonus_turns--;
        if (g_hero.strength_bonus_turns == 0) {
            g_hero.strength_bonus_damage = 0;
            printf("Эффект зелья силы закончился.\n");
        }
    }
}

int get_current_damage() {
    return g_hero.damage + g_hero.strength_bonus_damage;
}
