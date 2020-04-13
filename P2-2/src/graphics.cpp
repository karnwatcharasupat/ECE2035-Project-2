#include "graphics.h"

#include <algorithm>

#include "sprite.h"

int graphic_alt = 1;

int round(int x) {
    return (int)((float)(x) + 0.5);
}

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
}

void draw_rock(int u, int v) {
    if (graphic_alt) {
        draw_img(u, v, ROCK_IMG);
    } else {
        draw_img(u, v, ROCK_IMG2);
    }
}

void draw_wall(int u, int v) {
    draw_img(u, v, WALL_IMG);
}

void draw_potion(int u, int v) {
    draw_img(u, v, POTION_IMG);
}

void draw_player1sprite(int u, int v) {
    draw_img(u, v, PLAYER1_IMG);
}

void draw_player1attack(int u, int v) {
    draw_img(u, v, PLAYER1_ATK);
}

void draw_player1walk(int u, int v) {
    draw_img(u, v, PLAYER1_WALK);
}

void draw_player2sprite(int u, int v) {
    draw_img(u, v, PLAYER2_IMG);
}

void draw_player2attack(int u, int v) {
    draw_img(u, v, PLAYER2_ATK);
}

void draw_player2walk(int u, int v) {
    draw_img(u, v, PLAYER2_WALK);
}

void draw_footstep(int u, int v) {
    draw_img(u, v, FOOTSTEP_IMG);
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

void draw_enemybox(int u, int v) {
    u = (u + 5) * 11 + 3;
    v = (v + 4) * 11 + 15;

    uLCD.rectangle(u, v, u + 10, v + 10, RED);
}

#define TOP_Y_MAX 8
#define BTM_Y_MIN 119
#define CHANGE_SPEED_MS 2

void draw_status(int health, int change, int mode, int y0, int y1) {
    float old_health = ((float)health - change) / (MAX_HEALTH * NUM_CHARACTERS);
    float new_health = (float)health / (MAX_HEALTH * NUM_CHARACTERS);

    int old_health_x = (int)round(old_health * 127.0);
    int new_health_x = (int)round(new_health * 127.0);

    pc.printf("old: %d, new: %d\n", old_health_x, new_health_x);

    if (mode != STATUS_INIT) {
        uLCD.filled_rectangle(old_health_x + 1, y0, 127, y1, BLACK);
        uLCD.filled_rectangle(0, y0, old_health_x, y1, GREEN);
    }

    switch (mode) {
        case STATUS_ATTACKED:
            uLCD.filled_rectangle(new_health_x + 1, y0, old_health_x, y1, RED);
            wait_ms(500);
            for (int x = old_health_x; x > new_health_x; x--) {
                uLCD.filled_rectangle(x, y0, x, y1, BLACK);
                wait_ms(CHANGE_SPEED_MS);
            }
            break;
        case STATUS_HEALED:
            uLCD.filled_rectangle(old_health_x + 1, y0, new_health_x, y1, BLUE);
            wait_ms(500);
            for (int x = old_health_x; x <= new_health_x; x++) {
                uLCD.filled_rectangle(x, y0, x, y1, GREEN);
                wait_ms(CHANGE_SPEED_MS);
            }
            break;
        case STATUS_INIT:
            uLCD.filled_rectangle(old_health_x + 1, y0, 127, y1, BLACK);
            for (int x = 0; x <= old_health_x; x++) {
                uLCD.filled_rectangle(x + 1, y0, x + 5, y1, BLUE);
                uLCD.filled_rectangle(x, y0, x, y1, GREEN);
                wait_ms(CHANGE_SPEED_MS);
            }
            uLCD.filled_rectangle(0, y0, old_health_x, y1, GREEN);
            break;
        case STATUS_IDLE:
        default:
            return;
    }
}

void draw_upper_status(int health, int change, int mode) {
    // Draw bottom border of status bar
    uLCD.line(0, 9, 127, 9, GREEN);
    // Add other status info drawing code here
    draw_status(health, change, mode, 0, TOP_Y_MAX);
}

void draw_lower_status(int health, int change, int mode) {
    // Draw top border of status bar
    uLCD.line(0, 118, 127, 118, GREEN);

    // Add other status info drawing code here
    draw_status(health, change, mode, BTM_Y_MIN, 127);
}

void draw_border(int* active_player) {
    int color = (*active_player == 1) ? RED : BLUE;
    uLCD.filled_rectangle(0, 9, 127, 14, color);      // Top
    uLCD.filled_rectangle(0, 13, 2, 114, color);      // Left
    uLCD.filled_rectangle(0, 114, 127, 117, color);   // Bottom
    uLCD.filled_rectangle(124, 14, 127, 117, color);  // Right
}

void draw_info(Character* character) {
    int team = character->team;
    int color;
    DrawFunc sprite;
    if (team == 1) {
        color = RED;
        sprite = draw_player1attack;
    } else if (team == 2) {
        color = BLUE;
        sprite = draw_player2attack;
    } else {
        return;
    }
    uLCD.filled_rectangle(4, 16, 102, 92, BLACK);
    uLCD.rectangle(3, 15, 103, 93, color);
    sprite(5, 17);

    if (character->potion) {
        draw_potion(18, 17);
    }

    uLCD.locate(1, 4);
    uLCD.printf("HEALTH: %d/%d", character->health, MAX_HEALTH);
    uLCD.locate(1, 5);
    uLCD.printf("ATK:    %d", character->atk);
    uLCD.locate(1, 6);
    uLCD.printf("DEF:    %d", character->def);
    uLCD.locate(1, 7);
    uLCD.printf("RANGE:  %d", character->range);
    uLCD.locate(1, 8);
    uLCD.printf("AVOID:  %d", character->avoid);
    uLCD.locate(1, 9);
    uLCD.printf("SKILL:  %d", character->skill);
}

void draw_game_over(int winner) {
    if (winner == 0) {
        return;
    }

    DrawFunc draw[3];

    switch (winner) {
        case 1:
            draw[0] = draw_player1sprite;
            draw[1] = draw_player1attack;
            draw[2] = draw_player1walk;
            break;
        case 2:
            draw[0] = draw_player2sprite;
            draw[1] = draw_player2attack;
            draw[2] = draw_player2walk;
            break;
        default:
            return;
    }

    uLCD.cls();

    for (int u = 3; u < 120; u += 11) {
        for (int v = 12; v < 115; v += 22) {
            draw_wall(u, v);
        }
    }

    int i;
    int j = 0;
    while (1) {
        i = j % 3;
        for (int u = 1; u < 116; u += 11) {
            for (int v = 0; v < 115; v += 22) {
                draw[i](u, v);
                i++;
                i = i % 3;
            }
            i++;
            i = i % 3;
        }
        j++;
        // wait_ms(500);
    }
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