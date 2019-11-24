#include "shared.h"
#include "renderer.h"
#include "camera.h"
#include "particles.h"
#include "window.h"

//used to retrieve and print all errors from OpenGL API
void print_gl_errors(const char *caller_name) {

	GLenum err;

	//get and print all errors
	while ((err = glGetError()) != GL_NO_ERROR) {

		d_printf(LOG_ERROR, "%s: OpenGL ERROR: %u\n", caller_name, (unsigned)err);
	}
}

void draw_particle(particle_t *p) {

	glBegin(GL_POINTS);

	glColor4f(p->color[0], p->color[1], p->color[2], 1.f);

	glVertex2f(p->position[VEC_X], p->position[VEC_Y]);

	glEnd();
}

void draw_particles(int layer) {

	particle_t *first = head_particle();
	particle_t *current = first;

	if (!first) {

		return;
	}

	glPointSize(5.f * window_props.height / VIRTUAL_HEIGHT);
	glDisable(GL_TEXTURE_2D);

	while (current) {

		if (current->render_layer == layer) {

			draw_particle(current);
		}

		current = current->next;
	}
}

float uv_offset_for_sprite(float framestep, int current_frame) {

	return (framestep * current_frame) - framestep;
}

void draw_sprite(sprite_t *s) {

	//TODO: culling could be performed either here or when the camera is transformed.
	//is it worth doing? Each quad needs a camera bounds test and it might be cheaper to just draw it.

	float x = s->position[VEC_X];
	float y = s->position[VEC_Y];

	float sprite_size_x = SPRITE_SIZE * s->scale_x;
	float sprite_size_y = SPRITE_SIZE * s->scale_y;

	float framestep = 1.f / s->framecount;
	float uv_offset = uv_offset_for_sprite(framestep, s->current_frame);

	vec2_t uvs[4];

	//offset uvs to match the rotation
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

		if (i == RENDER_LAYER_FLOOR_PARTICLE || i == RENDER_LAYER_EFFECT) {

			draw_particles(i);
		}
	}
}

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

//keeps the framerate constant
void render_timer_callback(int value) {

	glutPostRedisplay();
	glutTimerFunc((unsigned)(1000.f / DISPLAY_FRAMERATE), render_timer_callback, value);
}

void init_render(void) {

	//blending for transparent textures
	//TODO: enable blending only for transparent sprites?
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLint range[2];
	glGetIntegerv(GL_POINT_SIZE_RANGE, range);

	d_printf(LOG_INFO, "%s: range: %d %d\n", __func__, range[0], range[1]);


	print_gl_errors(__func__);
}