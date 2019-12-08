#ifndef SHARED_H
#define SHARED_H

//detect OS
#ifdef _WIN32

#define WIN32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500	//for SetConsoleTextAttribute
#endif // !_WIN32_WINNT

#include <Windows.h>

#else

#define LINUX

#endif // _MSC_VER 

//basic includes
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

//unused macro marks variable as unused by purpose
#define UNUSED_VARIABLE(x) ((void)x)

//random
#define Random(min_val, max_val) ((min_val) + rand() % (((max_val) - (min_val)) + 1))
#define RandomBool Random(0, 1)

//fix for min and max macros: apparently they aren't standard for stdlib
#undef max
#undef min
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

//no definition for M_PI on some systems (windows?)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*---------
	   TIME
---------*/

//actual passed time between two last logic frames
extern int frame_msec;

#define LOGIC_MSEC	frame_msec
#define LOGIC_SEC	0.001f * LOGIC_MSEC

//tick milliseconds passed to each glut timer function
#define TICK_MSEC	10

/*---------
	   MATH
---------*/

#define Deg2Rad(deg) ((deg) * (float)M_PI / 180.f) //TODO: evaluate if this is needed
#define Rad2Deg(rad) ((rad) * 180.f / (float)M_PI)

//clamps
#define r_clamp(in, min_val, max_val) ((in) < (min_val)) ? (min_val) : (((in) > (max_val)) ? (max_val) : (in))
#define r_clamp_set(in, min_val, max_val) ((in) = ((in) < (min_val)) ? (min_val) : (((in) > (max_val)) ? (max_val) : (in)))

/*---------
	VECTORS
---------*/

#define VEC_X 0 //not necessary but it looks nice
#define VEC_Y 1

typedef float	vec_t;
typedef vec_t	vec2_t[2];	//2-dimensional vector
typedef double	mat4_t[16];	//4x4 matrix

//adds two vectors
#define Vec2Add(in1, in2, out) ((out)[VEC_X] = (in1)[VEC_X] + (in2)[VEC_X], \
								(out)[VEC_Y] = (in1)[VEC_Y] + (in2)[VEC_Y])

//substracts in2 from in1
#define Vec2Substract(in1, in2, out) ((out)[VEC_X] = (in1)[VEC_X] - (in2)[VEC_X], \
								      (out)[VEC_Y] = (in1)[VEC_Y] - (in2)[VEC_Y])

//sets vector to [0, 0]
#define Vec2Zero(in) ((in)[VEC_X] = 0, (in)[VEC_Y] = 0)

//sets in to [-x, -y]
#define Vec2Negative(in) ((in)[VEC_X] = -(in)[VEC_X], \
						  (in)[VEC_Y] = -(in)[VEC_Y])

//copies in values to out
#define Vec2Copy(in, out) ((out)[VEC_X] = (in)[VEC_X], \
						   (out)[VEC_Y] = (in)[VEC_Y])

//distance^2
#define Vec2DistanceSquared(in1, in2) (((in1)[VEC_X] - (in2)[VEC_X]) * ((in1)[VEC_X] - (in2)[VEC_X]) + \
									   ((in1)[VEC_Y] - (in2)[VEC_Y]) * ((in1)[VEC_Y] - (in2)[VEC_Y]))

//distance between two vectors
#define Vec2Distance(in1, in2) (sqrtf(Vec2DistanceSquared(in1, in2)))

//TODO: evaluate if still needed
//sets in to positive values
#define Vec2Abs(in) ((in)[VEC_X] = (float)fabs((in)[VEC_X]), (in)[VEC_Y] = (float)fabs((in)[VEC_Y]))

//finds a point factor% from start to end
#define Vec2Lerp(start, end, factor, out)  ((out)[VEC_X] = (start)[VEC_X] + (factor) * ((end)[VEC_X] - (start)[VEC_X]), \
											(out)[VEC_Y] = (start)[VEC_Y] + (factor) * ((end)[VEC_Y] - (start)[VEC_Y]))

