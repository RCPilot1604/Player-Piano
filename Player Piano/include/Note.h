#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <vector>
#include <iterator>
#include "Settings.h"
#include "Commands.h"

#ifndef NOTE_H
#define NOTE_H
class Note{
  private: 
    Settings &mySettings; //create a settings object
    std::vector<Commands> commandList{}; //create a vector of type Command object. This list a list of commands to be executed for this specific note
    uint8_t midiId;
    uint8_t maxVelocity;
    uint8_t minVelocity;
    
    bool noteState;
    bool lastScheduledState; //the state of the last scheduled note (ON/OFF)
    unsigned long lastScheduledAt; //The timestamp of when a note was last scheduled
    
  public:
    Note(Settings &mySettings, uint8_t midiId, uint8_t maxVelocity, uint8_t minVelocity) 
    : mySettings(mySettings), midiId(midiId), maxVelocity(maxVelocity), minVelocity(minVelocity), noteState(false), lastScheduledState(false), lastScheduledAt(millis()) {}

    std::vector<Commands> &returnCommands();
    
    void scheduleOn(uint8_t vel, unsigned long TD);
    void scheduleOff(unsigned long TD);
    void scheduleBB(int pwm, unsigned long TD);
    int getOldPWM(uint8_t index);
    
    void eraseCommands(uint8_t index);
    
    void setNoteState(bool state);
    bool getNoteState();
    
    void setLastScheduledState(bool state, unsigned long timenow);
    bool getLastScheduledState();
    
    unsigned long getLastScheduledAt();
    //set last scheduled at is handled by setLastScheduledState
    
    uint8_t getMidiId();
    uint8_t getMaxVel();
    uint8_t getMinVel();
    void setMaxVel(uint8_t maxVel);
    void setMinVel(uint8_t minVel);
    int getActivationTime();
    int getDeactivationTime();
    int getPulseDuration();
    uint8_t calculateVelocity(uint8_t midiVelocity);
    uint8_t calculatePWM(uint8_t scaledVelocity);
};
#endif
