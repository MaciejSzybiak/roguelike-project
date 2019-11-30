#include "ui.h"
#include "options.h"
#include "window.h"
#include "game.h"
#include "camera.h"
#include "player.h"
#include <string.h>
#include <GL/glut.h>

static text_t *main_menu[3];

static text_t *hud_texts[3];
static sprite_t *hud_sprites[3];

static sprite_t *menu_background;

static text_t *message_text;
static int message_call_time;

void toggle_hud(int enabled) {

	for (int i = 0; i < 3; i++) {

		if (enabled) {

			enable_text(hud_texts[i]);
		}
		else
		{
			hide_text(hud_texts[i]);
		}
		hud_sprites[i]->skip_render = !enabled;
	}
}

//toggles menu buttons as well as the background
//int enabled: toggle on or off?
void toggle_main_menu(int enabled) {

	for (int i = 0; i < 3; i++) {

		if (enabled) {

			//FIXME: do this only when necessary?
			if (is_ingame) { //if the menu is shown during a game make the start button say CONTINUE

				set_text(main_menu[0], "CONTINUE");
			}
			else
			{
				set_text(main_menu[0], "PLAY");
			}

			enable_text(main_menu[i]);

			disable_message_text();

			toggle_options(0);
		}
		else
		{
			hide_text(main_menu[i]);
		}
	}

	if (!is_options) {

		menu_background->skip_render = !enabled;
	
		//toggle the HUD
		toggle_hud(!enabled);
	}

}

void disable_message_text(void) {

	hide_text(message_text);
}

void disable_message_callback(int value) {

	if (value != message_call_time) {

		//message overwritten by a new message, don't disable using this callback
		return;
	}

	disable_message_text();
}
 
void display_message(char *message, int msec, color3_t color) {

	Color3Copy(color, message_text->color);

	set_text(message_text, message);
	enable_text(message_text);

	message_call_time = glutGet(GLUT_ELAPSED_TIME);

	if (msec > 0) {

		glutTimerFunc(msec, disable_message_callback, message_call_time);
	}
}

void action_play_click(sprite_t *s) {

	UNUSED_VARIABLE(s);

	d_printf(LOG_INFO, "%s\n", __func__);

	//hide the menu and unpause game state
	toggle_main_menu(0);
	is_paused = 0;

	if (!is_ingame) {

		//initialize game components
		init_game();
	}
}

void action_options_click(sprite_t *s) {

	UNUSED_VARIABLE(s);

	//TODO
	d_printf(LOG_INFO, "%s\n", __func__);

	is_options = 1;

	toggle_main_menu(0);
	toggle_options(1);

	menu_background->skip_render = 0;
}

void action_quit_click(sprite_t *s) {

	UNUSED_VARIABLE(s);

	d_printf(LOG_INFO, "%s\n", __func__);

	//destroy the window
	destroy_window();

	//and kill the application
	exit(EXIT_SUCCESS);
}

//creates the main menu buttons
void generate_main_menu(void) {

	//set text structs for 3 buttons
	for (int i = 0; i < 3; i++) {

		main_menu[i] = new_text();
		main_menu[i]->scale = MMENU_SIZES;
		main_menu[i]->anchor = ANCHOR_CENTER;
		main_menu[i]->collision_mask = COLLISION_UI;
		main_menu[i]->render_layer = RENDER_LAYER_UI;
		Vec2Zero(main_menu[i]->position);
	}

	main_menu[MMENU_PLAY]->position[VEC_Y] = MMENU_SIZES + UI_SPACING;  //PLAY goes above the screen center
	main_menu[MMENU_QUIT]->position[VEC_Y] = -MMENU_SIZES - UI_SPACING; //QUIT goes lower than the screen center

	//set actions
	main_menu[MMENU_PLAY]->action = action_play_click;
	main_menu[MMENU_OPTIONS]->action = action_options_click;
	main_menu[MMENU_QUIT]->action = action_quit_click;

	//set texts (applies all text properites by default)
	set_text(main_menu[MMENU_PLAY], "PLAY");
	set_text(main_menu[MMENU_OPTIONS], "OPTIONS");
	set_text(main_menu[MMENU_QUIT], "QUIT");
}

