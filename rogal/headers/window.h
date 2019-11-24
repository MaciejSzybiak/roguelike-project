#ifndef WINDOW_H
#define WINDOW_H

#include "shared.h"

//virtual height of the window is the initial window size
#define VIRTUAL_WIDTH	1280
#define VIRTUAL_HEIGHT	720

typedef struct {
	float scale;		//world scale
	float ratio;		//aspect ratio

	int width, height;	//window dimensions
	char *name;			//window name
} window_t;

extern window_t window_props;

void set_projection_from_props(void);
void create_window(int argc, char **argv);
void glut_change_size(int w, int h);
void destroy_window(void);

#endif // !WINDOW_H