#include "shared.h"
#include "window.h"

typedef struct {
	vec2_t position;
} camera_t;

void set_camera_position(vec2_t position);
void offset_camera_position(vec2_t offset);
void init_camera(void);

void set_camera_for_ui(void);
void unset_camera_for_ui(void);

vec2_t *get_camera_offset(void);
vec2_t *viewport_to_world_pos(vec2_t view_pos, int is_local);
void world_to_screen_coordinates(vec2_t world_pos, int *x, int *y);