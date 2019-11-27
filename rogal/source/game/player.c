#include "player.h"
#include "camera.h"
#include "ui.h"
#include "particles.h"
#include <GL/glut.h>

player_t player;

int is_player_move;

int lerp_max_msec;
int lerp_msec;
vec2_t lerp_start;
vec2_t lerp_end;

int particle_msec_accumulator;
int particle_msec_current_limit;

vec2_t attack_anim_target;

void set_player_default_weapon(void) {

	sprite_t *s;
	item_t *w = new_item();

	if (!w) {

		d_printf(LOG_WARNING, "%s: no default weapon!\n", __func__);
		return;
	}

	w->item_type = MAP_ITEM_SWORD;
	w->item_category = ITEM_CATEGORY_WEAPON;

	s = new_sprite();

	s->tex_id = get_texture_id(SWORD);
	s->framecount = get_texture_framecount(SWORD);
	s->render_layer = get_texture_render_layer(SWORD);
	s->frame_msec = get_texture_frametime(SWORD);
	w->modifier_value = 1;
	s->action = weapon_pickup_action;

	//item is picked up
	s->collision_mask = COLLISION_IGNORE;
	s->render_layer = RENDER_LAYER_EFFECT;
	s->skip_render = 1;

	s->scale_x = 0.5f;
	s->scale_y = 0.5f;

	w->sprite = s;

	Color3ItemCommon(w->rarity_color);

	player.weapon = w;
}

void player_drop_weapon(void) {

	item_t *w = player.weapon;

	if (!w) {

		d_printf(LOG_WARNING, "%s: player has no weapon!\n", __func__);
		return;
	}

	w->sprite->collision_mask = COLLISION_ITEM;
	w->sprite->render_layer = RENDER_LAYER_ITEM;
	w->sprite->skip_render = 0;

	//visibility
	Color3Copy(w->rarity_color, w->sprite->color);
	w->sprite->visibility = VIS_VISIBLE;

	w->sprite->scale_x = 1.f;
	w->sprite->scale_y = 1.f;

	Vec2Copy(player.sprite[0]->position, w->sprite->position);
}

void player_pickup_weapon(item_t *w) {

	player_drop_weapon();

	//set item sprite
	w->sprite->collision_mask = COLLISION_IGNORE;
	w->sprite->render_layer = RENDER_LAYER_EFFECT;
	w->sprite->skip_render = 1;

	w->sprite->scale_x = 0.5f;
	w->sprite->scale_y = 0.5f;

	//set player stats
	player.stats.attack_damage = w->modifier_value;

	player.weapon = w;

	//particles
	make_pickup_particles(player.sprite[0]->position, w->rarity_color[0], w->rarity_color[1], w->rarity_color[2]);

	hud_update_dmg();
}

void set_player_default_state(void) {

	player.stats.health = 10;
	player.stats.max_health = 10;
	player.stats.attack_damage = 1;
	player.stats.armor = 5;
	player.stats.armor_modifier = 0;
}

sprite_t *new_player_sprite(void) {

	sprite_t *s = new_sprite();

	s->animation_pause = 1;
	s->skip_render = 1;
	s->render_layer = RENDER_LAYER_PLAYER;
	s->collision_mask = COLLISION_IGNORE;

	return s;
}

void create_player_sprites(void) {

	sprite_t *s;

	s = new_player_sprite();
	s->tex_id = get_texture_id(PLAYER_R);
	s->framecount = get_texture_framecount(PLAYER_R);
	s->frame_msec = get_texture_frametime(PLAYER_R);
	s->skip_render = 0;
	s->collision_mask = COLLISION_IGNORE;
	player.sprite[0] = s;

	s = new_player_sprite();
	s->tex_id = get_texture_id(PLAYER_L);
	s->framecount = get_texture_framecount(PLAYER_L);
	s->frame_msec = get_texture_frametime(PLAYER_L);
	s->collision_mask = COLLISION_IGNORE;
	player.sprite[1] = s;

	s = new_player_sprite();
	s->tex_id = get_texture_id(PLAYER_B);
	s->framecount = get_texture_framecount(PLAYER_B);
	s->frame_msec = get_texture_frametime(PLAYER_B);
	s->collision_mask = COLLISION_IGNORE;
	player.sprite[2] = s;
}