//returns 1 if two vectors are identical
#define Vec2Compare(in1, in2) ((in1)[VEC_X] == (in2)[VEC_X] && (in1)[VEC_Y] == (in2)[VEC_Y])

//TODO: evaluate if still needed
//dot product
#define Vec2Dot(in1, in2) ((in1)[VEC_X] * (in2)[VEC_X] + (in1)[VEC_Y] * (in2)[VEC_Y])

/*---------
	  COLOR
---------*/

typedef vec_t color3_t[3];	//RGB in range [0,1]

//for visibility
#define COL_VIS_DISCOVERED				0.15f //discovered but not visible sprite brightness
#define COL_VIS_DISCOVERED__BLUE_TINT	1.4f
#define COL_VIS_DISCOVERED__RED_TINT	1.05f

//color setters
#define Color3White(in)		((in)[0] = 1.f, (in)[1] = 1.f, (in)[2] = 1.f)
#define Color3Red(in)		((in)[0] = 1.f, (in)[1] = 0.f, (in)[2] = 0.f)
#define Color3DarkRed(in)	((in)[0] = .5f, (in)[1] = 0.f, (in)[2] = 0.f)
#define Color3Green(in)		((in)[0] = 0.f, (in)[1] = 1.f, (in)[2] = 0.f)
#define Color3Blue(in)		((in)[0] = 0.f, (in)[1] = 0.f, (in)[2] = 1.f)
#define Color3Orange(in)	((in)[0] = 1.f, (in)[1] = .7f, (in)[2] = 0.f)
#define Color3Yellow(in)	((in)[0] = 1.f, (in)[1] = 1.f, (in)[2] = 0.f)
#define Color3Gray(in)		((in)[0] = .1f, (in)[1] = .1f, (in)[2] = .1f)
#define Color3Black(in)		((in)[0] = 0.f, (in)[1] = 0.f, (in)[2] = 0.f)
#define Color3LGray(in)		((in)[0] = COL_VIS_DISCOVERED * COL_VIS_DISCOVERED__RED_TINT, \
							(in)[1] = COL_VIS_DISCOVERED, \
							(in)[2] = COL_VIS_DISCOVERED * COL_VIS_DISCOVERED__BLUE_TINT)

#define Color3UIRed(in)		((in)[0] = .67f, (in)[1] = .17f, (in)[2] = 0.0f)
#define Color3UIGreen(in)	((in)[0] = .33f, (in)[1] = .73f, (in)[2] = .15f)
#define Color3UIOrange(in)	((in)[0] = 1.0f, (in)[1] = .75f, (in)[2] = .05f)

#define Color3Copy(in, out)	((out)[0] = (in)[0], \
							 (out)[1] = (in)[1], \
							 (out)[2] = (in)[2])

//item colors
#define Color3ItemCommon(in)	Color3White(in)
#define Color3ItemUncommon(in)	((in)[0] = .7f, (in)[1] = 1.f, (in)[2] = .7f)
#define Color3ItemRare(in)		((in)[0] = .7f, (in)[1] = .7f, (in)[2] = 1.f)
#define Color3ItemVeryRare(in)	((in)[0] = 1.f, (in)[1] = 1.f, (in)[2] = .7f)

/*---------
	LOGGING
---------*/

//debug logging message types
#define LOG_TEXT	0	//normal text
#define LOG_INFO	1	//important info
#define LOG_WARNING 2
#define LOG_ERROR	3

//debug logging: debug messages are formatted and printed to console
void d_printf(int type, const char *format, ...);
void d_spacer(void); //adds a spacer to separate debug messages
void out_of_memory_error(const char *caller); //displays a memory error and kills the program

/*---------
	SPRITES
---------*/

#define SPRITE_SIZE		0.5f //sprite side length is SPRITE_SIZE * 2

//sprite rotation
#define ROTATION_0		0
#define ROTATION_90		1
#define ROTATION_180	2
#define ROTATION_270	3

