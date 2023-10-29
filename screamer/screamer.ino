#include <Adafruit_NeoPixel.h>

#define LED_PIN 11
#define LED_COUNT 12
// Brightness min/max 0/255
#define BRIGHTNESS 50

#define NOISE_LEVEL 10

// LED constructor
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ400);

// LED variables
unsigned long last_change = 0;
unsigned long now = 0;

// Microphon variables
uint8_t mic = A0;
uint8_t micOut;
uint8_t average;
uint8_t sound_level;
uint8_t sound_level_max;
uint8_t sound_level_max_average;
uint8_t i;

void setup() {
  Serial.begin(9600);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);
}

void loop() {
  now = millis();

  micOut = analogRead(mic);

  // Remove DC component by tracking slow average
  average = (micOut + average * 9) / 10;
  sound_level = abs(micOut - average);

  // Serial.print("mic:");
  // Serial.println(micOut);
  // Serial.print("avg:");
  // Serial.println(average);
  // Serial.print("lev:");
  // Serial.println(sound_level);

  // Remove noise
  if(sound_level < NOISE_LEVEL)
  {
    sound_level = 0;
  }
  else
  {
    sound_level -= NOISE_LEVEL;
  }

  // Track peak levels
  if(sound_level_max < sound_level)
  {
    sound_level_max = sound_level;
    if(sound_level_max >= LED_COUNT)
    {
      sound_level_max = LED_COUNT - 1;
    }
    sound_level_max_average = sound_level_max;
  }
  sound_level_max = (sound_level_max + sound_level_max_average * 9) / 10;
  if(sound_level_max_average > 0)
  {
    sound_level_max_average--;
  }
  // Automatic Gain Control

  for(i = 0; i < LED_COUNT; i++)
  {
    // Display sound level
    if(i < sound_level)
    {
      strip.setPixelColor(i, 255, 0, 0);
    }
    else
    {
      strip.setPixelColor(i, 0, 0, 0);
    }
    // Display peak sound level
    if((i == sound_level_max) && (i > 0))
    {
      strip.setPixelColor(i, 0, 255, 0);
    }
  }
  strip.show();
}