void face_direction(int direction) {

	r_clamp_set(direction, LOOK_RIGHT, LOOK_UP);

	//toggle sprites
	for (int i = 0; i < 3; i++) {

		player.sprite[i]->skip_render = (i != direction);
	}

	player.look_direction = direction;
}

void player_next_level(void) {

	for (int i = 0; i < 3; i++) {

		Vec2Zero(player.sprite[i]->position);
	}
	face_direction(ROTATION_0);
	set_camera_position(player.sprite[0]->position);

	recalculate_sprites_visibility();
}

void init_player(void) {

	for (int i = 0; i < 3; i++) {

		if (player.sprite[i]) {

			delete_sprite(player.sprite[i]);
		}
	}

	set_player_default_state();
	set_player_default_weapon();

	create_player_sprites();

	is_player_move = 0;

	recalculate_sprites_visibility();

	hud_update_armor();
	hud_update_dmg();
	hud_update_health();
}

void get_player_pos(vec2_t *out) {

	Vec2Copy(player.sprite[0]->position, *out);
}

int direction_to_tile(vec2_t tile_pos) {

	if (tile_pos[VEC_X] > player.sprite[VEC_X]->position[VEC_X]) {

		return LOOK_RIGHT;
	}
	if (tile_pos[VEC_X] < player.sprite[VEC_X]->position[VEC_X]) {

		return LOOK_LEFT;
	}
	if (tile_pos[VEC_Y] > player.sprite[VEC_Y]->position[VEC_Y]) {

		return LOOK_UP;
	}

	return (player.look_direction != LOOK_UP) ? player.look_direction : LOOK_RIGHT;
}

void walk_routine(int value) {

	vec2_t v;
	vec2_t particle_v;
	vec2_t particle_v2;
	int msec = glutGet(GLUT_ELAPSED_TIME) - value;

	//calculate msec
	int lerp_current_msec = lerp_msec + msec;

	if (lerp_current_msec > lerp_max_msec) {

		lerp_current_msec = lerp_max_msec;
	}

	lerp_msec = lerp_current_msec;

	Vec2Zero(v);
	Vec2Lerp(lerp_start, lerp_end, (float)lerp_current_msec / lerp_max_msec, v);

	//particles
	particle_msec_accumulator += msec;

	if (particle_msec_accumulator >= 50) {

		//spawn particles for both feet
		Vec2Copy(v, particle_v);
		particle_v[VEC_Y] -= SPRITE_SIZE - 0.12f;

		Vec2Copy(particle_v, particle_v2);

		particle_v[VEC_X] += 0.1f;
		particle_v2[VEC_X] -= 0.1f;

		make_walk_dust_particle(particle_v, v[VEC_Y] - SPRITE_SIZE + 0.08f);
		make_walk_dust_particle(particle_v2, v[VEC_Y] - SPRITE_SIZE + 0.08f);

		particle_msec_accumulator = 0;
		particle_msec_current_limit = Random(20, 60);

	}

	//apply position to sprites
	for (int i = 0; i < 3; i++) {

		Vec2Copy(v, player.sprite[i]->position);
	}

	//set camera to follow the player
	set_camera_position(v);

	if (lerp_current_msec != lerp_max_msec) {

		//continue routine
		glutTimerFunc(TICK_MSEC, walk_routine, glutGet(GLUT_ELAPSED_TIME));
	}
	else 
	{
		//end move
		is_player_move = 0;
		player.sprite[player.look_direction]->animation_pause = 1;
		player.sprite[player.look_direction]->current_frame = 1;

		//recalculate visibility
		recalculate_sprites_visibility();
	}
}

