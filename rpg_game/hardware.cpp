// This header has all the (extern) declarations of the globals.
// "extern" means "this is instantiated somewhere, but here's what the name
// means.
#include "hardware.h"

#include "globals.h"

// We need to actually instantiate all of the globals (i.e. declare them once
// without the extern keyword). That's what this file does!

// Hardware initialization: Instantiate all the things!
uLCD_4DGL uLCD(p9, p10, p11);  // LCD Screen (tx, rx, reset)
//SDFileSystem sd(p5, p6, p7, p8, "sd");  // SD Card(mosi, miso, sck, cs)
Serial pc(USBTX, USBRX);        // USB Console (tx, rx)
MMA8452 acc(p28, p27, 100000);  // Accelerometer (sda, sdc, rate)
DigitalIn button1(p21);         // Pushbuttons (pin)
DigitalIn button2(p22);
DigitalIn button3(p23);
AnalogOut DACout(p18);  // Speaker (pin)
PwmOut speaker(p26);
wave_player waver(&DACout);

// Some hardware also needs to have functions called before it will set up
// properly. Do that here.
int hardware_init() {
    // Crank up the speed
    uLCD.baudrate(3000000);
    pc.baud(115200);

    //Initialize pushbuttons
    button1.mode(PullUp);
    button2.mode(PullUp);
    button3.mode(PullUp);

    //Initialize Accelerometer
    acc.activate();
    
    pc.baud(9600);

    return ERROR_NONE;
}

/**
 * Read all the user inputs. 
 * This is all input hardware interaction should happen.
 * Returns a GameInputs struct that has all the inputs recorded.
 * This GameInputs is used elsewhere to compute the game update.
 */
#define TRIES_THRESH 10;
GameInputs read_inputs() {
    GameInputs in;
    /**
    struct GameInputs {
        int b1, b2, b3;     // Button presses
        double ax, ay, az;  // Accelerometer readings
    };
    */

    in.b1 = !button1;
    in.b2 = !button2;
    in.b3 = !button3;

//    int ret = 0;
//    int num_tries = 0;
    acc.readXYZGravity(&(in.ax), &(in.ay), &(in.az));
    in.ay = -in.ay;
    //do {
//        if (num_tries > TRIES_THRESH) {
//            pc.printf("Exceed maximum trials. Writing acc as zeros\n\r");
//            in.ax = 0.0;
//            in.ay = 0.0;
//            in.az = 0.0;
//            break;
//        }
//
//        if (num_tries > 0) {
//            pc.printf("Read failed. Trying again...\n\r");
//        }
//        ret = readXYZGravity(&(in.ax), &(in.ay), &(in.az));
//        // returns 1 on failure
//
//    } while (ret);

    return in;
}

