#include "game.h"
#include <string.h>

sprite_t *sprite_map[MAP_SIZE][MAP_SIZE];
int map[MAP_SIZE][MAP_SIZE];

//the contents (items and mobs)
int map_contents[MAP_SIZE][MAP_SIZE];

//counters
int door_count;
int floor_count;

//marks the generation as failed
int is_failed;

int has_lock_room;

//creates an odd number from any number (never returns a zero)
#define MakeOddNumber(in) ((in) = (in) % 2 ? (in) : ((in) >= 0 ? (in) + 1 : (in) - 1))

//directions from tile
#define DIR_E	1		//x++ east
#define DIR_S	1<<1	//y-- south
#define DIR_W	1<<2	//x-- west
#define DIR_N	1<<3	//y++ north

//direction axes
#define AXIS_X (DIR_E | DIR_W)
#define AXIS_Y (DIR_S | DIR_N)

//direction signs
#define DIR_NEGATIVE (DIR_S | DIR_W)
#define DIR_POSITIVE (DIR_E | DIR_N)

//content types
#define CONTENT_MOB			0
#define CONTENT_ITEM		1

//----------
// helper functions
//----------

//returns a direction of the content
//the idea is that map items are bit shifted and mobs are on the lesser bits - unshifting will yeld 0 for mobs and > 0 if there's an item on the tile
int neighbor_has_content_of_type(int x, int y, int type) {

	if (x < MAP_SIZE - 1 && ((type == CONTENT_ITEM) ? (map_contents[x + 1][y] >> MAP_ITEM_BYTE_OFFSET) : (map_contents[x + 1][y] & 0xff))) {

		return DIR_E;
	}
	if (y > 0 && ((type == CONTENT_ITEM) ? (map_contents[x][y - 1] >> MAP_ITEM_BYTE_OFFSET) : (map_contents[x][y - 1] & 0xff))) {

		return DIR_S;
	}
	if (x > 0 && ((type == CONTENT_ITEM) ? (map_contents[x - 1][y] >> MAP_ITEM_BYTE_OFFSET) : (map_contents[x - 1][y] & 0xff))) {

		return DIR_W;
	}
	if (y < MAP_SIZE - 1 && ((type == CONTENT_ITEM) ? (map_contents[x][y + 1] >> MAP_ITEM_BYTE_OFFSET) : (map_contents[x][y + 1] & 0xff))) {

		return DIR_N;
	}
	return 0;
}

//returns a direction of the neighbor
int has_neighbor_of_type(int x, int y, int type) {

	if (x < MAP_SIZE - 1 && map[x + 1][y] == type) {

		return DIR_E;
	}
	if (y > 0 && map[x][y - 1] == type) {

		return DIR_S;
	}
	if (x > 0 && map[x - 1][y] == type) {

		return DIR_W;
	}
	if (y < MAP_SIZE - 1&& map[x][y + 1] == type) {
		
		return DIR_N;
	}
	return 0;
}

int count_neighbors_contents(int x, int y, int content) {

	int count = 0;

	if (x < MAP_SIZE - 1 && map_contents[x + 1][y] == content) {

		count++;
	}
	if (y > 0 && map_contents[x][y - 1] == content) {

		count++;
	}
	if (x > 0 && map_contents[x - 1][y] == content) {

		count++;
	}
	if (y < MAP_SIZE - 1 && map_contents[x][y + 1] == content) {

		count++;
	}
	return count;
}

int count_neighbors_of_type(int x, int y, int type) {

	int count = 0;

	if (x < MAP_SIZE - 1 && map[x + 1][y] == type) {

		count++;
	}
	if (y > 0 && map[x][y - 1] == type) {

		count++;
	}
	if (x > 0 && map[x - 1][y] == type) {

		count++;
	}
	if (y < MAP_SIZE - 1 && map[x][y + 1] == type) {

		count++;
	}
	return count;
}

int count_tiles_of_type(int type) {

	int count = 0;

	for (int x = 0; x < MAP_SIZE; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {

			if (map[x][y] == type) {

				count++;
			}
		}
	}
	return count;
}

int count_tiles_of_not_type(int type) {

	int count = 0;

	for (int x = 0; x < MAP_SIZE; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {

			if (map[x][y] != type) {

				count++;
			}
		}
	}
	return count;
}

