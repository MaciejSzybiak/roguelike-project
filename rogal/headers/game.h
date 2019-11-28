#ifndef GAME_H
#define GAME_H

#include "shared.h"
#include "text.h"

/*---------
	   MAPS
---------*/

//map dimensions and generation macros
#define MAP_SIZE			35
#define MAP_OFFSET			MAP_SIZE / 2

#define MAP_SAFE_ZONE		3

#define MIN_FEATURE_SIZE	3
#define MAX_FEATURE_SIZE	4

#define MAP_BUDGET			300

//map tile types
#define TILE_EMPTY			0
#define TILE_FLOOR			1
#define TILE_WALL			2
#define TILE_WATER			3
#define TILE_DOOR			4
#define TILE_EXIT			5
#define TILE_LOCK_DOOR		6
#define TILE_CHEST			7

#define TILE_INVALID		-1 //FIXME: ???

extern int map_contents[MAP_SIZE][MAP_SIZE]; //for mobs and items (non-tile elements)
extern sprite_t *sprite_map[MAP_SIZE][MAP_SIZE];

void generate_map(void);

/*---------
	  ITEMS
---------*/
#define MAX_ITEMS				8

#define ITEM_CATEGORY_WEAPON	1
#define ITEM_CATEGORY_ARMOR		2
#define ITEM_CATEGORY_HEALTH	3

typedef struct {
	sprite_t	*sprite;
	int			item_type;

	color3_t	rarity_color;

	int			item_category;

	int			modifier_value; //armor, attack damage etc
} item_t;

extern item_t map_items[MAX_ITEMS + 1];

void init_items(void);
item_t *new_item(void);
void weapon_pickup_action(sprite_t *s);

/*---------
	   MOBS
---------*/

#define MAX_MOBS		11 //maximum amount of mobs per level

#define MIN_MOB_DAMAGE	1
#define MAX_MOB_DAMAGE	2

#define MIN_MOB_HEALTH	1
#define MAX_MOB_HEALTH	5

#define MIN_MOB_ARMOR	0
#define MAX_MOB_ARMOR	1

typedef struct mob {
	sprite_t		*sprite[3];
	sprite_t		*attack_sprite;

	int				type;
	int				look_direction;

	player_stats_t	stats;
} mob_t;

extern int is_mob_move;
extern mob_t mobs[MAX_MOBS];

void init_mobs(void);
mob_t *find_mob(vec2_t position);
void mob_die(mob_t *mob);
void mobs_move(void);
void mob_receive_damage(mob_t *mob, int damage);

int alive_mobs_count(void);

/*---------
MAP CONTENTS
---------*/

#define MAP_ITEM_BYTE_OFFSET	8

#define MAP_NOTHING				0
#define MAP_MOB_SLIME			1

#define MAP_ITEM_SHIELD			1<<MAP_ITEM_BYTE_OFFSET
#define MAP_ITEM_SWORD			2<<MAP_ITEM_BYTE_OFFSET
#define MAP_ITEM_POTION_HP		3<<MAP_ITEM_BYTE_OFFSET

#define MAP_ITEM_LOCK_ROOM		64<<MAP_ITEM_BYTE_OFFSET

/*---------
	 PLAYER
---------*/

#define MOVE_SPEED			4 //tiles per second
#define ATTACK_ANIM_MSEC	200

extern int is_player_move;
extern int is_player_dead;

void attack_mob(mob_t *mob);
void player_receive_damage(int damage);

void add_health(int hp);
void add_armor(int val);
void increase_base_stat(int is_health, int value);

void player_pickup_weapon(item_t *w);

void get_player_pos(vec2_t *out);

void player_next_level(void);


/*---------
	   GAME
---------*/

void keyboard_press_event(unsigned char key, int x, int y);
void special_press_event(int key, int x, int y);
void mouse_click_event(int button, int state, int x, int y);
void logic_frame(int);

extern int is_ingame;
extern int is_paused;

void init_game(void);

void next_level_action(sprite_t *s);

/*---------
	OBJECTS
---------*/

void door_action(sprite_t *s);
void locked_door_action(sprite_t *s);
void chest_action(sprite_t *s);

/*---------
 VISIBILITY
---------*/

void recalculate_sprites_visibility(void);

#endif // !GAME_H