void walk_to_tile(vec2_t position, float dist) {

	player.sprite[player.look_direction]->animation_pause = 0;

	lerp_msec = 0;
	lerp_max_msec = (int)((1000 / MOVE_SPEED) * dist);
	Vec2Copy(player.sprite[0]->position, lerp_start);
	Vec2Copy(position, lerp_end);
	is_player_move = 1;

	particle_msec_accumulator = 0;
	particle_msec_current_limit = Random(20, 60);

	glutTimerFunc(TICK_MSEC, walk_routine, glutGet(GLUT_ELAPSED_TIME));
}

void attack_animation(int stop) {

	float diff_x, diff_y;
	// "animation"
	if (!stop) {

		//make the game wait for animation to end
		is_player_move = 1;

		//show the weapon
		diff_x = player.sprite[0]->position[VEC_X] - attack_anim_target[VEC_X];
		diff_y = player.sprite[0]->position[VEC_Y] - attack_anim_target[VEC_Y];

		player.weapon->sprite->scale_x = 0.7f * ((diff_x > 0 || player.look_direction == LOOK_LEFT) ? -1 : 1);
		player.weapon->sprite->scale_y = 0.7f * (diff_y > 0 ? -1 : 1);

		player.weapon->sprite->skip_render = 0;

		Vec2Lerp(player.sprite[0]->position, attack_anim_target, 0.5f, player.weapon->sprite->position);
		
		Color3Copy(player.weapon->rarity_color, player.weapon->sprite->color);

		glutTimerFunc(ATTACK_ANIM_MSEC, attack_animation, 1);
	}
	else
	{
		//hide the weapon
		player.weapon->sprite->scale_x = 0.7f;
		player.weapon->sprite->scale_y = 0.7f;

		player.weapon->sprite->skip_render = 1;

		is_player_move = 0;
	}

}

void attack_mob(mob_t *mob) {

	//show weapon animation
	Vec2Copy(mob->sprite[0]->position, attack_anim_target);
	attack_animation(0);

	//attack...
	mob_receive_damage(mob, player.stats.attack_damage);
}

void player_die(void) {

	color3_t c;
	Color3Red(c);

	d_printf(LOG_WARNING, "PLAYER DEATH\n");

	display_message("You didn't make it. Press any key to try again.", 0, c);

	make_death_particles(player.sprite[0]->position);
}

void player_receive_damage(int damage) {

	int armor_diff;

	armor_diff = player.stats.armor - damage;

	if (armor_diff < 0) {

		//armor destroyed
		armor_diff = damage - player.stats.armor; //leftover damage
		player.stats.armor = 0;

		//hp
		player.stats.health -= armor_diff;
	}
	else
	{
		//armor takes all the damage
		player.stats.armor -= damage;
	}

	//death?
	if (player.stats.health <= 0) {

		player.stats.health = 0;
		player.stats.armor = 0;

		player_die();
	}
	else
	{
		//add blood
		make_blood_particles(player.sprite[0]->position, player.sprite[0]->position[VEC_Y] - SPRITE_SIZE + 0.08f);
	}

	//update HUD
	hud_update_health();
	//update HUD
	hud_update_armor();
}

void add_health(int hp) {

	player.stats.health += hp;

	r_clamp_set(player.stats.health, 0, player.stats.max_health);

	d_printf(LOG_TEXT, "%s: added %dHP, total health: %dHP\n", __func__, hp, player.stats.health);

	//particles
	make_pickup_particles(player.sprite[0]->position, 0.5f, 0.f, 0.f);

	if (player.stats.health == 0) {

		player_die();
	}

	hud_update_health();
}

void add_armor(int val) {

	player.stats.armor += val;

	r_clamp_set(player.stats.armor, 0, MAX_ARMOR + player.stats.armor_modifier);

	d_printf(LOG_TEXT, "%s: added %d armor, total armor: %d\n", __func__, val, player.stats.armor);

	//particles
	make_pickup_particles(player.sprite[0]->position, 0.f, 0.6f, 0.f);

	hud_update_armor();
}

void increase_base_stat(int is_health, int value) {

	if (is_health) {

		player.stats.max_health += value;
		add_health(value);
	}
	else
	{
		player.stats.armor_modifier += value;
		add_armor(value);
	}
}