//returns 1 if the given bounds overlap another room/hallway space
int is_overlapping(int x_start, int y_start, int x_end, int y_end) {

	//keep the bounds inside the map array
	if (x_start < 0 || y_start < 0 || x_end >= MAP_SIZE || y_end >= MAP_SIZE) {

		return 1;
	}

	//check every tile within bounds
	for (int x = x_start; x <= x_end; x++) {
		for (int y = y_start; y <= y_end; y++) {

			if (map[x][y] != TILE_EMPTY && map[x][y] != TILE_WALL && map[x][y] != TILE_DOOR) { //ignore walls and doors!

				return 1;
			}
		}
	}
	return 0;
}

//----------
// doors
//----------

void add_random_doors(int start_x, int start_y, int end_x, int end_y) {

	int coord, x, y;
	int random_side = Random(0, 3); //make sure at least one door is created

	//bounds edge is 3 units or less long?
	int x_is_tiny = abs(start_x - end_x) < 3;
	int y_is_tiny = abs(start_y - end_y) < 3;

	//for each direction pick random entrances
	for (int i = 0; i < 4; i++) {

		if (RandomBool || i == random_side) {

			random_side = -1; //no need to force door anymore

			// a wall with length 3 can only have doors in the middle
			if ((i < 2 && x_is_tiny) ||
				y_is_tiny) {

				coord = (i < 2 ? start_x : start_y) + 1;
			}
			else
			{
				//pick random coordinate along the edge
				coord = (i < 2 ? start_x : start_y) + 1 + rand() % ((i < 2 ? (end_x - start_x) : (end_y - start_y)) - 1);
			}

			//set door coordinates
			x = (i < 2 ? coord : (i % 2 ? start_x : end_x));
			y = (i >= 2 ? coord : (i % 2 ? start_y : end_y));

			if (map[x][y] != TILE_WALL) {

				if (map[x][y] != TILE_DOOR) { //picking already created doors is not an issue

					//should never happen: this means the generator is broken (rooms overlapping / doors in an empty space)
					d_printf(LOG_ERROR, "%s: door not in a wall\n", __func__);
					is_failed = 1;
				}
				continue;
			}

			map[x][y] = TILE_DOOR;

			door_count++;
		}
	}
}

void pick_random_doors(int *out_x, int *out_y) {

	int n = Random(0, door_count - 1); //door number
	int m = 0;

	//skip doors until the correct number is found
	for (int x = 0; x < MAP_SIZE; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {

			if (map[x][y] == TILE_DOOR) {

				if (n == m) {

					*out_x = x;
					*out_y = y;
					return;
				}
				m++;
			}
		}
	}
	d_printf(LOG_ERROR, "%s: found no door\n", __func__);
	is_failed = 1;
}

//fixes doors after creating the map
void fix_doors(void) {

	int count = 0;

	//remove doors to nowhere
	for (int x = 0; x < MAP_SIZE; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {

			if (map[x][y] == TILE_DOOR && count_neighbors_of_type(x, y, TILE_FLOOR) != 2) {

				map[x][y] = TILE_WALL;
				door_count--;
				count++;
			}
		}
	}

	//remove side-by-side doors
	for (int x = 0; x < MAP_SIZE; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {

			if (map[x][y] == TILE_DOOR && has_neighbor_of_type(x, y, TILE_DOOR)) {

				map[x][y] = TILE_WALL;
				door_count--;
				count++;
			}
		}
	}
	d_printf(LOG_INFO, "%s: fixed %d doors\n", __func__, count);
}

void remove_some_doors(void) {

	int total = (int)(door_count * 0.4f);
	int count = 0;
	int x, y;

	for (int i = 0; i < total; i++) {

		pick_random_doors(&x, &y);

		map[x][y] = TILE_FLOOR;

		door_count--;
		count++;
	}

	d_printf(LOG_INFO, "%s: removed %d doors\n", __func__, count);
}

//----------
// feature generation
//----------

