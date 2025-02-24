// Project includes
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>

#include "globals.h"
#include "graphics.h"
#include "hardware.h"
#include "map.h"
#include "sound_fx.h"
#include "speech.h"

// Helper function declarations
void playSound(char* wav);
int get_action_from_accel(GameInputs inputs);
int check_char_select(int x, int y, int player_id);
void end_turn(int* active_player, int* mode);
void update_move_character(int x, int y);
void update_cursor(int x, int y, int dir);
void update_char_cursor(int x, int y, int dir, int range);
int check_attack(int x, int y, int player_id);
int attack_routine(int x, int y, int en_x, int en_y, int player_id);
int attack(Character* attacker, Character* defender);
int is_game_over();
void draw_game(int, int);
void check_potion(int, int, Character*);
void draw_possible_moves(int x, int y, int range);
int attack_success(int);

// Top down camera view
#define MAX_ROUTE 100
struct {
    int x, y;    // Current locations
    int px, py;  // Previous locations
    Character* selected;
    int cx, cy;  // Character location
    int charCount[NUM_PLAYERS];
    int routes[MAX_ROUTE][2];
    int health[2];
    int winner;
    int charMoved;
    Character* charMovedPtr[NUM_CHARACTERS];
    int* active_player;
} Camera;

void sound_fx(const char* filename) {
    FILE* file;
    pc.printf("%s opening\n", filename);
    file = fopen(filename, "r");

    if (!file) {
        pc.printf("file not found!\n");
        return;
    }

    pc.printf("playing\n");
    waver.play(file);
    fclose(file);
    pc.printf("played\n");
}

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
#define INFO_BUTTON 7

int get_action(GameInputs inputs) {
    if (inputs.b1) {
        pc.printf("ACTION\n");
        return ACTION_BUTTON;
    } else if (inputs.b2) {
        pc.printf("BACK\n");
        return BACK_BUTTON;
    } else if (inputs.b3) {
        pc.printf("INFO\n");
        return INFO_BUTTON;
    } else {
        int action = get_action_from_accel(inputs);
        return action;
    }
}

/**
 * Given the game inputs, determine what kind of movement needs to happen
 */
#define ACCEL_THRESHOLD 0.2
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
                    pc.printf("MODE: SELECTED\n");
                }
                update = NO_RESULT;
            } else if (*mode == MODE_SELECTED) {
                update_move_character(Camera.px, Camera.py);
                update = check_attack(Camera.px, Camera.py, *active_player);

                if (update != GAME_OVER) {
                    if (Camera.charMoved == Camera.charCount[*(Camera.active_player) - 1]) {
                        draw_game(true, *mode);
                        end_turn(active_player, mode);
                    } else {
                        *mode = MODE_FREE_ROAM;
                        draw_game(true, *mode);
                    }
                }
                pc.printf("character moved\n");
            }

            break;
        case BACK_BUTTON:
            if (*mode == MODE_FREE_ROAM) {
                end_turn(active_player, mode);
                pc.printf("character released\n");
            } else if (*mode == MODE_SELECTED) {
                Camera.selected = NULL;
                *mode = MODE_FREE_ROAM;
            }
            update = FULL_DRAW;
            break;
        case INFO_BUTTON:
            if (check_char_select(Camera.px, Camera.py, 0)) {
                Character* character = Camera.selected;
                Camera.selected = NULL;
                draw_info(character);

                GameInputs inputs = read_inputs();
                while (!inputs.b3) {
                    inputs = read_inputs();
                }
                update = FULL_DRAW;
            }
            break;
        case GO_UP:
        case GO_LEFT:
        case GO_DOWN:
        case GO_RIGHT:
            if (*mode == MODE_FREE_ROAM) {
                update_cursor(Camera.px, Camera.py, action);
                update = NO_RESULT;
            } else if (*mode == MODE_SELECTED) {
                update_char_cursor(Camera.px, Camera.py, action, Camera.selected->range);
                update = FULL_DRAW;
            }

            break;
        default:
            update = NO_RESULT;
            break;
    }

    return update;
}

