#include <Arduino.h>
#include <EEPROM.h>
#include "Program.h"
#include "FastLED.h"

/**
 * @brief Data Pin for the LEDs
 */
#define DATA_PIN 2
/**
 * @brief Pin brought high when the barrel jack is unplugged
 */
//#define UNPLUGGED_INTERRUPT 3
/**
 * @brief Array behind all of the LEDs
 */
CRGB leds[NUM_LEDS];
/**
 * @brief Controller object for an array of WS2812B LEDs
 */
CLEDController *controller = &FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

uint8_t brightness = 0;
static void isrBarrelJackChange();

static void save();
static void load();

#define MAX_PROGRAMS 20
static int currentProgram = 0;

void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(0);

#ifdef UNPLUGGED_INTERRUPT
  isrBarrelJackChange();
  attachInterrupt(digitalPinToInterrupt(UNPLUGGED_INTERRUPT), isrBarrelJackChange, CHANGE);
#else
  brightness = 255;
  controller->showLeds(brightness);
#endif

  programs.printPrograms();

  load();
}

#ifdef UNPLUGGED_INTERRUPT
/* Detect when power is plugged in or unplugged */
void isrBarrelJackChange()
{
  int val = digitalRead(UNPLUGGED_INTERRUPT);

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
#endif

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
      else if(rx == '+' || rx == '=')
      {
        uint16_t newDelay = programs[currentProgram]->getDelay();
        newDelay <<= 1;
        if(newDelay > 256)
        {
          newDelay = 256;
        }
        else if(newDelay <= 0)
        {
          newDelay = 1;
        }
        programs[currentProgram]->setDelay(newDelay);
        Serial.println(newDelay);
        return -2;
      }
      else if(rx == ']' || rx == '}')
      {
        uint16_t newDelay = programs[currentProgram]->getDelay();
        newDelay += newDelay / 10;
        programs[currentProgram]->setDelay(newDelay);
        Serial.println(newDelay);
        return -2;
      }
      else if(rx == '-' || rx == '_')
      {
        uint16_t newDelay = programs[currentProgram]->getDelay();
        newDelay >>= 1;
        programs[currentProgram]->setDelay(newDelay);
        Serial.println(newDelay);
        return -2;
      }
      else if(rx == '[' || rx == '{')
      {
        uint16_t newDelay = programs[currentProgram]->getDelay();
        newDelay -= newDelay / 10;
        programs[currentProgram]->setDelay(newDelay);
        Serial.println(newDelay);
        return -2;
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
  
  if(num != -1)
  {
    save();
  }
  
  auto program = programs[currentProgram];
  program->loop();
}

struct EepromData {
  unsigned long int magic;
  int currentProgram;
  struct {
    uint16_t delay;
  } data[REGISTRY_SIZE];
};

#define MAGIC 0x11223344

void save() 
{
  EepromData data = {MAGIC};
  data.currentProgram = currentProgram;
  for(int i = 0; i < programs.length(); ++i)
  {
    data.data[i] = {
      programs[i]->getDelay()
    };
  }
  EEPROM.put(0, data);
}

void load()
{
  EepromData data = {};
  EEPROM.get(0, data);
  if(data.magic != MAGIC)
  {
    save();
    return;
  }

  if (data.currentProgram > 0 && data.currentProgram < programs.length())
  {
    currentProgram = data.currentProgram;
  }
  for (int i = 0; i < programs.length(); ++i)
  {
    programs[i]->setDelay(data.data[i].delay);
  }
}