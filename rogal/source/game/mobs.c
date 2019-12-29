/*
* This file is responsible for managing mob allocation and behaviour.
* Mobs are the hostile creatures that player meets while exploring
* dungeons.
* 
* Mob behaviour includes walking (randomly or towards player) and attacking
* the player when next to him.
*/

#include "game.h"
#include "camera.h"
#include "raycast.h"
#include "particles.h"
#include <string.h>
#include <GL/glut.h>

#define TEXT_XOFFS 0.35f
#define TEXT_YOFFS 0.35f
#define TEXT_SCALE 0.25f

#define TEXT_BG_XSCALE ((TEXT_SCALE))
#define TEXT_BG_YSCALE (TEXT_BG_XSCALE * 2)

int		is_mob_move = 0;					//global mob state: if mobs are moving then no inputs are processed
mob_t	mobs[MAX_MOBS];						//all mobs are kept here

//behaviour
static vec2_t	lerp_starts[MAX_MOBS];		//each mob can have a movement start
static vec2_t	lerp_ends[MAX_MOBS];		//and movement end
static int		lerp_max_msecs[MAX_MOBS];	//how many miliseconds should the move take
static int		lerp_msecs[MAX_MOBS];		//how many miliseconds this move already took

static int		attacks_player[MAX_MOBS];	//1 => this mob attacks the player

static int		is_mob_attack = 0;			//local attack state (for holding movement until attacks are done)

/*
* Deletes the mob from current level.
*/
void delete_mob(mob_t *mob) {

	if (!mob) {

		d_printf(LOG_WARNING, "%s: NULL mob passed as the argument\n", __func__);
		return;
	}

	//delete mob's sprites
	for (int i = 0; i < 3; i++) {

		if (mob->sprite[i]) {

			delete_sprite(mob->sprite[i]);
		}
	}

	//delete attack sprite
	if (mob->attack_sprite) {

		delete_sprite(mob->attack_sprite);
	}

	//delete texts
	if (mob->health_text) {

		delete_text(mob->health_text);
	}
	if (mob->armor_text) {

		delete_text(mob->armor_text);
	}

	//delete text background
	if (mob->text_background) {

		delete_sprite(mob->text_background);
	}

	//wipe the entire structure
	memset(mob, 0, sizeof(mob_t));
}

/*
* Finds a free mob slot and returns a pointer to it.
*/
mob_t *new_mob(void) {

	for (int i = 0; i < MAX_MOBS; i++) {

		//empty mob
		if (mobs[i].type == MAP_NOTHING) {

			return &mobs[i];
		}
	}
	//array end
	d_printf(LOG_ERROR, "%s: max mobs exceeded!\n", __func__);

	//clear and return the last mob
	delete_mob(&mobs[MAX_MOBS - 1]);
	return &mobs[MAX_MOBS - 1];
}

/*
* Counts all alive mobs and returns the amount.
*/
int alive_mobs_count(void) {

	int count = 0;
	for (int i = 0; i < MAX_MOBS; i++) {

		if (mobs[i].type != MAP_NOTHING) {

			count++;
		}
	}
	return count;
}

/*
* Updates mob stat texts.
*/
void mob_update_texts(mob_t *mob) {

	char text[4];
	int x_mult = 1;
	int len;

	if (mob->health_text) {

		memset(text, 0, 4 * sizeof(char));
		snprintf(text, 4, "%d", mob->stats.health);

		Vec2Copy(mob->sprite[0]->position, mob->health_text->position);
		mob->health_text->position[VEC_X] += TEXT_XOFFS;
		mob->health_text->position[VEC_Y] -= TEXT_YOFFS - TEXT_SCALE;

		x_mult = strlen(text);

		set_text(mob->health_text, text);
	}

	if (mob->armor_text) {

		memset(text, 0, 4 * sizeof(char));
		snprintf(text, 4, "%d", mob->stats.armor);

		Vec2Copy(mob->sprite[0]->position, mob->armor_text->position);
		mob->armor_text->position[VEC_X] += TEXT_XOFFS;
		mob->armor_text->position[VEC_Y] -= TEXT_YOFFS;

		len = strlen(text);
		if (len > x_mult) {

			x_mult = len;
		}

		set_text(mob->armor_text, text);
	}

	if (mob->text_background) {

		//set background scale and position
		Vec2Lerp(mob->armor_text->position, mob->health_text->position, 0.6f, mob->text_background->position);

		mob->text_background->scale_x = TEXT_BG_XSCALE * x_mult; //make sure the background grows bigger if any stat is > 10
		mob->text_background->scale_y = TEXT_BG_YSCALE;
	}
}