#define END_ATTACK 0
#define CONTINUE_ATTACK 3
int check_attack(int x, int y, int player_id) {
    int con = attack_routine(x, y, x, y + 1, player_id);

    if (Camera.winner = is_game_over()) {
        return GAME_OVER;
    }

    if (con) {
        con = attack_routine(x, y, x, y - 1, player_id);

        if (Camera.winner = is_game_over()) {
            return GAME_OVER;
        }
    }

    if (con) {
        con = attack_routine(x, y, x + 1, y, player_id);
        if (Camera.winner = is_game_over()) {
            return GAME_OVER;
        }
    }

    if (con) {
        con = attack_routine(x, y, x - 1, y, player_id);
        if (Camera.winner = is_game_over()) {
            return GAME_OVER;
        }
    }

    return FULL_DRAW;
}

int is_game_over() {
    // only support 2 player at the moment
    if (Camera.charCount[0] == 0) {
        return 2;
    } else if (Camera.charCount[1] == 0) {
        return 1;
    } else {
        return 0;
    }
}

#define NO_CASUALTY 0
#define ENEMY_DEAD 1
#define PLAYER_DEAD 2

int attack_routine(int x, int y, int en_x, int en_y, int player_id) {
    Character* currentChar = Camera.selected;
    MapItem* item = get_here(en_x, en_y);
    if (!item) {
        return CONTINUE_ATTACK;
    }

    int result = -1;

    if (item->type == CHARACTERSPRITE) {
        Character* enemy = (Character*)(item->data);
        if (enemy->team != player_id) {
            draw_enemybox(en_x - Camera.x, en_y - Camera.y);
            speech("Attack?", "Yes: Action");

            while (result < 0) {
                GameInputs inputs = read_inputs();
                if (inputs.b1) {
                    pc.printf("changing sprite to attack\n");
                    MapItem* currentCharItem = get_here(x, y);
                    if (player_id == 1) {
                        pc.printf("player 1");
                        currentCharItem->draw = draw_player1attack;
                        item->draw = draw_player2attack;
                    } else if (player_id == 2) {
                        pc.printf("player 2");
                        currentCharItem->draw = draw_player2attack;
                        item->draw = draw_player1attack;
                    }
                    pc.printf("changed sprite\n");
                    draw_game(true, MODE_FREE_ROAM);
                    pc.printf("redrawn\n");
                    if (player_id == 1) {
                        pc.printf("player 1");
                        currentCharItem->draw = draw_player1sprite;
                        item->draw = draw_player2sprite;
                    } else if (player_id == 2) {
                        pc.printf("player 2");
                        currentCharItem->draw = draw_player2sprite;
                        item->draw = draw_player1sprite;
                    }

                    result = attack(currentChar, enemy);
                } else if (inputs.b2) {
                    speech("Thank you,", "Next");
                    result = CONTINUE_ATTACK;
                }
            }
        }
    }

    switch (result) {
        case PLAYER_DEAD:
            map_erase(x, y);
            return END_ATTACK;
        case ENEMY_DEAD:
            map_erase(en_x, en_y);
        default:
            return CONTINUE_ATTACK;
    }
}

