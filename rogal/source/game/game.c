#include "game.h"
#include "camera.h"
#include "player.h"
#include "raycast.h"
#include "ui.h"
#include "particles.h"
#include <GL/glut.h>

int is_ingame = 0;
int is_paused = 0;

int old_frame_msec = 0;
int frame_msec = 0;

void restart_game(void);

void on_player_action(sprite_t *s, int mouse_key) {

	if (is_player_move || is_mob_move) {

		//the player is currently moving
		return;
	}

	//calculate distance
	float dist = Vec2Distance(player.sprite[0]->position, s->position);
	int look_direction = direction_to_tile(s->position);

	//has enemy? attack with the action key
	if (dist <= SPRITE_SIZE * 2 && (s->collision_mask & COLLISION_MOB) && mouse_key == GLUT_LEFT_BUTTON) {

		//can attack
		face_direction(look_direction);
		mob_t *m = find_mob(s->position);

		if (m) {

			attack_mob(m);

			//run AI
			mobs_move();
		}
		return;
	}

	//items and action triggers only work at close distancess
	if (dist <= SPRITE_SIZE * 2) { // * 3 allows diagonal actions

		if(s->action && mouse_key == GLUT_LEFT_BUTTON)
		{
			//item pickup only when standing on it?
			if (s->render_layer == RENDER_LAYER_ITEM && dist > SPRITE_SIZE) {

				return;
			}
			//look at the target
			face_direction(look_direction);

			//do tile action
			s->action(s);

			//run AI
			mobs_move();
		}
		else if(s->collision_mask & (COLLISION_FLOOR | COLLISION_ITEM) && dist > SPRITE_SIZE && mouse_key == GLUT_RIGHT_BUTTON)
		{
			//look at the target
			face_direction(look_direction);

			//walk to the tile
			walk_to_tile(s->position, dist);

			//run AI
			mobs_move();
		}
	}
}

void mouse_click_event(int button, int state, int x, int y) {

	if (!is_player_move && !is_mob_move && is_player_dead) {

		restart_game();
		return;
	}

	if (is_player_move || state != GLUT_UP || (button != GLUT_LEFT_BUTTON && button != GLUT_RIGHT_BUTTON)) {
		return;
	}

	sprite_t *s = screen_to_world_raycast(x, y, COLLISION_ALL);

	if (s) {

		if (s->collision_mask == COLLISION_PLAYER) {

			//skip player click
			return;
		}

		if (s->action && s->collision_mask & COLLISION_UI) {

			//interact with UI
			(s->action)(s);
		}
		else if (s->collision_mask & COLLISION_PLAYER_ACTION)
		{
			//preform an action
			on_player_action(s, button);
		}
	}
}

//WSAD simulates mouse click on a sprite towards a rotation
void simulate_mouse_click(int rotation) {

	vec2_t dest;
	int x, y;

	Vec2Copy(player.sprite[0]->position, dest);

	dest[VEC_X] += (SPRITE_SIZE * 2 * !(rotation & 1)) * (rotation > 0 ? -1 : 1);
	dest[VEC_Y] += (SPRITE_SIZE * 2 * (rotation & 1)) * (rotation > 1 ? -1 : 1);

	world_to_screen_coordinates(dest, &x, &y);

	mouse_click_event(GLUT_RIGHT_BUTTON, GLUT_UP, x, y);
}

void keyboard_press_event(unsigned char key, int x, int y) {

	if (!is_player_move && !is_mob_move && is_player_dead) {

		restart_game();
		return;
	}

	if (!is_ingame) {

		return;
	}

	//esc key - pause menu
	if (key == (char)27) {

		if (is_paused) {

			toggle_main_menu(0);
		}
		else
		{
			toggle_main_menu(1);
		}
		is_paused = !is_paused;
	}

	if (is_paused) {

		return;
	}

	//e to pickup item
	if (key == 'e' || key == 'E') {

		//simulate a click on the player
		vec2_t dest;

		Vec2Copy(player.sprite[0]->position, dest);

		world_to_screen_coordinates(dest, &x, &y);

		mouse_click_event(GLUT_LEFT_BUTTON, GLUT_UP, x, y);
	}
	//WSAD simulates mouse click on a tile
	else if (key == 'd' || key == 'D') {

		simulate_mouse_click(ROTATION_0);
	}
	else if (key == 's' || key == 'S') {

		simulate_mouse_click(ROTATION_90);
	}
	else if (key == 'a' || key == 'A') {

		simulate_mouse_click(ROTATION_180);
	}
	else if (key == 'w' || key == 'W') {

		simulate_mouse_click(ROTATION_270);
	}
}

void special_press_event(int key, int x, int y) {

	//d_printf(LOG_TEXT, "%s: special key: %d at %dx%d\n", __func__, key, x, y);
	UNUSED_VARIABLE(x);
	UNUSED_VARIABLE(y);

	if (!is_player_move && !is_mob_move && is_player_dead) {

		restart_game();
		return;
	}

	if (is_paused) {

		return;
	}

	//arrow keys press
	if (key == GLUT_KEY_RIGHT) {

		simulate_mouse_click(ROTATION_0);
	}
	else if (key == GLUT_KEY_DOWN) {

		simulate_mouse_click(ROTATION_90);
	}
	else if (key == GLUT_KEY_LEFT) {

		simulate_mouse_click(ROTATION_180);
	}
	else if (key == GLUT_KEY_UP) {

		simulate_mouse_click(ROTATION_270);
	}
}

void update_sprite_animations(void) {

	sprite_t *current = sprite_head();

	while (current)
	{
		if (current->framecount > 1 && !current->animation_pause) {

			if (current->frame_msec_acc >= current->frame_msec) {

				//go to next anim frame
				current->current_frame = current->current_frame == current->framecount ? 1 : current->current_frame + 1;
				current->frame_msec_acc = LOGIC_MSEC;
			}
			else
			{
				current->frame_msec_acc += LOGIC_MSEC;
			}
		}

		current = current->next;
	}
}

//logic loop entry point
void logic_frame(int value) {

	int elapsed_time = glutGet(GLUT_ELAPSED_TIME);
	frame_msec = elapsed_time - value;
	old_frame_msec = elapsed_time;

	//run animations
	update_sprite_animations();

	//run particles
	run_particles(frame_msec);

	glutTimerFunc(TICK_MSEC, logic_frame, elapsed_time);
}

void next_level_action(sprite_t *s) {

	UNUSED_VARIABLE(s);

	d_spacer();

	generate_map();
	init_mobs();
	init_items();

	player_next_level();
}

void restart_game(void) {

	vec2_t v;
	Vec2Zero(v);

	init_particles();
	d_spacer();

	disable_message_text();

	generate_map();
	init_mobs();
	init_items();

	is_player_dead = 0;
	init_player();

	set_camera_position(v);
}

void init_game(void) {

	//log a debug spacer :)
	init_particles();
	d_spacer();

	generate_map();
	init_mobs();
	init_items();

	init_player();

	is_ingame = 1;
}