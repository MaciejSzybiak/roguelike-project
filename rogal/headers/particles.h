#include "shared.h"

#define PARTICLE_DEFAULT_GRAVITY	4
#define PARTICLE_DEFAULT_MSEC		1500 //default particle lifetime

typedef struct particle {

	//particle render color
	color3_t	color;

	//position and velocity. For each frame the particle will move to (position + velocity * frametime)
	vec2_t		velocity;
	vec2_t		position;

	int			gravity;		//gravity value for the particle. Could also be negative for effects like fire or smoke
	float		ground_height;	//Y height of the particle "ground". If the particle's position reaches this value it means the ground was hit

	int			life_msec;		//time left to live

	int			render_layer;	//sorting layer for the renderer

	//"one way" linked list
	struct particle *next;
} particle_t;

//general functions
particle_t *new_particle(void);
void delete_particle(particle_t *p);
void run_particles(int msec);
void init_particles(void);

particle_t *head_particle(void);

//particle geneerators
void make_walk_dust_particle(vec2_t position, float groundheight);
void make_blood_particles(vec2_t position, float groundheight);
void make_death_particles(vec2_t position);
void make_pickup_particles(vec2_t position, float color_r, float color_g, float color_b);
void make_chest_particles(vec2_t position, float color_r, float color_g, float color_b);