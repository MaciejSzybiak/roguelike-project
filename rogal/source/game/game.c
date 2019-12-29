/*
* This file is responsible for general management of the game state as well as
* responding to user inputs.
* 
* User inputs are received using GLUT library callbacks and then processed if
* necessary.
*/

#include "game.h"
#include "camera.h"
#include "player.h"
#include "raycast.h"
#include "ui.h"
#include "particles.h"
#include <GL/glut.h>
#include "options.h"

//game state
int is_ingame = 0;
int is_paused = 0;
int is_options = 0;

//frametime
int frame_msec = 0;

//level counter
int current_level = 1;

void restart_game(void);

/*
* Manages the response to player actions. Before that all actions are converted to mouse clicks on
* a certain sprite which is then used to determine the response. Each successfull action ends with
* the mobs making their moves.
* 
* Parameters:
* s - clicked sprite
* mouse_key - mouse key that was clicked:
*		> GLUT_LEFT_BUTTON - interaction (attack mob, open door, pick up item)
*		> GLUT_RIGHT_BUTTON - walk only
*/
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
	if (dist <= SPRITE_SIZE * 2) {

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

/*
* GLUT callback for mouse click events. Also executed when mouse click is simulated from keyboard key presses.
* 
* Parameters:
* button - the mouse button that was clicked
* state - information whether the button was just pressed, is being held down or just lifted up
* x, y - mouse position on the window
*/
void mouse_click_event(int button, int state, int x, int y) {

	//the player is dead, restart
	if (!is_player_move && !is_mob_move && is_player_dead) {

		restart_game();
		return;
	}

	//only act when the player isn't moving, button was lifted up and a valid button was pressed
	if (is_player_move || state != GLUT_UP || (button != GLUT_LEFT_BUTTON && button != GLUT_RIGHT_BUTTON)) {

		return;
	}

	//find the sprite that was clicked
	sprite_t *s = screen_to_world_raycast(x, y, COLLISION_ALL);

	//something was clicked
	if (s) {

		if (s->collision_mask == COLLISION_PLAYER) {

			//clicked on the player, no action
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

/*
* Converts keypresses to mouse clicks according to the rotation provided.
* 
* Parameters:
* rotation - rotation of a direction vector from the player. The end of that vector
*			 is the simulated click.
*/
void simulate_mouse_click(int rotation) {

	vec2_t dest;
	int x, y;

	//start at the player position
	Vec2Copy(player.sprite[0]->position, dest);

	//add 1 in the correct direction
	dest[VEC_X] += (SPRITE_SIZE * 2 * !(rotation & 1)) * (rotation > 0 ? -1 : 1);
	dest[VEC_Y] += (SPRITE_SIZE * 2 * (rotation & 1)) * (rotation > 1 ? -1 : 1);

	//convert that position to screen coordinates
	world_to_screen_coordinates(dest, &x, &y);

	//run mouse click event
	mouse_click_event(GLUT_RIGHT_BUTTON, GLUT_UP, x, y);
}

/*
* GLUT callback for keyboard events.
* 
* Parameters:
* key - pressed key character
* x, y - mouse position when the key was pressed
*/
void keyboard_press_event(unsigned char key, int x, int y) {

	//the player is dead, restart
	if (!is_player_move && !is_mob_move && is_player_dead) {

		restart_game();
		return;
	}

	if (!is_ingame) {

		//just check if options should toggle
		if (key == (char)27 && is_options) { //char 27 is ESC key

			//go back from options to main menu
			is_options = 0;
			toggle_main_menu(1);
		}
		return;
	}

	//esc key - pause menu
	if (key == (char)27) {

		if (is_paused) {

			//the game is paused
			if (is_options) {

				//go back from options to main menu
				is_options = 0;
				toggle_main_menu(1);

				is_paused = !is_paused; //HACK: don't toggle if this is options switch
			}
			else
			{
				//unpause
				toggle_main_menu(0);
			}
		}
		else
		{
			//pause the game
			toggle_main_menu(1);
		}
		is_paused = !is_paused;
	}

	//other keys aren't processed if the game is paused
	if (is_paused) {

		return;
	}

	//e to pickup item (if the player is standing on it)
	if (key == 'e' || key == 'E') {

		//simulate a click on the player
		vec2_t dest;

		Vec2Copy(player.sprite[0]->position, dest);

		//find screen coordinates
		world_to_screen_coordinates(dest, &x, &y);

		//run click event
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

/*
* GLUT special key press callback.
*
* Parameters:
* key - pressed key code
* x, y - mouse position when the key was pressed
*/
void special_press_event(int key, int x, int y) {

	UNUSED_VARIABLE(x);
	UNUSED_VARIABLE(y);

	//the player is dead, restart
	if (!is_player_move && !is_mob_move && is_player_dead) {

		restart_game();
		return;
	}

	if (is_paused) {

		return;
	}

	//arrow keys also allow the player to move
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

/*
* Updates sprite animation frames and their time accumulators.
*/
void update_sprite_animations(void) {

	sprite_t *current = sprite_head();

	//iterate over each sprite
	while (current)
	{
		//only animate sprites that allow it
		if (current->framecount > 1 && !current->animation_pause) {

			if (current->frame_msec_acc >= current->frame_msec) {

				//go to next anim frame
				current->current_frame = current->current_frame == current->framecount ? 1 : current->current_frame + 1;
				current->frame_msec_acc = LOGIC_MSEC;
			}
			else
			{
				//just add time
				current->frame_msec_acc += LOGIC_MSEC;
			}
		}

		current = current->next;
	}
}

/*
* Timed callback to run various game logic functions. Ideally executed on each frame.
*/
void logic_frame(int value) {

	int elapsed_time = glutGet(GLUT_ELAPSED_TIME);
	frame_msec = elapsed_time - value;

	//run animations
	update_sprite_animations();

	//run particles
	run_particles(frame_msec);

	//register the next call of this callback
	glutTimerFunc(TICK_MSEC, logic_frame, elapsed_time);
}

/*
* Click action for the exit tile. Switches the game to a next dungeon level.
*/
void next_level_action(sprite_t *s) {

	UNUSED_VARIABLE(s);

	d_spacer(); //debug spacer

	generate_map(); //generate new map
	init_mobs();	//reinitialize mobs
	init_items();	//reinitialize items

	//increment level counter
	current_level++;

	//prepare player for the next level
	player_next_level();
}

/*
* Executed when the player dies and clicks a key. Generates a new map
* and clears player's statistics.
*/
void restart_game(void) {

	vec2_t v;
	Vec2Zero(v);

	d_spacer();

	init_particles(); //delete all particles

	disable_message_text();

	//reset level counter
	current_level = 1;

	generate_map(); //make new map
	init_mobs();	//reinitialize mobs
	init_items();	//reinitialize items

	//the player is not dead anymore
	is_player_dead = 0;

	//fully reinitialize the player
	init_player();

	//reset camera position
	set_camera_position(v);
}

/*
* Returns the current level number.
*/
int get_current_level(void) {

	return current_level;
}

/*
* Runs the first initialization of the game.
*/
void init_game(void) {

	//log a debug spacer
	d_spacer();

	//initialize particles
	init_particles();

	generate_map(); //generate the first map
	init_mobs();	//make mobs
	init_items();	//make items

	init_player();	//initialize player

	//set ingame state
	is_ingame = 1;
}