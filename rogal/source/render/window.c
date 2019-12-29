/*
* This file manages the window creation process and handles
* window resizing.
*/

#include "window.h"
#include <GL/glut.h>

window_t window_props;

/*
* Sets the projection matrix to match the window settings.
* This allows the window to scale while the content scale is proportional
* to the height of the window.
*/
void set_projection_from_props(void) {

	glViewport(0, 0, window_props.width, window_props.height);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	//orthographic projection
	glOrtho(-window_props.ratio, window_props.ratio, -1.f, 1.f, -1.f, 1.f);
	glScalef(window_props.scale, window_props.scale, 1.f);

	glMatrixMode(GL_MODELVIEW);
}

/*
* Called when glut registers window size change.
*/
void glut_change_size(int w, int h) {

	if (h == 0) {
		h = 1;
	}

	//save the new properties
	window_props.width = w;
	window_props.height = h;

	window_props.ratio = (float)w / h;

	//adjust the content of the window
	set_projection_from_props();
}

/*
* Tells glut to close the window.
*/
void destroy_window(void) {

	glutDestroyWindow(glutGetWindow());
}

/*
* Sets the window fullscreen.
*/
void make_fullscreen(void) {

	window_props.w_width = window_props.width;
	window_props.w_height = window_props.height;
	window_props.w_pos_x = glutGet(GLUT_INIT_WINDOW_X);
	window_props.w_pos_y = glutGet(GLUT_INIT_WINDOW_Y);
	glutFullScreen();
}

/*
* Restores the windowed mode.
*/
void restore_windowed(void) {

	glutPositionWindow(window_props.w_pos_x, window_props.w_pos_y);
	glutReshapeWindow(window_props.w_width, window_props.w_height);
}

/*
* Creates the game window.
*/
void create_window(int argc, char **argv) {

	//set default props
	char name[] = "Rogal";
	window_props.name = &name[0];
	window_props.width = VIRTUAL_WIDTH;
	window_props.height = VIRTUAL_HEIGHT;
	window_props.w_width = VIRTUAL_WIDTH;
	window_props.w_height = VIRTUAL_HEIGHT;
	window_props.ratio = VIRTUAL_WIDTH / VIRTUAL_HEIGHT;
	window_props.scale = 0.2f;

	//send commands to glut
	glutInit(&argc, argv);
	glutInitWindowPosition(500, 250);
	glutInitWindowSize(window_props.width, window_props.height);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow(name);

	d_printf(LOG_INFO, "%s: Created window size: %dx%d\n", __func__, window_props.width, window_props.height);
}