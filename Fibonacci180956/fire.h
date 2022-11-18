void fire()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    uint16_t x = coordsX[i];
    uint16_t y = coordsY[i];

    uint8_t n = qsub8(inoise8((y << 2) - beat88(speed << 2), (x << 2)), y);

    leds[i] = ColorFromPalette(HeatColors_p, n);
  }
}

void firePalette()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    uint16_t x = coordsX[i];
    uint16_t y = coordsY[i];

    uint8_t n = qsub8(inoise8((y << 2) - beat88(speed << 2), (x << 2)), y);

    leds[i] = ColorFromPalette(currentPalette, n);
  }
}
