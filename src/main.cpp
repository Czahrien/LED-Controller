#include <Arduino.h>
#include "FastLED.h"

#define DATA_PIN 2
#define UNPLUGGED_INTERUPT 3
#define NUM_LEDS 84

CRGB leds[NUM_LEDS];
static CLEDController *controller = &FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

static uint8_t brightness = 0;
static void isrBarrelJackChange();

static void programChase(bool);
static void programHueSolid();
static void programHueCircle();
static void programSeizureWarning();

#define MAX_PROGRAMS 20
static int current_program = 7;
static struct ProgramDetails
{
  const char *PROGMEM description;
  void (*function)();
} programs[MAX_PROGRAMS] = {
    {"Red", []() {
       controller->showColor(CRGB::Red, brightness);
       delay(100);
     }},
    {"Blue", []() {
       controller->showColor(CRGB::Blue, brightness);
       delay(100);
     }},
    {"Green", []() {
       controller->showColor(CRGB::Green, brightness);
       delay(100);
     }},
    {"White", []() { 
      controller->showColor(CRGB::White, brightness);
      delay(100); }},
    {"Chase", []() { programChase(true); }},
    {"Chase Rev", []() { programChase(false); }},
    {"Hue Solid", programHueSolid},
    {"Hue Circle", programHueCircle},
    {"Seizure Warning", programSeizureWarning},
    {NULL}};

void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(0);

  isrBarrelJackChange();
  attachInterrupt(digitalPinToInterrupt(UNPLUGGED_INTERUPT), isrBarrelJackChange, CHANGE);
}

/* Detect when power is plugged in or unplugged */
void isrBarrelJackChange()
{
  int val = digitalRead(UNPLUGGED_INTERUPT);

  /* Use the built-in LED as a visble indicator */
  digitalWrite(LED_BUILTIN, val);

  if (0 == val)
  {
    /* External power is unplugged - Disable LEDs by setting the brightness to 0 */
    brightness = 0;
  }
  else
  {
    /* External power is plugged in - Enable LEDs by setting the brightness to max */
    brightness = 255;
  }
  controller->showLeds(brightness);
}

void loop()
{
  String s = Serial.readString();
  if(s.length() > 0)
  {
    Serial.println(s);
    Serial.println(s.toInt());
    current_program = s.toInt();
  }
  ProgramDetails &program = programs[current_program];
  if (program.function)
  {
    program.function();
  }
}

void programChase(bool dir)
{
  /* Chasing around the case */
  static int cntr = 0;
  if (dir)
  {
    ++cntr;
  }
  else
  {
    --cntr;
  }

  if (cntr >= NUM_LEDS)
  {
    cntr = 0;
  }
  else if (cntr < 0)
  {
    cntr = NUM_LEDS - 1;
  }

  CRGB color = CRGB::White;
  for (int i = 0; i < NUM_LEDS; ++i)
  {
    leds[i] /= 2;
  }
  leds[cntr] = color;
  controller->showLeds(brightness);
  delay(30);
}

void programHueSolid()
{
  /* Cycle through hue */
  static CHSV color = {0, 255, 255};
  color.h += 1;
  controller->showColor(color, brightness);
  delay(30);
}

void programHueCircle()
{
  static int start = 0;
  if (start >= NUM_LEDS)
    start = 0;

  for (int cntr = 0; cntr < NUM_LEDS; ++cntr)
  {
    int spot = cntr - start;
    if (spot < 0)
    {
      spot += NUM_LEDS;
    }
    CHSV color = {
        static_cast<byte>(0xFF & ((spot << 8) / (NUM_LEDS - 1))),
        255,
        255};
    leds[cntr] = color;
  }
  start += 1;
  delay(25);
  controller->showLeds(brightness);
}

void programSeizureWarning()
{
  /* RGB Seizure */
  controller->showColor(CRGB::Red, brightness);
  delay(60);
  controller->showColor(CRGB::Green, brightness);
  delay(60);
  controller->showColor(CRGB::Blue, brightness);
  delay(60);
}
