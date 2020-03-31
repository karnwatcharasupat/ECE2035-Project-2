// Project includes
#include <math.h>
#include <stdio.h>

#include "globals.h"
#include "graphics.h"
#include "hardware.h"
#include "map.h"
#include "speech.h"

// Helper function declarations
void playSound(char* wav);

// Top down camera view
struct {
    int x, y;    // Current locations
    int px, py;  // Previous locations
    MapItem* selected;
    int cx, cy;  // Character location
} Camera;

/**
 * Given the game inputs, determine what kind of update needs to happen.
 * Possible return values are defined below.
 */
#define NO_ACTION 0
#define ACTION_BUTTON 1
#define BACK_BUTTON 2
#define GO_LEFT 3
#define GO_RIGHT 4
#define GO_UP 5
#define GO_DOWN 6

int get_action(GameInputs inputs) {
    /*
    struct GameInputs {
        int b1, b2, b3;     // Button presses
        double ax, ay, az;  // Accelerometer readings
    };
    */

    if (inputs.b1) {
        return ACTION_BUTTON;
    } else if (inputs.b2) {
        return BACK_BUTTON;
    } else {
        int action = get_action_from_accel(inputs);
        return action;
    }

    return NO_ACTION;
}

/**
 * Given the game inputs, determine what kind of movement needs to happen
 */
#define ACCEL_THRESHOLD 0.0
int get_action_from_accel(GameInputs inputs) {
    double x_abs = fabs(inputs.ax);
    double y_abs = fabs(inputs.ay);
    int x_dir = (inputs.ax > 0);  // 1 for RIGHT, 0 for LEFT
    int y_dir = (inputs.ay > 0);  // 1 for UP, 0 for DOWN

    if ((x_abs > ACCEL_THRESHOLD) || (y_abs > ACCEL_THRESHOLD)) {
        // if either value is greater than the threshold
        if (x_abs > y_abs) {
            // if absolute value of ax > ay
            return x_dir ? GO_RIGHT : GO_LEFT;
        } else {
            // if absolute value of ay > ax
            return y_dir ? GO_UP : GO_DOWN;
        }
    }

    return NO_ACTION;
}

/**
 * Update the game state based on the user action. For example, if the user
 * requests GO_UP, then this function should determine if that is possible by
 * consulting the map, and update the Player position accordingly.
 *
 * Return values are defined below. FULL_DRAW indicates that for this frame,
 * draw_game should not optimize drawing and should draw every tile, even if
 * the player has not moved.
 */
#define NO_RESULT 0
#define GAME_OVER 1
#define FULL_DRAW 2

#define MODE_FREE_ROAM 1
#define MODE_SELECTED 2
int update_game(int action, int* mode, int* active_player) {
    // Save player previous location before updating
    Camera.px = Camera.x;
    Camera.py = Camera.y;

    int update = -1;
    // Do different things based on the action
    // You can define smaller functions that get called for each case
    switch (action) {
        case ACTION_BUTTON:
            if (*mode == MODE_FREE_ROAM) {
                if (check_char_select(Camera.px, Camera.py, *active_player)) {
                    *mode = MODE_SELECTED;
                }
                update = NO_RESULT
            } else if (*mode == MODE_SELECTED) {
                update_move_character(Camera.px, Camera.py);
                end_turn(active_player, mode);
                update = NO_RESULT;
            }

            break;
        case BACK_BUTTON:
            if (*mode == MODE_FREE_ROAM) {
                end_turn(active_player, mode);
                update = NO_RESULT;
            } else if (*mode == MODE_SELECTED) {
                Camera->selected = NULL;
                *mode = MODE_FREE_ROAM;
                update = NO_RESULT;
            }
            break;
        case GO_UP:
        case GO_LEFT:
        case GO_DOWN:
        case GO_RIGHT:
            if (*mode == MODE_FREE_ROAM) {
                update_cursor(Camera.px, Camera.py, action);
            } else if (*mode == MODE_SELECTED) {
                update_char_cursor(Camera.px, Camera.py, action, Camera->selected->data->range);
            }
            update = NO_RESULT;
            break;
        default:
            break;
    }
    return NO_RESULT;
}

/**
 *  Return true if the cursor is on the character owned by the current player
 */
int check_char_select(int x, int y, int player_id) {
    Map* map = get_active_map();
    MapItem* item = get_current(x, y);
    if (item->type == CHARACTERSPRITE) {
        if (item->data->team == player_id) {
            Camera->selected = item;
            Camera->cx = x;
            Camera->cy = y;
            return true;
        }
    }

    return false;
}

void end_turn(int* active_player, int* mode) {
    Camera->selected = NULL;
    *active_player = ((*active_player) + 1) % NUM_PLAYERS;
    *mode = MODE_FREE_ROAM;
}

void update_move_character(int x, int y) {
    MapItem* character = Camera->selected;
    if (!character) {
        return -1;
    }

    add_character(x, y, character->data);
    map_erase(Camera->cx, Camera->cy);
}

void update_cursor(int x, int y, int dir) {
    MapItem* item;

    switch (dir) {
        case GO_UP:
            item = get_current(x, ++y);
            break;
        case GO_DOWN:
            item = get_current(x, --y);
            break;
        case GO_LEFT:
            item = get_current(--x, y);
            break;
        case GO_RIGHT:
            item = get_current(++x, y);
            break;
        default:
            item = NULL;
            printf("update_char_cursor::INVALID DIRECTION!");
            break;
    }

    if ((!item) || (item->type != WALL)) {
        Camera.x = x;
        Camera.y = y;
    }
}

