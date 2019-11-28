/*
* This file allows an easier control over view projection transformation.
*/

#include "camera.h"
#include "renderer.h"
#include <GL/glut.h>

static camera_t cam;
static vec2_t view_to_world_vec;

/*
* Projects world position into a screen position.
*/
void world_to_screen_coordinates(vec2_t world_pos, int *x, int *y) {

	GLint viewport[4];
	mat4_t modelview_mat, projection_mat;
	GLdouble out_x, out_y, out_z;

	glGetIntegerv(GL_VIEWPORT, viewport);

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview_mat);
	glGetDoublev(GL_PROJECTION_MATRIX, projection_mat);

	gluProject(world_pos[VEC_X], world_pos[VEC_Y], 0.0, modelview_mat, projection_mat, viewport, &out_x, &out_y, &out_z);

	*x = (int)out_x;
	*y = (int)out_y;
}

/*
* Transforms position from viewport to world position. Useful for UI positioning.
* View position coordinates should be in [0, 1] range where [0, 0] is bottom left
* corner of the screen.
*/
vec2_t *viewport_to_world_pos(vec2_t view_pos, int is_ui_space) {

	mat4_t modelview_mat, projection_mat;
	GLdouble out_x, out_y, out_z;
	GLint viewport[4];

	view_pos[VEC_X] = r_clamp(view_pos[VEC_X], 0.f, 1.f) * window_props.width;
	view_pos[VEC_Y] = r_clamp(view_pos[VEC_Y], 0.f, 1.f) * window_props.height;

	Vec2Zero(view_to_world_vec);

	//get matrices and depth
	glGetIntegerv(GL_VIEWPORT, viewport);

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview_mat);
	glGetDoublev(GL_PROJECTION_MATRIX, projection_mat);

	//unproject viewport position to world position
	gluUnProject(view_pos[VEC_X], view_pos[VEC_Y], 1, modelview_mat, projection_mat, viewport, &out_x, &out_y, &out_z);

	view_to_world_vec[VEC_X] = (float)out_x + (is_ui_space ? cam.position[VEC_X] : 0);
	view_to_world_vec[VEC_Y] = (float)out_y + (is_ui_space ? cam.position[VEC_Y] : 0);

	print_gl_errors(__func__);

	return &view_to_world_vec;
}

/*
* Sets the camera position to the given vector.
*/
void set_camera_position(vec2_t position) {
	
	//back to (0, 0)
	glMatrixMode(GL_MODELVIEW);
	glTranslatef(-cam.position[VEC_X], -cam.position[VEC_Y], 0.f);

	Vec2Negative(position);
	Vec2Copy(position, cam.position);

	glMatrixMode(GL_MODELVIEW);
	glTranslatef(position[VEC_X], position[VEC_Y], 0.f);

	print_gl_errors(__func__);
}

/*
* Offests camera position by the given vector.
*/
void offset_camera_position(vec2_t offset) {

	Vec2Negative(offset);
	Vec2Add(cam.position, offset, cam.position);

	glMatrixMode(GL_MODELVIEW);
	glTranslatef(offset[VEC_X], offset[VEC_Y], 0.f);

	print_gl_errors(__func__);
}

vec2_t *get_camera_offset(void) {

	return &cam.position;
}

void set_camera_for_ui(void) {

	vec2_t negative;

	//find a vector negative to the current camera position
	Vec2Copy(cam.position, negative);
	Vec2Negative(negative);

	//push the current matrix
	glPushMatrix();

	//translate the camera to [0, 0]
	glMatrixMode(GL_MODELVIEW);
	glTranslatef(negative[VEC_X], negative[VEC_Y], 0.f);

	print_gl_errors(__func__);
}

void unset_camera_for_ui(void) {

	//pop back the old matrix
	glPopMatrix();

	print_gl_errors(__func__);
}

//FIXME: unused?
void set_camera_for_world(void) {

	set_camera_position(cam.position);
}

void init_camera(void) {

	Vec2Zero(cam.position);
}