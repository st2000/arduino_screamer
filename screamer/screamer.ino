#include <WS2812FX.h>

#define LED_COUNT 13
#define LED_PIN 11

#define TIMER_MS 5000

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

unsigned long last_change = 0;
unsigned long now = 0;

int mic = A0;
int micOut;
int average;
int sound_level;

int bandSize = 13;
WS2812FX::Segment* seg = ws2812fx.getSegment();
uint8_t i;


void setup() {
  ws2812fx.init();
  ws2812fx.setBrightness(255);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();
  
  Serial.begin(9600);
}

void loop() {
  now = millis();

  micOut = analogRead(mic);
  average = (float)micOut * (1/10.0f) + (float)average * (9/10.0f);
  sound_level = abs(micOut - average);

  // Serial.print("mic:");
  // Serial.println(micOut);
  // Serial.print("avg:");
  // Serial.println(average);
  Serial.print("lev:");
  Serial.println(sound_level);

  // if(now - last_change > TIMER_MS) {
  //   ws2812fx.setMode((ws2812fx.getMode() + 1) % ws2812fx.getModeCount());
  //   last_change = now;
  // }


  uint8_t scaledBand = (sound_level * bandSize) / 128;
  Serial.print("scl:");
  Serial.println(scaledBand);
  for(uint16_t j=0; j<bandSize; j++) {
    uint16_t index = seg->start + (i * bandSize) + j;
    if(j <= scaledBand) {
      if(j < bandSize - 4) ws2812fx.setPixelColor(index, seg->colors[0]);
      else if(j < bandSize - 2) ws2812fx.setPixelColor(index, seg->colors[1]);
      else ws2812fx.setPixelColor(index, seg->colors[2]);
    } else {
      ws2812fx.setPixelColor(index, BLACK);
    }
  }
  // ws2812fx.setCycle();
  ws2812fx.service();

}