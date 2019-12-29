/*
* Particles are used to display some visual effects in order
* to make the gameplay more responsive and to make the game
* look "prettier".
*
* Because the amount of particles is often changing and their
* amount can vary a lot particles are allocated dynamically 
* and stored in a linked list (similar to the sprites).
* 
* Particles are using a simple simulation algorithm: their
* position and velocity can change (the velocity only due to
* gravity).
* 
* Every particle is represented by a variable of particle_t
* struct type.
*/

#include "particles.h"
#include "game.h"
#include <string.h>

static particle_t *first_particle;
int are_particles_enabled = 1;

/*
* Used to retrieve the first particle in the particle list
*/
particle_t *head_particle(void) {

	return first_particle;
}

/**
* Allocates a new particle and links it to the list.
*/
particle_t *new_particle(void) {

	particle_t *p;
	particle_t *current;

	//allocate the particle
	p = malloc(sizeof(particle_t));
	memset(p, 0, sizeof(particle_t));

	//add to the list
	if (!first_particle) {

		//there's no particles currently allocated, this is the first one
		first_particle = p;
	}
	else
	{
		//find the last particle in the list and attach the new one after it
		current = first_particle;
		while (current->next) {

			current = current->next;
		}

		current->next = p;
	}
	p->visibility = 1;
	return p;
}

/**
* Removes the particle, clears memory allocated for it and deletes it from the list.
*/
void delete_particle(particle_t *p) {

	particle_t *current;
	particle_t *previous;

	//special treatment for the first particle in the list
	if (p == first_particle) {

		if (first_particle->next) {

			first_particle = first_particle->next;
		}
		else
		{
			first_particle = NULL;
		}

		free(p);

		return;
	}

	//find in the list
	current = first_particle->next;
	previous = first_particle;

	while (current != p) {

		if (!current->next && current != p) {

			d_printf(LOG_ERROR, "%s: particle not found!\n", __func__);
			return;
		}
		previous = current;
		current = current->next;
	}

	//patch up the list
	if (current->next) {

		previous->next = current->next;
	}
	else
	{
		previous->next = NULL;
	}

	//free the memory
	free(p);
}

/**
* Clears all existing particles. Useful for changing levels.
*/
void delete_all_particles(void) {

	particle_t *previous;
	particle_t *current;

	//nothing is allocated
	if (!first_particle) {

		return;
	}

	//iterate over the list and delete all elements
	current = first_particle;
	while (current) {

		previous = current;
		current = current->next;
		free(previous);
	}
	free(current); //delete the last particle

	first_particle = NULL;
}

/**
* Runs particle simulation step. This is executed by the game logic callback function in game.c (logic_frame())
*/
void run_particles(int msec) {

	particle_t *previous;
	particle_t *current = first_particle;
	vec2_t end;
	float time = msec * 0.001f; //frametime
	int x, y;
	sprite_t *s;

	//noting to simulate
	if (!first_particle) {

		return;
	}

	//check if alive
	while (current) {

		//decrease the lifetime
		current->life_msec -= msec;

		if (current->life_msec <= 0) {

			//jump to the next particle before removing the old one
			previous = current;
			current = current->next;

			delete_particle(previous);
			continue;
		}

		current = current->next;
	}

	//run simulation on each particle
	current = first_particle;
	while (current) {

		//calculate velocity and position
		if (current->position[VEC_Y] <= current->ground_height) {

			if (current->velocity[VEC_Y] < -.8f) { //hit ground hard

				//bounce
				current->velocity[VEC_Y] *= -(0.3f + 0.03f * Random(1, 3));
			}
			else {

				//stop falling
				current->velocity[VEC_Y] = 0;
			}
			//add some ground friciton
			current->velocity[VEC_X] *= r_clamp(1.f  - time * 5, 0, 1);
		}
		else {

			//add gravity
			current->velocity[VEC_Y] -= current->gravity * time;
		}

		//calculate new position using lerp
		Vec2Add(current->position, current->velocity, end); //movement done in 1 sec (position + velocity vector)
		Vec2Lerp(current->position, end, time, current->position); //move by time fraction

		//set visibility
		x = r_roundf(current->position[VEC_X]);
		y = r_roundf(current->position[VEC_Y]);

		s = sprite_map[x + MAP_OFFSET][y + MAP_OFFSET];
		if (s) {

			current->visibility = sprite_map[x + MAP_OFFSET][y + MAP_OFFSET]->visibility;
		}
		else
		{
			current->visibility = 0;
		}

		current = current->next;
	}
}

