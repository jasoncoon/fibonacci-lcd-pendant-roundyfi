void starsWithOffset(uint16_t stars[], uint8_t starCount, uint8_t offset = 21, bool setup = false, bool move = false)
{
  // use a number from the Fibonacci sequence for offset to follow a spiral out from the center

  const uint8_t hues = NUM_LEDS / 256;

  for (uint8_t i = 0; i < starCount; i++)
  {
    if (setup || stars[i] >= NUM_LEDS)
    {
      // reset star
      stars[i] = random8(offset - 1);
    }

    uint16_t index = stars[i];

    // draw the star
    CRGB newcolor = ColorFromPalette(currentPalette, (stars[i] / hues) + gHue); // i * (240 / starCount)

    nblend(leds[index], newcolor, 64);
  }

  // move the stars
  if (move)
  {
    for (uint8_t i = 0; i < starCount; i++)
    {
      stars[i] = stars[i] + offset;
    }
  }
}

const uint8_t starCount = 16;

void stars13(bool setup = false, bool move = false)
{
  static uint16_t stars[starCount];
  starsWithOffset(stars, starCount, 13, setup, move);
}

void stars21(bool setup = false, bool move = false)
{
  static uint16_t stars[starCount];
  starsWithOffset(stars, starCount, 21, setup, move);
}

void stars34(bool setup = false, bool move = false)
{
  static uint16_t stars[starCount];
  starsWithOffset(stars, starCount, 34, setup, move);
}

void stars55(bool setup = false, bool move = false)
{
  static uint16_t stars[starCount];
  starsWithOffset(stars, starCount, 55, setup, move);
}

void stars89(bool setup = false, bool move = false)
{
  static uint16_t stars[starCount];
  starsWithOffset(stars, starCount, 89, setup, move);
}

void stars()
{
  bool move = false;
  static bool setup = true;
  fadeToBlackBy(leds, NUM_LEDS, 8);

  EVERY_N_MILLIS(60)
  {
    move = true;
  }

  stars13(setup, move);
  stars21(setup, move);
  stars34(setup, move);
  // stars55(setup, move);
  // stars89(setup, move);
  
  setup = false;
}
