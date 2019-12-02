/*
* This file is responsible for loading and keeping track of all game textures.
* Every texture file is loaded and uploaded to GPU when the game is started to
* simplify the process.
* 
* Image file loading is handled by the stb_image library which is included as
* a single header file. Most of that header contents is skipped with correct
* directive setup (STBI_ONLY_PNG).
* 
* Textures are defined by texentry_t structs which contain:
*	- texture number (ID) 
*	- relative path to the file (starting at application working directory)
*	- the number of animation frames/sub-images for this file
*	- default frametime for animating that texture
*	- default render layer for that texture
* 
* Each texture can be a set of frames or sub-images. In that case the UV
* coordinates are recalculated so the rendered sprite shows only the correct
* part of the texture (described in detail in renderer.c)
*/

#include "renderer.h"
#include "textures.h"
#include "stb_image.h"
#include <GL/glut.h>

//opengl indices for each loaded texture
static GLuint textures[MAX_TEXTURES];

//texture entries: all textures used by the game are listed here
static const texentry_t texture_names[] = {

	//name          file path								frames	frametime				render layer
	{ FONT			, "resources/textures/font.png"				, 64	, DEFAULT_ANIM_MSEC		, RENDER_LAYER_UI },

	{ ICON_DMG		, "resources/textures/dmg_icon.png"			, 1		, DEFAULT_ANIM_MSEC		, RENDER_LAYER_UI },
	{ ICON_ARMOR	, "resources/textures/armor_icon.png"		, 1		, DEFAULT_ANIM_MSEC		, RENDER_LAYER_UI },
	{ ICON_HP		, "resources/textures/health_icon.png"		, 1		, DEFAULT_ANIM_MSEC		, RENDER_LAYER_UI },

	{ ROCK			, "resources/textures/rock.png"				, 1		, DEFAULT_ANIM_MSEC		, RENDER_LAYER_FLOOR },
	{ WALL			, "resources/textures/wall.png"				, 1		, DEFAULT_ANIM_MSEC		, RENDER_LAYER_WALL },
	{ WATER			, "resources/textures/water1.png"			, 8		, DEFAULT_ANIM_MSEC		, RENDER_LAYER_FLOOR },

	{ DOOR			, "resources/textures/door.png"				, 2		, DEFAULT_ANIM_MSEC		, RENDER_LAYER_WALL },
	{ LOCKED_DOOR	, "resources/textures/locked_door.png"		, 3		, DEFAULT_ANIM_MSEC		, RENDER_LAYER_WALL },
	{ LEVEL_EXIT	, "resources/textures/level_exit.png"		, 1		, DEFAULT_ANIM_MSEC		, RENDER_LAYER_FLOOR },
	{ CHEST			, "resources/textures/chest.png"			, 2		, DEFAULT_ANIM_MSEC		, RENDER_LAYER_FLOOR },

	{ PLAYER_B		, "resources/textures/player_b.png"			, 2		, PLAYER_MOVE_ANIM_MSEC	, RENDER_LAYER_PLAYER },
	{ PLAYER_R		, "resources/textures/player_r.png"			, 2		, PLAYER_MOVE_ANIM_MSEC	, RENDER_LAYER_PLAYER },
	{ PLAYER_L		, "resources/textures/player_l.png"			, 2		, PLAYER_MOVE_ANIM_MSEC	, RENDER_LAYER_PLAYER },

	{ SLIME_B		, "resources/textures/slime_b.png"			, 2		, PLAYER_MOVE_ANIM_MSEC	, RENDER_LAYER_MOB },
	{ SLIME_R		, "resources/textures/slime_r.png"			, 2		, PLAYER_MOVE_ANIM_MSEC	, RENDER_LAYER_MOB },
	{ SLIME_L		, "resources/textures/slime_l.png"			, 2		, PLAYER_MOVE_ANIM_MSEC	, RENDER_LAYER_MOB },
	{ SLIME_ATTACK	, "resources/textures/slime_attack.png"		, 1		, PLAYER_MOVE_ANIM_MSEC	, RENDER_LAYER_EFFECT },

	{ GOBLIN_B		, "resources/textures/goblin_b.png"			, 2		, PLAYER_MOVE_ANIM_MSEC	, RENDER_LAYER_MOB },
	{ GOBLIN_R		, "resources/textures/goblin_r.png"			, 2		, PLAYER_MOVE_ANIM_MSEC	, RENDER_LAYER_MOB },
	{ GOBLIN_L		, "resources/textures/goblin_l.png"			, 2		, PLAYER_MOVE_ANIM_MSEC	, RENDER_LAYER_MOB },
	{ GOBLIN_ATTACK	, "resources/textures/goblin_attack.png"	, 1		, PLAYER_MOVE_ANIM_MSEC	, RENDER_LAYER_EFFECT },

	{ SHIELD		, "resources/textures/shield.png"			, 1		, DEFAULT_ANIM_MSEC		, RENDER_LAYER_ITEM },
	{ SWORD			, "resources/textures/sword.png"			, 1		, DEFAULT_ANIM_MSEC		, RENDER_LAYER_ITEM },
	{ AXE			, "resources/textures/axe.png"				, 1		, DEFAULT_ANIM_MSEC		, RENDER_LAYER_ITEM },
	{ POTION_HP		, "resources/textures/potion_hp.png"		, 1		, DEFAULT_ANIM_MSEC		, RENDER_LAYER_ITEM }
};

