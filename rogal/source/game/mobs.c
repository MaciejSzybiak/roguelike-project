#include "game.h"
#include "camera.h"
#include "raycast.h"
#include "particles.h"
#include <string.h>
#include <GL/glut.h>

int		is_mob_move = 0;
mob_t	mobs[MAX_MOBS];

vec2_t	lerp_starts[MAX_MOBS];
vec2_t	lerp_ends[MAX_MOBS];
int		lerp_max_msecs[MAX_MOBS];
int		lerp_msecs[MAX_MOBS];

int		attacks_player[MAX_MOBS];

int		is_mob_attack = 0;

void delete_mob(mob_t *mob) {

	if (!mob) {

		d_printf(LOG_WARNING, "%s: NULL mob passed as the argument\n", __func__);
		return;
	}

	for (int i = 0; i < 3; i++) {

		if (mob->sprite[i]) {

			delete_sprite(mob->sprite[i]);
		}
	}

	if (mob->attack_sprite) {

		delete_sprite(mob->attack_sprite);
	}

	memset(mob, 0, sizeof(mob_t));
}

mob_t *new_mob(void) {

	for (int i = 0; i < MAX_MOBS; i++) {

		if (mobs[i].type == MAP_NOTHING) {

			return &mobs[i];
		}
	}
	d_printf(LOG_ERROR, "%s: max mobs exceeded!\n", __func__);
	return &mobs[MAX_MOBS - 1];
}

int alive_mobs_count(void) {

	int count = 0;
	for (int i = 0; i < MAX_MOBS; i++) {

		if (mobs[i].type != MAP_NOTHING) {

			count++;
		}
	}
	return count;
}

void randomize_mob(mob_t *mob) {

	mob->stats.health = mob->stats.max_health = MIN_MOB_HEALTH + rand() % (MAX_MOB_HEALTH + 1);
	mob->stats.armor = MIN_MOB_ARMOR + rand() % (MAX_MOB_ARMOR + 1);
	mob->stats.attack_damage = MIN_MOB_DAMAGE + rand() % (MAX_MOB_DAMAGE + 1);
}

void add_mob_sprite(mob_t *mob, int index, texname tname, float x_pos, float y_pos) {

	sprite_t *s;

	s = new_sprite();

	s->tex_id = get_texture_id(tname);
	s->framecount = get_texture_framecount(tname);
	s->render_layer = get_texture_render_layer(tname);
	s->frame_msec = get_texture_frametime(tname);
	s->render_layer = get_texture_render_layer(tname);
	s->animation_pause = 1;
	s->collision_mask = COLLISION_MOB;
	s->position[VEC_X] = x_pos;
	s->position[VEC_Y] = y_pos;
	s->skip_render = 1;

	mob->sprite[index] = s;
}

void generate_mobs(void) {

	texname tnames[3];
	texname attack_tname;
	mob_t *mob;
	sprite_t *s;

	for (int x = 0; x < MAP_SIZE; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {

			switch (map_contents[x][y])
			{
				case MAP_MOB_SLIME:
					tnames[0] = SLIME_R;
					tnames[1] = SLIME_L;
					tnames[2] = SLIME_B;
					attack_tname = SLIME_ATTACK;
					break;
				case MAP_NOTHING:
				default:
					continue;
			}

			mob = new_mob();
			mob->type = map_contents[x][y];

			for (int i = 0; i < 3; i++) {

				add_mob_sprite(mob, i, tnames[i], (x * SPRITE_SIZE * 2) - MAP_OFFSET, (y * SPRITE_SIZE * 2) - MAP_OFFSET);
			}

			mob->sprite[0]->skip_render = 0;

			//add attack sprite
			s = new_sprite();
			s->tex_id = get_texture_id(attack_tname);
			s->framecount = get_texture_framecount(attack_tname);
			s->render_layer = get_texture_render_layer(attack_tname);
			s->frame_msec = get_texture_frametime(attack_tname);
			s->render_layer = get_texture_render_layer(attack_tname);
			s->animation_pause = 1;
			s->collision_mask = COLLISION_IGNORE;
			s->position[VEC_X] = 0; //doesn't really matter at this moment?
			s->position[VEC_Y] = 0;
			s->skip_render = 1;
			s->scale_x = 0.7f;
			s->scale_y = 0.7f;

			mob->attack_sprite = s;

			randomize_mob(mob);
		}
	}
}

void init_mobs(void) {

	for (int i = 0; i < MAX_MOBS; i++) {

		if (mobs[i].sprite[0]) {

			delete_mob(&mobs[i]);
		}
	}

	memset(&mobs, 0, sizeof(mob_t) * MAX_MOBS);
	generate_mobs();
}

mob_t *find_mob(vec2_t position) {

	for (int i = 0; i < MAX_MOBS; i++) {

		if (mobs[i].type != MAP_NOTHING && Vec2Compare(mobs[i].sprite[0]->position, position)) {

			return &mobs[i];
		}
	}
	d_printf(LOG_WARNING, "%s: mob not found!\n", __func__);
	return NULL;
}

