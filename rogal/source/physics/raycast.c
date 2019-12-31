/*
* This file contains two raycast algorithms: the first one is used to
* determine if a sprite was hit by a ray casted from a screen point
* and the second one checks if anything blocks the vision between two
* sprites.
*/

#include "raycast.h"
#include "shared.h"
#include "camera.h"
#include <GL/glut.h>
#include <float.h>

static vec2_t mouse_world_pos;

/*
* A simple check that determines if the mouse is inside a sprite.
*/
int is_mouse_pos_inside_sprite(sprite_t *s) {

	//get the distance from mouse to sprite
	float diff_x = mouse_world_pos[VEC_X] - s->position[VEC_X];
	float diff_y = mouse_world_pos[VEC_Y] - s->position[VEC_Y];

	//see if the distance fits inside that sprite
	return (fabs(diff_x) < ((double)SPRITE_SIZE * fabs(s->scale_x)) && //cast to double to fix an arithmetic overflow warning
			fabs(diff_y) < ((double)SPRITE_SIZE * fabs(s->scale_y)));  //and use fabs on scale to fix scales less than 0
}

/*
* Projects mouse position from screen space to world space position.
*/
void mouse_to_world_coordinates(int x, int y) {

	GLint viewport[4];
	mat4_t modelview_mat, projection_mat;
	GLfloat wx, wy;
	GLdouble out_x, out_y, out_z;

	Vec2Zero(mouse_world_pos);

	//get viewport matrix
	glGetIntegerv(GL_VIEWPORT, viewport);
	y = viewport[3] - y;

	wx = (GLfloat)x;
	wy = (GLfloat)y;

	//get transformation matrices
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview_mat);
	glGetDoublev(GL_PROJECTION_MATRIX, projection_mat);

	//unproject screen position to world position
	gluUnProject(wx, wy, 0, modelview_mat, projection_mat, viewport, &out_x, &out_y, &out_z);

	//copy output values into the vector
	mouse_world_pos[VEC_X] = (vec_t)out_x;
	mouse_world_pos[VEC_Y] = (vec_t)out_y;
}

/*
* Performs screen to world raycast against UI layers using the raycast mask.
* This is done before raycasting all other render layers.
*/
sprite_t *screen_to_world_ui_raycast(sprite_t *first, int raycast_mask) {

	sprite_t *current = first;
	vec2_t original;
	vec2_t *camera_offset = get_camera_offset();

	//iterate over all sprites
	while (current) {

		if (!current->skip_render &&						//skip inactive sprites
			current->collision_mask != COLLISION_IGNORE &&	//don't do anything with ignored sprites (in case mask is 0)
			current->render_layer == RENDER_LAYER_UI &&		//top UI layer only
			(current->collision_mask & raycast_mask)) {		//check if the mask matches that sprite

			//temporarily translate the sprite to camera position
			//Normally all UI sprites are placed around the world center
			//but they neeed to be moved to perform a world space raycast.
			Vec2Copy(current->position, original);
			Vec2Substract(current->position, *camera_offset, current->position);

			//check this sprite
			if (is_mouse_pos_inside_sprite(current)) {

				//mouse hit
				Vec2Copy(original, current->position);
				return current;
			}

			Vec2Copy(original, current->position);
		}
		current = current->next;
	}
	//hit nothing
	return NULL;
}

/*
* Performs screen to world raycast against all layers, for sprites with matching raycast mask.
* Returns the first sprite that was hit.
*/
sprite_t *screen_to_world_raycast(int x, int y, int raycast_mask) {

	sprite_t *first = sprite_head();
	sprite_t *current = first;

	//nothing to raycast against
	if (!first) {

		return NULL;
	}

	//transform mouse position to world coordinates
	mouse_to_world_coordinates(x, y);

	//try UI first
	if (raycast_mask & COLLISION_UI) {

		sprite_t *out = screen_to_world_ui_raycast(first, raycast_mask);

		if (out) {

			return out;
		}
	}

	//walk layers top to bottom
	for (int i = RENDER_LAYER_ONTOP; i >= 0; i--) {

		//iterate over all sprites
		while (current)
		{
			if (current->collision_mask != COLLISION_IGNORE &&	//skip collision ignores
				current->render_layer == (unsigned)i &&			//check if this sprite is on the correct layer
				current->collision_mask & raycast_mask &&		//check if collision mask is a match
				!current->skip_render) {						//ignore inactive sprites

				//check this sprite
				if (is_mouse_pos_inside_sprite(current)) {

					//hit
					return current;
				}
			}
			current = current->next;
		}

		current = first;
	}
	//hit nothing
	return NULL;
}

