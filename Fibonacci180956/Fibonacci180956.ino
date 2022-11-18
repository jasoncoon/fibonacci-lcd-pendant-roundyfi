// based on https://github.com/Xinyuan-LilyGO/T-RGB/tree/main/example/GFX

#include <Arduino.h>
#include <Arduino_GFX_Library.h> /* https://github.com/moononournation/Arduino_GFX.git */
#include <FastLED.h>
#include "Wire.h"
#include "XL9535_driver.h"
#include "pin_config.h"
#include "lcd.h"

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

const uint16_t width = EXAMPLE_LCD_H_RES;
const uint16_t height = EXAMPLE_LCD_V_RES;

const uint16_t NUM_LEDS = 512;
const uint16_t ledRadius = 4;

CRGB leds[NUM_LEDS];

// pre-compute the XY coordinates of each "LED"
uint16_t coordsXf[NUM_LEDS] = {};
uint16_t coordsYf[NUM_LEDS] = {};

// one byte coordinates of each "LED"
uint8_t coordsX[NUM_LEDS] = {};
uint8_t coordsY[NUM_LEDS] = {};
uint8_t angles[NUM_LEDS] = {};
uint8_t radii[NUM_LEDS] = {};

uint8_t gHue = 0;

uint8_t speed = 30;

uint8_t currentPaletteIndex = 0;
uint8_t secondsPerPalette = 10;

uint8_t currentPatternIndex = 0;

uint8_t autoplay = 1;

uint8_t autoplayDuration = 10;
unsigned long autoPlayTimeout = 0;

void print_chip_info(void)
{
  Serial.print("Chip: ");
  Serial.println(ESP.getChipModel());
  Serial.print("ChipRevision: ");
  Serial.println(ESP.getChipRevision());
  Serial.print("Psram size: ");
  Serial.print(ESP.getPsramSize() / 1024);
  Serial.println("KB");
  Serial.print("Flash size: ");
  Serial.print(ESP.getFlashChipSize() / 1024);
  Serial.println("KB");
  Serial.print("CPU frequency: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println("MHz");
}

void setup()
{
  Serial.begin(115200);

  pinMode(BAT_VOLT_PIN, ANALOG);

  print_chip_info();

  setupLcd();

  gfx->fillRect(0, 0, width, height, gfx->color565(0, 0, 0));

  setupCoords();

  autoPlayTimeout = millis() + (autoplayDuration * 1000);
}

void setupCoords()
{
  // pre-compute a bunch of constants
  const float center = width / 2;
  const float radius = 0.98 * center;
  const float phi = (sqrt(5) + 1) / 2 - 1;
  const float goldenAngle = -phi * 2 * PI;

  const float maxArea = pow(radius, 2) * PI;
  // const float maxRadius = sqrt(maxArea / PI);
  const float circleArea = maxArea / NUM_LEDS;
  // const float circleRadius = sqrt(circleArea / PI);

  uint8_t minX, maxX, minY, maxY, minA, maxA, minR, maxR;
  float minXf, maxXf, minYf, maxYf, minAf, maxAf, minRf, maxRf;

  Serial.println("xf, yf, af, rf, x, y, a, r");

  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    float af = abs(fmod(i * goldenAngle, PI * 2));
    float rf = sqrt((circleArea * (i + 0.5)) / PI);
    float xf = center + cos(af) * rf;
    float yf = center + sin(af) * rf;

    minXf = min(minXf, xf);
    minYf = min(minYf, yf);
    minAf = min(minAf, af);
    minRf = min(minRf, rf);

    maxXf = max(maxXf, xf);
    maxYf = max(maxYf, yf);
    maxAf = max(maxAf, af);
    maxRf = max(maxRf, rf);

    coordsXf[i] = xf;
    coordsYf[i] = yf;

    uint8_t x = (uint8_t)((xf / 480.0) * 255.0);
    uint8_t y = (uint8_t)((yf / 480.0) * 255.0);
    uint8_t a = (uint8_t)((af / (PI * 2)) * 255.0);
    uint8_t r = (uint8_t)((rf / 240.0) * 255.0);

    coordsX[i] = x;
    coordsY[i] = y;
    angles[i] = a;
    radii[i] = r;

    minX = min(minX, x);
    minY = min(minY, y);
    minA = min(minA, a);
    minR = min(minR, r);

    maxX = max(maxX, x);
    maxY = max(maxY, y);
    maxA = max(maxA, a);
    maxR = max(maxR, r);

    Serial.printf("%f, %f, %f, %f, %u, %u, %u, %u\n", xf, yf, af, rf, x, y, a, r);
  }

  Serial.printf("%f, %f, %f, %f, %u, %u, %u, %u\n", minXf, minYf, minAf, minRf, minX, minY, minA, minR);
  Serial.printf("%f, %f, %f, %f, %u, %u, %u, %u\n", maxXf, maxYf, maxAf, maxRf, maxX, maxY, maxA, maxR);
}

