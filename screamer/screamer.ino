#include <Adafruit_NeoPixel.h>

#define LED_PIN 5
#define LED_COUNT 14
// Brightness min/max 0/255
#define BRIGHTNESS 100
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
unsigned long scream_start;
unsigned long scream_length;
uint8_t led_warm_up;

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

// State machine
enum {
  VU_METER_NORMAL,
  VU_METER_GATER_POWER,
  WARM_UP,
  POWER_GATHERED,
  POWER_DISCHARGED,
};
uint8_t state_machine = VU_METER_NORMAL;

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
  switch(state_machine)
  {
    case VU_METER_NORMAL:
      if(vu_meter() > 4)
      {
        state_machine = VU_METER_GATER_POWER;
      }
      scream_start = now;
      break;

    case VU_METER_GATER_POWER:
      if(vu_meter() == 0)
      {
        last_change = now;
        led_warm_up = 0;
        state_machine = WARM_UP;
      }
      else
      {
        // Count up power (length of sound)
        scream_length = millis() - scream_start;
      }
      break;

    case WARM_UP:
      if(warm_up() > 0)
      {
        led_warm_up = 0;
        state_machine = POWER_GATHERED;
      }
      break;

    case POWER_GATHERED:
      if(power_gathered() > 0)
      {
        led_warm_up = 0;
        state_machine = POWER_DISCHARGED;
      }
      break;

    case POWER_DISCHARGED:
      if(power_discharged() > 0)
      {
        state_machine = VU_METER_NORMAL;
      }
      break;
  }
  Serial.print("state:");
  Serial.println(state_machine);

}

// VU Meter effect
uint8_t vu_meter()
{
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
        leds.setPixelColor(i, 100, 0, 0);
      }
      else
      {
        leds.setPixelColor(i, 0, 0, 0);
      }
      // Display peak sound level
      if((i == sound_level_max) && (i > 0))
      {
        leds.setPixelColor(i, 0, 100, 0);
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
  return sound_level_max;
}

// Warmup effect
uint8_t warm_up(void)
{
  uint8_t return_value = 0;
  if(now > last_change)
  {
    last_change = now + 25;

    led_warm_up++;
    // Light up the LEDs
    for(i = 0; i < LED_COUNT; i++)
    {
      leds.setPixelColor(i, led_warm_up, led_warm_up * 0.8f, 0);
    }
    leds.show();

    // Serial.print("glow:");
    // Serial.println(led_warm_up);

    if(led_warm_up > 10)
    {
      return_value = 1;
    }
    else
    {
      return_value = 0;
    }
  }
  return return_value;
}

// Display the gathered power effect
uint8_t power_gathered(void)
{
  uint8_t return_value = 0;
  if(now > last_change)
  {
    last_change = now + 150;

    led_warm_up++;
    // Light up the LEDs
    for(i = 0; i < LED_COUNT; i++)
    {
      if(i < led_warm_up)
      {
        if((scream_length / 100) > i)
        {
          leds.setPixelColor(i, 255, 0, 0);
        }
      }
    }
    leds.show();

    if(led_warm_up > LED_COUNT)
    {
      return_value = 1;
    }
    else
    {
      return_value = 0;
    }
  }
  return return_value;
}

uint8_t power_discharged(void)
{
  uint8_t return_value = 0;
  if(now > last_change)
  {
    last_change = now + 25;

    led_warm_up++;
    // Light off all the LEDs
    leds.setBrightness(100 - led_warm_up);
    leds.show();

    if(led_warm_up >= 100)
    {
      // Light off all the LEDs
      for(i = 0; i < LED_COUNT; i++)
      {
        leds.setPixelColor(i, 0, 0, 0);
      }
      leds.setBrightness(BRIGHTNESS);
      leds.show();
      return_value = 1;
    }
    else
    {
      return_value = 0;
    }
  }
  return return_value;
}
