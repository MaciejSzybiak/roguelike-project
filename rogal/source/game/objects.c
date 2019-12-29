/*
* This file contains action handlers for interactable tiles in the game world.
*/

#include "game.h"
#include "particles.h"
#include "ui.h"

/*
* Normal door click action.
*/
void door_action(sprite_t *s) {

	//open the door
	s->current_frame = 2;
	s->collision_mask = COLLISION_FLOOR;
	s->render_layer = RENDER_LAYER_FLOOR;
	s->action = NULL;

	//recalculate visibility
	recalculate_sprites_visibility();
}

/*
* Locked door click action.
*/
void locked_door_action(sprite_t *s) {

	char text[256];
	color3_t c;
	int mob_count = alive_mobs_count();

	if (s->current_frame == 2) {

		//already unlocked -> open the door completely
		s->current_frame = 3;
		s->collision_mask = COLLISION_FLOOR;
		s->render_layer = RENDER_LAYER_FLOOR;
		s->action = NULL;
		recalculate_sprites_visibility();
	}
	else if (!mob_count) {

		//all mobs are dead and the door gets unlocked
		s->current_frame = 2;
	}
	else
	{
		//there is still mobs alive. Display a message on the screen with alive mob count.
		snprintf(text, 256, "The door won't open. There is still %d creature%s alive.", mob_count, (mob_count > 1 ? "s" : ""));
		Color3Orange(c);
		display_message(text, DEFAULT_MESSAGE_MSEC, c);
	}
}

/*
* Chest click action.
*/
void chest_action(sprite_t *s) {

	char text[64];
	color3_t c;

	//only act if not yet opened
	if (s->current_frame == 1) {

		//chest becomes a normal floor with a different texture frame
		s->current_frame = 2; //frame 2 is an image of the open chest
		s->collision_mask = COLLISION_FLOOR;
		s->render_layer = RENDER_LAYER_FLOOR;
		s->action = NULL;

		//if data is not present add health. else add armor
		if (!s->object_data) {

			Color3Red(c);
			increase_base_stat(1, 1);
		}
		else
		{
			Color3Green(c);
			increase_base_stat(0, 1);

			//clear object data
			free(s->object_data);
			s->object_data = NULL;
		}

		//generate particles
		make_chest_particles(s->position, c[0], c[1], c[2]);

		//display a message
		Color3Orange(c);
		snprintf(text, 64, "The chest opens to reveal its secret.");
		display_message(text, DEFAULT_MESSAGE_MSEC, c);
	}
}