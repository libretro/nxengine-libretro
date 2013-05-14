#ifndef _OBJFUNCPTRS_H
#define _OBJFUNCPTRS_H

extern void ai_mimiga_farmer(Object *o);
extern void ai_behemoth(Object *o);
extern void onspawn_mimiga_cage(Object *o);
extern void ai_ballos_platform(Object *o);
extern void ai_minicore_shot(Object *o);
extern void ai_omega_shot(Object *o);
extern void aftermove_ballos_rotator(Object *o);
extern void ondeath_omega_body(Object *o);

/* endgame/misc */

extern void ai_cloud_spawner(Object *o);
extern void ai_cloud(Object *o);
extern void ai_balrog_flying(Object *o);
extern void aftermove_balrog_passenger(Object *o);
extern void ai_balrog_medic(Object *o);
extern void ai_gaudi_patient(Object *o);
extern void ai_baby_puppy(Object *o);
extern void ai_turning_human(Object *o);
extern void ai_ahchoo(Object *o);
extern void ai_misery_wind(Object *o);
extern void ai_the_cast(Object *o);

/* intro/intro */

extern void ai_intro_kings(Object *o);
extern void ai_intro_crown(Object *o);
extern void ai_intro_doctor(Object *o);

/* almond */
extern void ai_waterlevel(Object *o);
extern void ai_shutter(Object *o);
extern void ai_shutter_stuck(Object *o);
extern void ai_almond_robot(Object *o);

/* boss/balfrog */
extern void ondeath_balfrog(Object *o);

/* boss/ballos */
extern void ondeath_ballos(Object *o);
extern void ai_ballos_rotator(Object *o);

/* boss/core */
extern void ai_core_back(Object *o);
extern void ai_core_front(Object *o);
extern void ai_core_ghostie(Object *o);
extern void ai_core_blast(Object *o);
extern void ai_minicore(Object *o);
extern void ai_minicore_blast(Object *o);

/* boss/heavypress */
extern void ai_hp_lightning(Object *o);

/* boss/ironhead */
extern void ondeath_ironhead(Object *o);
extern void ai_ironh_fishy(Object *o);
extern void ai_ironh_shot(Object *o);
extern void ai_brick_spawner(Object *o);
extern void ai_ironh_brick(Object *o);
extern void ai_ikachan_spawner(Object *o);
extern void ai_ikachan(Object *o);
extern void ai_motion_wall(Object *o);

/* boss/x */
extern void ai_x_fishy_missile(Object *o);
extern void ai_x_defeated(Object *o);
extern void ondeath_x_target(Object *o);
extern void ondeath_x_mainobject(Object *o);

/* boss/undead_core */

extern void onspawn_ud_minicore_idle(Object *o);
extern void ai_udmini_platform(Object *o);
extern void ai_ud_pellet(Object *o);
extern void ai_ud_smoke(Object *o);
extern void ai_ud_spinner(Object *o);
extern void ai_ud_spinner_trail(Object *o);
extern void ai_ud_blast(Object *o);

/* egg */

extern void ai_basil(Object *o);
extern void ai_forcefield(Object *o);
extern void ai_egg_elevator(Object *o);

extern void ai_critter(Object *o);
extern void ai_beetle_freefly(Object *o);
extern void ai_giant_beetle(Object *o);
extern void ai_dragon_zombie(Object *o);
extern void ai_generic_angled_shot(Object *o);
extern void ai_falling_spike_small(Object *o);
extern void ai_falling_spike_large(Object *o);
extern void ai_counter_bomb(Object *o);
extern void ai_counter_bomb_number(Object *o);

extern void ai_npc_igor(Object *o);
extern void ai_boss_igor(Object *o);
extern void ai_generic_angled_shot(Object *o);
extern void ai_boss_igor_defeated(Object *o);

/* final_battle/doctor */
extern void ai_boss_doctor(Object *o);
extern void aftermove_red_crystal(Object *o);
extern void ai_doctor_shot(Object *o);
extern void ai_doctor_shot_trail(Object *o);
extern void ai_doctor_blast(Object *o);
extern void ai_doctor_crowned(Object *o);

/* final_battle/doctor_frenzied */

extern void ai_boss_doctor_frenzied(Object *o);
extern void ai_doctor_bat(Object *o);

/* final_battle/misery_finalbattle */
extern void ai_boss_misery(Object *o);
extern void ai_misery_phase(Object *o);
extern void ai_misery_ring(Object *o);
extern void aftermove_misery_ring(Object *o);
extern void ai_misery_ball(Object *o);
extern void ai_black_lightning(Object *o);

/* final_battle/final_misc */
extern void ai_mimiga_caged(Object *o);
extern void ai_doctor_ghost(Object *o);
extern void ai_red_energy(Object *o);

/* final_battle/sidekicks */
extern void ai_sue_frenzied(Object *o);
extern void ai_misery_frenzied(Object *o);
extern void ai_misery_critter(Object *o);
extern void ai_misery_bat(Object *o);
extern void ai_misery_missile(Object *o);

