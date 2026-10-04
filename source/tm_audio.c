/*
 * tm_audio.c -- Ultimate Spider-Man: Total Mayhem HD native Switch audio.
 *
 * The Android build delegates OGG playback to Java SoundPool/MediaPlayer.
 * There is no Android Java audio service in the wrapper, so the game's
 * nativePlaySound/native*Emitter bridge used to hit a NULL JNI method ID.
 *
 * This backend hooks that native bridge, streams the original OGG files with
 * stb_vorbis, resamples them to Switch audout's 48 kHz stereo output and
 * mixes music/SFX/VFX on android32's rt_audout pump.
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <switch.h>

#include "rt_audout.h"
#include "rt_settings.h"
#include "util.h"

#define STB_VORBIS_HEADER_ONLY
#define STB_VORBIS_MAX_CHANNELS 2
#include "stb_vorbis.c"

#include "tm_audio.h"

#define TM_SOUND_COUNT 493
#define TM_BGM_COUNT 24
#define TM_MAX_VOICES 32
#define TM_SRC_FRAMES 4096
#define TM_CMD_CAP 256

static const char *const g_sound_path[TM_SOUND_COUNT] = {
    "music/m_title.ogg",
    "music/m_lose.ogg",
    "music/m_win.ogg",
    "music/m_score_screen.ogg",
    "music/m_downtown_calm.ogg",
    "music/m_downtown_mixed.ogg",
    "music/m_powerplant_calm.ogg",
    "music/m_powerplant_mixed.ogg",
    "music/m_bridge_calm.ogg",
    "music/m_bridge_mixed.ogg",
    "music/m_subway_calm.ogg",
    "music/m_subway_mixed.ogg",
    "music/m_stadium_calm.ogg",
    "music/m_stadium_mixed.ogg",
    "music/m_skyscraper_calm.ogg",
    "music/m_skyscraper_mixed.ogg",
    "music/m_laboratory_calm.ogg",
    "music/m_laboratory_mixed.ogg",
    "music/m_boss_rhino.ogg",
    "music/m_boss_electro.ogg",
    "music/m_boss_venom.ogg",
    "music/m_boss_green_goblin.ogg",
    "music/m_boss_sandman.ogg",
    "music/m_boss_octopus.ogg",
    "sfx/INTERFACE/sfx_interface_confirm.ogg",
    "sfx/INTERFACE/sfx_interface_back.ogg",
    "sfx/INTERFACE/sfx_interface_browse.ogg",
    "sfx/INTERFACE/sfx_interface_continue.ogg",
    "sfx/INTERFACE/sfx_interface_continue.ogg",
    "sfx/INTERFACE/sfx_point_spend.ogg",
    "sfx/INTERFACE/sfx_point_spend_no_points.ogg",
    "sfx/INTERFACE/sfx_upgrade.ogg",
    "sfx/INTERFACE/sfx_score_whoosh.ogg",
    "sfx/INTERFACE/sfx_counter.ogg",
    "sfx/INTERFACE/sfx_gold_rank.ogg",
    "sfx/INTERFACE/sfx_silver_rank.ogg",
    "sfx/INTERFACE/sfx_copper_rank.ogg",
    "sfx/INTERFACE/sfx_stone_rank.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_land.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_drop_floor.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_die.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_hurt_1.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_hurt_2.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_hurt_3.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_jump_1.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_jump_2.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_jump_3.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_jump_4.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_jump_5.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_jump_6.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_kick_impact_1.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_kick_impact_2.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_kick_swoosh_1.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_kick_swoosh_2.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_pellet_impact_1.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_pellet_shoot_1.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_pellet_shoot_2.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_pellet_shoot_3.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_punch_impact_1.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_punch_impact_2.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_punch_swoosh_1.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_punch_swoosh_2.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_multikick_miss.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_multikick_hit_1.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_multikick_hit_2.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_multikick_hit_3.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_multikick_hit_4.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_web_throw_1.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_web_throw_2.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_web_throw_3.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_web_swing_start.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_web_swing_end.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_horizontal_web_slam_swing.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_horizontal_web_slam_impact.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_vertical_web_slam_swing.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_vertical_web_slam.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_wall_climb_start.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_sliding.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_aerial_suspend.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_double_spin_kick.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_enemy_pull.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_double_flip_throw.ogg",
    "sfx/Main_Character/RED_SUIT_MOVES/sfx_super_web_ready.ogg",
    "sfx/Main_Character/RED_SUIT_MOVES/sfx_super_web_attack.ogg",
    "sfx/Main_Character/RED_SUIT_MOVES/sfx_super_web_final_attack.ogg",
    "sfx/Main_Character/RED_SUIT_MOVES/sfx_up_punch_attack.ogg",
    "sfx/Main_Character/RED_SUIT_MOVES/sfx_air_diagonal_kick.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_air_circle.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_torpedo_attack.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_flip.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_knock_wall.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_critical_hit_1.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_critical_hit_2.ogg",
    "sfx/Main_Character/Basic_Moves/sfx_critical_hit_3.ogg",
    "sfx/Main_Character/BLACK_SUIT_MOVES/sfx_tentacle_whip.ogg",
    "sfx/Main_Character/BLACK_SUIT_MOVES/sfx_tentacle_shoot.ogg",
    "sfx/Main_Character/BLACK_SUIT_MOVES/sfx_tentacle_spike.ogg",
    "sfx/Main_Character/BLACK_SUIT_MOVES/sfx_black_suit_special_swirl.ogg",
    "sfx/Objects/Collectibles/sfx_orbs_collect.ogg",
    "sfx/Objects/Collectibles/sfx_special_collect.ogg",
    "sfx/Objects/Destructibles/sfx_car_impact.ogg",
    "sfx/Objects/Destructibles/sfx_crate_impact.ogg",
    "sfx/Objects/Destructibles/sfx_flower_pot_impact.ogg",
    "sfx/Objects/Destructibles/sfx_hydrant_impact.ogg",
    "sfx/Objects/Destructibles/sfx_hydrant_leaking.ogg",
    "sfx/Objects/Destructibles/sfx_postal_box_impact.ogg",
    "sfx/Objects/Destructibles/sfx_shopping_cart_impact.ogg",
    "sfx/Objects/Destructibles/sfx_street_lamp_impact.ogg",
    "sfx/Objects/Destructibles/sfx_traffic_cone_impact.ogg",
    "sfx/Objects/Destructibles/sfx_trash_can_impact.ogg",
    "sfx/Objects/Destructibles/sfx_air_conditioner_impact.ogg",
    "sfx/Objects/Destructibles/sfx_antenna_impact.ogg",
    "sfx/Objects/Destructibles/sfx_barber_sign_impact.ogg",
    "sfx/Objects/Destructibles/sfx_barrier_impact.ogg",
    "sfx/Objects/Destructibles/sfx_battery_cell_impact.ogg",
    "sfx/Objects/Destructibles/sfx_book_stand_impact.ogg",
    "sfx/Objects/Destructibles/sfx_fire_extinguisher_impact.ogg",
    "sfx/Objects/Destructibles/sfx_gumball_machine_impact.ogg",
    "sfx/Objects/Destructibles/sfx_hotdog_stand_impact.ogg",
    "sfx/Objects/Destructibles/sfx_office_chair_impact.ogg",
    "sfx/Objects/Destructibles/sfx_bigoffice_table_impact.ogg",
    "sfx/Objects/Destructibles/sfx_office_table_impact.ogg",
    "sfx/Objects/Destructibles/sfx_bank_desk_impact.ogg",
    "sfx/Objects/Destructibles/sfx_newspaper_impact.ogg",
    "sfx/Objects/Destructibles/sfx_steam_pipe_impact.ogg",
    "sfx/Objects/Destructibles/sfx_bank_chair_impact.ogg",
    "sfx/Objects/Destructibles/sfx_vent_fan_box_impact.ogg",
    "sfx/Objects/Destructibles/sfx_bus_stop_impact.ogg",
    "sfx/Objects/Destructibles/sfx_vending_machine_impact.ogg",
    "sfx/Objects/Destructibles/sfx_bench_impact.ogg",
    "sfx/Objects/MISC/sfx_electric_door_loop.ogg",
    "sfx/Objects/MISC/sfx_obstacle_falling.ogg",
    "sfx/Objects/MISC/sfx_fountain_loop.ogg",
    "sfx/Objects/MISC/sfx_car_fire_loop.ogg",
    "sfx/Objects/MISC/sfx_bridge_falling.ogg",
    "sfx/Objects/MISC/sfx_fence_up.ogg",
    "sfx/Objects/MISC/sfx_fence_down.ogg",
    "sfx/Objects/MISC/sfx_glass_break.ogg",
    "sfx/Objects/MISC/sfx_drone_fly.ogg",
    "sfx/Objects/MISC/sfx_block_web_close.ogg",
    "sfx/Objects/MISC/sfx_drone_fly.ogg",
    "sfx/Objects/MISC/sfx_drone_fly.ogg",
    "sfx/Objects/MISC/sfx_drone_alarm_1.ogg",
    "sfx/Objects/MISC/sfx_drone_alarm_2.ogg",
    "sfx/Objects/MISC/sfx_laser_beam.ogg",
    "sfx/Objects/MISC/sfx_switch_off.ogg",
    "sfx/Objects/TRAPS/sfx_fire_trap.ogg",
    "sfx/Objects/TRAPS/sfx_fire_trap_hurt.ogg",
    "sfx/Objects/TRAPS/sfx_electric_alarm.ogg",
    "sfx/Objects/TRAPS/sfx_electric_trap.ogg",
    "sfx/Objects/TRAPS/sfx_electric_trap_hurt.ogg",
    "sfx/Objects/TRAPS/sfx_symbiote_action.ogg",
    "sfx/Objects/TRAPS/sfx_symbiote_move.ogg",
    "sfx/Objects/MISC/sfx_block_symbiote_close.ogg",
    "sfx/Objects/TRAPS/sfx_symbiote_trap.ogg",
    "sfx/Objects/TRAPS/sfx_symbiote_trap_hurt.ogg",
    "sfx/Objects/TRAPS/sfx_bridge_trap.ogg",
    "sfx/Objects/MISC/sfx_laser_beam.ogg",
    "sfx/Objects/TRAPS/sfx_subway_train_trumpet_1.ogg",
    "sfx/Objects/TRAPS/sfx_subway_train_trumpet_2.ogg",
    "sfx/Objects/TRAPS/sfx_subway_train_run.ogg",
    "sfx/Objects/TRAPS/sfx_subway_train_run.ogg",
    "sfx/NPC/HOSTAGE/sfx_woman_hostage_help.ogg",
    "sfx/NPC/HOSTAGE/sfx_woman_hostage_thank.ogg",
    "sfx/NPC/HOSTAGE/sfx_man_hostage_help.ogg",
    "sfx/NPC/HOSTAGE/sfx_man_hostage_thank.ogg",
    "sfx/NPC/Thugs/sfx_thug_voice_1.ogg",
    "sfx/NPC/Thugs/sfx_thug_voice_2.ogg",
    "sfx/NPC/Thugs/sfx_thug_voice_3.ogg",
    "sfx/NPC/Thugs/sfx_thug_voice_4.ogg",
    "sfx/NPC/Thugs/sfx_thug_voice_5.ogg",
    "sfx/NPC/Thugs/sfx_thug_voice_6.ogg",
    "sfx/NPC/Thugs/sfx_thug_voice_7.ogg",
    "sfx/NPC/Thugs/sfx_thug_voice_8.ogg",
    "sfx/NPC/Thugs/sfx_thug_voice_9.ogg",
    "sfx/NPC/Thugs/sfx_thug_voice_10.ogg",
    "sfx/NPC/Thugs/sfx_thug_voice_11.ogg",
    "sfx/NPC/Thugs/sfx_thug_voice_12.ogg",
    "sfx/NPC/Thugs/sfx_thug_swoosh.ogg",
    "sfx/NPC/Thugs/sfx_thug_gun_shoot.ogg",
    "sfx/NPC/Thugs/sfx_thug_molotov_shoot.ogg",
    "sfx/NPC/Thugs/sfx_thug_molotov_ground.ogg",
    "sfx/NPC/Thugs/sfx_thug_molotov_impact.ogg",
    "sfx/NPC/Thugs/sfx_thug_rocket_shoot.ogg",
    "sfx/NPC/Thugs/sfx_thug_rocket_impact.ogg",
    "sfx/NPC/Thugs/sfx_thug_hurt_1.ogg",
    "sfx/NPC/Thugs/sfx_thug_hurt_2.ogg",
    "sfx/NPC/Thugs/sfx_thug_hurt_3.ogg",
    "sfx/NPC/Thugs/sfx_thug_dies.ogg",
    "sfx/NPC/Thugs/sfx_thug_bat_hurt_1.ogg",
    "sfx/NPC/Thugs/sfx_thug_bat_hurt_2.ogg",
    "sfx/NPC/Thugs/sfx_thug_bat_hurt_3.ogg",
    "sfx/NPC/Thugs/sfx_thug_bat_dies.ogg",
    "sfx/NPC/Thugs/sfx_thug_molotov_hurt_1.ogg",
    "sfx/NPC/Thugs/sfx_thug_molotov_hurt_2.ogg",
    "sfx/NPC/Thugs/sfx_thug_molotov_hurt_3.ogg",
    "sfx/NPC/Thugs/sfx_thug_molotov_dies.ogg",
    "sfx/NPC/Thugs/sfx_thug_rocket_hurt_1.ogg",
    "sfx/NPC/Thugs/sfx_thug_rocket_hurt_2.ogg",
    "sfx/NPC/Thugs/sfx_thug_rocket_hurt_3.ogg",
    "sfx/NPC/Thugs/sfx_thug_rocket_dies.ogg",
    "sfx/NPC/Thugs/sfx_thug_gun_hurt_1.ogg",
    "sfx/NPC/Thugs/sfx_thug_gun_hurt_2.ogg",
    "sfx/NPC/Thugs/sfx_thug_gun_hurt_3.ogg",
    "sfx/NPC/Thugs/sfx_thug_gun_dies.ogg",
    "sfx/NPC/SLEDGER/sfx_sledger_swing.ogg",
    "sfx/NPC/SLEDGER/sfx_sledger_attack_1.ogg",
    "sfx/NPC/SLEDGER/sfx_sledger_attack_2.ogg",
    "sfx/NPC/SLEDGER/sfx_sledger_hurt_1.ogg",
    "sfx/NPC/SLEDGER/sfx_sledger_hurt_2.ogg",
    "sfx/NPC/SLEDGER/sfx_sledger_dies.ogg",
    "sfx/NPC/SYMBIOTE_ZOMBIES/sfx_zombie_voice_1.ogg",
    "sfx/NPC/SYMBIOTE_ZOMBIES/sfx_zombie_voice_2.ogg",
    "sfx/NPC/SYMBIOTE_ZOMBIES/sfx_zombie_voice_3.ogg",
    "sfx/NPC/SYMBIOTE_ZOMBIES/sfx_zombie_voice_4.ogg",
    "sfx/NPC/SYMBIOTE_ZOMBIES/sfx_zombie_voice_5.ogg",
    "sfx/NPC/SYMBIOTE_ZOMBIES/sfx_zombie_symbiote.ogg",
    "sfx/NPC/SYMBIOTE_ZOMBIES/sfx_zombie_bound.ogg",
    "sfx/NPC/SYMBIOTE_ZOMBIES/sfx_zombie_bound_fall.ogg",
    "sfx/NPC/SYMBIOTE_ZOMBIES/sfx_zombie_hurt_1.ogg",
    "sfx/NPC/SYMBIOTE_ZOMBIES/sfx_zombie_hurt_2.ogg",
    "sfx/NPC/SYMBIOTE_ZOMBIES/sfx_zombie_dies.ogg",
    "sfx/NPC/SYMBIOTE_CHARGERS/sfx_charger_voice_1.ogg",
    "sfx/NPC/SYMBIOTE_CHARGERS/sfx_charger_voice_2.ogg",
    "sfx/NPC/SYMBIOTE_CHARGERS/sfx_charger_voice_3.ogg",
    "sfx/NPC/SYMBIOTE_CHARGERS/sfx_charger_voice_4.ogg",
    "sfx/NPC/SYMBIOTE_CHARGERS/sfx_charger_voice_5.ogg",
    "sfx/NPC/SYMBIOTE_CHARGERS/sfx_charger_scream.ogg",
    "sfx/NPC/SYMBIOTE_CHARGERS/sfx_charger_smash.ogg",
    "sfx/NPC/SYMBIOTE_CHARGERS/sfx_chargers_whip.ogg",
    "sfx/NPC/SYMBIOTE_CHARGERS/sfx_charger_bound.ogg",
    "sfx/NPC/SYMBIOTE_CHARGERS/sfx_charger_bound_fall.ogg",
    "sfx/NPC/SYMBIOTE_CHARGERS/sfx_charger_hurt_1.ogg",
    "sfx/NPC/SYMBIOTE_CHARGERS/sfx_charger_hurt_2.ogg",
    "sfx/NPC/SYMBIOTE_CHARGERS/sfx_charger_dies.ogg",
    "sfx/NPC/GOBLIN/sfx_goblin_hurt_1.ogg",
    "sfx/NPC/GOBLIN/sfx_goblin_hurt_2.ogg",
    "sfx/NPC/GOBLIN/sfx_goblin_hurt_3.ogg",
    "sfx/NPC/GOBLIN/sfx_goblin_dies.ogg",
    "sfx/NPC/SECURITY_BOTS/sfx_bot_plasma_shoot.ogg",
    "sfx/NPC/SECURITY_BOTS/sfx_plasma_loop.ogg",
    "sfx/NPC/SECURITY_BOTS/sfx_bot_plasma_impact.ogg",
    "sfx/NPC/SECURITY_BOTS/sfx_bot_shield.ogg",
    "sfx/NPC/SECURITY_BOTS/sfx_bot_shield_break.ogg",
    "sfx/NPC/PHANTOM/sfx_phantom_double_slash_1.ogg",
    "sfx/NPC/PHANTOM/sfx_phantom_double_slash_2.ogg",
    "sfx/NPC/PHANTOM/sfx_phantom_slash.ogg",
    "sfx/NPC/PHANTOM/sfx_phantom_throw.ogg",
    "sfx/NPC/PHANTOM/sfx_phantom_throw_start.ogg",
    "sfx/NPC/PHANTOM/sfx_phantom_throw_loop.ogg",
    "sfx/NPC/PHANTOM/sfx_phantom_throw_end.ogg",
    "sfx/NPC/PHANTOM/sfx_phantom_hurt_1.ogg",
    "sfx/NPC/PHANTOM/sfx_phantom_hurt_2.ogg",
    "sfx/NPC/PHANTOM/sfx_phantom_hurt_3.ogg",
    "sfx/NPC/PHANTOM/sfx_phantom_die.ogg",
    "sfx/NPC/RHINO/sfx_rhino_strike_1.ogg",
    "sfx/NPC/RHINO/sfx_rhino_strike_2.ogg",
    "sfx/NPC/RHINO/sfx_rhino_tramp.ogg",
    "sfx/NPC/RHINO/sfx_rhino_bump.ogg",
    "sfx/NPC/RHINO/sfx_rhino_rotate_attack.ogg",
    "sfx/NPC/RHINO/sfx_rhino_car_throw.ogg",
    "sfx/NPC/RHINO/sfx_rhino_car_fall.ogg",
    "sfx/NPC/RHINO/sfx_rhino_grab.ogg",
    "sfx/NPC/RHINO/sfx_rhino_hurt_1.ogg",
    "sfx/NPC/RHINO/sfx_rhino_hurt_2.ogg",
    "sfx/NPC/RHINO/sfx_rhino_defense.ogg",
    "sfx/NPC/RHINO/sfx_rhino_dies.ogg",
    "sfx/NPC/RHINO/sfx_rhino_laugh.ogg",
    "sfx/NPC/ELECTRO/sfx_electro_whirlwind.ogg",
    "sfx/NPC/ELECTRO/sfx_electro_nova_charge.ogg",
    "sfx/NPC/ELECTRO/sfx_electro_nova_release.ogg",
    "sfx/NPC/ELECTRO/sfx_electro_lightning.ogg",
    "sfx/NPC/ELECTRO/sfx_electro_diving.ogg",
    "sfx/Objects/TRAPS/sfx_electric_trap.ogg",
    "sfx/NPC/ELECTRO/sfx_electro_charge.ogg",
    "sfx/NPC/ELECTRO/sfx_electro_hurt_1.ogg",
    "sfx/NPC/ELECTRO/sfx_electro_hurt_2.ogg",
    "sfx/NPC/ELECTRO/sfx_electro_hurt_3.ogg",
    "sfx/NPC/ELECTRO/sfx_electro_dies.ogg",
    "sfx/NPC/SANDMAN/sfx_sandman_Punch.ogg",
    "sfx/NPC/SANDMAN/sfx_sandman_strike.ogg",
    "sfx/NPC/SANDMAN/sfx_sandman_big_hands.ogg",
    "sfx/NPC/SANDMAN/sfx_sandman_get_in_ground.ogg",
    "sfx/NPC/SANDMAN/sfx_sandman_move_underground.ogg",
    "sfx/NPC/SANDMAN/sfx_sandman_get_out_ground.ogg",
    "sfx/NPC/SANDMAN/sfx_sandman_hurt_1.ogg",
    "sfx/NPC/SANDMAN/sfx_sandman_hurt_2.ogg",
    "sfx/NPC/SANDMAN/sfx_sandman_hurt_3.ogg",
    "sfx/NPC/SANDMAN/sfx_sandman_dies.ogg",
    "sfx/NPC/VENOM/sfx_venom_attack_1.ogg",
    "sfx/NPC/VENOM/sfx_venom_attack_2.ogg",
    "sfx/NPC/VENOM/sfx_venom_attack_3.ogg",
    "sfx/NPC/VENOM/sfx_venom_claw.ogg",
    "sfx/NPC/VENOM/sfx_venom_fall_1.ogg",
    "sfx/NPC/VENOM/sfx_venom_fall_2.ogg",
    "sfx/NPC/VENOM/sfx_venom_fall_3.ogg",
    "sfx/NPC/VENOM/sfx_venom_scream.ogg",
    "sfx/NPC/VENOM/sfx_venom_throw_land.ogg",
    "sfx/NPC/VENOM/sfx_venom_throw_symbiote.ogg",
    "sfx/NPC/VENOM/sfx_venom_whip.ogg",
    "sfx/NPC/VENOM/sfx_venom_pat_train.ogg",
    "sfx/NPC/VENOM/sfx_venom_hurt_1.ogg",
    "sfx/NPC/VENOM/sfx_venom_hurt_2.ogg",
    "sfx/NPC/VENOM/sfx_venom_hurt_3.ogg",
    "sfx/NPC/VENOM/sfx_venom_dies.ogg",
    "sfx/NPC/GREEN_GOBLIN/sfx_green_goblin_dash.ogg",
    "sfx/NPC/GREEN_GOBLIN/sfx_green_goblin_fireball_land.ogg",
    "sfx/NPC/GREEN_GOBLIN/sfx_green_goblin_jump.ogg",
    "sfx/NPC/GREEN_GOBLIN/sfx_green_goblin_land.ogg",
    "sfx/NPC/GREEN_GOBLIN/sfx_green_goblin_land.ogg",
    "sfx/NPC/GREEN_GOBLIN/sfx_green_goblin_roar.ogg",
    "sfx/NPC/GREEN_GOBLIN/sfx_green_goblin_throw.ogg",
    "sfx/NPC/GREEN_GOBLIN/sfx_green_goblin_roar_in_air.ogg",
    "sfx/NPC/GREEN_GOBLIN/sfx_green_goblin_spout_fireball.ogg",
    "sfx/Objects/TRAPS/sfx_fire_trap.ogg",
    "sfx/NPC/GREEN_GOBLIN/sfx_green_goblin_hurt_1.ogg",
    "sfx/NPC/GREEN_GOBLIN/sfx_green_goblin_hurt_2.ogg",
    "sfx/NPC/GREEN_GOBLIN/sfx_green_goblin_hurt_3.ogg",
    "sfx/NPC/GREEN_GOBLIN/sfx_green_goblin_die.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_tentacle_attack.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_electric_attack.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_tentacle_out.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_plasma_thin_start.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_plasma_thin_loop.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_plasma_thick.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_jump.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_land.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_walk_01.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_walk_02.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_walk_03.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_walk_04.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_weak.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_tentacle_hurt_01.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_tentacle_hurt_02.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_tentacle_hurt_03.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_hurt_1.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_hurt_2.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_hurt_3.ogg",
    "sfx/NPC/OCTOPUS/sfx_octopus_dies.ogg",
    "sfx/CUTSCENES/sfx_door_open.ogg",
    "sfx/CUTSCENES/sfx_battery_cell_fall.ogg",
    "sfx/CUTSCENES/sfx_battery_cell_explosion.ogg",
    "sfx/CUTSCENES/sfx_electro_sparkle_1.ogg",
    "sfx/CUTSCENES/sfx_electro_sparkle_2.ogg",
    "sfx/CUTSCENES/sfx_electro_sparkle_3.ogg",
    "sfx/CUTSCENES/sfx_rhino_bust.ogg",
    "sfx/CUTSCENES/sfx_rhino_fs_1.ogg",
    "sfx/CUTSCENES/sfx_rhino_fs_2.ogg",
    "sfx/CUTSCENES/sfx_police_arriving.ogg",
    "sfx/CUTSCENES/sfx_truck_arrives.ogg",
    "sfx/CUTSCENES/sfx_truck_ends.ogg",
    "sfx/CUTSCENES/sfx_truck_hit.ogg",
    "sfx/CUTSCENES/sfx_subway_leaving.ogg",
    "sfx/CUTSCENES/sfx_venom_end_cutscene.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv1_earthquake.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv1_sandman_land.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv1_sandman_out.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv2_police_car.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv3_spidey_arrives.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv3_spidey_lands.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv7_spidey_arrives.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv7_zombie_scream.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv7_zombie_swing_1.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv7_zombie_swing_2.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv7_security_bot_action.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv8_stone_impact.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv8_spidey_arm_swing_1.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv8_spidey_arm_swing_2.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv8_spidey_arm_swing_3.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv8_spidey_arm_swing_4.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv10_beat_airplane_1.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv10_beat_airplane_2.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv10_beat_airplane_3.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv10_goblin_scream_1.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv10_goblin_scream_2.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv10_goblin_scream_3.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv11_human_cheers.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv12_green_goblin_gasp.ogg",
    "sfx/CUTSCENES/sfx_venom_kicked_face.ogg",
    "sfx/CUTSCENES/sfx_venom_hits_ground.ogg",
    "sfx/CUTSCENES/sfx_venom_jump_sea.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv8_venom_tentacles.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv8_venom_tentacles_exits.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv8_venom_tentacles_returns.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv11_camera_woosh.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv11_earthquake.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv11_goblin_down.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv11_goblin_jump.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv11_jump_out_rail.ogg",
    "sfx/CUTSCENES/sfx_cutscene_lv11_spidey_swing.ogg",
    "sfx/Production_Elements/sfx_spider_sense_in.ogg",
    "sfx/Production_Elements/sfx_spider_sense_out.ogg",
    "sfx/Production_Elements/sfx_qte_success.ogg",
    "sfx/Production_Elements/sfx_qte_fail.ogg",
    "sfx/Production_Elements/sfx_qte_click.ogg",
    "sfx/Production_Elements/sfx_qte_untie.ogg",
    "sfx/Production_Elements/sfx_spider_logo_in.ogg",
    "sfx/Production_Elements/sfx_spider_logo_out.ogg",
    "sfx/Production_Elements/sfx_camera_shot.ogg",
    "VFX/vfx_prologue_spidy_01.ogg",
    "VFX/vfx_prologue_spidy_02.ogg",
    "VFX/vfx_prologue_girl_01.ogg",
    "VFX/vfx_prologue_spidy_03.ogg",
    "VFX/vfx_prologue_cop_01.ogg",
    "VFX/vfx_lv1_01_spidy_01.ogg",
    "VFX/vfx_lv1_01_spidy_02.ogg",
    "VFX/vfx_lv1_01_spidy_03.ogg",
    "VFX/vfx_lv1_01_sandman_01.ogg",
    "VFX/vfx_lv1_01_spidy_04.ogg",
    "VFX/vfx_lv1_01_sandman_02.ogg",
    "VFX/vfx_lv1_02_spidy_01.ogg",
    "VFX/vfx_lv1_02_spidy_02.ogg",
    "VFX/vfx_lv1_02_spidy_03.ogg",
    "VFX/vfx_lv2_01_rhino_01.ogg",
    "VFX/vfx_lv2_01_spidy_01.ogg",
    "VFX/vfx_lv2_01_rhino_02.ogg",
    "VFX/vfx_lv2_01_rhino_03.ogg",
    "VFX/vfx_lv2_02_spidy_01.ogg",
    "VFX/vfx_lv2_02_COP1_01.ogg",
    "VFX/vfx_lv2_02_COP2_01.ogg",
    "VFX/vfx_lv2_02_COP1_02.ogg",
    "VFX/vfx_lv2_02_spidy_02.ogg",
    "VFX/vfx_lv2_03_spidy_01.ogg",
    "VFX/vfx_lv3_01_spidy_01.ogg",
    "VFX/vfx_lv3_01_electro_01.ogg",
    "VFX/vfx_lv3_01_spidy_02.ogg",
    "VFX/vfx_lv3_01_spidy_03.ogg",
    "VFX/vfx_lv4_01_electro_01.ogg",
    "VFX/vfx_lv4_01_spidy_01.ogg",
    "VFX/vfx_lv4_02_COP1_01.ogg",
    "VFX/vfx_lv4_02_spidy_01.ogg",
    "VFX/vfx_lv4_02_COP1_02.ogg",
    "VFX/vfx_lv4_02_spidy_02.ogg",
    "VFX/vfx_lv5_01_spidy_01.ogg",
    "VFX/vfx_lv5_01_spidy_02.ogg",
    "VFX/vfx_lv5_01_spidy_03.ogg",
    "VFX/vfx_lv6_01_octo_01.ogg",
    "VFX/vfx_lv6_01_spidy_01.ogg",
    "VFX/vfx_lv6_01_octo_02.ogg",
    "VFX/vfx_lv6_01_spidy_02.ogg",
    "VFX/vfx_lv6_01_octo_03.ogg",
    "VFX/vfx_lv6_02_spidy_01.ogg",
    "VFX/vfx_lv7_01_spidy_01.ogg",
    "VFX/vfx_lv7_01_spidy_02.ogg",
    "VFX/vfx_lv7_02_leader_01.ogg",
    "VFX/vfx_lv7_02_spidy_01.ogg",
    "VFX/vfx_lv7_02_leader_02.ogg",
    "VFX/vfx_lv7_02_spidy_02.ogg",
    "VFX/vfx_lv7_02_leader_03.ogg",
    "VFX/vfx_lv7_02_spidy_03.ogg",
    "VFX/vfx_lv7_03_spidy_01.ogg",
    "VFX/vfx_lv7_03_robot_01.ogg",
    "VFX/vfx_lv7_03_spidy_02.ogg",
    "VFX/vfx_lv7_03_spidy_03.ogg",
    "VFX/vfx_lv8_01_venom_01.ogg",
    "VFX/vfx_lv8_01_spidy_04.ogg",
    "VFX/vfx_lv8_01_venom_02.ogg",
    "VFX/vfx_lv8_01_spidy_05.ogg",
    "VFX/vfx_lv8_01_venom_03.ogg",
    "VFX/vfx_lv8_01_spidy_06.ogg",
    "VFX/vfx_lv8_01_venom_04.ogg",
    "VFX/vfx_lv8_02_spidy_07.ogg",
    "VFX/vfx_lv8_01_spidy_01.ogg",
    "VFX/vfx_lv8_01_spidy_02.ogg",
    "VFX/vfx_lv8_01_researcher_01.ogg",
    "VFX/vfx_lv8_01_spidy_03.ogg",
    "VFX/vfx_lv9_01_spidy_01.ogg",
    "VFX/vfx_lv9_01_venom_01.ogg",
    "VFX/vfx_lv9_02_spidy_01.ogg",
    "VFX/vfx_lv9_02_leader_01.ogg",
    "VFX/vfx_lv9_02_spidy_02.ogg",
    "VFX/vfx_lv9_02_leader_02.ogg",
    "VFX/vfx_lv9_02_spidy_03.ogg",
    "VFX/vfx_lv10_02_spidy_02.ogg",
    "VFX/vfx_lv11_01_spidy_01.ogg",
    "VFX/vfx_lv11_02_goblin_03.ogg",
    "VFX/vfx_lv11_02_goblin_01.ogg",
    "VFX/vfx_lv11_01_spidy_02.ogg",
    "VFX/vfx_lv11_02_goblin_02.ogg",
    "VFX/vfx_lv12_01_goblin_01.ogg",
    "VFX/vfx_lv12_01_spidy_01.ogg",
    "VFX/vfx_lv12_02_goblin_01.ogg",
    "VFX/vfx_lv12_02_spidy_01.ogg",
    "VFX/vfx_lv12_02_leader_01.ogg",
    "VFX/vfx_lv12_02_spidy_02.ogg",
    "VFX/vfx_lv12_02_leader_02.ogg",
    "VFX/vfx_lv12_02_spidy_03.ogg",
    "VFX/vfx_lv12_02_leader_03.ogg",
    "VFX/vfx_lv12_02_spidy_04.ogg",
    "VFX/vfx_end_girl_01.ogg",
    "VFX/vfx_end_spidy_01.ogg",
    "VFX/vfx_end_girl_02.ogg",
    "VFX/vfx_end_spidy_02.ogg"
};

typedef struct {
    stb_vorbis *dec;
    int alive;
    int id;
    int group;
    int loop;
    int paused;
    float volume;
    unsigned rate;
    double pos;
    int src_frames;
    uint64_t age;
    float src[TM_SRC_FRAMES * 2];
} TMVoice;

enum {
    TM_CMD_PLAY = 1,
    TM_CMD_STOP_ID,
    TM_CMD_PAUSE_ID,
    TM_CMD_RESUME_ID,
    TM_CMD_STOP_ALL,
    TM_CMD_PAUSE_ALL,
    TM_CMD_RESUME_ALL,
    TM_CMD_STOP_GROUP,
    TM_CMD_PAUSE_GROUP,
    TM_CMD_RESUME_GROUP,
    TM_CMD_SET_ID_VOLUME
};

typedef struct {
    int op;
    int id;
    int arg;
    float value;
} TMCmd;

static TMVoice g_voice[TM_MAX_VOICES];
static volatile int g_alive_by_id[TM_SOUND_COUNT];
static volatile float g_last_volume[TM_SOUND_COUNT];
static volatile float g_group_volume[2] = {1.0f, 1.0f};

static Mutex g_cmd_mutex;
static int g_mutex_ready;
static TMCmd g_cmd[TM_CMD_CAP];
static unsigned g_cmd_r, g_cmd_w, g_cmd_n;

static int g_audio_started;
static Thread g_audio_thread;
static volatile int g_audio_stop;
static int g_audio_thread_up;
static uint64_t g_age;
static unsigned g_open_fail_logs;
static unsigned g_play_logs;
static unsigned g_queue_drop_logs;

static int tm_valid_id(int id) {
    return id >= 0 && id < TM_SOUND_COUNT;
}

static int tm_group_for_id(int id) {
    return id < TM_BGM_COUNT ? 1 : 2;
}

static int tm_group_index(int group_id) {
    return group_id == 1 ? 0 : 1;
}

static float tm_clampf(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static void tm_queue(int op, int id, int arg, float value) {
    if (!g_mutex_ready)
        return;

    mutexLock(&g_cmd_mutex);
    if (g_cmd_n < TM_CMD_CAP) {
        g_cmd[g_cmd_w].op = op;
        g_cmd[g_cmd_w].id = id;
        g_cmd[g_cmd_w].arg = arg;
        g_cmd[g_cmd_w].value = value;
        g_cmd_w = (g_cmd_w + 1u) % TM_CMD_CAP;
        g_cmd_n++;
    } else if (g_queue_drop_logs++ < 8) {
        debugPrintf("[stage1h-audio] command queue full; dropping op=%d id=%d\n",
                    op, id);
    }
    mutexUnlock(&g_cmd_mutex);
}

static int tm_pop_cmd(TMCmd *out) {
    int have = 0;
    mutexLock(&g_cmd_mutex);
    if (g_cmd_n) {
        *out = g_cmd[g_cmd_r];
        g_cmd_r = (g_cmd_r + 1u) % TM_CMD_CAP;
        g_cmd_n--;
        have = 1;
    }
    mutexUnlock(&g_cmd_mutex);
    return have;
}

static void tm_voice_close(TMVoice *v) {
    if (!v->alive)
        return;

    if (tm_valid_id(v->id) && g_alive_by_id[v->id] > 0)
        g_alive_by_id[v->id]--;

    if (v->dec)
        stb_vorbis_close(v->dec);

    memset(v, 0, sizeof(*v));
}

static int tm_voice_refill(TMVoice *v) {
    float keep_l = 0.0f, keep_r = 0.0f;
    int keep = 0;

    if (v->src_frames > 0) {
        int i = (int)v->pos;
        if (i >= 0 && i < v->src_frames) {
            keep_l = v->src[i * 2 + 0];
            keep_r = v->src[i * 2 + 1];
            v->pos -= (double)i;
            keep = 1;
        } else {
            v->pos = 0.0;
        }
    }

    if (keep) {
        v->src[0] = keep_l;
        v->src[1] = keep_r;
    }

    int got = stb_vorbis_get_samples_float_interleaved(
        v->dec, 2, v->src + keep * 2, (TM_SRC_FRAMES - keep) * 2);

    if (got == 0 && v->loop) {
        if (stb_vorbis_seek_start(v->dec)) {
            got = stb_vorbis_get_samples_float_interleaved(
                v->dec, 2, v->src + keep * 2,
                (TM_SRC_FRAMES - keep) * 2);
        }
    }

    v->src_frames = keep + got;
    return v->src_frames >= 2;
}

static int tm_try_open_path(int id, char *path, size_t cap, int uppercase_root,
                            stb_vorbis **out, int *err) {
    const char *rel = g_sound_path[id];

    if (uppercase_root) {
        if (!strncmp(rel, "music/", 6)) {
            snprintf(path, cap,
                     "sdmc:%s/gameloft/games/GloftSMHP/sound/MUSIC/%s",
                     PORT_ROOT_PATH, rel + 6);
        } else if (!strncmp(rel, "sfx/", 4)) {
            snprintf(path, cap,
                     "sdmc:%s/gameloft/games/GloftSMHP/sound/SFX/%s",
                     PORT_ROOT_PATH, rel + 4);
        } else {
            snprintf(path, cap,
                     "sdmc:%s/gameloft/games/GloftSMHP/sound/%s",
                     PORT_ROOT_PATH, rel);
        }
    } else {
        snprintf(path, cap,
                 "sdmc:%s/gameloft/games/GloftSMHP/sound/%s",
                 PORT_ROOT_PATH, rel);
    }

    *out = stb_vorbis_open_filename(path, err, NULL);
    return *out != NULL;
}

static void tm_start_voice(int id, float volume, int loop) {
    if (!tm_valid_id(id))
        return;

    int slot = -1;
    for (int i = 0; i < TM_MAX_VOICES; i++) {
        if (!g_voice[i].alive) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        uint64_t best_age = UINT64_MAX;
        for (int i = 0; i < TM_MAX_VOICES; i++) {
            if (g_voice[i].group == 2 && g_voice[i].age < best_age) {
                best_age = g_voice[i].age;
                slot = i;
            }
        }
        if (slot < 0)
            slot = 0;
        tm_voice_close(&g_voice[slot]);
    }

    TMVoice *v = &g_voice[slot];
    memset(v, 0, sizeof(*v));

    char path[768];
    int err = 0;
    if (!tm_try_open_path(id, path, sizeof(path), 0, &v->dec, &err) &&
        !tm_try_open_path(id, path, sizeof(path), 1, &v->dec, &err)) {
        if (g_open_fail_logs++ < 24) {
            debugPrintf("[stage1h-audio] OGG open failed id=%d err=%d: %s\n",
                        id, err, path);
            log_flush_ring();
        }
        return;
    }

    stb_vorbis_info info = stb_vorbis_get_info(v->dec);
    v->id = id;
    v->group = tm_group_for_id(id);
    v->loop = loop != 0;
    v->paused = 0;
    v->volume = tm_clampf(volume, 0.0f, 3.0f);
    v->rate = info.sample_rate ? info.sample_rate : 44100u;
    v->age = ++g_age;
    v->alive = 1;
    g_alive_by_id[id]++;
    g_last_volume[id] = v->volume;

    if (!tm_voice_refill(v)) {
        tm_voice_close(v);
        return;
    }

    if (g_play_logs++ < 20) {
        debugPrintf("[stage1h-audio] PLAY id=%d group=%d loop=%d rate=%u path=%s\n",
                    id, v->group, v->loop, v->rate, g_sound_path[id]);
        log_flush_ring();
    }
}

static void tm_for_id(int id, int op, float value) {
    for (int i = 0; i < TM_MAX_VOICES; i++) {
        TMVoice *v = &g_voice[i];
        if (!v->alive || v->id != id)
            continue;

        if (op == TM_CMD_STOP_ID)
            tm_voice_close(v);
        else if (op == TM_CMD_PAUSE_ID)
            v->paused = 1;
        else if (op == TM_CMD_RESUME_ID)
            v->paused = 0;
        else if (op == TM_CMD_SET_ID_VOLUME)
            v->volume = tm_clampf(value, 0.0f, 3.0f);
    }
}

static void tm_for_group(int group_id, int op) {
    int g = group_id == 1 ? 1 : 2;
    for (int i = 0; i < TM_MAX_VOICES; i++) {
        TMVoice *v = &g_voice[i];
        if (!v->alive || v->group != g)
            continue;

        if (op == TM_CMD_STOP_GROUP)
            tm_voice_close(v);
        else if (op == TM_CMD_PAUSE_GROUP)
            v->paused = 1;
        else if (op == TM_CMD_RESUME_GROUP)
            v->paused = 0;
    }
}

static void tm_process_commands(void) {
    TMCmd c;
    while (tm_pop_cmd(&c)) {
        switch (c.op) {
        case TM_CMD_PLAY:
            tm_start_voice(c.id, c.value, c.arg);
            break;
        case TM_CMD_STOP_ID:
        case TM_CMD_PAUSE_ID:
        case TM_CMD_RESUME_ID:
        case TM_CMD_SET_ID_VOLUME:
            tm_for_id(c.id, c.op, c.value);
            break;
        case TM_CMD_STOP_ALL:
        case TM_CMD_PAUSE_ALL:
        case TM_CMD_RESUME_ALL:
            for (int i = 0; i < TM_MAX_VOICES; i++) {
                TMVoice *v = &g_voice[i];
                if (!v->alive)
                    continue;
                if (c.op == TM_CMD_STOP_ALL)
                    tm_voice_close(v);
                else if (c.op == TM_CMD_PAUSE_ALL)
                    v->paused = 1;
                else
                    v->paused = 0;
            }
            break;
        case TM_CMD_STOP_GROUP:
        case TM_CMD_PAUSE_GROUP:
        case TM_CMD_RESUME_GROUP:
            tm_for_group(c.id, c.op);
            break;
        default:
            break;
        }
    }
}

static void tm_audio_fill(int16_t *out, int frames, void *ud) {
    (void)ud;

    static float mix[RT_AUDOUT_FRAMES * 2];
    if (frames > RT_AUDOUT_FRAMES)
        frames = RT_AUDOUT_FRAMES;

    memset(mix, 0, (size_t)frames * 2u * sizeof(float));

    tm_process_commands();

    const double out_rate = (double)rt_audout_rate();

    for (int vi = 0; vi < TM_MAX_VOICES; vi++) {
        TMVoice *v = &g_voice[vi];
        if (!v->alive || v->paused || !v->dec)
            continue;

        float gain = v->volume *
                     g_group_volume[tm_group_index(v->group)] *
                     0.70f;
        double step = (double)v->rate / out_rate;

        for (int n = 0; n < frames; n++) {
            while (v->alive && v->pos + 1.0 >= (double)v->src_frames) {
                if (!tm_voice_refill(v)) {
                    tm_voice_close(v);
                    break;
                }
            }
            if (!v->alive)
                break;

            int i = (int)v->pos;
            double frac = v->pos - (double)i;

            float l0 = v->src[i * 2 + 0];
            float r0 = v->src[i * 2 + 1];
            float l1 = v->src[(i + 1) * 2 + 0];
            float r1 = v->src[(i + 1) * 2 + 1];

            float l = l0 + (l1 - l0) * (float)frac;
            float r = r0 + (r1 - r0) * (float)frac;

            mix[n * 2 + 0] += l * gain;
            mix[n * 2 + 1] += r * gain;
            v->pos += step;
        }
    }

    for (int i = 0; i < frames * 2; i++) {
        float x = tm_clampf(mix[i], -1.0f, 1.0f);
        out[i] = (int16_t)(x * 32767.0f);
    }
}

static void tm_audio_thread_main(void *arg) {
    (void)arg;
    static int16_t out[RT_AUDOUT_FRAMES * 2];

    while (!g_audio_stop) {
        tm_audio_fill(out, RT_AUDOUT_FRAMES, NULL);
        if (rt_audout_submit(out) != 0 && !g_audio_stop)
            svcSleepThread(1000000ll);
    }
}

int tm_audio_init(void) {
    if (g_audio_started)
        return 0;

    mutexInit(&g_cmd_mutex);
    g_mutex_ready = 1;

    for (int i = 0; i < TM_SOUND_COUNT; i++)
        g_last_volume[i] = 1.0f;

    if (rt_audout_open() != 0) {
        debugPrintf("[stage1h-audio] audout open failed\n");
        log_flush_ring();
        return -1;
    }

    g_audio_stop = 0;
    Result trc = threadCreate(&g_audio_thread, tm_audio_thread_main, NULL, NULL,
                              0x40000, 0x2A, 2);
    if (R_SUCCEEDED(trc))
        trc = threadStart(&g_audio_thread);

    if (R_FAILED(trc)) {
        debugPrintf("[stage1h-audio] mixer thread failed: 0x%x\n",
                    (unsigned)trc);
        log_flush_ring();
        return -1;
    }

    g_audio_thread_up = 1;
    g_audio_started = 1;
    debugPrintf("[stage1h-audio] native OGG mixer started: %d sounds, %d voices, audout=%u Hz, stack=256KB\n",
                TM_SOUND_COUNT, TM_MAX_VOICES, rt_audout_rate());
    log_flush_ring();
    return 0;
}

void tm_audio_shutdown(void) {
    if (!g_audio_started)
        return;

    g_audio_stop = 1;
    rt_audout_cancel(1);

    if (g_audio_thread_up) {
        threadWaitForExit(&g_audio_thread);
        threadClose(&g_audio_thread);
        g_audio_thread_up = 0;
    }

    rt_audout_cancel(0);

    for (int i = 0; i < TM_MAX_VOICES; i++)
        tm_voice_close(&g_voice[i]);

    g_audio_started = 0;
}

void tm_audio_play_sound(int id, float volume, int loop) {
    if (!tm_valid_id(id))
        return;
    g_last_volume[id] = volume;
    tm_queue(TM_CMD_PLAY, id, loop, volume);
}

int tm_audio_create_emitter(int id, int unk, int start) {
    (void)unk;
    (void)start;
    return tm_valid_id(id) ? id : -1;
}

void tm_audio_play_emitter(int id, int loop) {
    if (!tm_valid_id(id))
        return;
    tm_queue(TM_CMD_PLAY, id, loop, g_last_volume[id]);
}

void tm_audio_stop_sound(int id)      { tm_queue(TM_CMD_STOP_ID, id, 0, 0.0f); }
void tm_audio_kill_sound(int id)      { tm_queue(TM_CMD_STOP_ID, id, 0, 0.0f); }
void tm_audio_pause_sound(int id)     { tm_queue(TM_CMD_PAUSE_ID, id, 0, 0.0f); }
void tm_audio_resume_sound(int id)    { tm_queue(TM_CMD_RESUME_ID, id, 0, 0.0f); }
void tm_audio_stop_emitter(int id)    { tm_queue(TM_CMD_STOP_ID, id, 0, 0.0f); }
void tm_audio_kill_emitter(int id)    { tm_queue(TM_CMD_STOP_ID, id, 0, 0.0f); }
void tm_audio_pause_emitter(int id)   { tm_queue(TM_CMD_PAUSE_ID, id, 0, 0.0f); }
void tm_audio_resume_emitter(int id)  { tm_queue(TM_CMD_RESUME_ID, id, 0, 0.0f); }

void tm_audio_stop_all(void)   { tm_queue(TM_CMD_STOP_ALL, 0, 0, 0.0f); }
void tm_audio_kill_all(void)   { tm_queue(TM_CMD_STOP_ALL, 0, 0, 0.0f); }
void tm_audio_pause_all(void)  { tm_queue(TM_CMD_PAUSE_ALL, 0, 0, 0.0f); }
void tm_audio_resume_all(void) { tm_queue(TM_CMD_RESUME_ALL, 0, 0, 0.0f); }

void tm_audio_stop_group(int group_id)   { tm_queue(TM_CMD_STOP_GROUP, group_id, 0, 0.0f); }
void tm_audio_pause_group(int group_id)  { tm_queue(TM_CMD_PAUSE_GROUP, group_id, 0, 0.0f); }
void tm_audio_resume_group(int group_id) { tm_queue(TM_CMD_RESUME_GROUP, group_id, 0, 0.0f); }

int tm_audio_is_media_playing(int id) {
    return tm_valid_id(id) && g_alive_by_id[id] > 0;
}

int tm_audio_is_emitter_playing(int id) {
    return tm_audio_is_media_playing(id);
}

int tm_audio_is_emitter_stopped(int id) {
    return !tm_audio_is_media_playing(id);
}

int tm_audio_is_emitter_alive(int id) {
    return tm_audio_is_media_playing(id);
}

void tm_audio_set_sound_volume(int id, float volume) {
    if (!tm_valid_id(id))
        return;
    g_last_volume[id] = volume;
    tm_queue(TM_CMD_SET_ID_VOLUME, id, 0, volume);
}

void tm_audio_set_emitter_volume(int id, float volume) {
    tm_audio_set_sound_volume(id, volume);
}

float tm_audio_get_emitter_volume(int id) {
    return tm_valid_id(id) ? g_last_volume[id] : 0.0f;
}

void tm_audio_set_emitter_pos(int id, float x, float y, float z) {
    (void)id; (void)x; (void)y; (void)z;
}

void tm_audio_set_listener_pos(float x, float y, float z) {
    (void)x; (void)y; (void)z;
}

void tm_audio_get_emitter_pos(int id, float *x, float *y, float *z) {
    (void)id;
    if (x) *x = 0.0f;
    if (y) *y = 0.0f;
    if (z) *z = 0.0f;
}

void tm_audio_get_listener_pos(float *x, float *y, float *z) {
    if (x) *x = 0.0f;
    if (y) *y = 0.0f;
    if (z) *z = 0.0f;
}

float tm_audio_get_farthest_distance(void) {
    return 0.0f;
}

int tm_audio_paused(void) {
    return 0;
}

float tm_audio_get_group_volume(int group_id) {
    return g_group_volume[tm_group_index(group_id)];
}

void tm_audio_set_group_volume(int group_id, float volume) {
    int gi = tm_group_index(group_id);
    g_group_volume[gi] = tm_clampf(volume, 0.0f, 1.0f);

    static unsigned logs;
    if (logs++ < 24) {
        debugPrintf("[stage1h-audio] group %d volume = %.3f\n",
                    group_id, g_group_volume[gi]);
        log_flush_ring();
    }
}

int tm_audio_find_farthest_emitter(int group_id) {
    (void)group_id;
    return -1;
}

int tm_audio_init_sound_pool(void) {
    return tm_audio_init() == 0 ? 1 : 0;
}

void tm_audio_destroy_sound_pool(void) {
    /* Keep audout alive; the game can rebuild its Java-side pool during
     * state changes. Individual stop/kill calls own voice lifetime. */
}
