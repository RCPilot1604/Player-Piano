#include "Note.h"

std::vector<Commands> &Note::returnCommands(){
  return this->commandList;
}
void Note::eraseCommands(uint8_t index){
  auto vend = commandList.rend();
  for(auto it = commandList.rbegin(); it != vend; it++){
    commandList.erase(std::next(it).base());
    index--;
    if(index==0) break;
  }
}
void Note::scheduleOn(uint8_t vel, unsigned long TD){
  if(vel > mySettings.MIDI_MIN_VELOCITY){ //velocity has to be larger than the MIN_VELOCITY
    uint8_t scaledVel = calculateVelocity(vel);
    int velocityDuration = round(((-25 * scaledVel) / (double)127) + mySettings.VELOCITY_DURATION); //the minimum delay is VELOCITY_DURATION - 25 
    int mappedPwm = calculatePWM(scaledVel);
    commandList.push_back(Commands(mySettings.ON_PWM, TD));
    commandList.push_back(Commands(mappedPwm,TD+mySettings.STARTUP_DURATION));
    commandList.push_back(Commands(mySettings.HOLD_PWM, TD+mySettings.STARTUP_DURATION+velocityDuration));
    setLastScheduledState(true, TD);
  }
}
void Note::scheduleOff(unsigned long TD){
  commandList.push_back(Commands(mySettings.MIN_PWM,TD));
  setLastScheduledState(false, TD);
}
void Note::scheduleBB(int pwm, unsigned long TD){
  Serial.println("Schedule Bounce Back");
  commandList.push_back(Commands(mySettings.BB_ON_PWM, TD)); //schedule the command for high current rush
  commandList.push_back(Commands(pwm, TD+mySettings.BB_STARTUP_DURATION)); //schedule the command for velocity stroke
  commandList.push_back(Commands(mySettings.HOLD_PWM, TD+mySettings.BB_STARTUP_DURATION+mySettings.BB_VELOCITY_DURATION)); //schedule holding command
  commandList.push_back(Commands(mySettings.getMinDPWM(), TD+mySettings.BB_TOTAL_DURATION)); //immediately shedule a deactivation to bounce back
  setLastScheduledState(false, TD+mySettings.BB_TOTAL_DURATION);
}
int Note::getOldPWM(uint8_t index){
  return this->commandList[commandList.size()-index].getPwm(); //index starts from 1 not 0
}
byte Note::getMidiId(){
  return this->midiId;
}
uint8_t Note::calculateVelocity(byte midiVelocity){
  uint8_t mappedVel = this->minVelocity + midiVelocity * (this->maxVelocity - this->minVelocity) / 127; //returns the mapped velocity according to the min and max velocity of each note object
  //Serial.print("mapped vel = "); Serial.println(mappedVel);
  //Serial.print("vol scaler = "); Serial.println(mySettings.getVolumeScaler());
  uint8_t scaledVel = round(((double)mappedVel / 100.0) * (double)mySettings.getVolumeScaler());
  //Serial.print("scaled vel = "); Serial.println(scaledVel);
  return scaledVel;
}
int Note::calculatePWM(uint8_t scaledVelocity){
  //Serial.print("MaxDPWM = "); Serial.println(mySettings.getMaxDPWM());
  //Serial.print("MinDPWM = "); Serial.println(mySettings.getMinDPWM());
  double slope = (mySettings.getMaxDPWM() - mySettings.getMinDPWM()) / 127.0;
  //Serial.print("Slope = "); Serial.println(slope,5);
  int mappedPWM = mySettings.getMinDPWM() + round((double)scaledVelocity * slope);
  //Serial.print("mappedPWM = "); Serial.println(mappedPWM);
  return mappedPWM;
}
void Note::setNoteState(bool state){
  this->noteState = state;
}
boolean Note::getNoteState(){
  return this->noteState;
}


unsigned long Note::getLastScheduledAt(){
  return this->lastScheduledAt;
}

boolean Note::getLastScheduledState(){
  return this->lastScheduledState;
}
  
void Note::setLastScheduledState(bool state, unsigned long timenow){
  this->lastScheduledState = state;
  this->lastScheduledAt = timenow;
}

uint8_t Note::getMaxVel(){
  return this->maxVelocity;
}
uint8_t Note::getMinVel(){
  return this->minVelocity;
}


void Note::setMaxVel(uint8_t maxVel){
  this->maxVelocity = maxVel;
}
void Note::setMinVel(uint8_t minVel){
  this->minVelocity = minVel;
}
