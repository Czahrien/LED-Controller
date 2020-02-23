#include "Program.h"

Program::Program(const String &description, const uint16_t delay) : description(description), loopDelay(delay) {}
String Program::getDescription() {
    return description;
}
void Program::setDelay(uint16_t ms)
{
    loopDelay = ms;
}
uint16_t Program::getDelay()
{
    return loopDelay;
}


class RedProgram : public RegisterProgram<RedProgram>
{
    public:
    RedProgram() : RegisterProgram<RedProgram>("Red") {}
    void loop()
    {
        controller->showColor(CRGB::Red, brightness);
        delay(loopDelay);
    }
};

class BlueProgram : public RegisterProgram<BlueProgram>
{
    public:
    BlueProgram() : RegisterProgram<BlueProgram>("Blue") {}
    void loop()
    {
        controller->showColor(CRGB::Blue, brightness);
        delay(loopDelay);
    }
};

class GreenProgram : public RegisterProgram<GreenProgram>
{
    public:
    GreenProgram() : RegisterProgram<GreenProgram>("Green") {}
    void loop()
    {
        controller->showColor(CRGB::Green, brightness);
        delay(loopDelay);
    }
};

void programChase(bool dir, int loopDelay)
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

    for (int i = 0; i < NUM_LEDS; ++i)
    {
        leds[i] /= 2;
    }
    leds[cntr] = CRGB::Red;
    leds[(NUM_LEDS / 3 + cntr) % NUM_LEDS] = CRGB::Green;
    leds[(2 * NUM_LEDS / 3 + cntr) % NUM_LEDS] = CRGB::Blue;
    controller->showLeds(brightness);
    delay(loopDelay);
}

class ChaseProgram : public RegisterProgram<ChaseProgram>
{
    public:
    ChaseProgram() : RegisterProgram<ChaseProgram>("Chase") {}
    void loop()
    {
        programChase(true, loopDelay);
    }
};

class ChaseReverseProgram : public RegisterProgram<ChaseReverseProgram>
{
    public:
    ChaseReverseProgram() : RegisterProgram<ChaseReverseProgram>("Chase Reverse") {}
    void loop()
    {
        programChase(true, loopDelay);
    }
};

class SeizureWarning : public RegisterProgram<SeizureWarning>
{
    public:
    SeizureWarning() : RegisterProgram<SeizureWarning>("Seizure Warning") {}
    void loop()
    {
        /* RGB Seizure */
        controller->showColor(CRGB::Red, brightness);
        delay(loopDelay);
        controller->showColor(CRGB::Green, brightness);
        delay(loopDelay);
        controller->showColor(CRGB::Blue, brightness);
        delay(loopDelay);
    }
};

class HueSolid : public RegisterProgram<HueSolid>
{
    public:
    HueSolid() : RegisterProgram<HueSolid>("Hue Solid") {}
    void loop()
    {
        /* Solid through hue */
        static CHSV color = {0, 255, 255};
        color.h += 1;
        controller->showColor(color, brightness);
        delay(loopDelay);
    }
};

class HueCircle : public RegisterProgram<HueCircle>
{
    public:
    HueCircle() : RegisterProgram<HueCircle>("Hue Circle") {}
    void loop()
    {
        static uint8_t offset = 0;
        for (int cntr = 0; cntr < NUM_LEDS; ++cntr)
        {
            CHSV color = {
                offset + static_cast<byte>(0xFF & ((cntr << 8) / (NUM_LEDS - 1))),
                255,
                255};
            leds[cntr] = color;
        }
        controller->showLeds(brightness);
        ++offset;
        delay(loopDelay);
    }
};

class Hertz : public RegisterProgram<Hertz>
{
    public:
    Hertz() : RegisterProgram<Hertz>("Hertz") {}
    void loop()
    {
        controller->showColor(CRGB::White);
        delay(loopDelay/2);
        controller->showColor(CRGB {0,0,0});
        delay(loopDelay/2);
    }
};

ProgramRegistry programs;

ProgramRegistry::ProgramRegistry() : programs(), programCount(0) {}

bool ProgramRegistry::addProgram(Program *program) {
    if(programCount < REGISTRY_SIZE)
    {
        programs[programCount] = program;
        ++programCount;
        return true;
    }
    else
    {
        return false;
    }
}
Program *ProgramRegistry::operator[](int idx) {
    if(idx >= 0 && idx < programCount)
    {
        return programs[idx];
    }
    else
    {
        return nullptr;
    }
}

void ProgramRegistry::printPrograms() const
{
  char buf[80] = "";
  for (int cntr = 0; cntr < programCount; ++cntr)
  {
    Program *program = programs[cntr];
    sprintf_P(buf, PSTR("%d: %s"), cntr, program->getDescription().c_str());
    Serial.println(buf);
  }
}

int ProgramRegistry::length() const 
{
    return this->programCount;
}