int attack(Character* attacker, Character* defender) {
    // sound_fx(SFX_ATTACK);
    speech("", "Attacking...");

    DrawStatusFunc draw_atk;
    DrawStatusFunc draw_def;
    if (attacker->team == 1) {
        draw_atk = draw_upper_status;
        draw_def = draw_lower_status;
    } else if (attacker->team == 2) {
        draw_atk = draw_lower_status;
        draw_def = draw_upper_status;
    } else {
        return -1;
    }

    pc.printf("%p, %p\n", draw_atk, draw_def);

    int hit_pc = (attacker->skill) - (defender->avoid);

    int damage = (attacker->atk) - (defender->def);
    damage = damage * attack_success(hit_pc);
    if (damage == 0) {
        speech("", "Avoided!");
    } else {
        if (damage > 0) {
            defender->health -= damage;
            Camera.health[defender->team - 1] -= damage;
            draw_def(Camera.health[defender->team - 1], max(-damage, -MAX_HEALTH), STATUS_ATTACKED);
            pc.printf("Enemy health: %d\n", defender->health);
            speech("", "Hit!");
            if (defender->potion) {
                if (defender->potion < (MAX_HEALTH - defender->health)) {
                    defender->health += defender->potion;
                    Camera.health[defender->team - 1] += defender->potion;
                    draw_def(Camera.health[defender->team - 1], defender->potion, STATUS_HEALED);
                    defender->potion = 0;
                    // sound_fx(SFX_HEAL);
                    speech("Potion activated!", "");
                }
            }

            if (defender->health <= 0) {
                Camera.charCount[defender->team - 1] -= 1;
                pc.printf("Enemy dead\n");
                // sound_fx(SFX_DEATH);
                speech("Enemy lost", "a player!");
                return ENEMY_DEAD;
            }
        } else {
            attacker->health += damage;
            Camera.health[attacker->team - 1] += damage;
            draw_atk(Camera.health[attacker->team - 1], max(damage, -MAX_HEALTH), STATUS_ATTACKED);
            speech("", "Backfired!");
            if (attacker->potion) {
                if (attacker->potion < (MAX_HEALTH - attacker->health)) {
                    attacker->health += attacker->potion;
                    Camera.health[attacker->team - 1] += defender->potion;
                    draw_atk(Camera.health[attacker->team - 1], defender->potion, STATUS_HEALED);
                    attacker->potion = 0;
                    // sound_fx(SFX_HEAL);
                    speech("Potion activated!", "");
                }
            }
            pc.printf("My health: %d\n", attacker->health);
            if (attacker->health <= 0) {
                Camera.charCount[attacker->team - 1] -= 1;
                pc.printf("Player dead\n");
                // sound_fx(SFX_DEATH);
                speech("You lost", "a player!");
                return PLAYER_DEAD;
            }
        }
    }

    speech("The enemy is", "attacking back!");
    hit_pc = (defender->skill) - (attacker->avoid);
    damage = (defender->def) - (attacker->atk);
    damage = damage * attack_success(hit_pc);
    if (damage == 0) {
        speech("", "Avoided!");
    } else {
        if (damage > 0) {
            attacker->health -= damage;
            Camera.health[attacker->team - 1] -= damage;
            draw_atk(Camera.health[attacker->team - 1], max(-damage, -MAX_HEALTH), STATUS_ATTACKED);
            speech("", "Hit!");
            if (attacker->potion) {
                if (attacker->potion < (MAX_HEALTH - attacker->health)) {
                    attacker->health += attacker->potion;
                    Camera.health[attacker->team - 1] += defender->potion;
                    draw_atk(Camera.health[attacker->team - 1], defender->potion, STATUS_HEALED);
                    attacker->potion = 0;
                    // sound_fx(SFX_HEAL);
                    speech("Potion activated!", "");
                }
            }
            pc.printf("My health: %d\n", attacker->health);
            if (attacker->health <= 0) {
                Camera.charCount[attacker->team - 1] -= 1;
                pc.printf("Player dead\n");
                // sound_fx(SFX_DEATH);
                speech("You lost", "a player!");
                return PLAYER_DEAD;
            }
        } else {
            defender->health += damage;
            Camera.health[defender->team - 1] += damage;
            draw_def(Camera.health[defender->team - 1], max(damage, -MAX_HEALTH), STATUS_ATTACKED);
            speech("", "Backfired!");
            if (defender->potion) {
                if (defender->potion < (MAX_HEALTH - defender->health)) {
                    defender->health += defender->potion;
                    Camera.health[defender->team - 1] += defender->potion;
                    draw_def(Camera.health[defender->team - 1], defender->potion, STATUS_HEALED);
                    defender->potion = 0;
                    // sound_fx(SFX_HEAL);
                    speech("Potion activated!", "");
                }
            }
            pc.printf("Enemy health: %d\n", defender->health);
            if (defender->health <= 0) {
                Camera.charCount[defender->team - 1] -= 1;
                // sound_fx(SFX_DEATH);
                speech("Enemy lost", "a player!");
                pc.printf("Enemy dead\n");
                return ENEMY_DEAD;
            }
        }
    }
    return NO_CASUALTY;
}

