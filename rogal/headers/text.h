#ifndef TEXT_H
#define TEXT_H

#include "shared.h"

#define MAX_STRINGS				64
#define LETTER_SPACING_SCALE	0.7f

#define ANCHOR_LEFT				0
#define ANCHOR_CENTER			1
#define ANCHOR_RIGHT			2

typedef struct {
	int			active;

	int			length;
	char		*text;
	sprite_t	**sprites;

	vec2_t		position;
	float		scale;

	color3_t	color;

	int			anchor;

	int			render_layer;
	int			collision_mask;

	void		*object_data;
	size_t		data_size;

	void		(*action)(sprite_t *s);
} text_t;

void delete_text(text_t *t);
text_t *new_text(void);
void update_text_properties(text_t *t);
void set_text(text_t *t, char *string);
void hide_text(text_t *t);
void enable_text(text_t *t);

#endif // !TEXT_H