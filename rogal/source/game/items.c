#include "game.h"
#include "player.h"
#include <string.h>

#define MIN_HEALTH_VAL	2
#define MAX_HEALTH_VAL	7

#define MIN_ARMOR_VAL	2
#define MAX_ARMOR_VAL	5

#define MIN_WEAPON_VAL	2
#define MAX_WEAPON_VAL	7

item_t map_items[MAX_ITEMS + 1];

//actions
void weapon_pickup_action(sprite_t *s);
void armor_pickup_action(sprite_t *s);
void health_pickup_action(sprite_t *s);

void delete_item(item_t *item) {

	if (!item) {

		d_printf(LOG_WARNING, "%s: NULL item passed as the argument\n", __func__);
		return;
	}

	if (item->sprite) {

		delete_sprite(item->sprite);
	}
	
	memset(item, 0, sizeof(item_t));
}

item_t *new_item(void) {

	for (int i = 0; i < (MAX_ITEMS + 1); i++) {

		if (map_items[i].sprite == NULL) {

			return &map_items[i];
		}
	}
	d_printf(LOG_ERROR, "%s: max items exceeded!\n", __func__);
	return &map_items[MAX_ITEMS - 1];
}

item_t *find_item_for_sprite(sprite_t *s) {

	for (int i = 0; i < (MAX_ITEMS + 1); i++) {

		if (map_items[i].sprite == s) {

			return &map_items[i];
		}
	}
	d_printf(LOG_ERROR, "%s: item not found!\n", __func__);
	return NULL;
}

void randomize_item(item_t *item) {

	color3_t color;
	int value = 0;
	float rarity = 0.f;

	switch (item->item_category)
	{
		case ITEM_CATEGORY_ARMOR:
			value = Random(MIN_ARMOR_VAL, MAX_ARMOR_VAL);
			rarity = (float)value / MAX_ARMOR_VAL;
			break;
		case ITEM_CATEGORY_HEALTH:
			value = Random(MIN_HEALTH_VAL, MAX_HEALTH_VAL);
			rarity = (float)value / MAX_HEALTH_VAL;
			break;
		case ITEM_CATEGORY_WEAPON:
			value = Random(MIN_WEAPON_VAL, MAX_WEAPON_VAL);
			rarity = (float)value / MAX_WEAPON_VAL;
			break;
	}

	if (rarity > 0.8f) {

		Color3ItemVeryRare(color);
	}
	else if (rarity > 0.5f)
	{
		Color3ItemRare(color);
	}
	else if (rarity > 0.25f)
	{
		Color3ItemUncommon(color);
	}
	else
	{
		Color3ItemCommon(color);
	}

	item->modifier_value = value;
	Color3Copy(color, item->sprite->color);
	Color3Copy(color, item->rarity_color);
}

void generate_items(void) {

	texname tname;
	item_t *item;
	sprite_t *s;
	int category = 0;
	void (*action)(sprite_t *s);

	for (int x = 0; x < MAP_SIZE; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {

			action = NULL;
			switch (map_contents[x][y])
			{
				case MAP_ITEM_SHIELD:
					tname = SHIELD;
					category = ITEM_CATEGORY_ARMOR;
					action = armor_pickup_action;
					break;
				case MAP_ITEM_SWORD:
					category = ITEM_CATEGORY_WEAPON;
					tname = RandomBool ? SWORD : AXE;
					action = weapon_pickup_action;
					break;
				case MAP_ITEM_POTION_HP:
					category = ITEM_CATEGORY_HEALTH;
					action = health_pickup_action;
					tname = POTION_HP;
					break;
				case MAP_NOTHING:
				default:
					continue;
			}

			item = new_item();
			item->item_type = map_contents[x][y];
			item->item_category = category;

			//add sprite
			s = new_sprite();

			s->tex_id = get_texture_id(tname);
			s->framecount = get_texture_framecount(tname);
			s->render_layer = get_texture_render_layer(tname);
			s->frame_msec = get_texture_frametime(tname);
			s->collision_mask = COLLISION_ITEM;
			s->position[VEC_X] = (x * SPRITE_SIZE * 2) - MAP_OFFSET;
			s->position[VEC_Y] = (y * SPRITE_SIZE * 2) - MAP_OFFSET;

			s->animation_pause = 0;
			s->skip_render = 0;

			if (action) {

				s->action = action;
			}

			item->sprite = s;

			randomize_item(item);
		}
	}
}

void init_items(void) {

	for (int i = 0; i < MAX_ITEMS + 1; i++) {

		if (map_items[i].sprite) {

			//skip player's current weapon
			if (is_ingame && !is_player_dead && player.weapon == &map_items[i]) {

				continue;
			}

			delete_item(&map_items[i]);
			memset(&map_items[i], 0, sizeof(item_t));
		}
	}

	//memset(&map_items, 0, sizeof(item_t) * (MAX_ITEMS + 1));
	generate_items();
}

//ACTIONS

void weapon_pickup_action(sprite_t *s) {

	item_t *item = find_item_for_sprite(s);

	d_printf(LOG_TEXT, "%s\n", __func__);

	if (!item) {

		return;
	}

	//pickup
	if (item->item_category != ITEM_CATEGORY_WEAPON) {

		d_printf(LOG_WARNING, "%s: incorrect item category: %d\n", __func__, item->item_category);
		return;
	}

	player_pickup_weapon(item);
}

void armor_pickup_action(sprite_t *s) {

	item_t *item = find_item_for_sprite(s);

	d_printf(LOG_TEXT, "%s\n", __func__);

	if (!item) {

		return;
	}

	add_armor(item->modifier_value);
	delete_item(item);
}

void health_pickup_action(sprite_t *s) {

	item_t *item = find_item_for_sprite(s);

	d_printf(LOG_TEXT, "%s\n", __func__);

	if (!item) {

		return;
	}

	add_health(item->modifier_value);
	delete_item(item);
}