int attack_success(int p) {
    p = min(max(p, 0), 100);
    float r = 100.0 * (float)rand() / RAND_MAX;
    pc.printf("SUCCESS PC: %g, HIT PC: %d\n", r, p);
    return (r < p);
}

/**
 *  Return true if the cursor is on the character owned by the current player
 */
int check_char_select(int x, int y, int player_id) {
    Map* map = get_active_map();
    MapItem* item = get_current(x, y);
    if (item->type == CHARACTERSPRITE) {
        Character* character = (Character*)(item->data);
        if (player_id == 0) {
            Camera.selected = character;
            Camera.cx = x;
            Camera.cy = y;
            return true;
        }

        if (character->team == player_id) {
            if (character->moved == 0) {
                Camera.selected = character;
                Camera.cx = x;
                Camera.cy = y;
                return true;
            }
        }
    }
    return false;
}

void draw_possible_moves(int x, int y, int range_left) {
    if (range_left <= 0) {
        return;
    }

    if (abs(x - Camera.x) > 5 || (abs(y - Camera.y) > 4)) {
        return;
    }

    MapItem* item;

    item = get_here(x + 1, y);
    if (!item || item->walkable) {
        draw_range(x + 1 - Camera.x, y - Camera.y);
        draw_possible_moves(x + 1, y, range_left - 1);
    }

    item = get_here(x - 1, y);
    if (!item || item->walkable) {
        draw_range(x - 1 - Camera.x, y - Camera.y);
        draw_possible_moves(x - 1, y, range_left - 1);
    }

    item = get_here(x, y + 1);
    if (!item || item->walkable) {
        draw_range(x - Camera.x, y + 1 - Camera.y);
        draw_possible_moves(x, y + 1, range_left - 1);
    }

    item = get_here(x, y - 1);
    if (!item || item->walkable) {
        draw_range(x - Camera.x, y - 1 - Camera.y);
        draw_possible_moves(x, y - 1, range_left - 1);
    }
}

void end_turn(int* active_player, int* mode) {
    pc.printf("end turn\n");
    Camera.selected = NULL;

    for (int i = 0; i < Camera.charMoved; i++) {
        Camera.charMovedPtr[i]->moved = 0;
    }

    Camera.charMoved = 0;
    *active_player = (*active_player) + 1;
    if (*active_player > NUM_PLAYERS) {
        *active_player = 1;
    }
    *mode = MODE_FREE_ROAM;

    speech("It's your turn!", "Let's go!");
}

void update_move_character(int x, int y) {
    Character* character = Camera.selected;

    check_potion(x, y, character);

    if ((Camera.cx != x) || (Camera.cy != y)) {
        add_character(x, y, character);
        map_erase(Camera.cx, Camera.cy);
    }

    Camera.charMovedPtr[Camera.charMoved] = character;
    Camera.charMoved++;
    character->moved = 1;

    MapItem* item;
    int rx, ry;
    for (int i = 0; i < MAX_ROUTE; i++) {
        rx = Camera.routes[i][0];
        if (rx < 0) {
            break;
        }

        ry = Camera.routes[i][1];
        item = get_here(rx, ry);

        if (item && item->type == FOOTSTEP) {
            map_erase(rx, ry);
        }
    }
}

