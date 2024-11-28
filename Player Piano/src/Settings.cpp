#include "Settings.h"

void Settings::setMaxDPWM(uint8_t pwm){
  this->MAX_D_PWM = pwm;
}
void Settings::setMinDPWM(uint8_t pwm){
  this->MIN_D_PWM = pwm;
}
uint8_t Settings::getMaxDPWM(){
  return this->MAX_D_PWM;
}
uint8_t Settings::getMinDPWM(){
  return this->MIN_D_PWM;
}
void Settings::setVolumeScaler(uint8_t scaler){
  this->volumeScaler=scaler;
}
uint8_t Settings::getVolumeScaler(){
  return this->volumeScaler;
}
boolean Settings::getSolenoidState(){
  return this->solenoidState;
}
void Settings::setSolenoidState(boolean state){
  this->solenoidState = state;
}
boolean Settings::getSolenoidLastState(){
  return this->solenoidLastState;
}
void Settings::setSolenoidLastState(boolean state){
  this->solenoidLastState = state;
}
