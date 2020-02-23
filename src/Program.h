#pragma once
#include <Arduino.h>
#include "FastLED.h"

#define REGISTRY_SIZE 100
#define NUM_LEDS 84
extern CRGB leds[NUM_LEDS];
extern CLEDController *controller;
extern uint8_t brightness;

/**
 * @brief 
 * 
 */
class Program {
    protected:
    Program(const String &description, const uint16_t delay = 100);
    public:
    virtual void loop() = 0;
    String getDescription();
    void setDelay(uint16_t ms);
    uint16_t getDelay();

    protected:
    String description;
    uint16_t loopDelay;
};

/**
 * @brief A registry of LED light programs
 */
class ProgramRegistry {
    public:
    ProgramRegistry();
    /**
     * @brief Adds a program to the registry
     * 
     * @param program 
     * @return true - if there was space in the registry
     * @return false  - if the registry is full
     */
    bool addProgram(Program *program);
    void printPrograms() const;
    Program *operator[](int idx);
    int length() const;

    protected:
    Program *programs[REGISTRY_SIZE];
    int programCount;
};
extern ProgramRegistry programs;

/**
 * @brief Class to extend in order to add automatically register an LED program
 * 
 * @tparam T The class extending RegisterProgram.
 */
template <typename T> class RegisterProgram : public Program {
    public:
    /**
     * @brief Construct a new Register Program object
     * 
     * @param description The description of the program.
     */
    RegisterProgram(const String &description) : Program(description), _initialized(initialized) {}

    /** 
     * @brief Static variable to hold the result RegisterProgram<T>::init.
     */
    static bool initialized;
    /**
     * @brief When this class is created init() is called to add the Program to the ProgramRegistry
     */
    static bool init()
    {
        Program * t = new T();
        return programs.addProgram(t);
    }
    protected:
    /**
     * @brief Set to the value of RegisterProgram<T>::initialized.
     * 
     *  This forces the compiler to create the static class member and
     *  add the class to the registry.
     */
    bool _initialized;
};

/**
 * @brief Static variable to hold the result RegisterProgram<T>::init.
 * 
 * @tparam T - The Program
 */
template <typename T> bool RegisterProgram<T>::initialized = RegisterProgram<T>::init();