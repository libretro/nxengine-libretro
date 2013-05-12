#ifndef _OBJFUNCPTRS_H
#define _OBJFUNCPTRS_H

/* ONTICK */

/* firstcave */
extern void ai_bat_up_down(Object *o);
extern void ai_critter(Object *o);
extern void ai_hermit_gunsmith(Object *o);
extern void ai_door_enemy(Object *o);

/* weapons */
extern void ai_blade_l3_shot(Object *o);
extern void ai_bubbler_l12(Object *o);
extern void ai_bubbler_l3(Object *o);
extern void ai_bubbler_sharp(Object *o);
extern void ai_fireball(Object *o);
extern void ai_fireball_level_23(Object *o);
extern void ai_fireball_trail(Object *o);
extern void ai_nemesis_shot(Object *o);
extern void ai_polar_shot(Object *o);
extern void ai_mgun_trail(Object *o);
extern void ai_mgun_spawner(Object *o);
extern void ai_whimsical_star(Object *o);
extern void ai_spur_shot(Object *o);
extern void ai_spur_trail(Object *o);
extern void ai_snake(Object *o);
extern void ai_snake_23(Object *o);
extern void ai_snake_trail(Object *o);

/* sym */
extern void ai_smokecloud(Object *o);

/* AFTERMOVE */

extern void aftermove_blade_l12_shot(Object *o);
extern void aftermove_blade_slash(Object *o);
extern void ai_missile_shot(Object *o);
extern void ai_missile_boom_spawner(Object *o);

#endif
