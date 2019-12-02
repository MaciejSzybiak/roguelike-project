#ifndef WINDOW_H
#define WINDOW_H

#include "shared.h"

//virtual height of the window is the initial window size
#define VIRTUAL_WIDTH	1280
#define VIRTUAL_HEIGHT	720

typedef struct {
	float	scale;				//world scale
	float	ratio;				//aspect ratio

	int		width, height;		//window dimensions
	char	*name;				//window name

	int		w_width, w_height;	//last windowed mode dimensions
	int		w_pos_x, w_pos_y;	//last windowed mode position
} window_t;

extern window_t window_props;

void set_projection_from_props(void);
void create_window(int argc, char **argv);
void glut_change_size(int w, int h);
void destroy_window(void);

void make_fullscreen(void);
void restore_windowed(void);

#endif // !WINDOW_H