void check_potion(int x, int y, Character* character) {
    MapItem* item = get_here(x, y);
    int heal = *((int*)item->data);
    if (item->type == POTION) {
        if (character->potion == 0) {
            character->potion = heal;
            speech("You picked up", "a potion!");
        } else {
            speech("You already have", "a potion!");
            map_erase(x, y);

            int dx = 0, dy = 0;
            while (item && (dx == 0) && (dy == 0)) {
                dx = 10.0 * (((float)rand() / RAND_MAX) - 0.5);
                dy = 10.0 * (((float)rand() / RAND_MAX) - 0.5);
                item = get_here(x + dx, y + dx);
            }
            add_potion(x + dx, y + dx, &heal);
        }
    }
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
            pc.printf("update_char_cursor::INVALID DIRECTION!");
            break;
    }

    if ((!item) || (item->type != WALL)) {
        Camera.x = x;
        Camera.y = y;
    }
}

void update_char_cursor(int x, int y, int dir, int range) {
    //    int range = data->range;
    int cx = Camera.cx;
    int cy = Camera.cy;

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
            pc.printf("update_char_cursor::INVALID DIRECTION!");
            break;
    }

    if ((!item) || (item->walkable)) {
        int dist = abs(cx - x) + abs(cy - y);
        if (dist <= range) {
            Camera.x = x;
            Camera.y = y;

            MapItem* character = get_here(cx, cy);
            if (graphic_alt) {
                if (Camera.selected->team == 1) {
                    character->draw = draw_player1sprite;
                } else if (Camera.selected->team == 2) {
                    character->draw = draw_player2sprite;
                }
            } else {
                if (Camera.selected->team == 1) {
                    character->draw = draw_player1walk;
                } else if (Camera.selected->team == 2) {
                    character->draw = draw_player2walk;
                }
            }

            int px = Camera.px;
            int py = Camera.py;
            MapItem* item = get_here(px, py);

            if (!item) {
                add_footstep(px, py);
                for (int i = 0; i < MAX_ROUTE; i++) {
                    if (Camera.routes[i][0] < 0) {
                        Camera.routes[i][0] = px;
                        Camera.routes[i][1] = py;
                        Camera.routes[i + 1][0] = -1;
                        break;
                    }
                }
            }
        } else {
            pc.printf("out of range\n");
        }
    }
}

/**
 * Entry point for frame drawing. This should be called once per iteration of
 * the game loop. This draws all tiles on the screen, followed by the status
 * bars. Unless init is nonzero, this function will optimize drawing by only
 * drawing tiles that have changed from the previous frame.
 */
void draw_game(int init, int mode) {
    // Draw game border first
    if (init) draw_border(Camera.active_player);

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
            if (x >= 0 && y >= 0 && x < map_width() && y < map_height()) {  // Current (i,j) in the map
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

            if (i == 0 && j == 0) {
                // Decide what to draw at the 0 position
                draw = draw_selection;
                draw(u, v);
            }
        }
    }

    if (mode == MODE_SELECTED) {
        draw_range(Camera.cx - Camera.x, Camera.cy - Camera.y);
        Character* character = Camera.selected;
        draw_possible_moves(Camera.cx, Camera.cy, character->range);
    }

    if (init) {
        graphic_alt = !graphic_alt;
    }

    draw_lower_status(Camera.health[1], 0, STATUS_IDLE);
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

    for (int i = 0; i < map_width(); i++) {
        for (int j = 0; j < map_height(); j++) {
            if ((i * map_height() + j) % 47 == 0) {
                add_rock(i, j);
                pc.printf("r");
            }

            if ((i * map_height() + j) % 179 == 0) {
                int* heal = (int*)malloc(sizeof(int));
                // printf("%d\n", *(heal - 1));
                *heal = 10;
                add_potion(i, j, heal);
                pc.printf("p");
            }
        }
    }
    pc.printf("rocks\r\n");

    pc.printf("Adding walls!\r\n");
    add_wall(0, 0, HORIZONTAL, map_width());
    pc.printf("w1\n");
    add_wall(0, map_height() - 1, HORIZONTAL, map_width());
    pc.printf("w2\n");
    add_wall(0, 0, VERTICAL, map_height());
    pc.printf("w3\n");
    add_wall(map_width() - 1, 0, VERTICAL, map_height());
    pc.printf("Walls done!\r\n");

    // print_map();
}

