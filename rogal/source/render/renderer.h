#include <GL/glut.h>

#define DISPLAY_FRAMERATE	60

void render_timer_callback(int value);
void display_frame(void);
void init_render(void);

void print_gl_errors(const char *caller_name);