/*
* Gives mob random statistics in range of the preset limits.
*/
void randomize_mob(mob_t *mob) {

	int current_level = get_current_level();
	int min_health = MIN_MOB_HEALTH + current_level / 2;
	int min_armor = MIN_MOB_ARMOR;
	int min_damage = MIN_MOB_DAMAGE;

	mob->stats.health = mob->stats.max_health = (MIN_MOB_HEALTH + current_level / 2) + rand() % ((MAX_MOB_HEALTH + current_level) - min_health);
	mob->stats.armor = MIN_MOB_ARMOR + rand() % ((MAX_MOB_ARMOR + current_level / 3) - min_armor);
	mob->stats.attack_damage = MIN_MOB_DAMAGE + rand() % ((MAX_MOB_DAMAGE + current_level / 2) - min_damage);
}

/*
* Adds a sprite to the *mob at the given index, with the given name and position.
*/
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

/*
* Generates mobs based on map_contents array.
*/
void generate_mobs(void) {

	texname tnames[3];		//names for all mob sprites
	texname attack_tname;	//name of the attack sprite
	mob_t *mob;
	sprite_t *s;

	//check all map contents
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
				case MAP_MOB_GOBLIN:
					tnames[0] = GOBLIN_R;
					tnames[1] = GOBLIN_L;
					tnames[2] = GOBLIN_B;
					attack_tname = GOBLIN_ATTACK;
					break;
				case MAP_NOTHING:
				default:
					continue;
			}

			//get a new mob
			mob = new_mob();
			mob->type = map_contents[x][y];

			//set sprites
			for (int i = 0; i < 3; i++) {

				add_mob_sprite(mob, i, tnames[i], (x * SPRITE_SIZE * 2) - MAP_OFFSET, (y * SPRITE_SIZE * 2) - MAP_OFFSET);
			}

			mob->sprite[0]->skip_render = 0; //activate first sprite by default

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

			randomize_mob(mob); //set random statistics

			//set statistics texts
			//health
			mob->health_text = new_text();
			mob->health_text->scale = TEXT_SCALE;
			mob->health_text->render_layer = RENDER_LAYER_ONTOP;
			Color3UIRed(mob->health_text->color);

			//armor
			mob->armor_text = new_text();
			mob->armor_text->scale = TEXT_SCALE;
			mob->armor_text->render_layer = RENDER_LAYER_ONTOP;
			Color3UIGreen(mob->armor_text->color);

			//text background
			mob->text_background = new_sprite();
			mob->text_background->tex_id = get_texture_id(MOB_UI_BG);
			mob->text_background->framecount = get_texture_framecount(MOB_UI_BG);
			mob->text_background->render_layer = get_texture_render_layer(MOB_UI_BG);
			mob->text_background->frame_msec = get_texture_frametime(MOB_UI_BG);
			mob->text_background->render_layer = get_texture_render_layer(MOB_UI_BG);

			mob_update_texts(mob);
		}
	}
}

/*
* Runs mob initialization (executed after map generation)
*/
void init_mobs(void) {

	//clear all existing mobs
	for (int i = 0; i < MAX_MOBS; i++) {

		if (mobs[i].sprite[0]) {

			delete_mob(&mobs[i]);
		}
	}

	//wipe the entire array
	memset(&mobs, 0, sizeof(mob_t) * MAX_MOBS);

	//generate new mobs
	generate_mobs();
}

