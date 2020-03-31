#include "speech.h"

#include "globals.h"
#include "hardware.h"

/**
 * Draw the speech bubble background.
 */
static void draw_speech_bubble();

/**
 * Erase the speech bubble.
 */
static void erase_speech_bubble();

/**
 * Draw a single line of the speech bubble.
 * @param line The text to display
 * @param which If TOP, the first line; if BOTTOM, the second line.
 */
#define TOP 0
#define BOTTOM 1
static void draw_speech_line(const char* line, int which);

/**
 * Delay until it is time to scroll.
 */
static void speech_bubble_wait();

void draw_speech_bubble() {
    uLCD.filled_rectangle(0, 100, 127, 118, WHITE);
}

void erase_speech_bubble() {
    uLCD.filled_rectangle(0, 100, 127, 118, WHITE);
}

void draw_speech_line(const char* line, int which) {
    char x = 1, y;
    switch (which) {
        case TOP:
            y = 13;
            break;
        case BOTTOM:
            y = 14;
            break;
        default:
            return;
    }
    uLCD.locate(x, y);
    uLCD.printf(line);
}

#define SPEECH_WAIT_MS 1000
void speech_bubble_wait() {
    wait_ms(SPEECH_WAIT_MS);
}

void speech(const char* line1, const char* line2) {
    draw_speech_bubble();
    draw_speech_line(line1, TOP);
    draw_speech_line(line2, BOTTOM);
    speech_bubble_wait();
    erase_speech_bubble();
}

void long_speech(const char* lines[], int n) {
}