void make_room_start_end(int start_x, int start_y, int end_x, int end_y) {

	if(start_x < 0 || start_y < 0 || end_x >= MAP_SIZE || end_y >= MAP_SIZE) {

		d_printf(LOG_ERROR, "%s: room out of map bounds\n", __func__);
		is_failed = 1;
		return;
	}

	for (int x = start_x; x <= end_x; x++) {
		for (int y = start_y; y <= end_y; y++) {

			if (map[x][y] != TILE_EMPTY) {

				if (map[x][y] == TILE_FLOOR) {

					//should never happen: this means the room is misaligned
					d_printf(LOG_ERROR, "%s: tile not empty\n", __func__);
					is_failed = 1;
				}

				//walls or doors overlapping are fine
				continue;
			}

			if (x == start_x || y == start_y || x == end_x || y == end_y) {

				//edge is a wall
				map[x][y] = TILE_WALL;
			}
			else
			{
				//inside is a floor
				map[x][y] = TILE_FLOOR;
				floor_count++;
			}
		}
	}

	//add doors
	add_random_doors(start_x, start_y, end_x, end_y);
}

//make room using a center point
void make_room(int x_center, int y_center, int x_size, int y_size) {

	int start_x = x_center - x_size / 2;
	int start_y = y_center - y_size / 2;

	int end_x = x_center + x_size / 2;
	int end_y = y_center + y_size / 2;

	make_room_start_end(start_x, start_y, end_x, end_y);
}

//hallway is described as a start point and an end point
void make_hallway(int start_x, int start_y, int end_x, int end_y) {

	//no support for diagonal hallways
	if (start_x != end_x && start_y != end_y) {

		d_printf(LOG_ERROR, "%s: hallway is diagonal\n", __func__);
		is_failed = 1;
		return;
	}

	//expand bounds in the correct direction
	if (start_x == end_x) {

		start_x--;
		end_x++;
	}
	else
	{
		start_y--;
		end_y++;
	}

	//went out of bounds...
	if (start_x < 0 || start_y < 0 || end_x >= MAP_SIZE || end_y >= MAP_SIZE) {

		d_printf(LOG_ERROR, "%s: hallway out of map bounds\n", __func__);
		is_failed = 1;
		return;
	}

	for (int x = start_x; x <= end_x; x++) {
		for (int y = start_y; y <= end_y; y++) {

			if (map[x][y] != TILE_EMPTY) {

				continue;
			}

			if (x == start_x || y == start_y || x == end_x || y == end_y) {

				map[x][y] = TILE_WALL;
			}
			else
			{
				map[x][y] = TILE_FLOOR;
				floor_count++;
			}
		}
	}

	//add doors
	add_random_doors(start_x, start_y, end_x, end_y);
}

//tries to add a room or a hallway starting from a door
void make_feature(void) {

	//door coordinates
	int x0 = 0;
	int y0 = 0;

	//feature expansion direction
	int direction;

	int i, offset;
	int mins_x, mins_y;
	int maxs_x, maxs_y;
	int offset_x, offset_y;

	//is this a room or a hallway feature?
	int is_room = RandomBool;

	//try find a door with nothing on the other side
	for (i = 0; i < 500; i++) {

		pick_random_doors(&x0, &y0);

		//make sure the picked door leads to an empty space
		direction = has_neighbor_of_type(x0, y0, TILE_EMPTY);

		if (direction) {

			break;
		}
	}
	if (i == 500) {

		//this is fine
		//d_printf(LOG_TEXT, "%s: failed to make a feature\n", __func__);
		return;
	}

	if (is_room) {

		//find room coordinates
		for (i = 0; i < 100; i++) {

			//room size
			offset_x = Random(MIN_FEATURE_SIZE, MAX_FEATURE_SIZE);
			offset_y = Random(MIN_FEATURE_SIZE, MAX_FEATURE_SIZE);

			MakeOddNumber(offset_x);
			MakeOddNumber(offset_y);

			//door edge offset
			offset = Random(0, ((direction & AXIS_X ? offset_x : offset_y) / 2) - 1);
			offset = 0;

			//room bounds
			mins_x = direction & AXIS_X ? (direction & DIR_NEGATIVE ? x0 - offset_x : x0) : x0 - offset_x / 2;
			mins_y = direction & AXIS_X ? y0 - offset_x / 2 : (direction & DIR_NEGATIVE ? y0 - offset_y : y0);
			maxs_x = mins_x + offset_x;
			maxs_y = mins_y + offset_y;

			if (mins_x < 0 || mins_y < 0 || maxs_x >= MAP_SIZE || maxs_y >= MAP_SIZE) {

				//out of bounds, try again
				continue;
			}

			if (!is_overlapping(mins_x, mins_y, maxs_x, maxs_y)) {

				//valid room
				break;
			}
		}

		if (i == 100) {

			//this is fine
			//d_printf(LOG_TEXT, "%s: failed to make room\n", __func__);
			return;
		}

		make_room_start_end(mins_x, mins_y, maxs_x, maxs_y);
	}
	else
	{
		//find hallway coordinates
		for (i = 0; i < 100; i++) {

			//hallway length
			offset = Random(MIN_FEATURE_SIZE, MAX_FEATURE_SIZE);

			offset_x = x0 + (direction & AXIS_X ? (offset * (direction & DIR_NEGATIVE ? -1 : 1)) : 0);
			offset_y = y0 + (direction & AXIS_X ? 0 : (offset * (direction & DIR_NEGATIVE ? -1 : 1)));

			//hallway bounds
			mins_x = direction & DIR_NEGATIVE ? offset_x : x0;
			mins_y = direction & DIR_NEGATIVE ? offset_y : y0;
			maxs_x = direction & DIR_NEGATIVE ? x0 : offset_x;
			maxs_y = direction & DIR_NEGATIVE ? y0 : offset_y;

			if (!is_overlapping(mins_x, mins_y, maxs_x, maxs_y)) {

				//valid hallway
				break;
			}
		}

		if (i == 100) {

			//this is fine
			//d_printf(LOG_TEXT, "%s: failed to make hallway\n", __func__);
			return;
		}

		make_hallway(mins_x, mins_y, maxs_x, maxs_y);
	}
}

