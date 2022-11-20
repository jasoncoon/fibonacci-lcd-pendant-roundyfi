# 1 "/var/folders/9r/y8320f552q3f3jt2fn17vtsc0000gn/T/tmp8ep68cu7"
#include <Arduino.h>
# 1 "/Users/jasoncoon/Documents/Arduino/Fibonacci180956/FibonacciLcdPendant/Fibonacci180956.ino"


#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <EEPROM.h>
#include <HTTPUpdateServer.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <Wire.h>

#include "XL9535_driver.h"
#include "pin_config.h"
#include "lcd.h"

WebServer webServer(80);
HTTPUpdateServer httpUpdateServer;
WiFiManager wifiManager;

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

const uint16_t width = EXAMPLE_LCD_H_RES;
const uint16_t height = EXAMPLE_LCD_V_RES;

const uint16_t NUM_LEDS = 512;
const uint16_t ledRadius = 4;

CRGB leds[NUM_LEDS];


uint16_t coordsXf[NUM_LEDS] = {};
uint16_t coordsYf[NUM_LEDS] = {};


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
void print_chip_info(void);
void setup();
void setupCoords();
void loop();
#line 63 "/Users/jasoncoon/Documents/Arduino/Fibonacci180956/FibonacciLcdPendant/Fibonacci180956.ino"
void print_chip_info(void)
{
# 78 "/Users/jasoncoon/Documents/Arduino/Fibonacci180956/FibonacciLcdPendant/Fibonacci180956.ino"
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{


  File root = fs.open(dirname);
  if (!root)
  {

    return;
  }
  if (!root.isDirectory())
  {

    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {


      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {




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


  pinMode(BAT_VOLT_PIN, ANALOG);



  setupLcd();

  gfx->fillRect(0, 0, width, height, gfx->color565(0, 0, 0));

  SPIFFS.begin();


  loadFieldsFromEEPROM(fields, fieldCount);

  setupWeb();

  setupCoords();

  autoPlayTimeout = millis() + (autoplayDuration * 1000);
}

void setupCoords()
{

  const float center = width / 2;
  const float radius = 0.98 * center;
  const float phi = (sqrt(5) + 1) / 2 - 1;
  const float goldenAngle = -phi * 2 * PI;

  const float maxArea = pow(radius, 2) * PI;

  const float circleArea = maxArea / NUM_LEDS;


  uint8_t minX, maxX, minY, maxY, minA, maxA, minR, maxR;
  float minXf, maxXf, minYf, maxYf, minAf, maxAf, minRf, maxRf;



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


  }



}

void loop()
{

  handleWeb();

  if (cyclePalettes == 1 && (millis() > paletteTimeout))
  {
    nextPalette();
    paletteTimeout = millis() + (paletteDuration * 1000);
  }

  EVERY_N_MILLISECONDS(40)
  {

    nblendPaletteTowardPalette(currentPalette, targetPalette, 8);
    gHue++;
  }

  if (autoplay && (millis() > autoPlayTimeout))
  {
    adjustPattern(true);
    autoPlayTimeout = millis() + (autoplayDuration * 1000);
  }


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


}