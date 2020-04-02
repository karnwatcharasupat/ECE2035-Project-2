#include "graphics.h"

#include <algorithm>

#include "globals.h"
#include "sprite.h"

int graphic_alt = 1;

void draw_nothing(int u, int v) {
    uLCD.filled_rectangle(u, v, u + 11, v + 11, BLACK);
}

void draw_img(int u, int v, const char* img) {
    int colors[11 * 11];
    for (int i = 0; i < 11 * 11; i++) {
        if (img[i] == 'R')
            colors[i] = RED;
        else if (img[i] == 'Y')
            colors[i] = 0xFFFF00;  // Yellow
        else if (img[i] == 'G')
            colors[i] = GREEN;
        else if (img[i] == 'D')
            colors[i] = 0xD2691E;  // "Dirt"
        else if (img[i] == '5')
            colors[i] = LGREY;  // 50% grey
        else if (img[i] == '3')
            colors[i] = DGREY;
        else if (img[i] == 'B')
            colors[i] = BLUE;
        else if (img[i] == 'W')
            colors[i] = WHITE;
        else if (img[i] == 'S')
            colors[i] = 0Xf7d368;  // skin
        else
            colors[i] = BLACK;
    }
    uLCD.BLIT(u, v, 11, 11, colors);
    wait_us(250);  // Recovery time!
}

void draw_plant(int u, int v) {
    if (graphic_alt) {
        draw_img(u, v, PLANT_IMG);
    } else {
        draw_img(u, v, PLANT_IMG2);
    }

    // uLCD.filled_rectangle(u, v, u + 10, v + 10, GREEN);
}

void draw_rock(int u, int v) {
    if (graphic_alt) {
        draw_img(u, v, ROCK_IMG);
    } else {
        draw_img(u, v, ROCK_IMG2);
    }
    // uLCD.filled_rectangle(u, v, u + 10, v + 10, GREEN);
}

void draw_wall(int u, int v) {
    draw_img(u, v, WALL_IMG);
    // uLCD.filled_rectangle(u, v, u + 10, v + 10, 0xA52A2A);
}

void draw_potion(int u, int v) {
    draw_img(u, v, POTION_IMG);
    // uLCD.filled_rectangle(u, v, u + 10, v + 10, 0xA52A2A);
}

void draw_player1sprite(int u, int v) {
    //    pc.printf("Drawing Player 1 Sprite\r\n");
    // uLCD.filled_rectangle(u, v, u + 10, v + 10, RED);
    // uLCD.filled_circle(u + 5, v + 5, 5, RED);
    draw_img(u, v, PLAYER1_IMG);
}

void draw_player1attack(int u, int v) {
    //    pc.printf("Drawing Player 1 Sprite\r\n");
    // uLCD.filled_rectangle(u, v, u + 10, v + 10, RED);
    // uLCD.filled_circle(u + 5, v + 5, 5, RED);
    draw_img(u, v, PLAYER1_ATK);
}

void draw_player1walk(int u, int v) {
    //    pc.printf("Drawing Player 1 Sprite\r\n");
    // uLCD.filled_rectangle(u, v, u + 10, v + 10, RED);
    // uLCD.filled_circle(u + 5, v + 5, 5, RED);
    draw_img(u, v, PLAYER1_WALK);
}

void draw_player2sprite(int u, int v) {
    //    pc.printf("Drawing Player 2 Sprite\r\n");
    // uLCD.filled_rectangle(u, v, u + 10, v + 10, 0xa232a8);
    // uLCD.filled_circle(u + 5, v + 5, 5, 0xa232a8);
    draw_img(u, v, PLAYER2_IMG);
}

void draw_player2attack(int u, int v) {
    //    pc.printf("Drawing Player 1 Sprite\r\n");
    // uLCD.filled_rectangle(u, v, u + 10, v + 10, RED);
    // uLCD.filled_circle(u + 5, v + 5, 5, RED);
    draw_img(u, v, PLAYER2_ATK);
}

void draw_player2walk(int u, int v) {
    //    pc.printf("Drawing Player 1 Sprite\r\n");
    // uLCD.filled_rectangle(u, v, u + 10, v + 10, RED);
    // uLCD.filled_circle(u + 5, v + 5, 5, RED);
    draw_img(u, v, PLAYER2_WALK);
}

void draw_selection(int u, int v) {
    uLCD.rectangle(u, v, u + 10, v + 10, LGREY);
}

#define MIN_X 2
#define MAX_X 124
#define MIN_Y 14
#define MAX_Y 114
void draw_range(int i, int j) {
    int u = (i + 5) * 11 + 3;
    int v = (j + 4) * 11 + 15;
    int u2 = min(u + 10, MAX_X - 1);
    int v2 = min(v + 10, MAX_Y - 1);
    u = max(MIN_X + 1, u);
    v = max(MIN_Y + 1, v);

    uLCD.rectangle(u, v, u2, v2, 0xFFFF00);
}

void draw_upper_status() {
    // Draw bottom border of status bar
    uLCD.line(0, 9, 127, 9, GREEN);
    // Add other status info drawing code here
}

void draw_lower_status() {
    // Draw top border of status bar
    uLCD.line(0, 118, 127, 118, GREEN);

    // Add other status info drawing code here
}

void draw_border() {
    uLCD.filled_rectangle(0, 9, 127, 14, WHITE);      // Top
    uLCD.filled_rectangle(0, 13, 2, 114, WHITE);      // Left
    uLCD.filled_rectangle(0, 114, 127, 117, WHITE);   // Bottom
    uLCD.filled_rectangle(124, 14, 127, 117, WHITE);  // Right
}

void draw_game_over() {
    uLCD.cls();
    uLCD.locate(3, 5);
    uLCD.printf("GAME OVER");
}

void draw_welcome(int difficulty, int full) {
    if (full) {
        for (int u = 3; u < 123; u += 11) {
            for (int v = 15; v < 113; v += 11) {
                draw_wall(u, v);
            }
        }

        uLCD.locate(4, 5);
        uLCD.printf("Welcome to");
        uLCD.locate(3, 6);
        uLCD.printf("ECE 2035 RPG");
        uLCD.locate(3, 8);
        uLCD.printf("ACTION: start");

        uLCD.locate(1, 9);
        uLCD.printf("BACK: change lvl");

        draw_player1attack(36, 102);

        draw_plant(58, 102);

        draw_player2attack(80, 102);
    }

    uLCD.locate(1, 11);
    switch (difficulty) {
        case 0:
            uLCD.printf("Difficulty: EASY");
            break;
        case 1:
            uLCD.printf("Difficulty: MED ");
            break;
        case 2:
            uLCD.printf("Difficulty: HARD");
            break;
        default:
            return;
    }
}