#include "Settings.h"

void Settings::setMaxDPWM(int pwm){
  this->MAX_D_PWM = pwm;
}
void Settings::setMinDPWM(int pwm){
  this->MIN_D_PWM = pwm;
}
int Settings::getMaxDPWM(){
  return this->MAX_D_PWM;
}
int Settings::getMinDPWM(){
  return this->MIN_D_PWM;
}
void Settings::setVolumeScaler(uint8_t scaler){
  this->volumeScaler=scaler;
}
int Settings::getVolumeScaler(){
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