typedef struct sprite {
	//general
	vec2_t			position;			//world position of that sprite
	float			scale_x;
	float			scale_y;
	int				rotation;			//rotation defined by ROTATION_...

	//rendering
	unsigned int	tex_id;				//opengl texture id
	color3_t		color;

	unsigned int	render_layer;		//for rendering order
	int				skip_render;		//skip rendering this sprite?

	//vis
	int				visibility;			//TODO: unused var?
	
	//raycast
	int				collision_mask;		//collision type mask

	//linked list
	struct sprite	*previous;
	struct sprite	*next;

	//animation data
	int				framecount;			//frames on this sprite
	int				current_frame;		//current frame to display
	int				frame_msec;			//frame change time
	int				frame_msec_acc;		//msec accumulator for animation
	int				animation_pause;	//pauses the animation

	//click action handler
	void (*action)(struct sprite *s);	

	//object data for object's use (any type)
	void			*object_data;
} sprite_t;

sprite_t *sprite_head(void);		//first sprite in the linked list
sprite_t *new_sprite(void);			//allocates and returns a new sprite
void delete_sprite(sprite_t *s);	//frees the sprite

/*---------
	 PLAYER
---------*/

//stats for player and mobs
typedef struct {
	int health;
	int attack_damage;
	int armor;
	int max_health;
	int armor_modifier;
} player_stats_t;

//look directions for player and mobs
#define LOOK_RIGHT			0
#define LOOK_LEFT			1
#define LOOK_UP				2

//FIXME: evaluate if needed
//look skips down direction, so it needs to be converted into rotations
#define LookToRot(look) ((look) > 0 ? (look) + 1 : (look)) 

/*---------
   TEXTURES
---------*/

//texture names with their indexes
typedef enum {
	WALL,
	ROCK,
	WATER,

	DOOR,
	LOCKED_DOOR,
	LEVEL_EXIT,
	CHEST,

	PLAYER_B,
	PLAYER_R,
	PLAYER_L,

	SLIME_B,
	SLIME_R,
	SLIME_L,
	SLIME_ATTACK,

	GOBLIN_B,
	GOBLIN_R,
	GOBLIN_L,
	GOBLIN_ATTACK,

	FONT,

	ICON_DMG,
	ICON_ARMOR,
	ICON_HP,

	MOB_UI_BG,

	SHIELD,
	SWORD,
	POTION_HP,
	AXE
} texname;

//texture property getters for sprites
unsigned int get_texture_id(texname name);
int get_texture_framecount(texname name);
int get_texture_frametime(texname name);
int get_texture_render_layer(texname name);

/*---------
	 RENDER
---------*/

//render layer defines the depth of a sprite (for rendering and raycasting)
#define RENDER_LAYER_FLOOR			0
#define RENDER_LAYER_FLOOR_PARTICLE	1
#define RENDER_LAYER_WALL			2
#define RENDER_LAYER_ITEM			3
#define RENDER_LAYER_MOB			4
#define RENDER_LAYER_PLAYER			5
#define RENDER_LAYER_EFFECT			6
#define RENDER_LAYER_ONTOP			7
#define RENDER_LAYER_UI_BG			8
#define RENDER_LAYER_UI				9

/*---------
 COLLISIONS
---------*/

//collision masks
#define COLLISION_IGNORE	0

#define COLLISION_FLOOR		1
#define COLLISION_WALL		(1<<1)
#define COLLISION_WATER		(1<<2)
#define COLLISION_OBSTACLE  (1<<3)
#define COLLISION_ITEM		(1<<4)
#define COLLISION_MOB		(1<<5)
#define COLLISION_PLAYER	(1<<6)

#define COLLISION_UI		(1<<8)

//interactable collision layers
#define COLLISION_PLAYER_ACTION	(COLLISION_FLOOR | COLLISION_WALL | COLLISION_OBSTACLE | COLLISION_ITEM | COLLISION_MOB)

#define COLLISION_ALL		~0

/*---------
 VISIBILITY
---------*/

//visibility states
#define VIS_HIDDEN			0
#define VIS_DISCOVERED		1
#define VIS_VISIBLE			2

#define VIS_DISTANCE		4.5f //maximum vision distance

/*---------
	  OTHER
---------*/

//counting array elements
#define CountOf(in) (sizeof(in) / sizeof(in[0]))

#endif // !SHARED_H