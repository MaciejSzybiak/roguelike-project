#include "shared.h"

//raycast
sprite_t *screen_to_world_raycast(int x, int y, int raycast_mask);
int sprite_ray_intersection(vec2_t start, vec2_t end, int raycast_mask, vec2_t *point);