void generate_menu_background(void) {

	//menu background is simply a big black sprite with no texture
	//it's meant to block anything below ui from being visible and to serve as a raycast blocker

	menu_background = new_sprite();

	Vec2Zero(menu_background->position);
	Color3Black(menu_background->color);

	menu_background->scale_x = 100.f;
	menu_background->scale_y = 100.f;

	menu_background->render_layer = RENDER_LAYER_UI_BG;
	menu_background->collision_mask = COLLISION_UI;
}

void hud_update_health(void) {

	char text[16];

	snprintf(text, 16, "%d/%d", player.stats.health, player.stats.max_health);

	set_text(hud_texts[HUD_HP], text);
}

void hud_update_armor(void) {

	char text[16];

	snprintf(text, 16, "%d/%d", player.stats.armor, MAX_ARMOR + player.stats.armor_modifier);

	set_text(hud_texts[HUD_ARMOR], text);
}

void hud_update_dmg(void) {

	char text[16];

	snprintf(text, 16, "%d", player.stats.attack_damage);

	set_text(hud_texts[HUD_DMG], text);
}

void refresh_hud(int value) {

	vec2_t text_pos;
	vec2_t *world_pos;
	sprite_t *s;
	float x_pos = -HUD_SPACING;

	UNUSED_VARIABLE(value);

	//find the ui space position of the text
	text_pos[VEC_X] = 0.f;
	text_pos[VEC_Y] = 0.05f;

	world_pos = viewport_to_world_pos(text_pos, 1);

	//only refresh when in game
	if (is_ingame) {

		for (int i = 0; i < 3; i++) {

			//set text
			Vec2Copy(*world_pos, hud_texts[i]->position);
			hud_texts[i]->position[VEC_X] = x_pos;
			update_text_properties(hud_texts[i]);

			//set sprite
			s = hud_sprites[i];
			Vec2Copy(hud_texts[i]->position, s->position);
			s->position[VEC_Y] += 0.6f;

			//HUD element 1 is at X=0, element 0 goes to the left and el. 2 to the right
			x_pos += HUD_SPACING;
		}
	}

	glutTimerFunc(TICK_MSEC, refresh_hud, 0);
}

void generate_hud(void) {

	sprite_t *s;
	texname tnames[3] = { ICON_HP, ICON_ARMOR, ICON_DMG };

	//make three texts
	for (int i = 0; i < 3; i++) {

		hud_texts[i] = new_text();
		hud_texts[i]->scale = 0.5f;
		hud_texts[i]->anchor = ANCHOR_CENTER;
		hud_texts[i]->collision_mask = COLLISION_IGNORE;
		hud_texts[i]->render_layer = RENDER_LAYER_UI;
		Vec2Zero(hud_texts[i]->position);
	}

	Color3UIRed(hud_texts[HUD_HP]->color);
	set_text(hud_texts[HUD_HP], "12/34");
	hide_text(hud_texts[HUD_HP]);

	Color3UIGreen(hud_texts[HUD_ARMOR]->color);
	set_text(hud_texts[HUD_ARMOR], "12/34");
	hide_text(hud_texts[HUD_ARMOR]);

	Color3UIOrange(hud_texts[HUD_DMG]->color);
	set_text(hud_texts[HUD_DMG], "123");
	hide_text(hud_texts[HUD_DMG]);

	//make the icons
	for (int i = 0; i < 3; i++) {

		s = new_sprite();
		s->tex_id = get_texture_id(tnames[i]);
		s->render_layer = RENDER_LAYER_UI;
		s->collision_mask = COLLISION_IGNORE;
		s->framecount = get_texture_framecount(tnames[i]);
		s->skip_render = 1;

		s->scale_x = 0.5f;
		s->scale_y = 0.5f;
		Vec2Zero(s->position);

		hud_sprites[i] = s;
	}

	//make the message box
	message_text = new_text();
	message_text->scale = 0.35f;
	message_text->anchor = ANCHOR_CENTER;
	message_text->collision_mask = COLLISION_IGNORE;
	message_text->render_layer = RENDER_LAYER_UI;
	Vec2Zero(message_text->position);
	message_text->position[VEC_Y] += 3.f;
	Color3Orange(message_text->color);
	set_text(message_text, "test");
	hide_text(message_text);
}

//initializes the UI
void generate_ui(void) {

	generate_main_menu();
	generate_menu_background();
	generate_hud();

	//generate options
	generate_options_texts();
	toggle_options(0);

	//set HUD refresh callback
	glutTimerFunc(TICK_MSEC, refresh_hud, 0);
}