## Camera
In this project the camera is a general term for the modelview matrix manipulation. The 
view is transformed in order to follow the player and render the UI. Relevant code can be
found in the [camera.c](../../rogal/source/render/camera.c) file.

#### Moving the camera
The camera moves along the XY plane of the 2D world. In order to put it in a different position
the function __void set_camera_position(vec2_t position)__ uses the *glTransaltef()* function
to reset the translation and then to set a new position which is passed as an argument.

#### Setting camera for UI
The __void set_camera_for_ui(void)__ function's role is to set the camera in order to allow UI
rendering. Because UI elements aren't translated every time the camera is moved they remain placed
around the world origin. In order to make them visible the camera position is put back at the (0, 0)
point, right after using the *glPushMatrix()* function.

*glPushMatrix()* is used in order to store the current modelview matrix. This allows an easy return
from UI camera position using the __void unset_camera_for_ui(void)__ function. The only thing 
that this function needs to do is to pop back the stored matrix using *glPopMatrix()*.

#### Coordinate space conversion
The __vec2_t *viewport_to_world_pos(vec2_t view_pos, int is_ui_space)__ function is used when a viewport
coordinates need to be translated to world position. This function is used for UI alignment which is
described [here]().

The __void world_to_screen_coordinates(vec2_t world_pos, int *x, int *y)__ function is used when
a world coordinate needs to be converted to screen space. Screen space coordinates are a 2D vector
where each unit is a pixel on the screen.

There is also one more similar function placed in the [raycast.c](../../rogal/source/physics/raycast.c)
file (__void mouse_to_world_coordinates(int x, int y)__). The use of this function is described [here]().

All of the above functions use either the *gluUnProject()* or *gluProject()* function in order
to transform the coordinates, so no manual matrix multiplication is used.

- Previous article: [Game window](window.md)
- Next article: [Rendering](rendering.md)