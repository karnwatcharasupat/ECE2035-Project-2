#include "graphics.h"

#include "globals.h"
#include "sprite.h"

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
    uLCD.draw_img(u, v, PLANT_IMG);
    // uLCD.filled_rectangle(u, v, u + 10, v + 10, GREEN);
}

void draw_wall(int u, int v) {
    uLCD.draw_img(u, v, WALL_IMG);
    // uLCD.filled_rectangle(u, v, u + 10, v + 10, 0xA52A2A);
}

void draw_player1sprite(int u, int v) {
    //    pc.printf("Drawing Player 1 Sprite\r\n");
    // uLCD.filled_rectangle(u, v, u + 10, v + 10, RED);
    // uLCD.filled_circle(u + 5, v + 5, 5, RED);
    uLCD.draw_img(u, v, PLAYER1_IMG);
}
void draw_player2sprite(int u, int v) {
    //    pc.printf("Drawing Player 2 Sprite\r\n");
    // uLCD.filled_rectangle(u, v, u + 10, v + 10, 0xa232a8);
    // uLCD.filled_circle(u + 5, v + 5, 5, 0xa232a8);
    uLCD.draw_img(u, v, PLAYER2_IMG);
}

void draw_selection(int u, int v) {
    uLCD.filled_rectangle(u, v, u + 10, v + 10, LGREY);
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
