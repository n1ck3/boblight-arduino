#include <Adafruit_NeoPixel.h>

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

#define PIN 13
#define PIXELS 218

// specified under `rate` in the `[device]` section of /etc/boblight.conf
#define serialRate 115200

// boblightd sends a prefix (defined in /etc/boblight.conf) before sending the pixel data
uint8_t prefix[] = {0x41, 0x64, 0x61, 0x00, 0xF3, 0xA6};

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
// Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXELS, PIN, NEO_KHZ800);

void setup() {
  // initialise our LED strip
  strip.begin();
  // make the first pixel random color as an indicator that it is awaiting data
  strip.setPixelColor(0, 0, 0, 0);
  strip.show();
  delay(200);
  strip.setPixelColor(0, 0, 100, 0);
  // turn all the other pixels on so we know they're working
  for(uint16_t i = 1; i < PIXELS; ++i)
    strip.setPixelColor(i, 1, 1, 1);
  strip.show();
  Serial.begin(serialRate);
}

void loop() {
  // wait until we see the prefix
  for(byte i = 0; i < sizeof prefix; ++i) {
    waitLoop: while (!Serial.available()) ;;
    // look for the next byte in the sequence if we see the one we want
    if(prefix[i] == Serial.read()) continue;
    // otherwise, start over
    i = 0;
    goto waitLoop;
  }
  // read the transmitted data
  for (uint8_t i=0; i<PIXELS; i++) {
    byte r, g, b;
    while(!Serial.available());
    r = Serial.read();
    while(!Serial.available());
    g = Serial.read();
    while(!Serial.available());
    b = Serial.read();
    strip.setPixelColor(i, g, r, b);
  }
  strip.show();
}
