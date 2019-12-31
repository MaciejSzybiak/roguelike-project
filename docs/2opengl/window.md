## Game window

Because this project uses the freeglut library the window setup is simplified when compared
to a traditional OpenGL application. Every OS-specific part of it is handled by the library
and window resizing is handled by a special callback function. All the window related code
is placed in the [window.c](../../rogal/source/render/window.c) file.

#### Creating the window
Function __void create_window(int argc, char **argv)__ called right after the game starts
takes care of this process. The argc and argv arguments are required by the glutInit() function
and are exactly the same parameters as the main() function receives.

```c
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
```

The function starts by saving all the parameters into a struct (defined in 
[window.h](../../rogal/headers/window.h)). After that the freeglut library is told to set up
the window according to parameters. 

It's worth noting that the function **glutInitDisplayMode** sets only the RGB buffer (no depth
or stencil buffer is used).

#### Window resize callback
When GLUT registers a window size change it will call a registered window resize callback. The
__void glut_change_size(int w, int h)__ function takes this role.

In order to keep the window displaying roughly the same amount of content at every resolution
a ratio of window's width to height is used to rescale the projection. The projection setup
takes place in __void set_projection_from_props(void)__ called by the resize callback.

```c
void set_projection_from_props(void) {

	glViewport(0, 0, window_props.width, window_props.height);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	//orthographic projection
	glOrtho(-window_props.ratio, window_props.ratio, -1.f, 1.f, -1.f, 1.f);
	glScalef(window_props.scale, window_props.scale, 1.f);

	glMatrixMode(GL_MODELVIEW);
}
```

First of all the viewport size is set to match the window size. The next step is to set up
a new projection matrix. Because this game is 2D an 
[orthographic projection](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glOrtho.xml) 
is used. The height of the projection is kept constant at 2 while the projection's width is
set to 2*window ratio.

Setting the projection dimensions to H[-1, 1] and W[-ratio, ratio] is important because
later when the camera is translated using modelview matrix the camera view center matches
the desired position. Setting only positive values (e.g. H[0, 1] and W[0, ratio]) would put 
camera's pivot in the top left corner, which is inconvenient.

#### Fullscreen
The game has an option to be played in fullscreen mode. Once again GLUT can take care of all
the difficulties.

Function __void make_fullscreen(void)__ saves the current window size and position and simply
orders GLUT to make the window fullscreen.

In order to go back to windowed mode function __void restore_windowed(void)__ restores the
saved window properties and tells GLUT to apply them to the game window.

- Previous article: [Debug logging](../1general/debug.md)
- Next article: [Camera](../2opengl/camera.md)