/* final_battle/balcony */
extern void ai_helicopter(Object *o);
extern void ai_helicopter_blade(Object *o);
extern void ai_igor_balcony(Object *o);
extern void ai_falling_block(Object *o);
extern void ai_falling_block_spawner(Object *o);

/* first_cave */
extern void ai_bat_up_down(Object *o);
extern void ai_critter(Object *o);
extern void ai_hermit_gunsmith(Object *o);
extern void ai_door_enemy(Object *o);

/* hell/ballos_misc */
extern void ai_ballos_skull(Object *o);
extern void ai_ballos_spikes(Object *o);
extern void ai_green_devil(Object *o);
extern void ai_green_devil_spawner(Object *o);
extern void ai_bute_sword_red(Object *o);
extern void ai_bute_archer_red(Object *o);
extern void ai_wall_collapser(Object *o);

/* hell/ballos_priest */

extern void ai_ballos_priest(Object *o);
extern void ai_ballos_target(Object *o);
extern void ai_ballos_bone_spawner(Object *o);
extern void ai_ballos_bone(Object *o);

/* last_cave */
extern void ai_critter_hopping_red(Object *o);
extern void ai_lava_drip_spawner(Object *o);
extern void ai_lava_drip(Object *o);
extern void ai_red_bat_spawner(Object *o);
extern void ai_red_bat(Object *o);
extern void ai_red_demon(Object *o);
extern void ai_droll_shot(Object *o);
extern void ai_proximity_press_vert(Object *o);

/* hell/hell */
extern void ai_bute_flying(Object *o);
extern void ai_bute_dying(Object *o);
	
extern void ai_bute_spawner(Object *o);
extern void ai_bute_falling(Object *o);
	
extern void ai_bute_sword(Object *o);
extern void ai_bute_archer(Object *o);
extern void ai_bute_arrow(Object *o);
	
extern void ai_mesa(Object *o);
extern void ai_mesa_block(Object *o);
extern void ai_bute_dying(Object *o);

extern void ai_deleet(Object *o);
extern void ai_rolling(Object *o);

extern void ai_statue(Object *o);
extern void ai_statue_base(Object *o);

extern void ai_puppy_ghost(Object *o);

/* maze/balrog_boss_missiles */
extern void ai_balrog_boss_missiles(Object *o);
extern void ondeath_balrog_boss_missiles(Object *o);
extern void ai_balrog_missile(Object *o);

extern void ai_critter_shooting_purple(Object *o);

/* maze/critter_purple */
extern void ai_critter_shooting_purple(Object *o);
extern void ai_generic_angled_shot(Object *o);

/* maze/gaudi */
extern void ai_gaudi(Object *o);
extern void ai_gaudi_armored(Object *o);
extern void ai_gaudi_armored_shot(Object *o);
extern void ai_gaudi_flying(Object *o);
extern void ai_gaudi_dying(Object *o);

/* maze/labyrinth_m */

extern void ai_firewhirr(Object *o);
extern void ai_firewhirr_shot(Object *o);
extern void ai_gaudi_egg(Object *o);
extern void ai_fuzz_core(Object *o);
extern void ai_fuzz(Object *o);
extern void aftermove_fuzz(Object *o);
extern void ai_buyobuyo_base(Object *o);
extern void ai_buyobuyo(Object *o);

/* maze/maze */

extern void ai_block_moveh(Object *o);
extern void ai_block_movev(Object *o);
extern void ai_boulder(Object *o);

/* maze/pooh_black */

extern void ai_pooh_black(Object *o);
extern void ai_pooh_black_bubble(Object *o);
extern void ai_pooh_black_dying(Object *o);

/* npc/balrog */
extern void onspawn_balrog(Object *o);
extern void ai_balrog(Object *o);
extern void ai_balrog_drop_in(Object *o);
extern void ai_balrog_bust_in(Object *o);

/* npc/curly */
extern void ai_curly(Object *o);
extern void aftermove_curly_carried(Object *o);
extern void ai_curly_carried_shooting(Object *o);
extern void ai_ccs_gun(Object *o);

/* npc/curly_ai */
extern void ai_curly_ai(Object *o);
extern void ai_cai_gun(Object *o);
extern void aftermove_cai_gun(Object *o);
extern void aftermove_cai_watershield(Object *o);

/* npc/balrog */
extern void onspawn_balrog(Object *o);
extern void ai_balrog(Object *o);
extern void ai_balrog_drop_in(Object *o);
extern void ai_balrog_bust_in(Object *o);

/* npc/misery */
extern void ai_misery_float(Object *o);
extern void ai_misery_stand(Object *o);
extern void ai_miserys_bubble(Object *o);

/* npc/npcguest */
extern void ai_npc_mahin(Object *o);
extern void onspawn_set_frame_from_id2(Object *o);
extern void ai_yamashita_pavilion(Object *o);
extern void ai_chthulu(Object *o);

extern void onspawn_generic_npc(Object *o);
extern void ai_generic_npc_nofaceplayer(Object *o);

/* npc/npcplayer */
extern void ai_npc_player(Object *o);
extern void ai_ptelin(Object *o);
extern void ai_ptelout(Object *o);

