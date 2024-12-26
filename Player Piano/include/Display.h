#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <vector>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ezButton.h>
#include <ArduinoNvs.h> 
#include "Note.h"
#include "Settings.h"

class Display{
private:
  std::vector<Note> &notes;
  Settings &mySettings; 
  ArduinoNvs mynvs;
  const int ROTARY_ENC_A = 16;
  const int ROTARY_ENC_B = 17;
  const int ROTARY_ENC_BUTTON = 18;
  const int LEFT_BUTTON = 19;
  const int HOME_BUTTON = 33;
  const int RIGHT_BUTTON = 32;
  const int BLE_ALERT_LED = 23;
  
  LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,16,2); 
  //declaring relevant pins
  ezButton rotaryEncA = ezButton(ROTARY_ENC_A);
  ezButton rotaryEncB = ezButton(ROTARY_ENC_B);
  ezButton rotaryEncButton = ezButton(ROTARY_ENC_BUTTON);
  ezButton leftButton = ezButton(LEFT_BUTTON);
  ezButton homeButton = ezButton(HOME_BUTTON);
  ezButton rightButton = ezButton(RIGHT_BUTTON);
 
  //cleanup the code here
  int CURRENT_NOTE = mySettings.MIN_NOTE_ID; //this is the current note that will be saved to EEPROM
  int MAX_NOTE = 108; //max note check for settings page
  int MIN_NOTE = 21; //min note check for settings page
  int VOLUME = 100; //current volume that will be saved to EEPROM
  int MAX_VOLUME = 100; //max volume check for settings page
  int MIN_VOLUME = 0; //min volume check for settings page
  int MAX_VEL = 127; //max V check for settings page
  int MIN_VEL = 0; //min V check for settings page
  int MAX_PWM_LIMIT = 4095; //max PWM check for settings page
  int MIN_PWM_LIMIT = 0; //min PWM check for settings page

  //states
  boolean lastEncA = false, lastEncButton = false, lastLeftButton = false, lastHomeButton = false, lastRightButton = false;
  boolean flashing = false; //this is the flag for flashing the "<" sign
  boolean flash = false;
  
  //debounce
  int DEBOUNCE_DELAY = 500;
  unsigned long debounceTimer = 0;
  
  //declaring the variables for SETTINGS page
  uint8_t currentState = 0; 
  uint8_t currentSubstate = 0; //either 0 - not in sublevel, 1 - top row, 2 - bottom row
  uint8_t tempNote; //this variable will temporarily hold a value for note selection
  uint8_t placeholder1; //placeholder 1 for data
  uint8_t placeholder2; //placeholder 2 for data

  //misc
  unsigned long FLASH_TIMER = 0;
  int FLASH_PERIOD = 500;
  
public:
  Display(Settings &mySettings, std::vector<Note> &notes) : mySettings(mySettings), notes(notes){}
  void LCDBegin();
  void flashCursor();
  void runProcesses();
  boolean debounce(uint8_t pin);
  void checkDisplayStates();
  void handleEncoder(boolean dir); //function that is called when the encoder is moved
  void handleDirBtnClick(boolean dir); //function that is called when LEFT/RIGHT button is clicked
  void handleHomeBtnClick(); //function called when HOME button is clicked
  void handleEncdrClick();//function that is called when the rotary encoder button is clicked
  void replaceContent(uint8_t posCol, uint8_t posRow, String msg); //function that is called to replace current content at a given position
  void showStateScreen(uint8_t screen);
  void setupNVS();
  boolean canSendBLE = true;
};
