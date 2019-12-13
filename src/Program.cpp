#include "Program.h"

class RedProgram : public RegisterProgram<RedProgram>
{
    public:
    RedProgram() : RegisterProgram<RedProgram>("Red") {}
    void loop()
    {
        controller->showColor(CRGB::Red, brightness);
        delay(250);
    }
};

class BlueProgram : public RegisterProgram<BlueProgram>
{
    public:
    BlueProgram() : RegisterProgram<BlueProgram>("Blue") {}
    void loop()
    {
        controller->showColor(CRGB::Blue, brightness);
        delay(250);
    }
};

class GreenProgram : public RegisterProgram<GreenProgram>
{
    public:
    GreenProgram() : RegisterProgram<GreenProgram>("Green") {}
    void loop()
    {
        controller->showColor(CRGB::Green, brightness);
        delay(250);
    }
};

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

class ChaseProgram : public RegisterProgram<ChaseProgram>
{
    public:
    ChaseProgram() : RegisterProgram<ChaseProgram>("Chase") {}
    void loop()
    {
        programChase(true);
    }
};

class ChaseReverseProgram : public RegisterProgram<ChaseReverseProgram>
{
    public:
    ChaseReverseProgram() : RegisterProgram<ChaseReverseProgram>("Chase Reverse") {}
    void loop()
    {
        programChase(false);
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
        delay(60);
        controller->showColor(CRGB::Green, brightness);
        delay(60);
        controller->showColor(CRGB::Blue, brightness);
        delay(60);
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
        delay(30);
    }
};

class HueCircle : public RegisterProgram<HueCircle>
{
    public:
    HueCircle() : RegisterProgram<HueCircle>("Hue Circle") {}
    void loop()
    {
        static int start = 0;
        if (start >= NUM_LEDS)
        {
            start = 0;
        }

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
        delay(25);
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