/* npc/npcregu */
extern void ai_npc_at_computer(Object *o);
extern void ai_jenka(Object *o);
extern void ai_blue_robot(Object *o);
extern void ai_doctor(Object *o);
extern void ai_toroko(Object *o);
extern void ai_toroko_teleport_in(Object *o);
extern void ai_npc_sue(Object *o);
extern void aftermove_npc_sue(Object *o);
extern void onspawn_npc_sue(Object *o);
extern void ai_sue_teleport_in(Object *o);
extern void ai_king(Object *o);
extern void aftermove_StickToLinkedActionPoint(Object *o);
extern void ai_kanpachi_fishing(Object *o);
extern void ai_professor_booster(Object *o);
extern void ai_booster_falling(Object *o);
extern void ai_generic_npc(Object *o);

/* oside */
extern void ai_sky_dragon(Object *o);
extern void ai_sandcroc(Object *o);
extern void ai_night_spirit(Object *o);
extern void ai_night_spirit_shot(Object *o);
extern void ai_hoppy(Object *o);
extern void ai_pixel_cat(Object *o);
extern void ai_little_family(Object *o);

/* plantation */
extern void ai_orangebell(Object *o);
extern void ai_orangebell_baby(Object *o);
extern void ai_stumpy(Object *o);
extern void ai_midorin(Object *o);
extern void ai_gunfish(Object *o);
extern void ai_gunfish_shot(Object *o);
extern void ai_droll(Object *o);
extern void ai_droll_shot(Object *o);
extern void ai_droll_guard(Object *o);
extern void ai_rocket(Object *o);
extern void ai_proximity_press_hoz(Object *o);
extern void ai_puppy_wag(Object *o);
extern void ai_numahachi(Object *o);
extern void ai_npc_itoh(Object *o);
extern void ai_kanpachi_standing(Object *o);
extern void ai_npc_momorin(Object *o);

/* sand/curly_boss */
extern void ai_curly_boss(Object *o);
extern void ai_curlyboss_shot(Object *o);

/* sand/puppy */
extern void ai_puppy_wag(Object *o);
extern void ai_puppy_bark(Object *o);
extern void ai_zzzz_spawner(Object *o);
extern void ai_puppy_run(Object *o);

extern void aftermove_puppy_carry(Object *o);

/* sand/sand */
extern void ai_beetle_horiz(Object *o);
extern void ai_polish(Object *o);

extern void ondeath_polish(Object *o);

extern void ai_polishbaby(Object *o);
extern void ai_sandcroc(Object *o);

extern void ai_curlys_mimigas(Object *o);
extern void ai_sunstone(Object *o);

extern void ai_armadillo(Object *o);
extern void ai_crow(Object *o);
extern void ai_crowwithskull(Object *o);
extern void ai_skullhead(Object *o);
extern void ai_skullhead_carried(Object *o);

extern void aftermove_skullhead_carried(Object *o);

extern void ai_skullstep(Object *o);
extern void ai_skullstep_foot(Object *o);
extern void ai_skeleton(Object *o);
extern void ai_skeleton_shot(Object *o);

/* sand/toroko_frenzied */
extern void ai_toroko_frenzied(Object *o);
extern void ai_toroko_block(Object *o);
extern void aftermove_toroko_block(Object *o);
extern void ai_toroko_flower(Object *o);

/* sym/smoke */
extern void ai_smokecloud(Object *o);

/* sym/sym */
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

/* village/balrog_boss_running */
extern void ai_balrog_boss_running(Object *o);
extern void ondeath_balrog_boss_running(Object *o);

/* village/ma_pignon */
extern void ai_ma_pignon(Object *o);
extern void ai_ma_pignon_rock(Object *o);
extern void ai_ma_pignon_clone(Object *o);

/* village/village */
extern void ai_toroko_shack(Object *o);
extern void ai_mushroom_enemy(Object *o);
extern void ai_gravekeeper(Object *o);
extern void ai_cage(Object *o);
extern void onspawn_snap_to_ground(Object *o);

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

extern void aftermove_blade_l12_shot(Object *o);
extern void aftermove_blade_slash(Object *o);
extern void ai_missile_shot(Object *o);
extern void ai_missile_boom_spawner(Object *o);

/* weed/balrog_boss_flying */
extern void ai_balrog_boss_flying(Object *o);
extern void ondeath_balrog_boss_flying(Object *o);
extern void ai_balrog_shot_bounce(Object *o);

/* weed/frenzied_mimiga */
extern void ai_frenzied_mimiga(Object *o);

/* weed/weed */
extern void ai_bat_hang(Object *o);
extern void ai_bat_circle(Object *o);
extern void ai_jelly(Object *o);
extern void ai_giant_jelly(Object *o);
extern void ai_mannan(Object *o);
extern void ai_mannan_shot(Object *o);
extern void ai_frog(Object *o);
extern void ai_hey_spawner(Object *o);
extern void ai_motorbike(Object *o);

extern void ai_animate3(Object *o);
extern void ai_animate1(Object *o);
extern void ai_malco(Object *o);
extern void ai_malco_broken(Object *o);

#endif
