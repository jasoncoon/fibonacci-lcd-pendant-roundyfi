void emitter()
{
  static CRGB ledBuffer[NUM_LEDS]; // buffer for better fade behavior
  const uint8_t dAngle = 32;       // angular span of the traces
  const uint8_t dRadius = 12;      // radial width of the traces
  const uint8_t vSpeed = 16;       // max speed variation

  static const uint8_t eCount = 7;    // Number of simultaneous traces
  static uint8_t angle[eCount];       // individual trace angles
  static uint16_t timeOffset[eCount]; // individual offsets from beat8() function
  static uint8_t speedOffset[eCount]; // individual speed offsets limited by vSpeed
  static uint8_t sparkIdx = 0;        // randomizer cycles through traces to spark new ones

  // spark new trace
  EVERY_N_MILLIS(20)
  {
    if (random8(17) <= (speed >> 4))
    { // increase change rate for higher speeds
      angle[sparkIdx] = random8();
      speedOffset[sparkIdx] = random8(vSpeed); // individual speed variation
      timeOffset[sparkIdx] = beat8(qadd8(speed, speedOffset[sparkIdx]));
      sparkIdx = addmod8(sparkIdx, 1, eCount); // continue randomizer at next spark
    }
  }

  // fade traces
  fadeToBlackBy(ledBuffer, NUM_LEDS, 6 + (speed >> 3));

  // draw traces
  for (uint8_t e = 0; e < eCount; e++)
  {
    uint8_t startRadius = sub8(beat8(qadd8(speed, speedOffset[e])), timeOffset[e]);
    uint8_t endRadius = add8(startRadius, dRadius + (speed >> 5)); // increase radial width for higher speeds
    antialiasPixelAR(angle[e], dAngle, startRadius, endRadius, ColorFromPalette(currentPalette, startRadius), ledBuffer, NUM_LEDS);
  }

  // copy buffer to actual strip
  memcpy(leds, ledBuffer, sizeof(ledBuffer[0]) * NUM_LEDS);
}