void update_char_cursor(int x, int y, int dir, int range) {
    int cx = Camera->cx;
    int cy = Camera->cy;

    MapItem* item;

    switch (dir) {
        case GO_UP:
            item = get_current(x, ++y);
            break;
        case GO_DOWN:
            item = get_current(x, --y);
            break;
        case GO_LEFT:
            item = get_current(--x, y);
            break;
        case GO_RIGHT:
            item = get_current(++x, y);
            break;
        default:
            item = NULL;
            printf("update_char_cursor::INVALID DIRECTION!");
            break;
    }

    if ((!item) || (item->walkable)) {
        if (!(abs(x - cx) > range) && !(abs(y - cy) > range)) {
            Camera.x = x;
            Camera.y = y;
        }
    }
}

/**
 * Entry point for frame drawing. This should be called once per iteration of
 * the game loop. This draws all tiles on the screen, followed by the status
 * bars. Unless init is nonzero, this function will optimize drawing by only
 * drawing tiles that have changed from the previous frame.
 */
void draw_game(int init) {
    // Draw game border first
    if (init) draw_border();

    // Iterate over all visible map tiles
    for (int i = -5; i <= 5; i++) {      // Iterate over columns of tiles
        for (int j = -4; j <= 4; j++) {  // Iterate over one column of tiles
            // Here, we have a given (i,j)

            // Compute the current map (x,y) of this tile
            int x = i + Camera.x;
            int y = j + Camera.y;

            // Compute the previous map (px, py) of this tile
            int px = i + Camera.px;
            int py = j + Camera.py;

            // Compute u,v coordinates for drawing
            int u = (i + 5) * 11 + 3;
            int v = (j + 4) * 11 + 15;

            // Figure out what to draw
            DrawFunc draw = NULL;
            if (i == 0 && j == 0) {
                // Decide what to draw at the 0 position
                draw = draw_selection;
            } else if (x >= 0 && y >= 0 && x < map_width() && y < map_height()) {  // Current (i,j) in the map
                MapItem* curr_item = get_here(x, y);
                MapItem* prev_item = get_here(px, py);
                if (init || curr_item != prev_item) {  // Only draw if they're different
                    if (curr_item) {                   // There's something here! Draw it
                        draw = curr_item->draw;
                    } else {  // There used to be something, but now there isn't
                        draw = draw_nothing;
                    }
                }
            } else if (init) {  // If doing a full draw, but we're out of bounds, draw the walls.
                draw = draw_wall;
            }
            // Actually draw the tile
            if (draw) draw(u, v);
        }
    }

    // Draw status bars
    draw_upper_status();
    draw_lower_status();
}

/**
 * Initialize the main world map. Add walls around the edges, interior chambers,
 * and plants in the background so you can see motion.
 */
void init_main_map() {
    // "Random" plants
    Map* map = set_active_map(0);
    for (int i = map_width() + 3; i < map_area(); i += 39) {
        add_plant(i % map_width(), i / map_width());
    }
    pc.printf("plants\r\n");

    pc.printf("Adding walls!\r\n");
    add_wall(0, 0, HORIZONTAL, map_width());
    add_wall(0, map_height() - 1, HORIZONTAL, map_width());
    add_wall(0, 0, VERTICAL, map_height());
    add_wall(map_width() - 1, 0, VERTICAL, map_height());
    pc.printf("Walls done!\r\n");

    print_map();
}

/**
 * Program entry point! This is where it all begins.
 * This function orchestrates all the parts of the game. Most of your
 * implementation should be elsewhere - this holds the game loop, and should
 * read like a road map for the rest of the code.
 */
#define NUM_CHARACTERS 3
#define NUM_PLAYERS 2
int main() {
    // First things first: initialize hardware
    ASSERT_P(hardware_init() == ERROR_NONE, "Hardware init failed!");

    pc.printf("Program Starting");
    // Initialize the maps
    maps_init();
    init_main_map();

    Camera.x = Camera.px = 3;
    Camera.y = Camera.px = 3;

    // Initialize Characters
    Character characters[NUM_PLAYERS][NUM_CHARACTERS];
    for (int i = 0; i < NUM_PLAYERS; i++) {
        for (int j = 0; j < NUM_CHARACTERS; j++) {
            characters[i][j].atk = 10;
            characters[i][j].def = 20;
            characters[i][j].rng = 2;
            characters[i][j].health = 100;
            characters[i][j].team = i + 1;

            characters[i][j].x = i;
            characters[i][j].y = j;

            add_character(characters[i][j].x, characters[i][j].y, &characters[i][j]);
        }
    }

    // Initialize game state
    set_active_map(0);
    GameInputs inputs = read_inputs();
    int action = -1, update = -1;
    int mode = MODE_FREE_ROAM;
    int active_player = 1;

    // Initial drawing
    draw_game(true);

    // Main game loop
    while (1) {
        // Timer to measure game update speed
        Timer t;
        t.start();

        // 1. Read inputs
        inputs = read_inputs();
        // 2. Determine action (move, act, menu, etc.)
        action = get_action(inputs);
        // 3. Update game
        update = update_game(action, &mode, &active_player);
        // 3b. Check for game over
        if (update == GAME_OVER) {
            // call game over routine
        }
        // 4. Draw screen
        draw_game((update == FULL_DRAW));

        // Compute update time
        t.stop();
        int dt = t.read_ms();
        if (dt < 100) wait_ms(100 - dt);
    }
}