// given an angle and radius (and delta for both), set pixels that fall inside that range,
// fading the color from full-color at center, to off (black) at the outer edges.
void antialiasPixelAR(uint8_t angle, uint8_t dAngle, uint8_t startRadius, uint8_t endRadius, CRGB color, CRGB leds[], int _NUM_LEDS)
{
  for (uint16_t i = 0; i < _NUM_LEDS; i++)
  {
    uint8_t ro = radii[i];
    // only mess with the pixel when it's radius is within the target radius
    if (ro <= endRadius && ro >= startRadius)
    {
      // Get pixel's angle (unit256)
      uint8_t ao = angles[i];
      // set adiff to abs(ao - angle) ... relies on unsigned underflow resulting in larger value
      uint8_t adiff = min(sub8(ao, angle), sub8(angle, ao));
      // only mess with the pixel when it's angle is within range of target
      if (adiff <= dAngle)
      {
        // map the intensity of the color so it fades to black at edge of allowed angle
        uint8_t fade = map(adiff, 0, dAngle, 0, 255);
        CRGB faded = color;
        // fade the target color based on how far the angle was from the target
        faded.fadeToBlackBy(fade);
        // add the faded color (as an overlay) to existing colors
        leds[i] += faded;
      }
    }
  }
}

#include "gradientPalettes.h"

CRGBPalette16 currentPalette(CRGB::Black);
CRGBPalette16 targetPalette(gradientPalettes[0]);

#include "colorWaves.h"
#include "emitter.h"
#include "fire.h"
#include "noise.h"
#include "pride.h"
#include "stars.h"
#include "swirl.h"
#include "water.h"

typedef void (*Pattern)();
typedef Pattern PatternList[];
typedef struct
{
  Pattern pattern;
  String name;
} PatternAndName;
typedef PatternAndName PatternAndNameList[];

PatternAndNameList patterns = {
    {colorWaves, "Color Waves"},
    {pride, "Pride"},

    {paletteNoise, "Noise"},
    {polarNoise, "Polar Noise"},

    {fire, "Fire"},
    {water, "Water"},

    {firePalette, "Palette Fire"},
    {waterPalette, "Palette Water"},

    {stars, "Stars"},
    {emitter, "Emitter"},

    {swirl, "Swirl"},
};

const uint8_t patternCount = ARRAY_SIZE(patterns);

// increase or decrease the current pattern number, and wrap around at the ends
void adjustPattern(bool up)
{
  if (up)
    currentPatternIndex++;
  else
    currentPatternIndex--;

  // wrap around at the ends
  if (currentPatternIndex < 0)
    currentPatternIndex = patternCount - 1;
  if (currentPatternIndex >= patternCount)
    currentPatternIndex = 0;

  // if (autoplay == 0)
  // {
  //   EEPROM.write(1, currentPatternIndex);
  //   EEPROM.commit();
  // }

  // broadcastInt("pattern", currentPatternIndex);
}

void loop()
{
  // unsigned long start = millis();

  // change to a new cpt-city gradient palette
  EVERY_N_SECONDS(secondsPerPalette)
  {
    currentPaletteIndex = addmod8(currentPaletteIndex, 1, gradientPaletteCount);
    targetPalette = gradientPalettes[currentPaletteIndex];
  }

  EVERY_N_MILLISECONDS(40)
  {
    // slowly blend the current palette to the next
    nblendPaletteTowardPalette(currentPalette, targetPalette, 8);
    gHue++; // slowly cycle the "base color" through the rainbow
  }

  if (autoplay && (millis() > autoPlayTimeout))
  {
    adjustPattern(true);
    autoPlayTimeout = millis() + (autoplayDuration * 1000);
  }

  // Call the current pattern function once, updating the display
  patterns[currentPatternIndex].pattern();

  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    CRGB color = leds[i];

    uint16_t x = coordsXf[i];
    uint16_t y = coordsYf[i];
    uint8_t r = color.r;
    uint8_t g = color.g;
    uint8_t b = color.b;

    uint16_t c = gfx->color565(r, g, b);

    gfx->fillCircle(x, y, ledRadius, c);
  }

  // Serial.println(millis() - start);
}