/**
* Makes sure the particle system is initialized correctly.
*/
void init_particles(void) {

	delete_all_particles();
}

//----------
// Particle generators
//----------

/**
* Creates a single gray particle, meant to serve as a dust coming from player's feet as he moves.
*/
void make_walk_dust_particle(vec2_t position, float groundheight) {

	particle_t *p;
	float color_variation = 1.f / Random(4, 10); //color added to the base particle color (gray)
	float rand_x, rand_y;

	//don't spawn particles when disabled
	if (!are_particles_enabled) {

		return;
	}

	//get a new particle
	p = new_particle();

	//set position
	Vec2Copy(position, p->position);
	p->ground_height = groundheight;

	p->life_msec = 300 + Random(0, 20) * 5; //lifetime with some random range
	p->gravity = PARTICLE_DEFAULT_GRAVITY;	//dust needs to fall down

	p->render_layer = RENDER_LAYER_FLOOR_PARTICLE; //render behind everything but the floor

	//set color
	Color3Gray(p->color);
	p->color[0] += color_variation;
	p->color[1] += color_variation;
	p->color[2] += color_variation;

	//set small, random velocity
	rand_x = Random(-2, 2) * 0.1f;
	rand_y = Random(0, 2) * 0.2f;

	p->velocity[VEC_X] = rand_x;
	p->velocity[VEC_Y] = rand_y;
}

/**
* Creates 7 particles that make a blood effect when something takes a damage.
*/
void make_blood_particles(vec2_t position, float groundheight) {

	particle_t *p;
	float color_variation;
	float rand_x, rand_y;
	float offs_x, offs_y;

	//don't spawn particles when disabled
	if (!are_particles_enabled) {

		return;
	}

	//make a few particles
	for (int i = 0; i < 7; i++) {

		//color
		color_variation = 1.f / Random(5, 12);

		//position
		offs_x = Random(-2, 2) * 0.03f;
		offs_y = Random(0, 2) * 0.03f;
		
		//make the particle
		p = new_particle();

		Vec2Copy(position, p->position);
		p->position[VEC_X] += offs_x;
		p->position[VEC_Y] += offs_y;

		p->ground_height = groundheight;

		p->life_msec = PARTICLE_DEFAULT_MSEC + Random(0, 100);
		p->gravity = PARTICLE_DEFAULT_GRAVITY;

		p->render_layer = RENDER_LAYER_EFFECT;

		//set color
		Color3DarkRed(p->color);
		p->color[0] += color_variation;
		p->color[1] += color_variation;
		p->color[2] += color_variation;

		//velocity
		rand_x = Random(-2, 2) * 0.15f;
		rand_y = Random(0, 2) * 0.5f;

		p->velocity[VEC_X] = rand_x;
		p->velocity[VEC_Y] = rand_y;
	}
}