//----------
// water
//----------

//recursively fills neighbor tiles with water
void flood_fill_water(int x, int y) {

	int x0, y0;
	
	if (map[x][y] == TILE_FLOOR) {

		floor_count--;
	}

	map[x][y] = TILE_WATER;

	for (int i = 0; i < 4; i++) {

		x0 = x + (i % 2) * (i > 1 ? 1 : -1);
		y0 = y + (!(i % 2)) * (i > 1 ? 1 : -1);

		if (map[x0][y0] != TILE_WATER && !has_neighbor_of_type(x0, y0, TILE_WALL) && !has_neighbor_of_type(x0, y0, TILE_DOOR) &&
			x0 != MAP_OFFSET && y0 != MAP_OFFSET && RandomBool) {

			flood_fill_water(x0, y0);
		}
	}
}

//creates water on the map
void add_water_pools(void) {

	int skip;
	int current;
	int water_count = 0;

	for (int i = 0; i < 300; i++) {

restart:
		skip = Random(0, floor_count); //pick a random number of floor tiles to skip
		current = 0;
		for (int x = 0; x < MAP_SIZE; x++) {
			for (int y = 0; y < MAP_SIZE; y++) {

				if (current == skip && map[x][y] == TILE_FLOOR && 
					!has_neighbor_of_type(x, y, TILE_WALL) && !has_neighbor_of_type(x, y, TILE_DOOR) && !has_neighbor_of_type(x, y, TILE_WATER) && !has_neighbor_of_type(x, y, TILE_LOCK_DOOR) &&
					(x > MAP_OFFSET + MAP_SAFE_ZONE || x < MAP_OFFSET - MAP_SAFE_ZONE) && (y > MAP_OFFSET + MAP_SAFE_ZONE || y < MAP_OFFSET - MAP_SAFE_ZONE) ) {

					flood_fill_water(x, y);
					water_count++;

					i++;
					goto restart;
				}
				if (map[x][y] == TILE_FLOOR) {

					current++;
				}
			}
		}
	}

	if (water_count) {

		d_printf(LOG_INFO, "%s: map with %d water pools\n", __func__, water_count);
	}
	else
	{
		d_printf(LOG_INFO, "%s: map with no water\n", __func__);
	}
}

//----------
// exit
//----------

