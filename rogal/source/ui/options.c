/*
* This file is responsible for controlling options submenu.
* The list of options can be extended so that new options can
* be easily added. Only ON-OFF (toggle) options are supported.
*/

#include "options.h"
#include "window.h"
#include "particles.h"

void set_fullscreen(int value);
void set_particles(int value);

//options list
static option_t options[] = {

	//name			action			initial value	text_t
	{ "fullscreen", set_fullscreen, 0,				NULL },
	{ "particles",	set_particles,	1,				NULL }
};

//----------
//   ACTIONS
//----------

/*
* Action for particle toggle.
*/
void set_particles(int value) {

	are_particles_enabled = value; //this variable toggles generation of new particles
}

/*
* Action for fullscreen toggle.
*/
void set_fullscreen(int value) {

	if (value) {

		make_fullscreen();
	}
	else
	{
		restore_windowed();
	}
}

/*
* Updates option's text to match the current setting.
*/
void set_option_text_to_value(option_t *o) {

	char buffer[128];

	snprintf(buffer, 128, "%s: %s", o->name, (o->value ? "ON" : "OFF")); //make the text say ON or OFF
	set_text(o->text, buffer);
}

/*
* Action for every sprite that belongs to an option string.
*/
void option_click(sprite_t *s) {

	int index = *(int *)s->object_data; //get options array index from sprite's data

	d_printf(LOG_INFO, "%s: option \"%s\"\n", __func__, options[index].name);

	options[index].value = !options[index].value; //toggle the value

	options[index].on_set(options[index].value); //call the on_set function (option's action)

	//update option's text
	set_option_text_to_value(&options[index]);
}

/*
* Toggles options texts visibility.
*/
void toggle_options(int are_enabled) {

	for (unsigned i = 0; i < CountOf(options); i++) {

		if (are_enabled) {

			enable_text(options[i].text);
		}
		else
		{
			hide_text(options[i].text);
		}
	}
}

/*
* Creates text for every option in options array.
*/
void generate_options_texts(void) {

	int count = CountOf(options);
	text_t *t;
	float y_offset = (count / 2.f) * OPTIONS_TEXT_SCALE; //vertical offset between the texts
	float y_first = -(y_offset * count / 2); //how high does the list start

	//generate texts as a vertical, centered list
	for (int i = 0; i < count; i++) {

		t = new_text();
		t->scale = OPTIONS_TEXT_SCALE;
		t->anchor = ANCHOR_CENTER;
		t->collision_mask = COLLISION_UI;
		t->render_layer = RENDER_LAYER_UI;

		Vec2Zero(t->position);
		t->position[VEC_Y] -= y_first + y_offset * i; //move down by "i" offsets

		t->action = option_click; //action for text's sprites

		//add data to each sprite
		t->data_size = sizeof(int);
		t->object_data = malloc(sizeof(int));

		*(int *)t->object_data = i; //set data to index of the current option

		options[i].text = t; //keep track of the option's text (for updating it)

		set_option_text_to_value(&options[i]); //set text
	}
}