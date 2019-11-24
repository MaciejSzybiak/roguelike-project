#include "game.h"
#include "player.h"
#include "raycast.h"

void set_sprite_invisible(sprite_t *s) {

	if (s->visibility != VIS_HIDDEN) {

		//already seen this sprite
		s->visibility = VIS_DISCOVERED;
		Color3LGray(s->color);
	}
	else
	{
		//invisible
		s->visibility = VIS_HIDDEN;
		Color3Black(s->color);
	}
}

void set_mob_visibility(mob_t *mob, int vis) {

	sprite_t *s;

	for (int i = 0; i < 3; i++) {

		s = mob->sprite[i];

		if (!s) {

			continue;
		}

		s->visibility = vis;
		
		switch (vis)
		{
			case VIS_DISCOVERED: //not really valid for a mob
			case VIS_HIDDEN:
				s->skip_render = 1; //don't render this mob
				break;
			default:
				//visible
				if (mob->look_direction == i) {

					s->skip_render = 0;
				}
				break;
		}
	}
}

void recalculate_item_visibility(void) {

	item_t *item;
	sprite_t *s;
	vec2_t p;
	vec2_t s_offset;
	float dist;
	int intersects;

	for (int j = 0; j < MAX_ITEMS + 1; j++) {

		item = &map_items[j];
		s = item->sprite;

		if (!s) {

			continue;
		}

		dist = Vec2Distance(player.sprite[0]->position, s->position);

		if (dist > VIS_DISTANCE) {

			set_sprite_invisible(s);
		}
		else {

			//check if mob visibility is broken by a sprite
			intersects = 1;
			for (int i = 0; i < 4; i++) {

				//check if any sprite side is visible from player's perspective
				s_offset[VEC_X] = s->position[VEC_X] + ((i % 2) ? SPRITE_SIZE * 0.95f * s->scale_x : 0) * ((i > 1) ? -1 : 1);
				s_offset[VEC_Y] = s->position[VEC_Y] + ((i % 2) ? 0 : SPRITE_SIZE * 0.95f * s->scale_y) * ((i > 1) ? 1 : -1);

				//raycast
				if (!sprite_ray_intersection(player.sprite[0]->position, s_offset, COLLISION_WALL | COLLISION_OBSTACLE, &p)) {

					intersects = 0;
					break;
				}
			}
			if (intersects) {

				set_sprite_invisible(s);
			}
			else
			{
				s->visibility = VIS_VISIBLE;
				Color3Copy(item->rarity_color, s->color);
			}
		}
	}
}

void recalculate_mob_visibility(void) {

	mob_t *mob;
	sprite_t *s;
	vec2_t p;
	vec2_t s_offset;
	float dist;
	int intersects;

	//mobs can only be visible or hidden
	for (int j = 0; j < MAX_MOBS; j++) {

		mob = &mobs[j];
		s = mob->sprite[0];

		if (!s) {

			//unused
			continue;
		}

		//calculate the distance between player and the sprite
		dist = Vec2Distance(player.sprite[0]->position, s->position);

		if (dist > VIS_DISTANCE) {
			
			set_mob_visibility(mob, VIS_HIDDEN);
		}
		else
		{
			//check if mob visibility is broken by a sprite
			intersects = 1;
			for (int i = 0; i < 4; i++) {

				//check if any sprite side is visible from player's perspective
				s_offset[VEC_X] = s->position[VEC_X] + ((i % 2) ? SPRITE_SIZE * 0.95f * s->scale_x : 0) * ((i > 1) ? -1 : 1);
				s_offset[VEC_Y] = s->position[VEC_Y] + ((i % 2) ? 0 : SPRITE_SIZE * 0.95f * s->scale_y) * ((i > 1) ? 1 : -1);

				//raycast
				if (!sprite_ray_intersection(player.sprite[0]->position, s_offset, COLLISION_WALL | COLLISION_OBSTACLE, &p)) {

					intersects = 0;
					break;
				}
			}
			set_mob_visibility(mob, intersects ? VIS_HIDDEN : VIS_VISIBLE);
		}
	}
}

//checks which tiles are currently visible and sets an apropriate color to them
void recalculate_sprites_visibility(void) {

	vec2_t p;
	vec2_t s_offset;
	sprite_t *s;
	float dist;
	int intersects;

	for (int x = 0; x < MAP_SIZE; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {

			s = sprite_map[x][y];

			if (s) {

				//act only on these objects
				/*if (s->render_layer != RENDER_LAYER_WALL && s->render_layer != RENDER_LAYER_FLOOR && s->render_layer != RENDER_LAYER_ITEM) {

					continue;
				}*/

				//calculate the distance between player and the sprite
				dist = Vec2Distance(player.sprite[0]->position, s->position);

				//too far from player
				if (dist > VIS_DISTANCE) {

					set_sprite_invisible(s);
				}
				else
				{
					//check if sprite visibility is broken by another sprite
					intersects = 1;
					for (int i = 0; i < 4; i++) {

						//check if any sprite side is visible from player's perspective
						s_offset[VEC_X] = s->position[VEC_X] + ((i % 2) ? SPRITE_SIZE * 0.95f * s->scale_x : 0) * ((i > 1) ? -1 : 1);
						s_offset[VEC_Y] = s->position[VEC_Y] + ((i % 2) ? 0 : SPRITE_SIZE * 0.95f * s->scale_y) * ((i > 1) ? 1 : -1);

						//raycast
						if (!sprite_ray_intersection(player.sprite[0]->position, s_offset, COLLISION_WALL | COLLISION_OBSTACLE, &p)) {

							intersects = 0;
							break;
						}
					}
					if (intersects) {

						//sprite visibility is blocked
						set_sprite_invisible(s);
					}
					else
					{
						s->visibility = VIS_VISIBLE;
						Color3White(s->color);
					}
				}
			}
		}
	}

	//recalculate vis for mobs and items
	recalculate_mob_visibility();
	recalculate_item_visibility();
}