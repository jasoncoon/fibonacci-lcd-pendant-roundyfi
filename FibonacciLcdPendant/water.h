void water()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    uint16_t x = coordsX[i];
    uint16_t y = coordsY[i];

    uint8_t n = inoise8((y << 2) + beat88(speed << 2), (x << 4));

    leds[i] = ColorFromPalette(IceColors_p, n);
  }
}

void waterPalette()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    uint16_t x = coordsX[i];
    uint16_t y = coordsY[i];

    uint8_t n = inoise8((y << 2) + beat88(speed << 2), (x << 4));

    leds[i] = ColorFromPalette(currentPalette, n);
  }
}
