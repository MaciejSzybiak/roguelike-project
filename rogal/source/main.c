#include "shared.h"
#include "game.h"
#include "window.h"
#include "camera.h"
#include "particles.h"
#include "render/renderer.h"
#include "render/textures.h"
#include "ui.h"

void register_glut_callbacks(void) {

	//register display update function
	glutDisplayFunc(display_frame);

	//register display timer will automatically register itself
	render_timer_callback(0);

	//register window resize
	glutReshapeFunc(glut_change_size);

	//register logic loop
	glutTimerFunc(LOGIC_MSEC, logic_frame, glutGet(GLUT_ELAPSED_TIME));

	//register input events
	glutKeyboardFunc(keyboard_press_event);
	glutSpecialFunc(special_press_event);
	glutMouseFunc(mouse_click_event);

	d_printf(LOG_INFO, "%s: callbacks set\n", __func__);
}

int main(int argc, char **argv) {

#ifdef WIN32
#ifndef _DEBUG
	//hide Windows console
	HWND hWnd = GetConsoleWindow();
	//ShowWindow(hWnd, SW_MINIMIZE);
	ShowWindow(hWnd, SW_HIDE);
#endif // !_DEBUG
#endif // WIN32

	//debug message test
	d_printf(LOG_INFO, "test LOG_INFO message\n");
	d_printf(LOG_TEXT, "test LOG_TEXT message\n");
	d_printf(LOG_WARNING, "test LOG_WARNING message\n");
	d_printf(LOG_ERROR, "test LOG_ERROR message\n");
	d_spacer();

	//create game window
	create_window(argc, argv);

	//get errors from creating window
	print_gl_errors(__func__);

	//register callback functions
	register_glut_callbacks();

	//initialize render components
	init_camera();
	init_render();
	init_particles();

	//load textures into opengl context
	load_textures();

	//create UI sprites
	generate_ui();

	//pass control to glut
	glutMainLoop();
	
	return 0;
}