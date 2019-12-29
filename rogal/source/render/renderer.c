/*
* This file is responsible for drawing sprites and particles on
* the screen. No depth buffer is used and sprites are drawn top
* to bottom. The rendering pipeline starts with rendering world
* sprites mixed with particles on their respective layers. Then
* the camera is set to the UI mode and UI sprites are drawn.
* 
* Culling is not implemented to keep the program as simple as
* possible.
* 
* Drawing is always done using the immediate mode method. Sprites
* are drawn using GL_QUADS mode and particles are drawn using
* GL_POINTS mode.
*/

#include "shared.h"
#include "renderer.h"
#include "camera.h"
#include "particles.h"
#include "window.h"

/*
* Used to retrieve and print all errors from OpenGL API.
*/
void print_gl_errors(const char *caller_name) {

	GLenum err;

	//get and print all errors
	while ((err = glGetError()) != GL_NO_ERROR) {

		d_printf(LOG_ERROR, "%s: OpenGL ERROR: %u\n", caller_name, (unsigned)err);
	}
}

/*
* Draws a single particle on the screen.
*/
void draw_particle(particle_t *p) {

	glBegin(GL_POINTS);

	//set color
	glColor4f(p->color[0], p->color[1], p->color[2], 1.f);

	//set position
	glVertex2f(p->position[VEC_X], p->position[VEC_Y]);

	glEnd();
}

/*
* Orders drawing of all particles that belong to the passed layer.
*/
void draw_particles(int layer) {

	particle_t *first = head_particle();
	particle_t *current = first;

	//nothing to draw
	if (!first) {

		return;
	}

	//make sure the particle size is scaled when the screen is being scaled.
	glPointSize(5.f * window_props.height / VIRTUAL_HEIGHT); //5.f is the particle size
	
	//disable texturing mode
	glDisable(GL_TEXTURE_2D);

	//loop over all particles
	while (current) {

		//check if the layer matches and if the particle is visible
		if (current->render_layer == layer && current->visibility == VIS_VISIBLE) {

			//draw it
			draw_particle(current);
		}

		//go to the next particle
		current = current->next;
	}

	print_gl_errors(__func__);
}

/*
* Calculates UV offset for a sprite.
* framestep: the fraction of texture size for the sprite.
* current_frame: current animation frame of the sprite.
*/
float uv_offset_for_sprite(float framestep, int current_frame) {

	return (framestep * current_frame) - framestep;  //offset position by n-1 framesteps
}

/*
* Draws a sprite on the screen.
*/
void draw_sprite(sprite_t *s) {

	//position
	float x = s->position[VEC_X];
	float y = s->position[VEC_Y];

	//size
	float sprite_size_x = SPRITE_SIZE * s->scale_x;
	float sprite_size_y = SPRITE_SIZE * s->scale_y;

	//UV X axis offset
	float framestep = 1.f / s->framecount;
	float uv_offset = uv_offset_for_sprite(framestep, s->current_frame);

	vec2_t uvs[4];

	//offset uvs to match the rotation
	//if the sprite is rotated its texture coorinates move clockwise to match the rotation
	for (int i = 0; i < 4; i++) {

		uvs[(i + s->rotation) & 3][VEC_X] = uv_offset + (i > 1 ? framestep : 0.f);
		uvs[(i + s->rotation) & 3][VEC_Y] = (!(i % 3) ? 1.f : 0.f);
	}

	//draw using immediate mode
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, (GLuint)s->tex_id);

	glBegin(GL_QUADS);

	//set color
	glColor3f(s->color[0], s->color[1], s->color[2]);

	//send four vertices to the GPU
	glTexCoord2f(uvs[0][VEC_X], uvs[0][VEC_Y]);
	glVertex2f(-sprite_size_x + x, -sprite_size_y + y);

	glTexCoord2f(uvs[1][VEC_X], uvs[1][VEC_Y]);
	glVertex2f(-sprite_size_x + x, sprite_size_y + y);

	glTexCoord2f(uvs[2][VEC_X], uvs[2][VEC_Y]);
	glVertex2f(sprite_size_x + x, sprite_size_y + y);

	glTexCoord2f(uvs[3][VEC_X], uvs[3][VEC_Y]);
	glVertex2f(sprite_size_x + x, -sprite_size_y + y);

	glEnd();

	print_gl_errors(__func__);
}

/*
* Draws all non-ui sprites as well as the particles.
*/
void draw_world_sprites(void) {

	sprite_t *first = sprite_head();
	sprite_t *current;

	//iterate over all but UI layers (bottom->top direction)
	for (unsigned i = 0; i <= RENDER_LAYER_ONTOP; i++) {

		current = first;

		//get all drawable sprites from this layer and draw them
		while (current)
		{
			if (current->render_layer == i && !current->skip_render) {

				draw_sprite(current);
			}
			current = current->next;
		}

		//particle layer?
		if (i == RENDER_LAYER_FLOOR_PARTICLE || i == RENDER_LAYER_EFFECT) {

			draw_particles(i);
		}
	}
}

/*
* Sets the camera to UI mode and draws the UI sprites on the screen.
* Afterwards the camera is set back to world mode.
*/
void draw_ui_sprites(void) {

	sprite_t *first = sprite_head();
	sprite_t *current;

	//set up the camera
	set_camera_for_ui();

	for (unsigned i = RENDER_LAYER_UI_BG; i <= RENDER_LAYER_UI; i++) {

		current = first;

		while (current)
		{
			if (current->render_layer == i && !current->skip_render) {

				draw_sprite(current);
			}
			current = current->next;
		}
	}

	//reset the camera
	unset_camera_for_ui();
}

/*
* Frame drawing sequence: this function executes drawing functions
* in the correct order.
*/
void display_frame(void) {

	/*
	Instead of using the depth buffer the application does all drawing from bottom to the
	top. This is slower because the entire sprite list is iterated for each layer but it
	also simplifies the entire drawing process: normally opaque geometry would be drawn
	using the depth buffer and transparency is drawn from bottom to the top.
	*/

	//clear the last frame data from color buffer
	glClear(GL_COLOR_BUFFER_BIT);
	
	//draw everything but ui
	draw_world_sprites();

	//draw ui
	draw_ui_sprites();

	glutSwapBuffers();

	print_gl_errors(__func__);
}

/*
* A callback to draw the frame on the screen using constant DISPLAY_FRAMERATE.
*/
void render_timer_callback(int value) {

	glutPostRedisplay(); //force glut to execute display callback
	glutTimerFunc((unsigned)(1000.f / DISPLAY_FRAMERATE), render_timer_callback, value); //set the next callback execution
}

/*
* Sets up the renderer (currently only blending mode).
*/
void init_render(void) {

	//blending mode
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	print_gl_errors(__func__);
}