void make_exit(void) {

	//pick a random floor tile
	int skip = Random(0, floor_count);
	int current = 0;
	int i, x, y;

	//FIXME: low success ratio
	for (i = 0; i < 15000; i++) {

		current = 0;
		for (x = 0; x < MAP_SIZE; x++) {
			for (y = 0; y < MAP_SIZE; y++) {

				if (current == skip) {

					//keep the "small" safe zone in mind
					if (map[x][y] == TILE_FLOOR &&
						(x > MAP_OFFSET + MAP_SAFE_ZONE / 2 || x < MAP_OFFSET - MAP_SAFE_ZONE / 2) &&
						(y > MAP_OFFSET + MAP_SAFE_ZONE / 2 || y < MAP_OFFSET - MAP_SAFE_ZONE / 2) &&
						map_contents[x][y] == MAP_NOTHING) {

						map[x][y] = TILE_EXIT;
						floor_count--;

						d_printf(LOG_INFO, "%s: dungeon exit at [%d, %d]\n", __func__, x - MAP_OFFSET, y - MAP_OFFSET);
						return;
					}
				}

				if (map[x][y] == TILE_FLOOR) {

					current++;
				}
			}
		}
	}

	d_printf(LOG_ERROR, "%s: failed to make dungeon exit!\n", __func__);
	is_failed = 1;
}

//----------
// map contents
//----------

//adds a random item
void make_item(int x, int y) {

	int random = Random(1, 8);

	if (random > 6) {

		map_contents[x][y] = MAP_ITEM_SWORD;
	}
	else if(random > 3)
	{
		map_contents[x][y] = MAP_ITEM_SHIELD;
	}
	else {

		map_contents[x][y] = MAP_ITEM_POTION_HP;
	}
}

//adds a random mob
void make_mob(int x, int y) {

	map_contents[x][y] = MAP_MOB_SLIME; //only one mob type so far
}


//randomly puts the given amount of content on the map
void add_contents_of_type(int type, int count) {

	int skip;
	int current;
	int total_attempts = 0;
	int i = 0;

	//place contents
restart:
	while (i < count) {

		if (total_attempts > 1000) {

			d_printf(LOG_TEXT, "%s: too many total attempts\n", __func__);
			break;
		}

		skip = Random(0, floor_count); //amount of floor tiles to skip
		current = 0;
		for (int x = 0; x < MAP_SIZE; x++) {
			for (int y = 0; y < MAP_SIZE; y++) {

				if (current == skip) {

					if (map[x][y] == TILE_FLOOR &&												//must be on the floor
						(x > MAP_OFFSET + MAP_SAFE_ZONE || x < MAP_OFFSET - MAP_SAFE_ZONE) &&	//keep the safe zone in mind - mobs spawning next to a player are bad
						(y > MAP_OFFSET + MAP_SAFE_ZONE || y < MAP_OFFSET - MAP_SAFE_ZONE) &&
						map_contents[x][y] == MAP_NOTHING &&									//make sure nothing is placed on this tile
						!neighbor_has_content_of_type(x, y, type) &&							//don't spawn side by side with another content of the same type
						!has_neighbor_of_type(x, y, TILE_DOOR)) {								//don't spawn next to a door

						switch (type)
						{
							case CONTENT_ITEM:
								make_item(x, y);
								break;
							case CONTENT_MOB:
								make_mob(x, y);
								break;
						}

						//next
						i++;
						goto restart;
					}
					total_attempts++;
				}

				if (map[x][y] == TILE_FLOOR) {

					current++;
				}
			}
		}
	}
}

void add_map_contents(void) {

	int count;

	//items
	count = Random(MAX_ITEMS / 2, MAX_ITEMS);
	d_printf(LOG_INFO, "%s: placing %d items...\n", __func__, count);
	add_contents_of_type(CONTENT_ITEM, count);

	//mobs
	count = Random(MAX_MOBS / 2, MAX_MOBS);
	d_printf(LOG_INFO, "%s: placing %d mobs...\n", __func__, count);
	add_contents_of_type(CONTENT_MOB, count);
}

//----------
// locked room
//----------

void make_locked_room_chest(int x_min, int y_min, int x_max, int y_max) {

	//pick a random position inside the locked room
	int x = Random(x_min + 1, x_max - 1);
	int y = Random(y_min + 1, y_max - 1);

	map[x][y] = TILE_CHEST;
	floor_count--;

	d_printf(LOG_INFO, "%s: chest at [%d, %d]\n", __func__, x, y);
}

