#include <Arduino.h>
#include "Program.h"
#include "FastLED.h"

#define DATA_PIN 2
#define UNPLUGGED_INTERUPT 3
CRGB leds[NUM_LEDS];
CLEDController *controller = &FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

uint8_t brightness = 0;
static void isrBarrelJackChange();

#define MAX_PROGRAMS 20
static int currentProgram = 0;

void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(0);

  isrBarrelJackChange();
  attachInterrupt(digitalPinToInterrupt(UNPLUGGED_INTERUPT), isrBarrelJackChange, CHANGE);

  programs.printPrograms();

  /* Workaround - For now select Hue Circle */
  for (int cntr = 0; cntr < programs.length(); ++cntr)
  {
    Program *program = programs[cntr];
    if (String("Hue Circle") == program->getDescription())
    {
      currentProgram = cntr;
    }
  }
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

void printPrograms() {

}

int processSerial()
{
  static const int BUF_SIZE = 4;
  static String serialBuf;
  static bool init = true;
  if (init)
  {
    init = false;
    serialBuf.reserve(BUF_SIZE);
  }
  while (Serial.available())
  {
    char rx = Serial.read();
    int length = serialBuf.length();
    if (length > 0 && rx == '\010') /* Backspace */
    {
      serialBuf.remove(serialBuf.length() - 1);
    }
    else if (length > 0 && rx == '\n') /* Newline */
    {
      int i = serialBuf.toInt();
      serialBuf = "";
      return i;
    }
    else if (serialBuf.length() < BUF_SIZE)
    {
      if (rx >= '0' && rx <= '9')
      {
        serialBuf += rx;
      }
    }
  }

  return -1;
}

void loop()
{
  int num = processSerial();
  if (num >= 0 && num < programs.length())
  {
    currentProgram = num;
    auto program = programs[currentProgram];
    char buf[80] = "";
    sprintf_P(buf, PSTR("Selecting %d: %s"), num, program->getDescription().c_str());
    Serial.println(buf);
  }
  auto program = programs[currentProgram];
  program->loop();
}