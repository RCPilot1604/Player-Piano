#pragma once
#include <Arduino.h>
#include <stdint.h>

class Settings{
private:
  uint8_t MAX_D_PWM; //this is the variable that will store the Max PWM values
  uint8_t MIN_D_PWM; //this is the variable that will store the Min PWM values
  uint8_t volumeScaler = 100; //this is the volume scaler
public:
  void setMaxDPWM(uint8_t pwm);
  void setMinDPWM(uint8_t pwm);
  uint8_t getMaxDPWM();
  uint8_t getMinDPWM();
  void setVolumeScaler(uint8_t scaler);
  uint8_t getVolumeScaler();
  boolean getSolenoidState();
  void setSolenoidState(boolean state);
  boolean getSolenoidLastState();
  void setSolenoidLastState(boolean state);
    
  const uint8_t MIN_NOTE_ID = 21;
  const uint8_t MAX_NOTE_ID = 108;
  
  const uint8_t HOLD_PWM = 127; //this must be the solenoid's nominal voltage, 12V
  const uint8_t ON_PWM = 255; //this supplies the pulse of high voltage that allows the solenoid to move quickly.
  
  //MIN_PWM and MAX_PWM are for mapping the velocity of the notes to PWM for the VELOCITY PHASE. As such the maximum PWM
  //should be calculated based off a reasonable voltage for the solenoid.
  const uint8_t MIN_PWM = 0; //corresponds to 0V
  const uint8_t MAX_PWM = 159; //corresponds to 15V
  
  //This is for scaling the volume between notes. 
  //Do not use these variables to limit voltage as this is already done using MAX_PWM. MIDI_MAX_VELOCITY will correspond to MAX_PWM
  const uint8_t MIDI_MIN_VELOCITY = 0;
  const uint8_t MIDI_MAX_VELOCITY = 127;
  
  const int DELAY_TIME = 200; //delay time in ms for the scheduling.
  //This is the time gap between a command being scheduled and it being executed
  //Essentially this is the lookahead. This value should be set just higher than the activation time + deactivation time 
  const int STARTUP_DURATION = 10; //time spent on the startup (high current pulse) phase, corresponds to ON_PWM
  const int VELOCITY_DURATION = 50; //minimum time spent on the velocity (MIDI-dependent) phase.
  //The equation for calculating the total time spent in the velocity phase is: 
  // VELOCITY_DURATION = round(((-25 * midi_vel) / (double)127) + VELOCITY_DURATION)

  //derived empirically
  const int ACTIVATION_DURATION = 50; //the time taken for the note to become fully activated (this can be thought of as the
  //actual time it takes for the note to fully depress. Changing this value will affect the CRITERIA for when BounceBack / rescheduling
  //is implemented but not the actual implementation of these features themselves.
  const int DEACTIVATION_DURATION = 75; //similar to ACTIVATION_DURATION, the time taken for a note to become fully deactivated
  //Does not affect how BounceBack / rescheduling is implemented but instead WHEN these are implemented. 
  const int BOUNCEBACK_DURATION = 50; //time taken for note to complete a bounce back cycle
  
  const int BB_ON_PWM = 255; //Pulse PWM for BounceBack
  const int BB_STARTUP_DURATION = 20; //Duration of Pulse PWM for BounceBack
  const int BB_VELOCITY_DURATION = 50; //Duration of Velocity for BounceBack
  const int BB_HOLD_DURATION = 0; //Hold Duration for Bounceback (can be 0)
  const int BB_TOTAL_DURATION = BB_STARTUP_DURATION + BB_VELOCITY_DURATION + BB_HOLD_DURATION;
  
  const int NOTE_TIMEOUT = 10000; //After x amount of time, any note left on will be auto turned off
  const uint8_t MAX_NOTES = 10; //maximum number of notes that can be played at any point in time

  // BOARD_MIDI_VALUES
  const uint8_t BOARD_1_MIN_ID = 21;
  const uint8_t BOARD_1_MAX_ID = 36;
  const uint8_t BOARD_2_MIN_ID = 37;
  const uint8_t BOARD_2_MAX_ID = 52;
  const uint8_t BOARD_3_MIN_ID = 53;
  const uint8_t BOARD_3_MAX_ID = 68;
  const uint8_t BOARD_4_MIN_ID = 69;
  const uint8_t BOARD_4_MAX_ID = 84;
  const uint8_t BOARD_5_MIN_ID = 85;
  const uint8_t BOARD_5_MAX_ID = 100;
  const uint8_t BOARD_6_MIN_ID = 101;
  const uint8_t BOARD_6_MAX_ID = 108;

  const int PWM_FREQ = 1600;

  const int SOLENOID_ON_PIN = 25;
  const int OE_PIN = 26;
  boolean solenoidState = false;
  boolean solenoidLastState = false;
};  