/*
* Checks if two lines intersect. The point variable is set as their contact point if they do.
*/
int line_line_intersection(vec2_t start1, vec2_t end1, vec2_t start2, vec2_t end2, vec2_t *point) {

	//using the document found at http://www.cs.swan.ac.uk/~cssimon/line_intersection.html

	float a1, a2, b, t1, t2;

	b = (end2[VEC_X] - start2[VEC_X]) * (start1[VEC_Y] - end1[VEC_Y]) - (end2[VEC_Y] - start2[VEC_Y]) * (start1[VEC_X] - end1[VEC_X]);

	if (b == 0) {

		return 0;
	}

	a1 = (start2[VEC_Y] - end2[VEC_Y]) * (start1[VEC_X] - start2[VEC_X]) + (end2[VEC_X] - start2[VEC_X]) * (start1[VEC_Y] - start2[VEC_Y]);
	a2 = (start1[VEC_Y] - end1[VEC_Y]) * (start1[VEC_X] - start2[VEC_X]) + (end1[VEC_X] - start1[VEC_X]) * (start1[VEC_Y] - start2[VEC_Y]);

	t1 = a1 / b;
	t2 = a2 / b;

	if (t1 >= 0 && t1 <= 1 && t2 >= 0 && t2 <= 1) {

		Vec2Lerp(start1, end1, t1, *point);
		return 1;
	}
	return 0;
}

/*
* This function checks if two sprites can "see" each other (one placed at the "start" and the other at the "end" position).
* Designed to be used with the visibility calculations.
* parameter vec2_t *point: this is set as the point that was hit by the ray. If null is passed to the function then
* the function returns true if _anything_ was hit
*/
int sprite_ray_intersection(vec2_t start, vec2_t end, int raycast_mask, vec2_t *point) {

	sprite_t *first = sprite_head();
	sprite_t *current = first;
	vec2_t v1, v2, out_p;
	float dist;
	float min_dist = FLT_MAX;

	//nothing to test
	if (!first) {

		return 0;
	}

	//an axis aligned rectangle where the ray is its diagonal
	float x_max = max(start[VEC_X], end[VEC_X]);
	float x_min = min(start[VEC_X], end[VEC_X]);

	float y_max = max(start[VEC_Y], end[VEC_Y]);
	float y_min = min(start[VEC_Y], end[VEC_Y]);

	while (current)
	{
		//the first condition checks if the sprite fits in the axis aligned rectangle
		if ((current->position[VEC_X] <= x_max && current->position[VEC_X] >= x_min && current->position[VEC_Y] <= y_max && current->position[VEC_Y] >= y_min) &&
			current->collision_mask != COLLISION_IGNORE &&
			current->collision_mask & raycast_mask &&
			!current->skip_render &&
			!(Vec2Distance(start, current->position) < SPRITE_SIZE) && //ignore the source sprites...
			!(Vec2Distance(end, current->position) < SPRITE_SIZE)) {

			//check each edge of the current sprite
			for (int i = 0; i < 4; i++) {

				//get start and end of the current edge
				v1[VEC_X] = current->position[VEC_X] + current->scale_x * SPRITE_SIZE * ((i < 2) ? -1 : 1);
				v1[VEC_Y] = current->position[VEC_Y] - current->scale_y * SPRITE_SIZE * ((i % 3) ? 1 : -1);

				v2[VEC_X] = current->position[VEC_X] + current->scale_x * SPRITE_SIZE * ((i % 3) ? -1 : 1);
				v2[VEC_Y] = current->position[VEC_Y] + current->scale_y * SPRITE_SIZE * ((i < 2) ? 1 : -1);

				//try intersecting the ray with this edge
				if (line_line_intersection(start, end, v1, v2, &out_p)) {

					if (!point) {

						return 1;
					}

					//take the closest ray hit point
					dist = Vec2Distance(start, out_p);

					if (dist < min_dist) {

						min_dist = dist;
						Vec2Copy(out_p, *point);
					}
				}
			}
		}
		current = current->next;
	}

	return min_dist < FLT_MAX;
}
