
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

#include "colorWaves.h"
#include "emitter.h"
#include "fire.h"
// #include "noise.h"
#include "pride.h"
#include "stars.h"
// #include "swirl.h"
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

    // {paletteNoise, "Noise"},
    // {polarNoise, "Polar Noise"},

    {fire, "Fire"},
    {water, "Water"},

    {firePalette, "Palette Fire"},
    {waterPalette, "Palette Water"},

    {stars, "Stars"},
    {emitter, "Emitter"},

    // {swirl, "Swirl"},
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

void nextPalette()
{
  currentPaletteIndex = (currentPaletteIndex + 1) % paletteCount;
  targetPalette = palettes[currentPaletteIndex];
}
