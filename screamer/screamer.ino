#include <Adafruit_NeoPixel.h>

#define LED_PIN 5
#define LED_COUNT 12
// Brightness min/max 0/255
#define BRIGHTNESS 50
// Time in milliseconds between updates
// About 50 is more responsive and 100 is more steady.
#define INTERVAL_UPDATE 50

#define MIC_PIN A0
// About 10 keeps most background noise from lighting LEDs.
#define NOISE_LEVEL 10
#define INTERVAL_UPDATE_AGC 50

// LED constructor
// Argument 1 = Number of pixels in NeoPixel leds
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel leds(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ400);

// LED variables
unsigned long last_change = 0;
unsigned long last_change_agc = 0;
unsigned long now = 0;

// Microphon variables
uint8_t mic = MIC_PIN;
uint8_t micOut;
uint16_t average;
uint16_t sound_level;
uint16_t sound_level_max_interval;
uint16_t sound_level_max;
uint16_t sound_level_max_average;
uint16_t sound_level_average;
uint16_t i;

void setup() {
  Serial.begin(9600);

  leds.begin();           // INITIALIZE NeoPixel leds object (REQUIRED)
  leds.show();            // Turn OFF all pixels ASAP
  leds.setBrightness(BRIGHTNESS);

  last_change = millis();
  last_change_agc = millis();
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
  
  // // Automatic Gain Control
  // if(now > last_change_agc)
  // {
  //   last_change_agc = now + INTERVAL_UPDATE_AGC;
  //   sound_level_average = (sound_level_average + sound_level * 99) / 100;
  //   Serial.print("snd lev avg:");
  //   Serial.println(sound_level_average);
  //   // If we go beyond the number of LEDs invoke AGC control
  //   if (sound_level_average > LED_COUNT)
  //   {
  //     sound_level = sound_level * (LED_COUNT / (float)sound_level_average);
  //   }
  // }

  if(now > last_change)
  {
    last_change = now + INTERVAL_UPDATE;

    // Track peak levels
    if(sound_level_max < sound_level_max_interval)
    {
      sound_level_max = sound_level_max_interval;
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

    // Light up the LEDs
    for(i = 0; i < LED_COUNT; i++)
    {
      // Display sound level
      if(i < sound_level_max_interval)
      {
        leds.setPixelColor(i, 255, 0, 0);
      }
      else
      {
        leds.setPixelColor(i, 0, 0, 0);
      }
      // Display peak sound level
      if((i == sound_level_max) && (i > 0))
      {
        leds.setPixelColor(i, 0, 255, 0);
      }
    }
    leds.show();
    sound_level_max_interval = 0;
  }
  else
  {
    // Track the maxmimum until the next LED update.
    if(sound_level_max_interval < sound_level)
    {
      sound_level_max_interval = sound_level;
    }
  }
}

