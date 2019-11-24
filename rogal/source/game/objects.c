#include "game.h"

void door_action(sprite_t *s) {

	//opens the door
	s->current_frame = 2;
	s->collision_mask = COLLISION_FLOOR;
	s->render_layer = RENDER_LAYER_FLOOR;
	s->action = NULL;

	//recalculate visibility
	recalculate_sprites_visibility();
}

void locked_door_action(sprite_t *s) {

	if (s->current_frame == 2) {

		s->current_frame = 3;
		s->collision_mask = COLLISION_FLOOR;
		s->render_layer = RENDER_LAYER_FLOOR;
		s->action = NULL;
		recalculate_sprites_visibility();
	}
	else if (!alive_mobs_count()) {

		s->current_frame = 2;
	}
}