//----------
// texture property getters
//----------

/*
* Returns index in texture_names for the given texname.
*/
int tex_index_for_name(texname name) {

	for (unsigned i = 0; i < CountOf(texture_names); i++) {

		if (texture_names[i].num == name) {

			return i;
		}
	}
	d_printf(LOG_ERROR, "%s: texture not found: %d\n", __func__, (int)name);
	return 0;
}

/*
* Returns opengl texture index for the given texture type.
*/
unsigned int get_texture_id(texname name) {

	//return textures[tex_index_for_name(name)];
	return textures[tex_index_for_name(name)];
}

/*
* Returns texture framecount.
*/
int get_texture_framecount(texname name) {

	return texture_names[tex_index_for_name(name)].frame_count;
}

/*
* Returns texture frametime.
*/
int get_texture_frametime(texname name) {

	return texture_names[tex_index_for_name(name)].frame_msec;
}

/*
* Returns texture render layer.
*/
int get_texture_render_layer(texname name) {

	return texture_names[tex_index_for_name(name)].render_layer;
}

/*
* Loads a png texture and returns the opengl index for that texture.
*/
GLuint load_image(char *filename) {

	GLuint tex_id;
	GLint format;
	int w, h;
	int components = 0;

	//try to pull texture data from stb library
	unsigned char *data = stbi_load(filename, &w, &h, &components, 0);

	//if the file failed to load then components = 0 - there is no need for explicit load success check
	if(components != 4 && components != 3) { //4 = RGBA, 3 = RGB

		d_printf(LOG_ERROR, "%s: failed to load texture: %s, reason: %s\n", __func__, filename, stbi_failure_reason());
		return 0;
	}

	//generate opengl texture
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);

	//decide if this is a transparent texture
	format = components == 3 ? GL_RGB : GL_RGBA;

	//set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //don't interpolate colors when sampling the texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
	
	//the image was uploaded to the GPU, free it from the program's memory
	stbi_image_free(data);

	d_printf(LOG_TEXT, "%s: texture %s with id: %d\n", __func__, filename, tex_id);

	print_gl_errors(__func__);

	return tex_id;
}

/*
* Loads all textures using definitions from texture_names
*/
void load_textures(void) {

	int i;
	int texcount = CountOf(texture_names);

	if (texcount > MAX_TEXTURES) {

		d_printf(LOG_ERROR, "%s: too many textures defined\n", __func__);
		return;
	}

	for (i = 0; i < texcount; i++) {
		
		textures[i] = load_image(texture_names[i].name);
	}
}