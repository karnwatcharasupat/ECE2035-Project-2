#include "map.h"

#include "globals.h"
#include "graphics.h"
/**
 * The Map structure. This holds a HashTable for all the MapItems, along with
 * values for the width and height of the Map.
 */
struct Map {
    HashTable* items;
    int w, h;
};

#define NUM_MAPS 2
static Map maps[NUM_MAPS];
static int active_map;

/**
 * The first step in HashTable access for the map is turning the two-dimensional
 * key information (x, y) into a one-dimensional unsigned integer.
 * This function should uniquely map (x,y) onto the space of unsigned integers.
 */
static unsigned XY_KEY(int X, int Y) {
    // int and unsigned are 4 bytes = 32 bits
    unsigned int key = ((unsigned)X << 16) + (unsigned)Y;
    return key;
}

/**
 * This is the hash function actually passed into createHashTable. It takes an
 * unsigned key (the output of XY_KEY) and turns it into a hash value (some
 * small non-negative integer).
 */
#define NUM_BUCKETS 223
unsigned map_hash(unsigned key) {
    // TODO: Fix me!
    return key % NUM_BUCKETS;
}

/**
 * Initializes the internal structures for all maps. This does not populate
 * the map with items, but allocates space for them, initializes the hash tables, 
 * and sets the width and height.
 */
void maps_init() {
    // TODO: Implement!
    // Initialize hash table
    // Set width & height

    for (int i = 0; i < NUM_MAPS; i++) {
        maps[i].w = map_width();
        maps[i].h = map_height();
        maps[i].items = createHashTable(map_hash, NUM_BUCKETS);
    }
}

/**
 * Returns a pointer to the active map.
 */
Map* get_active_map() {
    return &maps[active_map];
}

/**
 * Sets the active map to map m, where m is the index of the map to activate.
 * Returns a pointer to the new active map.
 */
Map* set_active_map(int m) {
    active_map = m;
    return &maps[active_map];
}

/**
 * Returns the map m, regardless of whether it is the active map. This function
 * does not change the active map.
 */
Map* get_map(int m) {
    return &maps[m];
}

/**
 * Print the active map to the serial console.
 */
void print_map() {
    char lookup[] = {'W', 'D', 'P', 'A', 'K', 'C', 'N', ' ', 'S'};
    Map* map = get_active_map();
    for (int j = 0; j < map->h; j++) {
        for (int i = 0; i < map->w; i++) {
            MapItem* item = (MapItem*)getItem(map->items, XY_KEY(i, j));
            if (item)
                pc.printf("%c", lookup[item->type]);
            else
                pc.printf(" ");
        }
        pc.printf("\r\n");
    }
}

int map_width() {
    // FIX THIS
    return 100;
}

int map_height() {
    // FIX THIS
    return 50;
}

int map_area() {
    return map_width() * map_height();
}

MapItem* get_current(int x, int y) {
    Map* map = get_active_map();
    MapItem* item = (MapItem*)getItem(map->items, XY_KEY(x, y));
    return item;
}

MapItem* get_north(int x, int y) {
    //TODO: edge case y = 0;
    Map* map = get_active_map();
    MapItem* item = (MapItem*)getItem(map->items, XY_KEY(x, y + 1));
    return item;
}

MapItem* get_south(int x, int y) {
    //TODO: edge case y = y_max;
    Map* map = get_active_map();
    MapItem* item = (MapItem*)getItem(map->items, XY_KEY(x, y - 1));
    return item;
}

MapItem* get_east(int x, int y) {
    //TODO: edge case x = x_max;
    Map* map = get_active_map();
    MapItem* item = (MapItem*)getItem(map->items, XY_KEY(x + 1, y));
    return item;
}

MapItem* get_west(int x, int y) {
    //TODO: edge case x = 0;
    Map* map = get_active_map();
    MapItem* item = (MapItem*)getItem(map->items, XY_KEY(x - 1, y));
    return item;
}

MapItem* get_here(int x, int y) {
    Map* map = get_active_map();
    MapItem* item = (MapItem*)getItem(map->items, XY_KEY(x, y));
    return item;
}

/**
 * If there is a MapItem at (x,y), remove it from the map.
 */
void map_erase(int x, int y) {
    Map* map = get_active_map();
    removeItem(map->items, XY_KEY(x, y));
}

/**
 * Add WALL items in a line of length len beginning at (x,y).
 * If dir == HORIZONTAL, the line is in the direction of increasing x.
 * If dir == VERTICAL, the line is in the direction of increasing y.
 *
 * If there are already items in the map that collide with this line, they are
 * erased.
 */
void add_wall(int x, int y, int dir, int len) {
    for (int i = 0; i < len; i++) {
        MapItem* w1 = (MapItem*)malloc(sizeof(MapItem));
        w1->type = WALL;
        w1->draw = draw_wall;
        w1->walkable = false;
        w1->data = NULL;
        unsigned key = (dir == HORIZONTAL) ? XY_KEY(x + i, y) : XY_KEY(x, y + i);
        void* val = insertItem(get_active_map()->items, key, w1);
        if (val) free(val);  // If something is already there, free it
    }
}

/**
 * Add a PLANT item at (x,y). If there is already a MapItem at (x,y), erase it
 * before adding the plant.
 */
void add_plant(int x, int y) {
    MapItem* w1 = (MapItem*)malloc(sizeof(MapItem));
    w1->type = PLANT;
    w1->draw = draw_plant;
    w1->walkable = true;
    w1->data = NULL;
    void* val = insertItem(get_active_map()->items, XY_KEY(x, y), w1);
    if (val) free(val);  // If something is already there, free it
}

void add_character(int x, int y, Character* player_data) {
    MapItem* c = (MapItem*)malloc(sizeof(MapItem));
    c->type = CHARACTERSPRITE;
    if (player_data->team == 1) {
        c->draw = draw_player1sprite;
    } else if (player_data->team == 2) {
        c->draw = draw_player2sprite;
    }
    c->walkable = false;
    c->data = player_data;
    void* val = insertItem(get_active_map()->items, XY_KEY(x, y), (void*)c);
    if (val) free(val);  // If something is already there, free it
}

