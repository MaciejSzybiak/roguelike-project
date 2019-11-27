#include "shared.h"
#include "text.h"

#define UI_SPACING			0.1f
#define MMENU_SIZES			1.5f

#define HUD_SPACING			2.f

#define MMENU_PLAY			0
#define MMENU_OPTIONS		1
#define MMENU_QUIT			2

#define HUD_HP				0
#define HUD_ARMOR			1
#define HUD_DMG				2

#define DEFAULT_MESSAGE_MSEC 3000

void generate_ui(void);
void toggle_main_menu(int enabled);

void hud_update_health(void);
void hud_update_armor(void);
void hud_update_dmg(void);

//message text
void display_message(char *message, int msec, color3_t color);
void disable_message_text(void);