/**
 * Program entry point! This is where it all begins.
 * This function orchestrates all the parts of the game. Most of your
 * implementation should be elsewhere - this holds the game loop, and should
 * read like a road map for the rest of the code.
 */

#define REFRESH_PERIOD 200
int main() {
    // First things first: initialize hardware
    ASSERT_P(hardware_init() == ERROR_NONE, "Hardware init failed!");

    pc.printf("Program Starting");
    // Initialize the maps
    maps_init();
    init_main_map();
    set_active_map(0);

    Camera.x = Camera.px = 10;
    Camera.y = Camera.px = 10;
    Camera.routes[0][0] = -1;

    GameInputs inputs = read_inputs();
    int action = -1, update = -1;
    int mode = MODE_FREE_ROAM;
    int active_player = 1, old_player = 0;
    Camera.active_player = &active_player;

    // Start page
    int difficulty = 0;
    draw_welcome(difficulty, true);
    while (1) {
        inputs = read_inputs();
        if (inputs.b2) {
            difficulty++;
            difficulty = difficulty % 3;
            draw_welcome(difficulty, false);
            wait_ms(250);
        }
        if (inputs.b1) {
            // draw_game_over(1);
            break;
        }
    }

    // Initialize Characters
    Character characters[NUM_PLAYERS][NUM_CHARACTERS];
    for (int i = 0; i < NUM_PLAYERS; i++) {
        Camera.charCount[i] = NUM_CHARACTERS;
        Camera.health[i] = NUM_CHARACTERS * MAX_HEALTH;
        for (int j = 0; j < NUM_CHARACTERS; j++) {
            characters[i][j].atk = 10;
            characters[i][j].def = 5;
            characters[i][j].range = 3;
            characters[i][j].health = MAX_HEALTH;
            characters[i][j].team = i + 1;
            characters[i][j].avoid = 20;
            characters[i][j].skill = 100;
            characters[i][j].moved = 0;
            if (i == 1) {
                characters[i][j].atk += 20 * difficulty;
                characters[i][j].def += 2 * difficulty;
                characters[i][j].range += difficulty;
                characters[i][j].avoid += 10 * difficulty;
                characters[i][j].skill += 30 * difficulty;
            }
            characters[i][j].potion = 0;

            MapItem* item;
            int x, y;
            do {
                x = (map_width() * 0.1 * ((float)rand() / RAND_MAX - 0.5)) + Camera.x;
                y = (map_height() * 0.1 * ((float)rand() / RAND_MAX - 0.5)) + Camera.y;
                pc.printf("x: %d, y: %d", x, y);
                item = get_here(x, y);
            } while (item || x < 1 || y < 1 || x > map_width() - 1 && y > map_height() - 1);

            add_character(x, y, &characters[i][j]);
        }
    }

    // Initial drawing
    draw_upper_status(Camera.health[0], 0, STATUS_INIT);
    draw_lower_status(Camera.health[1], 0, STATUS_INIT);
    draw_game(true, MODE_FREE_ROAM);

    // Main game loop
    while (1) {
        // Timer to measure game update speed
        Timer t;
        t.start();

        //        pc.printf("STARTING\n");
        // 1. Read inputs
        inputs = read_inputs();
        //        pc.printf("INPUT READ\n");
        // 2. Determine action (move, act, menu, etc.)
        action = get_action(inputs);
        //        pc.printf("ACTION RETRIEVED\n");
        // 3. Update game
        old_player = active_player;
        update = update_game(action, &mode, &active_player);
        if (active_player != old_player) {
            pc.printf("ACTIVE PLAYER: %d\n", active_player);
        }

        //        pc.printf("UPDATED\n");
        // 3b. Check for game over
        if (update == GAME_OVER) {
            draw_game_over(Camera.winner);
            return 0;
        }
        // 4. Draw screen

        draw_game((update == FULL_DRAW), mode);
        //        pc.printf("DRAWN\n");

        // Compute update time
        t.stop();
        int dt = t.read_ms();
        if (dt < REFRESH_PERIOD) wait_ms(REFRESH_PERIOD - dt);
    }
}
