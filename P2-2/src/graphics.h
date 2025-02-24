#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "globals.h"
typedef void (*DrawStatusFunc)(int health, int change, int mode);
typedef void (*DrawFunc)(int u, int v);

/**
 * Takes a string image and draws it to the screen. The string is 121 characters
 * long, and represents an 11x11 tile in row-major ordering (across, then down,
 * like a regular multi-dimensional array). The available colors are:
 *      R = Red
 *      Y = Yellow
 *      G = Green
 *      D = Brown ("dirt")
 *      5 = Light grey (50%)
 *      3 = Dark grey (30%)
 *      Any other character is black
 * More colors can be easily added by following the pattern already given.
 */
void draw_img(int u, int v, const char* img);

/**
 * DrawFunc functions. 
 * These can be used as the MapItem draw functions.
 */
void draw_nothing(int u, int v);
void draw_wall(int u, int v);
void draw_plant(int u, int v);
void draw_rock(int u, int v);
void draw_potion(int u, int v);
void draw_footstep(int u, int v);

void draw_player1sprite(int u, int v);
void draw_player1attack(int u, int v);
void draw_player1walk(int u, int v);
void draw_player2sprite(int u, int v);
void draw_player2attack(int u, int v);
void draw_player2walk(int u, int v);
void draw_selection(int u, int v);
void draw_enemybox(int u, int v);
void draw_range(int u, int v);
void draw_game_over(int);

/**
 * Draw the upper status bar.
 */
void draw_upper_status(int health, int change, int mode);

/**
 * Draw the lower status bar.
 */
void draw_lower_status(int health, int change, int mode);

/**
 * Draw the border for the map.
 */
void draw_border(int*);

void draw_welcome(int, int);

void draw_info(Character* character);

#define STATUS_IDLE 0
#define STATUS_ATTACKED 1
#define STATUS_HEALED 2
#define STATUS_INIT 3

#endif  // GRAPHICS_H