void mob_die(mob_t *mob) {

	delete_mob(mob);
}

void mob_look_at_rotation(mob_t *mob, int rot) {

	for (int i = 0; i < 3; i++) {

		mob->sprite[i]->skip_render = 1;
	}
	switch (rot)
	{
		case ROTATION_0:
		case ROTATION_90:
			mob->sprite[0]->skip_render = 0;
			mob->look_direction = LOOK_RIGHT;
			break;
		case ROTATION_180:
			mob->sprite[1]->skip_render = 0;
			mob->look_direction = LOOK_LEFT;
			break;
		case ROTATION_270:
			mob->sprite[2]->skip_render = 0;
			mob->look_direction = LOOK_UP;
			break;
	}
}

void lerp_all_mobs(int value) {

	vec2_t v;
	int lerp_current_msec;
	int msec = glutGet(GLUT_ELAPSED_TIME) - value;
	int all_done = 1;

	for (int i = 0; i < MAX_MOBS; i++) {

		if (lerp_max_msecs[i] == 0 || lerp_msecs[i] == lerp_max_msecs[i]) {

			continue;
		}
		all_done = 0;

		lerp_current_msec = lerp_msecs[i] + msec;

		if (lerp_current_msec >= lerp_max_msecs[i]) {

			lerp_current_msec = lerp_max_msecs[i];

			//last frame, pause the animation
			mobs[i].sprite[mobs[i].look_direction]->animation_pause = 1;
		}
		else
		{
			//unpause the animation
			mobs[i].sprite[mobs[i].look_direction]->animation_pause = 0;
		}

		lerp_msecs[i] = lerp_current_msec;

		Vec2Zero(v);
		Vec2Lerp(lerp_starts[i], lerp_ends[i], (float)lerp_current_msec / lerp_max_msecs[i], v);

		for (int j = 0; j < 3; j++) {

			Vec2Copy(v, mobs[i].sprite[j]->position);
		}
	}

	if (!all_done) {

		//continue
		glutTimerFunc(TICK_MSEC, lerp_all_mobs, glutGet(GLUT_ELAPSED_TIME));
	}
	else
	{
		//end
		is_mob_move = 0;
		recalculate_sprites_visibility();
	}
}

void lerp_mobs_wait_for_attack(int value) {

	UNUSED_VARIABLE(value);

	if (is_mob_attack) {

		glutTimerFunc(TICK_MSEC, lerp_mobs_wait_for_attack, 0);
	}
	else
	{
		glutTimerFunc(TICK_MSEC, lerp_all_mobs, glutGet(GLUT_ELAPSED_TIME));
	}
}

void attack_routine(int value) {

	float diff_x, diff_y;
	vec2_t player_pos;

	if (!value) {

		get_player_pos(&player_pos);

		for (int i = 0; i < MAX_MOBS; i++) {

			if (attacks_player[i]) {

				//show the attack sprite
				diff_x = player_pos[VEC_X] - mobs[i].sprite[0]->position[VEC_X];
				diff_y = player_pos[VEC_Y] - mobs[i].sprite[0]->position[VEC_Y];

				//set rotation
				mobs[i].attack_sprite->rotation = LookToRot(mobs[i].look_direction);

				if (diff_x < 0) {

					mobs[i].attack_sprite->rotation = 2;
				}
				else if(diff_x > 0)
				{
					mobs[i].attack_sprite->rotation = 0;
				}
				else if (diff_y < 0)
				{
					mobs[i].attack_sprite->rotation = 1;
				}
				else
				{
					mobs[i].attack_sprite->rotation = 3;
				}

				mobs[i].attack_sprite->skip_render = 0;

				Vec2Lerp(mobs[i].sprite[0]->position, player_pos, 0.5f, mobs[i].attack_sprite->position);

				//make player receive the damage
				player_receive_damage(mobs[i].stats.attack_damage);
			}
		}
		glutTimerFunc(ATTACK_ANIM_MSEC, attack_routine, 1);
	}
	else
	{
		for (int i = 0; i < MAX_MOBS; i++) {

			if (attacks_player[i]) {

				mobs[i].attack_sprite->skip_render = 1;
			}
		}

		is_mob_attack = 0;
	}
}

void all_mobs_attack(void) {

	is_mob_attack = 1;
	attack_routine(0);
}

