#include "Settings.h"

uint8_t Settings::getMaxDPWM(){
  return this->MAX_PWM;
}
uint8_t Settings::getMinDPWM(){
  return this->MIN_PWM;
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