/*
* Finds mob with a given position (if there is any).
*/
mob_t *find_mob(vec2_t position) {

	for (int i = 0; i < MAX_MOBS; i++) {

		//check if mob is active and position matches
		if (mobs[i].type != MAP_NOTHING && Vec2Compare(mobs[i].sprite[0]->position, position)) {

			return &mobs[i];
		}
	}
	//found nothing
	d_printf(LOG_WARNING, "%s: mob not found!\n", __func__);
	return NULL;
}

/*
* Executed when mob's health goes equal or below 0HP
*/
void mob_die(mob_t *mob) {

	delete_mob(mob);
}

/*
* Activates correct mob sprite and sets look direction according to the rotation.
*/
void mob_look_at_rotation(mob_t *mob, int rot) {

	//deactivate all sprites
	for (int i = 0; i < 3; i++) {

		mob->sprite[i]->skip_render = 1;
	}
	//activate the correct sprite
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

/*
* Glut timer callback that makes all mobs move from lerp_start to lerp_end
*/
void lerp_all_mobs(int value) {

	vec2_t v;
	int lerp_current_msec;
	int msec = glutGet(GLUT_ELAPSED_TIME) - value; //get delta time (value is elapsed time on last frame)
	int all_done = 1;

	//iterate over all mobs (they all move at once)
	for (int i = 0; i < MAX_MOBS; i++) {

		if (lerp_max_msecs[i] == 0 || lerp_msecs[i] == lerp_max_msecs[i]) {

			//this mob doesn't move
			continue;
		}
		all_done = 0; //this one needs to be moved, so it's not "all done"

		//increment lerp miliseconds
		lerp_current_msec = lerp_msecs[i] + msec;

		//maximum lerp time reached?
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

		//find new position of the mob (lerp between move start and end by total time's fraction in this frame
		Vec2Zero(v);
		Vec2Lerp(lerp_starts[i], lerp_ends[i], (float)lerp_current_msec / lerp_max_msecs[i], v);

		//apply position to all sprites
		for (int j = 0; j < 3; j++) {

			Vec2Copy(v, mobs[i].sprite[j]->position);
		}

		//update stats texts
		mob_update_texts(&mobs[i]);
	}

	if (!all_done) {

		//execute again at next tick time
		glutTimerFunc(TICK_MSEC, lerp_all_mobs, glutGet(GLUT_ELAPSED_TIME)); //pass current time as value
	}
	else
	{
		//end
		is_mob_move = 0;
		recalculate_sprites_visibility(); //recalculate visibility after movement
	}
}

/*
* This timer callback waits for attack routine to end and executes movement callback.
*/
void lerp_mobs_wait_for_attack(int value) {

	UNUSED_VARIABLE(value);

	if (is_mob_attack) {

		//still attacking, recheck at next tick
		glutTimerFunc(TICK_MSEC, lerp_mobs_wait_for_attack, 0);
	}
	else
	{
		//run movement lerp routine
		glutTimerFunc(TICK_MSEC, lerp_all_mobs, glutGet(GLUT_ELAPSED_TIME));
	}
}

/*
* Attack routine callback. Basically this function makes the mob attack sprite appear for a while.
* Value = 0: start attack, value = 1: end attack.
*/
void attack_routine(int value) {

	float diff_x, diff_y;
	vec2_t player_pos;

	if (!value) {

		get_player_pos(&player_pos);

		for (int i = 0; i < MAX_MOBS; i++) {

			if (attacks_player[i]) {

				//show the attack sprite

				//set rotation
				mobs[i].attack_sprite->rotation = LookToRot(mobs[i].look_direction);

				//set attack sprite rotation
				diff_x = player_pos[VEC_X] - mobs[i].sprite[0]->position[VEC_X];
				diff_y = player_pos[VEC_Y] - mobs[i].sprite[0]->position[VEC_Y];

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

				//activate the attack sprite
				mobs[i].attack_sprite->skip_render = 0;

				//move to the correct position
				Vec2Lerp(mobs[i].sprite[0]->position, player_pos, 0.5f, mobs[i].attack_sprite->position);

				//make player receive the damage
				player_receive_damage(mobs[i].stats.attack_damage);
			}
		}
		//set to be called again to end after attack msecs have passed
		glutTimerFunc(ATTACK_ANIM_MSEC, attack_routine, 1); 
	}
	else
	{
		//end attack - hide all attack sprites
		for (int i = 0; i < MAX_MOBS; i++) {

			if (attacks_player[i]) {

				mobs[i].attack_sprite->skip_render = 1;
			}
		}

		is_mob_attack = 0;
	}
}

/*
* Starts the attack routine.
*/
void all_mobs_attack(void) {

	is_mob_attack = 1;
	attack_routine(0);
}

/*
* Pathfinding. Uses heuristic to find best tile to move towards the player. Simply tries to decrease the distance
* between the two as much as possible. If the tile cannot be stepped on (is water for example) then
* movement is cancelled.
* Returns 0 if no action can be performed, returns 1 if movement is performed and returns 2 if attack
* is performed against the player.
* dir_out: set to the direction of the movement (if any)
* i: mob array index
*/
int to_player_mob_destination(mob_t *m, int i, int *dir_out) {

	vec2_t dominant_dir;
	vec2_t dominant_dir2;
	vec2_t player_pos;
	sprite_t *s;
	sprite_t *s2 = NULL;
	float diff_x, diff_y, abs_x, abs_y;
	int k, x, y;
	int rotation = 0;
	int rotation2 = 0;
	int try_both = 0;
	int is_first_invalid = 0;

	get_player_pos(&player_pos);

	//find the best tile towards player

	//position differences
	diff_x = m->sprite[0]->position[VEC_X] - player_pos[VEC_X];
	diff_y = m->sprite[0]->position[VEC_Y] - player_pos[VEC_Y];
	//position distances
	abs_x = fabsf(diff_x);
	abs_y = fabsf(diff_y);

	Vec2Copy(m->sprite[0]->position, dominant_dir);

	//largest distance wins
	if (abs_x > abs_y) {

		dominant_dir[VEC_X] += SPRITE_SIZE * 2 * (diff_x > 0 ? -1 : 1);
		rotation = diff_x > 0 ? 2 : 0;
	}
	else if(abs_x < abs_y)
	{
		dominant_dir[VEC_Y] += SPRITE_SIZE * 2 * (diff_y > 0 ? -1 : 1);
		rotation = diff_y > 0 ? 1 : 3;
	}
	else
	{
		//if both distances are equal then both need to be tried (one might be blocked by a wall for example)
		try_both = 1;
		Vec2Copy(m->sprite[0]->position, dominant_dir2);

		dominant_dir[VEC_X] += SPRITE_SIZE * 2 * (diff_x > 0 ? -1 : 1);
		rotation = diff_x > 0 ? 2 : 0;

		dominant_dir2[VEC_Y] += SPRITE_SIZE * 2 * (diff_y > 0 ? -1 : 1);
		rotation2 = diff_y > 0 ? 1 : 3;
	}

	//check if attacks the player (must be close enough)
	if (Vec2Distance(m->sprite[0]->position, player_pos) <= SPRITE_SIZE * 2) {

		attacks_player[i] = 1;

		mob_look_at_rotation(m, rotation);

		return 2;
	}

	//check other mobs
	for (k = 0; k < MAX_MOBS; k++) {

		if (!mobs[k].sprite[0]) {

			//inactive mob
			continue;
		}

		if (Vec2Distance(dominant_dir, mobs[k].sprite[0]->position) < SPRITE_SIZE) {

			//another mob is on that tile
			if (try_both) {

				if (Vec2Distance(dominant_dir2, mobs[k].sprite[0]->position) < SPRITE_SIZE) {

					//also on the second tile
					return 0;
				}
			}
			else
			{
				return 0;
			}
			is_first_invalid = 1;
		}

		//if lerp vector is set and the distance to it is too small... (another mob will take this tile)
		if (lerp_max_msecs[k] && Vec2Distance(dominant_dir, lerp_ends[k]) < SPRITE_SIZE) {

			if (try_both) {

				if (lerp_max_msecs[k] && Vec2Distance(dominant_dir2, lerp_ends[k]) < SPRITE_SIZE) {

					//both will be taken
					return 0;
				}
			}
			else
			{
				return 0;
			}
			is_first_invalid = 1;
		}
	}

	//check tiles
	x = (int)((MAP_OFFSET + dominant_dir[VEC_X]) / (SPRITE_SIZE * 2));
	y = (int)((MAP_OFFSET + dominant_dir[VEC_Y]) / (SPRITE_SIZE * 2));

	s = sprite_map[x][y];

	if (!s) {

		if (!try_both) {

			return 0;
		}
		is_first_invalid = 1;
	}
	if (try_both) {

		x = (int)((MAP_OFFSET + dominant_dir2[VEC_X]) / (SPRITE_SIZE * 2));
		y = (int)((MAP_OFFSET + dominant_dir2[VEC_Y]) / (SPRITE_SIZE * 2));

		s2 = sprite_map[x][y];

		if (!s2 && is_first_invalid) {

			return 0;
		}
	}

	if ((s->collision_mask & COLLISION_FLOOR) && !is_first_invalid) { //is target a floor?

		//walk here

		//calculate lerp
		Vec2Copy(m->sprite[0]->position, lerp_starts[i]);
		Vec2Copy(m->sprite[0]->position, lerp_ends[i]);

		lerp_ends[i][VEC_X] += (SPRITE_SIZE * 2 * !(rotation & 1)) * (rotation > 0 ? -1 : 1);
		lerp_ends[i][VEC_Y] += (SPRITE_SIZE * 2 * (rotation & 1)) * (rotation > 1 ? 1 : -1);
		lerp_max_msecs[i] = 1; //temporary set for other mob checks

		*dir_out = rotation;

		return 1;
	}
	else if (try_both && s2 && s2->collision_mask & COLLISION_FLOOR) { //is the second target a flooor?

		//or here
		Vec2Copy(m->sprite[0]->position, lerp_starts[i]);
		Vec2Copy(m->sprite[0]->position, lerp_ends[i]);

		lerp_ends[i][VEC_X] += (SPRITE_SIZE * 2 * !(rotation2 & 1)) * (rotation2 > 0 ? -1 : 1);
		lerp_ends[i][VEC_Y] += (SPRITE_SIZE * 2 * (rotation2 & 1)) * (rotation2 > 1 ? 1 : -1);
		lerp_max_msecs[i] = 1; //temporary set for other mob checks

		*dir_out = rotation2;

		return 1;
	}
	//nowhere to go
	return 0;
}

/*
* Pathfinding. Finds a random direction for a mob to move to. First finds all available targets, then randomly
* picks one of them if there is any.
* Returns 0 if no action can be performed, returns 1 if movement is performed and returns 2 if attack
* is performed against the player.
* dir_out: set to the direction of the movement (if any)
* i: mob array index
*/
int random_mob_destination(mob_t *m, int i, int *rand_out) {

	vec2_t vtemp;
	vec2_t player_pos;
	int available_angles[4];
	int x, y, random, no_angles, j, k;
	sprite_t *s;

	get_player_pos(&player_pos);

	no_angles = 1;
	memset(&available_angles, 0, sizeof(int) * 4);

	//check all directions
	for (j = 0; j < 4; j++) {

		Vec2Copy(m->sprite[0]->position, vtemp);

		//add direction unit vector to vtemp
		vtemp[VEC_X] += (SPRITE_SIZE * 2 * !(j & 1)) * (j > 0 ? -1 : 1);
		vtemp[VEC_Y] += (SPRITE_SIZE * 2 * (j & 1)) * (j > 1 ? 1 : -1);

		//check attacks the player at this angle
		if (Vec2Distance(vtemp, player_pos) < SPRITE_SIZE) {

			attacks_player[i] = 1;

			//look at the player
			mob_look_at_rotation(m, j);

			return 2;
		}

		//check other mobs
		for (k = 0; k < MAX_MOBS; k++) {

			if (!mobs[k].sprite[0]) {

				//mob inactive
				continue;
			}

			if (Vec2Distance(vtemp, mobs[k].sprite[0]->position) < SPRITE_SIZE) {

				//another mob is on that tile
				goto next_angle;
			}
			//if lerp vector is set and the distance is too small... (another mob will take this tile)
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

			//this tile is available
			available_angles[j] = 1;
			no_angles = 0;
		}

	next_angle:
		continue;
	}

	//mob can't go anywhere
	if (no_angles) {

		return 0;
	}

	//pick a random available angle
	do {
		random = Random(0, 3);
	} while (!available_angles[random]);

	//calculate lerp data
	Vec2Copy(m->sprite[0]->position, lerp_starts[i]);
	Vec2Copy(m->sprite[0]->position, lerp_ends[i]);

	lerp_ends[i][VEC_X] += (SPRITE_SIZE * 2 * !(random & 1)) * (random > 0 ? -1 : 1);
	lerp_ends[i][VEC_Y] += (SPRITE_SIZE * 2 * (random & 1)) * (random > 1 ? 1 : -1);
	lerp_max_msecs[i] = 1; //temporary set for other mob checks

	*rand_out = random;

	return 1;
}

/*
* Determines behaviour of each active mob.
*/
void calculate_mob_destinations(void) {

	int current, i;
	int direction = 0;
	int any_attacks = 0;

	//clear all behaviour arrays
	memset(&lerp_starts, 0, sizeof(vec2_t) * MAX_MOBS);
	memset(&lerp_ends, 0, sizeof(vec2_t) * MAX_MOBS);
	memset(&lerp_msecs, 0, sizeof(int) * MAX_MOBS);
	memset(&lerp_max_msecs, 0, sizeof(int) * MAX_MOBS);
	memset(&attacks_player, 0, sizeof(int) * MAX_MOBS);

	for (i = 0; i < MAX_MOBS; i++) {

		if (!mobs[i].sprite[0] || !mobs[i].sprite[1]|| !mobs[i].sprite[2]	//no sprite?
			|| (mobs[i].sprite[0]->skip_render == 1 &&						//inactive?
			mobs[i].sprite[1]->skip_render == 1 && 
			mobs[i].sprite[2]->skip_render == 1)) {

			//mob inactive
			continue;
		}

		if (mobs[i].type == MAP_MOB_SLIME) {

			//slime moves randomly
			current = random_mob_destination(&mobs[i], i, &direction);
		}
		else
		{
			//goblin follows the player
			current = to_player_mob_destination(&mobs[i], i, &direction);
		}

		if (current == 2) { //2 == attack

			any_attacks = 1;
		}

		if (current != 1) { //1 == move

			continue;
		}

		//make mobs move faster than the player (slow movement ruins the game "dynamics"...)
		lerp_max_msecs[i] = (int)((1000 / (MOVE_SPEED * 8)) * SPRITE_SIZE * 2);

		//make mob look at the direction
		mob_look_at_rotation(&mobs[i], direction);
	}

	//start attacks
	if (any_attacks) {

		all_mobs_attack();

	}

	//start waiting for attacks to end in order to perform move
	glutTimerFunc(TICK_MSEC, lerp_mobs_wait_for_attack, 0);
}

/*
* Callback that waits for player movement to end. After that happens it starts mob behaviours calculation.
*/
void wait_for_player(int value) {

	if (is_player_move) { //still moving, wait anoher tick

		glutTimerFunc(TICK_MSEC, wait_for_player, value);
	}
	else
	{
		calculate_mob_destinations();
	}
}

/*
* Starts mobs' turn. Executed after player movement starts.
*/
void mobs_move(void) {

	is_mob_move = 1;

	glutTimerFunc(TICK_MSEC, wait_for_player, 0);
}

/*
* Calculates damage received by the mob and triggers a kill if necessary.
*/
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

		//refresh stats texts
		mob_update_texts(mob);
	}
}