//FIXME: this is a mess
void calculate_mob_destinations(void) {

	vec2_t vtemp;
	vec2_t player_pos;
	int available_angles[4];
	int x, y, random, no_angles, i, j, k;
	sprite_t *s;

	int any_attacks = 0;

	//clear all arrays
	memset(&lerp_starts, 0, sizeof(vec2_t) * MAX_MOBS);
	memset(&lerp_ends, 0, sizeof(vec2_t) * MAX_MOBS);
	memset(&lerp_msecs, 0, sizeof(int) * MAX_MOBS);
	memset(&lerp_max_msecs, 0, sizeof(int) * MAX_MOBS);
	memset(&attacks_player, 0, sizeof(int) * MAX_MOBS);

	get_player_pos(&player_pos);

	for (i = 0; i < MAX_MOBS; i++) {

next_mob:
		if (!mobs[i].sprite[0] || !mobs[i].sprite[1]|| !mobs[i].sprite[2]
			|| (mobs[i].sprite[0]->skip_render == 1 &&
			mobs[i].sprite[1]->skip_render == 1 && 
			mobs[i].sprite[2]->skip_render == 1)) {

			//mob inactive
			continue;
		}

		no_angles = 1;
		memset(&available_angles, 0, sizeof(int) * 4);

		for (j = 0; j < 4; j++) {

			Vec2Copy(mobs[i].sprite[0]->position, vtemp);

			vtemp[VEC_X] += (SPRITE_SIZE * 2 * !(j & 1)) * (j > 0 ? -1 : 1);
			vtemp[VEC_Y] += (SPRITE_SIZE * 2 * (j & 1)) * (j > 1 ? 1 : -1);

			//check if blocked by player
			if (Vec2Distance(vtemp, player_pos) < SPRITE_SIZE) {

				attacks_player[i] = 1;
				any_attacks = 1;

				//look at the player
				mob_look_at_rotation(&mobs[i], j);

				i++;
				goto next_mob;
			}

			//check other mobs
			for (k = 0; k < MAX_MOBS; k++) {

				if (!mobs[k].sprite[0]) {

					continue;
				}

				if (Vec2Distance(vtemp, mobs[k].sprite[0]->position) < SPRITE_SIZE) {

					//another mob is on that tile
					goto next_angle;
				}
				//if lerp vector is set and the distance is too small...
				if (lerp_max_msecs[k] && Vec2Distance(vtemp, lerp_ends[k]) < SPRITE_SIZE) {

					goto next_angle;
				}
			}

			//check tiles
			x = (int)((MAP_OFFSET + vtemp[VEC_X]) / (SPRITE_SIZE * 2));
			y = (int)((MAP_OFFSET + vtemp[VEC_Y]) / (SPRITE_SIZE * 2));

			s = sprite_map[x][y];

			if (!s) {

				continue;
			}
			if (s->collision_mask & COLLISION_FLOOR) {

				available_angles[j] = 1;
				no_angles = 0;
			}

next_angle:
			continue;
		}

		//mob can't go anywhere
		if (no_angles) {

			continue;
		}

		//pick a random available angle
		do {
			random = Random(0, 3);
		} while (!available_angles[random]);

		//calculate lerp
		Vec2Copy(mobs[i].sprite[0]->position, lerp_starts[i]);
		Vec2Copy(mobs[i].sprite[0]->position, lerp_ends[i]);

		lerp_ends[i][VEC_X] += (SPRITE_SIZE * 2 * !(random & 1)) * (random > 0 ? -1 : 1);
		lerp_ends[i][VEC_Y] += (SPRITE_SIZE * 2 * (random & 1)) * (random > 1 ? 1 : -1);

		//make mobs move faster than the player (slow movement ruins the game flow...)
		lerp_max_msecs[i] = (int)((1000 / (MOVE_SPEED * 8)) * SPRITE_SIZE * 2);

		//make mob look at the direction
		mob_look_at_rotation(&mobs[i], random);
	}

	//attack
	if (any_attacks) {

		all_mobs_attack();

	}

	//start lerp routine wait
	glutTimerFunc(TICK_MSEC, lerp_mobs_wait_for_attack, 0);
}

void wait_for_player(int value) {

	UNUSED_VARIABLE(value);

	if (is_player_move) {

		glutTimerFunc(TICK_MSEC, wait_for_player, 0);
	}
	else
	{
		calculate_mob_destinations();
	}
}

void mobs_move(void) {

	is_mob_move = 1;

	glutTimerFunc(TICK_MSEC, wait_for_player, 0);
}

void mob_receive_damage(mob_t *mob, int damage) {

	int armor_diff;

	armor_diff = mob->stats.armor - damage;

	if (armor_diff < 0) {

		//armor destroyed
		armor_diff = damage - mob->stats.armor; //leftover damage
		mob->stats.armor = 0;

		//hp
		mob->stats.health -= armor_diff;
	}
	else
	{
		//armor takes all the damage
		mob->stats.armor -= damage;
	}

	//death?
	if (mob->stats.health <= 0) {

		mob->stats.health = 0;
		mob->stats.armor = 0;

		//death particles
		make_death_particles(mob->sprite[0]->position);

		mob_die(mob);
	}
	else
	{
		//add blood effect
		make_blood_particles(mob->sprite[0]->position, mob->sprite[0]->position[VEC_Y] - SPRITE_SIZE + 0.08f);
	}
}