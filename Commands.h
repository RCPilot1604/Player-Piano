#pragma once
#include <Arduino.h>
#include <stdint.h>
#ifndef COMMANDS_H
#define COMMANDS_H
class Commands{
  private:
    int pwm;
    unsigned long runAt;
  public:
    Commands(int pwm, unsigned long runAt) : pwm(pwm), runAt(runAt) {}
    int getPwm();
    unsigned long getRunAt();
};
#endif
