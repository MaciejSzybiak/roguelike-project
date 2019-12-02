#include "shared.h"
#include "game.h"

#define MAX_ARMOR			20

typedef struct {
	player_stats_t	stats;

	int				look_direction;
	sprite_t		*sprite[3];

	item_t			*weapon;
} player_t;

extern player_t player;

void init_player(void);
void walk_to_tile(vec2_t position, float dist);
int direction_to_tile(vec2_t tile_pos);
void face_direction(int direction);
void attack_animation(int stop);