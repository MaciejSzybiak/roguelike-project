#include "text.h"
#include <ctype.h>
#include <string.h>

text_t strings[MAX_STRINGS];

//removes all letter sprites allocated by the text
void free_text_sprites(text_t *t) {

	sprite_t *s;
	if (t->sprites) {

		for (int i = 0; i < t->length; i++) {

			s = *(t->sprites + i);
			if (s) {

				delete_sprite(s);
			}
		}

		free(t->sprites);
	}
	t->sprites = NULL;
}

//removes the text
void delete_text(text_t *t) {

	free_text_sprites(t);

	free(t->text);

	memset(t, 0, sizeof(text_t));
}

//sets and returns a new text
text_t *new_text(void) {

	//find an empty text slot
	for (int i = 0; i < MAX_STRINGS; i++) {

		if (!strings[i].active) {

			strings[i].active = 1;
			strings[i].scale = 1.f;
			strings[i].anchor = ANCHOR_CENTER;
			Color3White(strings[i].color);
			strings[i].render_layer = RENDER_LAYER_UI;
			strings[i].collision_mask = COLLISION_IGNORE;
			return &strings[i];
		}
	}
	d_printf(LOG_ERROR, "%s: max strings exceeded!\n", __func__);
	return &strings[MAX_STRINGS - 1];
}

//hides text by disabling its sprites
void hide_text(text_t *t) {

	sprite_t *s;

	for (int i = 0; i < t->length; i++) {

		s = *(t->sprites + i);

		s->skip_render = 1;
	}
}

//activates all text sprites
void enable_text(text_t *t) {

	sprite_t *s;

	for (int i = 0; i < t->length; i++) {

		s = *(t->sprites + i);

		s->skip_render = 0;
	}
}

//recalculates text properties and applies them to sprites
void update_text_properties(text_t *t) {

	sprite_t *s;
	float scale = t->scale * SPRITE_SIZE * 2;		//letter size
	float spacing = scale * LETTER_SPACING_SCALE;	//spacing between sprite positions
	float x0 = 0;	//the x offset applied because of anchor setting
	vec2_t position;

	Vec2Zero(position);

	//find the anchor position
	switch (t->anchor)
	{
		case ANCHOR_RIGHT:
			x0 = -(t->length * spacing);
			break;
		case ANCHOR_CENTER:
			x0 = -(t->length * spacing / 2) + (spacing / 2);
			break;
		case ANCHOR_LEFT:
		default:
			break;
	}

	//go through each letter
	for (int i = 0; i < t->length; i++) {

		s = *(t->sprites + i);

		//scale
		s->scale_x = scale;
		s->scale_y = scale;

		//color
		Color3Copy(t->color, s->color);

		//position
		position[VEC_X] = x0 + (spacing * i) + t->position[VEC_X];
		position[VEC_Y] = t->position[VEC_Y];
		Vec2Copy(position, s->position);

		//render layer and collision mask
		s->render_layer = t->render_layer;
		s->collision_mask = t->collision_mask;

		//action
		if (t->action) {

			s->action = t->action;
		}
	}
}

//generate letter sprites for the text
void set_text_sprites(text_t *t) {

	sprite_t *s;
	int char_num;

	t->sprites = malloc(sizeof(sprite_t *) * t->length);

	if (!t->sprites) {

		out_of_memory_error(__func__);
		return;
	}

	for (int i = 0; i < t->length; i++) {

		s = new_sprite();
		
		s->tex_id = get_texture_id(FONT);
		s->framecount = get_texture_framecount(FONT);
		s->animation_pause = 1;
		
		char_num = (int)(toupper(*(t->text + i)));

		if (char_num < 32 || char_num > 95) {

			d_printf(LOG_WARNING, "%s: char '%c' is out of range.\n", __func__, (char)char_num);
			char_num = 32;
		}

		s->current_frame = char_num - 31;

		*(t->sprites + i) = s;
	}
	update_text_properties(t);
}

void set_text(text_t *t, char *string) {

	free_text_sprites(t);

	t->text = string;
	t->length = strlen(string);

	set_text_sprites(t);
}
