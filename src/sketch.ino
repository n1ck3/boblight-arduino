/* BEGIN IR */
#include <IRremote.h>

#define IR_PIN 11

// Buttons
/*
#define PLAY        0xA173823E
#define PAUSE       0x79017250
#define STOP        0xB326197C
#define REC         0x53B9034A
#define RED         0x240C9161
#define GREEN       0xA26409C9
#define YELLOW      0xE01F9A81
#define BLUE        0x68E839F1
#define UP          0x4DE74847
#define DOWN        0xB8781EF
*/
#define PLAY        0x6A618E02
#define PAUSE       0x79017250
#define STOP        0xC863D6C8
#define REC         0x53B9034A
#define RED         0xDAEA83EC
#define GREEN       0x2BAFCEEC
#define YELLOW      0xB5210DA6
#define BLUE        0x71A1FE88
#define UP          0xC26BF044
#define DOWN        0xC4FFB646

boolean stateChanged = false;
int state = 0;

IRrecv irrecv(IR_PIN);
decode_results results;
/* END IR */

/* BEGIN LED */
#include <Adafruit_NeoPixel.h>

#define LED_PIN 13
/*#define PIXELS 218*/
#define PIXELS 12
#define serialRate 115200

#define INTENSITY_STEP 0.05
#define MINIMUM_INTENSITY 0.0
#define MAXIMUM_INTENSITY 0.2

#define BUSYWAIT_UNTIL(cond, max_time) \
    do { \
        unsigned long t0; \
        t0 = millis(); \
        while(!(cond) && (millis() < t0 + (max_time))); \
    } while(0)

int COLOR_RED[2][3] = {
    {255, 0, 0},
    {255, 0, 157}
};
int COLOR_GREEN[2][3] = {
    {0, 255, 0},
    {156, 255, 0}
};
int COLOR_YELLOW[2][3] = {
    {255, 255, 255},
    {200, 200, 255}
};
int COLOR_BLUE[2][3] = {
    {0, 0, 255},
    {0, 155, 255}
};
int COLOR_BLACK[3] = {0, 0, 0};

int (*current_color) = COLOR_YELLOW[0];

float current_intensity = MAXIMUM_INTENSITY;

uint8_t prefix[] = {0x41, 0x64, 0x61, 0x00, 0xF3, 0xA6};

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXELS, LED_PIN, NEO_KHZ800);
/* END LED */

void adjust_intensity(char *direction) {
    Serial.println("adjust intensity ");
    Serial.println(direction);
    Serial.println(" ");

    float intensity;

    if (state != 0) {
        if (direction == "UP") {
            intensity = current_intensity + INTENSITY_STEP;
        } else if (direction == "DOWN") {
            intensity = current_intensity - INTENSITY_STEP;
        }

        if (intensity > MAXIMUM_INTENSITY) {
            intensity = MAXIMUM_INTENSITY;
        } else if (intensity < MINIMUM_INTENSITY) {
            intensity = MINIMUM_INTENSITY;
        }

        if (intensity != current_intensity) {
            current_intensity = intensity;
            stateChanged = true;
            Serial.println("Intnsity set to: ");
            Serial.println(current_intensity);
            Serial.println(" ");
        }
    }

}

void off() {
    Serial.println("off");

    stateChanged = false;

    current_color = COLOR_BLACK;
    for(uint16_t pixel=0; pixel<PIXELS; pixel++) {
        strip.setPixelColor(pixel, current_color[0], current_color[1], current_color[2]);
    }
    strip.show();

    current_intensity = MAXIMUM_INTENSITY;
}

void constant() {
    Serial.println("constant");

    stateChanged = false;

    for(uint16_t pixel=0; pixel<PIXELS; pixel++) {
        // RGB -> GRB
        strip.setPixelColor(
            pixel,
            (int)current_color[1]*current_intensity,
            (int)current_color[0]*current_intensity,
            (int)current_color[2]*current_intensity
        );
    }
    strip.show();
}

