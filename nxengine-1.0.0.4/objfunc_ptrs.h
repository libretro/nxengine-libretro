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

extern void ai_null(Object *o);
extern void ai_hvtrigger(Object *o);
extern void ai_xp(Object *o);
extern void ai_powerup(Object *o);
extern void ai_hidden_powerup(Object *o);
extern void ai_xp_capsule(Object *o);
extern void ai_save_point(Object *o);
extern void ai_recharge(Object *o);
extern void ai_chest_closed(Object *o);
extern void ai_chest_open(Object *o);
extern void ai_lightning(Object *o);
extern void ai_teleporter(Object *o);
extern void ai_door(Object *o);
extern void ai_largedoor(Object *o);
extern void ai_press(Object *o);
extern void ai_terminal(Object *o);
extern void ai_fan_vert(Object *o);
extern void ai_fan_hoz(Object *o);
extern void ai_fan_droplet(Object *o);
extern void ai_sprinkler(Object *o);
extern void ai_droplet_spawner(Object *o);
extern void ai_water_droplet(Object *o);
extern void ai_bubble_spawner(Object *o);
extern void ai_chinfish(Object *o);
extern void ai_fireplace(Object *o);
extern void ai_straining(Object *o);
extern void ai_smoke_dropper(Object *o);
extern void ai_scroll_controller(Object *o);
extern void ai_quake(Object *o);
extern void ai_generic_angled_shot(Object *o);
extern void onspawn_spike_small(Object *o);
extern void ai_animate2(Object *o);
extern void ai_animate4(Object *o);

/* ai/npc/balrog */
extern void onspawn_balfrog(Object *o);
extern void ai_balrog(Object *o);
extern void ai_balrog_drop_in(Object *o);
extern void ai_balrog_bust_in(Object *o);

/* ai/npc/misery */
extern void ai_misery_float(Object *o);
extern void ai_misery_stand(Object *o);
extern void ai_miserys_bubble(Object *o);

/* oside */
extern void ai_sky_dragon(Object *o);
extern void ai_sandcroc(Object *o);
extern void ai_night_spirit(Object *o);
extern void ai_night_spirit_shot(Object *o);
extern void ai_hoppy(Object *o);
extern void ai_pixel_cat(Object *o);
extern void ai_little_family(Object *o);

/* ai/npc/npcplayer */
extern void ai_npc_player(Object *o);
extern void ai_ptelin(Object *o);
extern void ai_ptelout(Object *o);

/* AFTERMOVE */

extern void aftermove_blade_l12_shot(Object *o);
extern void aftermove_blade_slash(Object *o);
extern void ai_missile_shot(Object *o);
extern void ai_missile_boom_spawner(Object *o);

#endif