void make_locked_room(void) {

	int x0 = 1, y0 = 1;
	int i, direction, offset_x, offset_y, mins_x, mins_y, maxs_x, maxs_y;
	//try find a door with nothing on the other side
	for (i = 0; i < 500; i++) {

		pick_random_doors(&x0, &y0);

		//make sure the picked door leads to an empty space
		direction = has_neighbor_of_type(x0, y0, TILE_EMPTY);

		if (direction) {

			break;
		}
	}
	if (i == 500) {

		//this is fine
		d_printf(LOG_ERROR, "%s: failed to make the locked room\n", __func__);
		return;
	}

	//find room coordinates
	for (i = 0; i < 100; i++) {

		//room size
		offset_x = Random(MIN_FEATURE_SIZE, MAX_FEATURE_SIZE);
		offset_y = Random(MIN_FEATURE_SIZE, MAX_FEATURE_SIZE);

		MakeOddNumber(offset_x);
		MakeOddNumber(offset_y);

		//room bounds
		mins_x = direction & AXIS_X ? (direction & DIR_NEGATIVE ? x0 - offset_x : x0) : x0 - offset_x / 2;
		mins_y = direction & AXIS_X ? y0 - offset_x / 2 : (direction & DIR_NEGATIVE ? y0 - offset_y : y0);
		maxs_x = mins_x + offset_x;
		maxs_y = mins_y + offset_y;

		if (mins_x < 0 || mins_y < 0 || maxs_x >= MAP_SIZE || maxs_y >= MAP_SIZE) {

			//out of bounds, try again
			continue;
		}

		if (!is_overlapping(mins_x, mins_y, maxs_x, maxs_y)) {

			for (int x1 = mins_x; x1 < maxs_x; x1++) {

				if (x1 != x0 && (map[x1][mins_y] == TILE_DOOR || map[x1][maxs_y] == TILE_DOOR)) {

					//has random door somewhere
					continue;
				}
			}
			for (int y1 = mins_y; y1 < maxs_y; y1++) {

				if (y1 != y0 && (map[mins_x][y1] == TILE_DOOR || map[maxs_x][y1] == TILE_DOOR)) {

					//has random door somewhere
					continue;
				}
			}

			//valid room
			break;
		}
	}
	if (i == 100) {

		d_printf(LOG_ERROR, "%s: couldn't fit a room\n", __func__);
		is_failed = 1;
		return;
	}

	if (mins_x < 0 || mins_y < 0 || maxs_x >= MAP_SIZE || maxs_y >= MAP_SIZE) {

		d_printf(LOG_ERROR, "%s: room out of map bounds\n", __func__);
		is_failed = 1;
		return;
	}

	for (int x = mins_x; x <= maxs_x; x++) {
		for (int y = mins_y; y <= maxs_y; y++) {

			if (map[x][y] != TILE_EMPTY) {

				if (map[x][y] == TILE_FLOOR) {

					//should never happen: this means the room is misaligned
					d_printf(LOG_ERROR, "%s: tile not empty\n", __func__);
					is_failed = 1;
				}
				else if (map[x][y] == TILE_DOOR) {

					map[x][y] = TILE_WALL; //seal all other entrances
				}

				//walls or doors overlapping are fine
				continue;
			}

			if (x == mins_x || y == mins_y || x == maxs_x || y == maxs_y) {

				//edge is a wall
				map[x][y] = TILE_WALL;
			}
			else
			{
				//inside is a floor
				map[x][y] = TILE_FLOOR;
				map_contents[x][y] = MAP_ITEM_LOCK_ROOM;
				floor_count++;
			}
		}
	}

	map[x0][y0] = TILE_LOCK_DOOR;
	door_count--;

	make_locked_room_chest(mins_x, mins_y, maxs_x, maxs_y);

	d_printf(LOG_INFO, "%s: locked door at [%d, %d]\n", __func__, x0, y0);
}

//----------
// sprite builder
//----------

