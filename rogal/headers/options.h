#include "shared.h"
#include "text.h"

//options texts are smaller than menu buttons
#define OPTIONS_TEXT_SCALE 0.75f

typedef struct {
	char	*name;					//display name
	void	(*on_set)(int value);	//function that handles value change of this option
	int		value;					//value cache

	text_t *text;					//option's text reference
} option_t;

void toggle_options(int are_enabled);
void generate_options_texts(void);