#include <Arduino.h>
#include <Arduino_GFX_Library.h> /* https://github.com/moononournation/Arduino_GFX.git */
#include <FastLED.h>
#include <Wire.h>
#include <EEPROM.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager/tree/development

#include "lcd.h"

#if defined(DISPLAY_DEV_KIT)
Arduino_GFX *gfx = create_default_Arduino_GFX();
#else /* !defined(DISPLAY_DEV_KIT) */
Arduino_DataBus *bus = new Arduino_ESP8266SPI(2 /* DC */, 15 /* CS */);
Arduino_GFX *gfx = new Arduino_GC9A01(bus, 16 /* RST */, 0 /* rotation */, true /* IPS */);

#endif /* !defined(DISPLAY_DEV_KIT) */

ESP8266WebServer webServer(80);
ESP8266HTTPUpdateServer httpUpdateServer;
WiFiManager wifiManager;

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

const uint16_t width = 240;
const uint16_t height = 240;

const uint16_t NUM_LEDS = 128;
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

uint8_t power = 1;
uint8_t brightness = 8;

uint8_t speed = 30;

uint8_t cyclePalettes = 0;
uint8_t paletteDuration = 10;
uint8_t currentPaletteIndex = 0;
unsigned long paletteTimeout = 0;

uint8_t currentPatternIndex = 0;

uint8_t autoplay = 1;

uint8_t autoplayDuration = 10;
unsigned long autoPlayTimeout = 0;

void print_chip_info(void)
{
  Serial.println();
  Serial.println(F("System Info:"));
  Serial.print(F("Heap: "));
  Serial.println(system_get_free_heap_size());
  Serial.print(F("Boot Vers: "));
  Serial.println(system_get_boot_version());
  Serial.print(F("CPU: "));
  Serial.println(system_get_cpu_freq());
  Serial.print(F("SDK: "));
  Serial.println(system_get_sdk_version());
  Serial.print(F("Chip ID: "));
  Serial.println(system_get_chip_id());
  Serial.print(F("Flash ID: "));
  Serial.println(spi_flash_get_id());
  Serial.print(F("Flash Size: "));
  Serial.println(ESP.getFlashChipRealSize());
  Serial.print(F("Vcc: "));
  Serial.println(ESP.getVcc());
  Serial.print(F("MAC Address: "));
  Serial.println(WiFi.macAddress());
  Serial.println();
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  // Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname, "r");
  if (!root)
  {
    // Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    // Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      // Serial.print("  DIR : ");
      // Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      // Serial.print("  FILE: ");
      // Serial.print(file.name());
      // Serial.print("  SIZE: ");
      // Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

#include "palettes.h"
#include "patterns.h"

#include "field.h"
#include "fields.h"

#include "web.h"

void setup()
{
  Serial.begin(115200);

  delay(1000);

  gfx->begin();
  gfx->fillRect(0, 0, width, height, gfx->color565(0, 0, 0));

  SPIFFS.begin();
  listDir(SPIFFS, "/", 1);

  loadFieldsFromEEPROM(fields, fieldCount);

  setupWeb();

  setupCoords();

  autoPlayTimeout = millis() + (autoplayDuration * 1000);
}

void setupCoords()
{
  // pre-compute a bunch of constants
  const float center = width / 2;
  const float radius = 0.96 * center;
  const float phi = (sqrt(5) + 1) / 2 - 1;
  const float goldenAngle = -phi * 2 * PI;

  const float maxArea = pow(radius, 2) * PI;
  // const float maxRadius = sqrt(maxArea / PI);
  const float circleArea = maxArea / NUM_LEDS;
  // const float circleRadius = sqrt(circleArea / PI);

  uint8_t minX, maxX, minY, maxY, minA, maxA, minR, maxR;
  float minXf, maxXf, minYf, maxYf, minAf, maxAf, minRf, maxRf;

  // Serial.println("xf, yf, af, rf, x, y, a, r");

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

    uint8_t x = (uint8_t)((xf / 240.0) * 255.0);
    uint8_t y = (uint8_t)((yf / 240.0) * 255.0);
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

    // Serial.printf("%f, %f, %f, %f, %u, %u, %u, %u\n", xf, yf, af, rf, x, y, a, r);
  }

  // Serial.printf("%f, %f, %f, %f, %u, %u, %u, %u\n", minXf, minYf, minAf, minRf, minX, minY, minA, minR);
  // Serial.printf("%f, %f, %f, %f, %u, %u, %u, %u\n", maxXf, maxYf, maxAf, maxRf, maxX, maxY, maxA, maxR);
}

void loop()
{
  unsigned long start = millis();
  handleWeb();

  if (cyclePalettes == 1 && (millis() > paletteTimeout))
  {
    nextPalette();
    paletteTimeout = millis() + (paletteDuration * 1000);
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
    gfx->fillRect(0, 0, width, height, gfx->color565(0, 0, 0));
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

  unsigned long elapsed = millis() - start;

  EVERY_N_SECONDS(1)
  {
    Serial.println(elapsed);
  }
}
