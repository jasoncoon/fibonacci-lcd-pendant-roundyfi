void swirl()
{
  const float z = 2.5; // zoom (2.0)
  const float w = 3.0; // number of wings (3)
  const float p_min = 0.1;
  const float p_max = 2.0; // puff up (default: 1.0)
  const float d_min = 0.1;
  const float d_max = 2.0; // dent (default: 0.5)
  const float s_min = -3.0;
  const float s_max = 2.0; // swirl (default: -2.0)
  const float g_min = 0.1;
  const float g_max = 0.5; // glow (default: 0.2)
  const float b = 240;     // inverse brightness (240)

  const float p = p_min + beatsin88(13 * speed) / (float)UINT16_MAX * (p_max - p_min);
  const float d = d_min + beatsin88(17 * speed) / (float)UINT16_MAX * (d_max - d_min);
  const float s = s_min + beatsin88(7 * speed) / (float)UINT16_MAX * (s_max - s_min);
  const float g = g_min + beatsin88(27 * speed) / (float)UINT16_MAX * (g_max - g_min);

  CRGBPalette16 palette(palettes[1]); // es_rivendell_15_gp

  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    float r = i / 256.0 * z;
    float a = (angles[i] + (beat88(3 * speed) >> 3)) / 256.0 * TWO_PI;
    float v = r - p + d * sin(w * a + s * r * r);
    float c = 255 - b * pow(fabs(v), g);
    if (c < 0)
      c = 0;
    else if (c > 255)
      c = 255;

    leds[i] = ColorFromPalette(palette, (uint8_t)c);
  }
}