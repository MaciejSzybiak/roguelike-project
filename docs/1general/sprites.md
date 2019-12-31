## Sprites
Sprites are the main entities of the game. Each sprite is a textured square with its own position and size,
animation data and optional click action handler. They are used as map tiles, player character images, UI
elements and text letters.

#### The sprite structure
This structure can be found in [shared.h](../../rogal/headers/shared.h).

```c
typedef struct sprite {
	vec2_t position;
	float scale_x;
	float scale_y;
	int rotation;

	unsigned int tex_id;
	color3_t color;

	unsigned int render_layer;
	int	skip_render;

	int	visibility;

	int	collision_mask;

	//linked list
	struct sprite *previous;
	struct sprite *next;

	int framecount;
	int current_frame;
	int frame_msec;
	int frame_msec_acc;
	int animation_pause;

	//click action handler
	void (*action)(struct sprite *s);	

	//object data for object's use (any type)
	void *object_data;
} sprite_t;
```
parameter | description
--------- | -----------
position | a world position of this sprite
scale_x, scale_y | X and Y scale of this sprite (default sprite size is 1 unit)
rotation | one of four rotations (ROTATION_0, ROTATION_90, ROTATION_180, ROTATION_270). The texture on a sprite is rotated according to this value
tex_id | OpenGL texture index
color | the color tint of that sprite. Texture colour is multiplied with this value by the GPU
render_layer | the layer a sprite is rendered on
skip_render | setting this to 1 disables the sprite from being visible and interactable
visibility | visibility state (explained in detail [here]())
collision_mask | collision layer mask for raycasting
previous, next | linked list connections
framecount | the number of animation frames for sprite's texure
current_frame | current animation frame
frame_msec | the duration of a single animation frame
frame_msec_acc | accumulated duration of the current frame
animation_pause | set to 1 disables the animation
action | a pointer to action handler. The struct parameter will always be the struct that caused that function to be called
object_data | a sprite can store a reference to custom data which can be used by the action functions

#### Linked sprite list
For simplicity all sprites are stored in a single linked list. The list logic is located in [sprites.c](../../rogal/source/game/sprites.c).
This is the most standard linked list implementation: a pointer to the first sprite in the list is stored a a variable and each sprite contains 
a pointer to the next sprite in the list.

#### Render layers
Because the game doesn't use a depth buffer all sprites are drawn in order from bottom to the top layer. This is the first point where performance
is compromised for the sake of simplicity. Because all sprites are stored in a single list, the list needs to be looped over multiple times to
find all sprites that belong to the currently rendered layer.

The render layers are listed in [shared.h](../../rogal/headers/shared.h).
```c
#define RENDER_LAYER_FLOOR 0
#define RENDER_LAYER_FLOOR_PARTICLE 1
#define RENDER_LAYER_WALL 2
#define RENDER_LAYER_ITEM 3
#define RENDER_LAYER_MOB 4
#define RENDER_LAYER_PLAYER 5
#define RENDER_LAYER_EFFECT 6
#define RENDER_LAYER_ONTOP 7
#define RENDER_LAYER_UI_BG 8
#define RENDER_LAYER_UI 9
```

#### Collision masks
Because collisions should be detected for multiple object types at once it makes sense to use a bitmask as collision layer indicator.
The available collision layers are listed in [shared.h](../../rogal/headers/shared.h).

```c
#define COLLISION_IGNORE 0
#define COLLISION_FLOOR 1
#define COLLISION_WALL (1<<1)
#define COLLISION_WATER (1<<2)
#define COLLISION_OBSTACLE (1<<3)
#define COLLISION_ITEM (1<<4)
#define COLLISION_MOB (1<<5)
#define COLLISION_PLAYER (1<<6)
#define COLLISION_UI (1<<8)
```

As an example the following bitmask can be used for detection of player's clicks on game objects:
```c
#define COLLISION_PLAYER_ACTION (COLLISION_FLOOR | COLLISION_WALL | COLLISION_OBSTACLE | COLLISION_ITEM | COLLISION_MOB)
```

#### Sprite animation
The sprite animation logic is located in [game.c](../../rogal/source/game/game.c)'s **void update_sprite_animations(void)** function. This function
checks every valid sprite and it's time accumulators and changes animation frames accordingly. Sprite animation frames described in detail in the
[rendering section]().

- Previous article: [Textures](textures.md)
- Next article: [Debug logging](debug.md)