/**
* Death effect particles - a yellow particle clouds expanding in each way, disappears completely in less that a second.
*/
void make_death_particles(vec2_t position) {

	particle_t *p;
	float color_variation;
	float rand_x, rand_y;
	float offs_x, offs_y;

	//don't spawn particles when disabled
	if (!are_particles_enabled) {

		return;
	}

	//make some particles
	for (int i = 0; i < 20; i++) {

		//color
		color_variation = 1.f / Random(5, 10);

		//position
		offs_x = Random(-2, 2) * 0.01f;
		offs_y = Random(0, 2) * 0.01f;

		//make the particle
		p = new_particle();

		Vec2Copy(position, p->position);
		p->position[VEC_X] += offs_x;
		p->position[VEC_Y] += offs_y;

		p->ground_height = -10000.f; //put the ground very low so the particles never hits it

		p->life_msec = 600 + Random(0, 50) * 6;
		p->gravity = 1;

		p->render_layer = RENDER_LAYER_EFFECT;

		//set color
		Color3Orange(p->color);
		p->color[0] += color_variation;
		p->color[1] += color_variation;
		p->color[2] += color_variation;

		//velocity
		rand_x = Random(1, 2) * 0.3f * (RandomBool ? (-1) : 1);
		rand_y = Random(1, 2) * 0.3f * (RandomBool ? (-1) : 1);

		p->velocity[VEC_X] = rand_x;
		p->velocity[VEC_Y] = rand_y;
	}
}

/**
* For item pickup effect. This was added because picking up items wasn't always obvious when playing.
*/
void make_pickup_particles(vec2_t position, float color_r, float color_g, float color_b) {

	particle_t *p;
	float color_variation;
	float rand_x, rand_y;
	float offs_x, offs_y;

	//don't spawn particles when disabled
	if (!are_particles_enabled) {

		return;
	}

	//make some particles
	for (int i = 0; i < 20; i++) {

		//color
		color_variation = 1.f / Random(5, 10);

		//position
		offs_x = Random(-2, 2) * 0.07f;
		offs_y = Random(0, 2) * 0.07f;

		//make the particle
		p = new_particle();

		Vec2Copy(position, p->position);
		p->position[VEC_X] += offs_x;
		p->position[VEC_Y] += offs_y;

		p->ground_height = -10000.f;

		p->life_msec = 400 + Random(0, 50) * 3;
		p->gravity = 0;

		p->render_layer = RENDER_LAYER_EFFECT;

		//set color
		p->color[0] = color_r + color_variation;
		p->color[1] = color_g + color_variation;
		p->color[2] = color_b + color_variation;

		//velocity
		rand_x = Random(1, 2) * 0.1f * (RandomBool ? (-1) : 1);
		rand_y = Random(1, 2) * 0.1f * (RandomBool ? (-1) : 1);

		p->velocity[VEC_X] = rand_x;
		p->velocity[VEC_Y] = rand_y;
	}
}

void make_chest_particles(vec2_t position, float color_r, float color_g, float color_b) {

	particle_t *p;
	float color_variation;
	float rand_x, rand_y;
	float offs_x, offs_y;

	//don't spawn particles when disabled
	if (!are_particles_enabled) {

		return;
	}

	//cone shape
	for (int i = 0; i < 30; i++) {

		//color
		color_variation = 1.f / Random(5, 10);

		//position
		offs_x = Random(-2, 2) * 0.07f;
		offs_y = Random(0, 2) * 0.07f;

		//make the particle
		p = new_particle();

		Vec2Copy(position, p->position);
		p->position[VEC_X] += offs_x;
		p->position[VEC_Y] += offs_y;

		p->ground_height = position[VEC_Y] - (SPRITE_SIZE * 0.8f);

		p->life_msec = 1500 + Random(0, 100) * 3;
		p->gravity = PARTICLE_DEFAULT_GRAVITY;

		p->render_layer = RENDER_LAYER_EFFECT;

		//set color
		p->color[0] = color_r + color_variation;
		p->color[1] = color_g + color_variation;
		p->color[2] = color_b + color_variation;

		//velocity
		rand_x = Random(1, 2) * 0.1f * (RandomBool ? (-1) : 1);
		rand_y = Random(7, 10) * 0.2f;

		p->velocity[VEC_X] = rand_x;
		p->velocity[VEC_Y] = rand_y;
	}
}