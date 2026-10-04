#ifndef TM_AUDIO_H
#define TM_AUDIO_H

int tm_audio_init(void);
void tm_audio_shutdown(void);

void tm_audio_play_sound(int id, float volume, int loop);
int  tm_audio_create_emitter(int id, int unk, int start);
void tm_audio_play_emitter(int id, int loop);

void tm_audio_stop_sound(int id);
void tm_audio_kill_sound(int id);
void tm_audio_pause_sound(int id);
void tm_audio_resume_sound(int id);

void tm_audio_stop_emitter(int id);
void tm_audio_kill_emitter(int id);
void tm_audio_pause_emitter(int id);
void tm_audio_resume_emitter(int id);

void tm_audio_stop_all(void);
void tm_audio_kill_all(void);
void tm_audio_pause_all(void);
void tm_audio_resume_all(void);

void tm_audio_stop_group(int group_id);
void tm_audio_pause_group(int group_id);
void tm_audio_resume_group(int group_id);

int   tm_audio_is_media_playing(int id);
int   tm_audio_is_emitter_playing(int id);
int   tm_audio_is_emitter_stopped(int id);
int   tm_audio_is_emitter_alive(int id);

void  tm_audio_set_sound_volume(int id, float volume);
void  tm_audio_set_emitter_volume(int id, float volume);
float tm_audio_get_emitter_volume(int id);

void  tm_audio_set_emitter_pos(int id, float x, float y, float z);
void  tm_audio_set_listener_pos(float x, float y, float z);
void  tm_audio_get_emitter_pos(int id, float *x, float *y, float *z);
void  tm_audio_get_listener_pos(float *x, float *y, float *z);
float tm_audio_get_farthest_distance(void);
int   tm_audio_paused(void);

float tm_audio_get_group_volume(int group_id);
void  tm_audio_set_group_volume(int group_id, float volume);

int tm_audio_find_farthest_emitter(int group_id);

int tm_audio_init_sound_pool(void);
void tm_audio_destroy_sound_pool(void);

#endif
