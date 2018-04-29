//universal strip wrapper for both fastled and neopixelbus
#pragma once

///////////
#ifdef STRIP_WS2812GRB
#define USE_NEO
#include <NeoPixelBus.h>
NeoPixelBus<NeoGrbFeature,Neo800KbpsMethod> *strip=new NeoPixelBus<NeoGrbFeature,Neo800KbpsMethod>(LED_COUNT);
typedef ColorClass RgbColor
#endif

///////////
#ifdef STRIP_WS2812GRBW
#define USE_NEO
#define USE_RGBW
#include <NeoPixelBus.h>
NeoPixelBus<NeoGrbwFeature,Neo800KbpsMethod> *strip=new NeoPixelBus<NeoGrbwFeature,Neo800KbpsMethod>(LED_COUNT);
typedef RgbwColor ColorClass;
#endif

///////////
#ifdef STRIP_LPD8806
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include "FastLED.h"
#define USE_FASTLED
FASTLED_USING_NAMESPACE;
CRGB strip[LED_COUNT];
typedef CRGB ColorClass;
#endif




#ifdef USE_FASTLED

void stripInit()
{
  FastLED.addLeds< LPD8806,7,5,GRB >(strip, LED_COUNT);
  FastLED.setDither(DISABLE_DITHER);
}

void stripClear(const ColorClass & color)
{
  for (int i=0; i<LED_COUNT; i++)
    strip[i]=color;
}

void inline stripShow()
{
  FastLED.show();
}

void inline stripShiftRight()
{
  for (int i=LED_COUNT-1; i>0; i--)
  {
    strip[i]=strip[i-1];
  }
}

void inline stripSet(const int led, const ColorClass & color)
{
  strip[led]=color;
}

#endif



#ifdef USE_NEO
void stripInit()
{
  strip->Begin();
}

void stripClear(const ColorClass & color)
{
  strip->ClearTo(color);
}

void inline stripShow()
{
  strip->Show();
}

void inline stripShiftRight()
{
  strip->ShiftRight(1);
}

void inline stripSet(const int led, const ColorClass & color)
{
  strip->SetPixelColor(led, color);
}

#endif
