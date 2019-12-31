## Textures
The game loads and validates all texture files during its initialization process. All textures are immediately uploaded to the GPU
and only their OpenGL index is stored for later reference. 

The [textures.h](../../rogal/source/render/textures.h) file contains a definition
for a helper struct (texentry_t) that is used to store all necessary texture data.

All texture entries are stored in a constant array called *texture_names[]* in [textures.c](../../rogal/source/render/textures.c). When
the game is initialized the **void load_textures(void)** function is executed. This will run the __GLuint load_image(char *filename)__ function
which loads the file using stb_image library and generates an OpenGL texture. The index of that texture is then returned and assigned to the
matching index in *textures[MAX_TEXTURES]* array.

Later the texture information can be pulled using an enum value from *texname* enum found in [shared.h](../../rogal/headers/shared.h):

```c
typedef enum {
	WALL,
	ROCK,
	WATER,
    //...

} texname;
```

and one of the texture property getters (which can be found in [textures.c](../../rogal/source/render/textures.c)):

```c
unsigned int get_texture_id(texname name);
int get_texture_framecount(texname name);
int get_texture_frametime(texname name);
int get_texture_render_layer(texname name);
```

#### Texture properties
Each texture is a horizontal atlas containing a set of frames that make the texture animated.
- texture id - OpenGL index of the texture
- framecount - the amount of frames this texture contains
- frametime - default frametime that each animation frame is displayed for (in miliseconds)
- render layer - default render layer for a texture (more about render layers in the [next article](sprites.md))

These values are constant and cannot be altered but it's not required to use them when creating a sprite.

- Previous article: [Vectors](vectors.md)
- Next article: [Sprites](sprites.md)