//generates tile sprites from the map array
void build_sprites(void) {

	sprite_t *s;
	texname tname;
	int collision_mask;
	int anim_pause;
	int rotation;
	void (*action)(sprite_t *s);
	void *object_data;

	//for each tile...
	for (int x = 0; x < MAP_SIZE; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {

			anim_pause = 0;
			rotation = 0;
			action = NULL;
			object_data = NULL;

			switch (map[x][y])
			{
				case TILE_WALL:
					tname = WALL;
					collision_mask = COLLISION_WALL;
					break;
				case TILE_FLOOR:
					tname = ROCK;
					collision_mask = COLLISION_FLOOR;
					rotation = Random(0, 3);
					break;
				case TILE_WATER:
					tname = WATER;
					collision_mask = COLLISION_WATER;
					rotation = Random(0, 3);
					break;
				case TILE_DOOR:
					tname = DOOR;
					anim_pause = 1;
					collision_mask = COLLISION_OBSTACLE;
					action = door_action;
					break;
				case TILE_LOCK_DOOR:
					tname = LOCKED_DOOR;
					anim_pause = 1;
					collision_mask = COLLISION_OBSTACLE;
					action = locked_door_action;
					break;
				case TILE_EXIT:
					tname = LEVEL_EXIT;
					anim_pause = 1;
					collision_mask = COLLISION_FLOOR;
					action = next_level_action;
					break;
				case TILE_CHEST:
					tname = CHEST;
					anim_pause = 1;
					collision_mask = COLLISION_FLOOR;
					action = chest_action;
					if (RandomBool) {

						object_data = malloc(sizeof(int)); //armor chest type
					}
					break;
				default:
					continue;
			}

			//create a new sprite
			s = new_sprite();

			vec2_t position;
			position[VEC_X] = (float)((x * SPRITE_SIZE * 2) - MAP_OFFSET); //map has an offset (the real center is at [0, 0] but
			position[VEC_Y] = (float)((y * SPRITE_SIZE * 2) - MAP_OFFSET); //the array can't have negative indexes)

			Vec2Copy(position, s->position);

			s->tex_id = get_texture_id(tname);
			s->framecount = get_texture_framecount(tname);
			s->render_layer = get_texture_render_layer(tname);
			s->frame_msec = get_texture_frametime(tname);
			s->render_layer = get_texture_render_layer(tname);
			s->animation_pause = anim_pause;
			s->collision_mask = collision_mask;
			s->action = action;
			s->rotation = rotation;
			s->object_data = object_data;

			sprite_map[x][y] = s;
		}
	}
}

//removes all map sprites before creating a new map
void clear_sprite_map(void) {

	for (int x = 0; x < MAP_SIZE; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {

			if (sprite_map[x][y]) {

				delete_sprite(sprite_map[x][y]);
			}
		}
	}

	//wipe all data
	memset(&sprite_map, 0, sizeof(sprite_t *) * MAP_SIZE * MAP_SIZE);
}

//creates a new map
void generate_map(void) {

	int tile_count = 0;

	d_printf(LOG_INFO, "%s: generating a map...\n", __func__);

	srand((unsigned int)time(NULL));

	clear_sprite_map();

	//compensate for the small chance of map generator failure
	for (int k = 0; k < 10; k++) { //10 attempts should be enough?

		//reset counters
		is_failed = 0;
		door_count = 0;
		floor_count = 0;
		has_lock_room = 0;

		//wipe map data
		memset(&map, TILE_EMPTY, sizeof(int) * MAP_SIZE * MAP_SIZE);
		memset(&map_contents, 0, sizeof(int) * MAP_SIZE * MAP_SIZE);

		//make center room
		make_room(MAP_OFFSET, MAP_OFFSET, Random(MIN_FEATURE_SIZE, MAX_FEATURE_SIZE), Random(MIN_FEATURE_SIZE, MAX_FEATURE_SIZE));

		//500 attempts to make a feature (usually enough for MAP_BUDGET < 2000)
		for (int i = 0; i < 500; i++) {

			make_feature();

			tile_count = count_tiles_of_not_type(TILE_EMPTY);
			if (tile_count > MAP_BUDGET) {

				break;
			}
		}

		if (is_failed || tile_count < MAP_BUDGET) {

			d_printf(LOG_ERROR, "%s: map budget not met: %d tiles out of %d\n", __func__, tile_count, MAP_BUDGET);
			goto retry;
		}

		//add locked room
		make_locked_room();

		//remove redundant doors
		fix_doors();

		if (is_failed) {

			goto retry;
		}

		//add water tiles
		add_water_pools();

		//make an exit
		make_exit();

		//remove some doors to make the map feel more open
		remove_some_doors();

		if (!is_failed) {

			break;
		}

retry:
		if (k < 4) {

			d_printf(LOG_WARNING, "%s: failed to generate the map, retrying... (%d tiles)\n", __func__, tile_count);
		}
	}

	//still failed...
	if (is_failed) {

		d_printf(LOG_ERROR, "%s: failed to generate the map (%d tiles)\n", __func__, tile_count);
	}
	
	d_printf(LOG_INFO, "%s: map tiles: %d\n", __func__, tile_count);

	//set map contents
	add_map_contents();

	//build the sprite map
	build_sprites();
}