void boblight() {
    if (stateChanged == true) {
        Serial.println("boblight");

        stateChanged = false;

        // If we just went to serial, turn off all LEDs
        current_color = COLOR_BLACK;
        for(uint16_t pixel=0; pixel<PIXELS; pixel++) {
            strip.setPixelColor(pixel, current_color[0], current_color[1], current_color[2]);
        }
        strip.show();
    }
/*
    if (Serial.available()) {
        // wait until we see the prefix
        byte idx;
        int test = 0;
        for(idx = 0; idx < sizeof prefix; ++idx) {
            waitLoop: while (!Serial.available()) ;;
            byte data = Serial.read();
            //Serial.println(data, HEX);
            // look for the next byte in the sequence if we see the one we want
            if(data == prefix[idx]) continue;
            test++;
            // otherwise, start over
            idx = 0;
            goto waitLoop;
        }
        Serial.println(test);
        Serial.println(idx);
        Serial.println(sizeof prefix);
        // read the transmitted data
        if (idx == sizeof prefix) {
            for (uint8_t pixel=0; pixel<PIXELS; pixel++) {
                byte red, green, blue;

                while(!Serial.available());
                red = Serial.read();

                while(!Serial.available());
                green = Serial.read();

                while(!Serial.available());
                blue = Serial.read();

                strip.setPixelColor(
                    pixel,
                    (int)green*current_intensity,
                    (int)red*current_intensity,
                    (int)blue*current_intensity
                );
            }
            strip.show();
        }
    }
*/


    BUSYWAIT_UNTIL(Serial.available(), 5);
    if(Serial.available()) {
        // Wait until we see the prefix
        byte idx = 0;
        static int fails = 0;
        for(idx=0; idx<sizeof prefix; idx++) {
            BUSYWAIT_UNTIL(Serial.available(), 5);
            byte data = Serial.read();
            /*
            Serial.print("idx=");
            Serial.println(idx);

            Serial.print("prefix[idx]=");
            Serial.println(prefix[idx], HEX);

            Serial.print("serial=");
            Serial.println(data, HEX);
            */
            if(data == prefix[idx]) {
                // The byte of data is correct, test the next one.
                continue;
            } else {
                // The byte is incorrect, evac!

                fails++;
                idx = 0;
                break;
            }
        }

        if(idx == sizeof prefix) {
            Serial.println(fails);
            fails = 0;
            // If we have seen the whole prefix array
            //Serial.println(" found sync! ");
            // read the transmitted data
            for (uint8_t pixel=0; pixel<PIXELS; pixel++) {
                byte red, green, blue;

                BUSYWAIT_UNTIL(Serial.available(), 5);
                red = Serial.read();
                //Serial.println(red, HEX);

                BUSYWAIT_UNTIL(Serial.available(), 5);
                green = Serial.read();
                //Serial.println(green, HEX);

                BUSYWAIT_UNTIL(Serial.available(), 5);
                blue = Serial.read();
                //Serial.println(blue, HEX);

                strip.setPixelColor(
                    pixel,
                    (int)green*current_intensity,
                    (int)red*current_intensity,
                    (int)blue*current_intensity
                );
            }
            strip.show();
        }
    }
}

void handle_ir_signal() {
    if (irrecv.decode(&results)) {
        switch (results.value) {
            case PLAY:
                //Serial.println("PLAY");
                if (state != 2) {
                    state = 2;
                    stateChanged = true;
                }
                break;

            case PAUSE:
                //Serial.println("PAUSE");
                /*
                if (state != 1) {
                    state = 1;
                    stateChanged = true;
                }
                */
                break;

            case STOP:
                //Serial.println("STOP");
                if (state != 0) {
                    state = 0;
                    stateChanged = true;
                }
                break;

            case RED:
                //Serial.println("RED");
                state = 1;
                stateChanged = true;
                if (current_color != COLOR_RED[0]) {
                    current_color = COLOR_RED[0];
                } else {
                    current_color = COLOR_RED[1];
                }
                break;

            case GREEN:
                //Serial.println("GREEN");
                state = 1;
                stateChanged = true;
                if (current_color != COLOR_GREEN[0]) {
                    current_color = COLOR_GREEN[0];
                } else {
                    current_color = COLOR_GREEN[1];
                }
                break;

            case YELLOW:
                //Serial.println("YELLOW");
                state = 1;
                stateChanged = true;
                if (current_color != COLOR_YELLOW[0]) {
                    current_color = COLOR_YELLOW[0];
                } else {
                    current_color = COLOR_YELLOW[1];
                }
                break;

            case BLUE:
                //Serial.println("BLUE");
                state = 1;
                stateChanged = true;
                if (current_color != COLOR_BLUE[0]) {
                    current_color = COLOR_BLUE[0];
                } else {
                    current_color = COLOR_BLUE[1];
                }
                break;

            case UP:
                //serial.println("UP");
                adjust_intensity("UP");
                break;

            case DOWN:
                //serial.println("DOWN");
                adjust_intensity("DOWN");
                break;

            default:
                Serial.println(results.value, HEX);

        }
        delay(20);
        irrecv.resume();
    }
}

void setup() {
    Serial.begin(serialRate);

    /* BEGIN IR */
    irrecv.enableIRIn();
    state = 1;
    stateChanged = true;
    /* END IR */


    /* BEGIN LED */
    strip.begin();
    /* END LED */

    Serial.println("Initialized...");
}

void loop() {
    handle_ir_signal();

    if (state == 0  && stateChanged == true) {
        off();
    } else if (state == 1 && stateChanged == true) {
        constant();
    } else if (state == 2) {
        boblight();
    }
}
