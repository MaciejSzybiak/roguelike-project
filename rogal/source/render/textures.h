#include "shared.h"

//for stb_image
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION

//maximum texture count
#define MAX_TEXTURES 32

//default texture properties
#define DEFAULT_ANIM_MSEC 100
#define PLAYER_MOVE_ANIM_MSEC 100

typedef struct {
	texname num;			//ID
	char	*name;			//file name (relative path)
	int		frame_count;	//the amount of frames on this texture
	int		frame_msec;		//the amount of msec for each frame
	int		render_layer;	//default render layer
} texentry_t;

//for initialization
void load_textures(void);