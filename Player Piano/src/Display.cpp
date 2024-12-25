#include "Display.h"

#define MEMORY_DEBUG //uncomment this to view debug messages relating to NVS

void Display::flashCursor() {
  if (millis() - FLASH_TIMER > FLASH_PERIOD) {
    if (flash) {
      replaceContent(7, currentSubstate - 1, "<"); //flash on
    } else {
      replaceContent(7, currentSubstate - 1, " "); //flash off
    }
    flash = !flash;
    FLASH_TIMER = millis();
  }
}
boolean Display::debounce(uint8_t pin) {
  boolean firstReading = digitalRead(pin);
  if (digitalRead(pin) != firstReading) {
    debounceTimer = millis();
  }
  if (millis() - debounceTimer > 100) {
    return !firstReading;
  }
}
void Display::LCDBegin() {
  pinMode(ROTARY_ENC_A, INPUT_PULLUP);
  pinMode(ROTARY_ENC_B, INPUT_PULLUP);
  pinMode(ROTARY_ENC_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(HOME_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(BLE_ALERT_LED, OUTPUT);
  digitalWrite(BLE_ALERT_LED, LOW); 
  
  //rotaryEncA.setDebounceTime(50);
  //rotaryEncB.setDebounceTime(50);
  rotaryEncButton.setDebounceTime(50);
  leftButton.setDebounceTime(50);
  homeButton.setDebounceTime(50);
  rightButton.setDebounceTime(50);
  //set up NVS
  setupNVS();
  //set up LCD
  lcd.init();
  lcd.backlight();
  currentSubstate = 0;
  currentState = 0;
  placeholder1 = mySettings.getVolumeScaler();
  placeholder2 = CURRENT_NOTE;
  showStateScreen(currentState);  
}
void Display::runProcesses() { //this function executes all the essential Display processes
  if (flashing) flashCursor();
  checkDisplayStates();
}

void Display::checkDisplayStates() {

  rotaryEncA.loop();
  rotaryEncB.loop();
  rotaryEncButton.loop();
  leftButton.loop();
  homeButton.loop();
  rightButton.loop();

  boolean currentEncA = (boolean)rotaryEncA.getState();
  boolean currentEncB = (boolean)rotaryEncB.getState();
  if (currentEncA != lastEncA) {
    if (!currentEncA) {
      if (currentEncB != currentEncA) { //encoder is moving CW
        handleEncoder(true); //reverse direction
      } else { //encoder is moving CCW
        handleEncoder(false); //forward direction
      }
    }
    lastEncA = currentEncA;
  }
  boolean currentEncButton = rotaryEncButton.isPressed();
  if (currentEncButton != lastEncButton) {
    if (currentEncButton) handleEncdrClick();
    lastEncButton = currentEncButton;
  }
  boolean currentLeftButton = leftButton.isPressed();
  if (currentLeftButton != lastLeftButton) {
    if (currentLeftButton) handleDirBtnClick(true);
    lastLeftButton = currentLeftButton;
  }
  boolean currentRightButton = rightButton.isPressed();
  if (currentRightButton != lastRightButton) {
    if (currentRightButton) handleDirBtnClick(false);
    lastRightButton = currentRightButton;
  }
  boolean currentHomeButton = homeButton.isPressed();
  if (currentHomeButton != lastHomeButton) {
    if (currentHomeButton) handleHomeBtnClick();
    lastHomeButton = currentHomeButton;
  }
}

void Display::handleEncoder(boolean dir) {
  Serial.print("Encoder was turned in the "); Serial.print(dir?"CW":"CCW"); Serial.println(" direction");
  uint8_t col = 0;
  if (flashing) {
    col = 9;
  } else {
    if (currentState > 0) col = 7;
    else col = 9;
  }
  if (dir) { //if moving clockwise
    if (flashing) {
      //flashing means that we are editing the settings. Thus encoder should increment the value of the settings placeholder
      if (currentState == 1) {
        if (currentSubstate == 1) {
          if (placeholder1 < MAX_VOLUME) placeholder1++;
        } else {
          if (placeholder2 < MAX_NOTE) placeholder2++;
        }
      } else if (currentState == 2) {
        if (currentSubstate == 1) {
          if (placeholder1 < MAX_VEL) placeholder1++;
        } else {
          if (placeholder2 < MAX_VEL) placeholder2++;
        }
      } else {
        if (currentSubstate == 1) {
          if (placeholder1 < MAX_PWM_LIMIT) placeholder1++;
        } else {
          if (placeholder2 < MAX_PWM_LIMIT) placeholder2++;
        }
      }
      String message = String((currentSubstate == 1) ? placeholder1 : placeholder2);
      if (currentSubstate == 1 && currentState == 1) message += "%";
      lcd.setCursor(col, currentSubstate - 1);
      lcd.print("    ");
      replaceContent(col, currentSubstate - 1, message);
    } else {
      if (currentSubstate < 2) {
        if (currentSubstate > 0) replaceContent(col, currentSubstate - 1, " ");
        currentSubstate ++; //increment the substate counter
        replaceContent(col, currentSubstate - 1, "<");
      } else {
        replaceContent(col, currentSubstate - 1, " ");
        currentSubstate = 0;
      }
    }
  } else { //if moving counterclockwise
    if (flashing) {
      //flashing means that we are editing the settings. Thus encoder should increment the value of the settings placeholder
      if (currentState == 1) {
        if (currentSubstate == 1) {
          if (placeholder1 > MIN_VOLUME) placeholder1--;
        } else {
          if (placeholder2 > MIN_NOTE) placeholder2--;
        }
      } else if (currentState == 2) {
        if (currentSubstate == 1) {
          if (placeholder1 > MIN_VEL) placeholder1--;
        } else {
          if (placeholder2 > MIN_VEL) placeholder2--;
        }
      } else {
        if (currentSubstate == 1) {
          if (placeholder1 > MIN_PWM_LIMIT) placeholder1--;
        } else {
          if (placeholder2 > MIN_PWM_LIMIT) placeholder2--;
        }
      }
      String message = String((currentSubstate == 1) ? placeholder1 : placeholder2);
      if (currentSubstate == 1 && currentState == 1) message += "%";
      lcd.setCursor(col, currentSubstate - 1);
      lcd.print("    ");
      replaceContent(col, currentSubstate - 1, message);
    } else {
      if (currentSubstate == 0) {
        currentSubstate = 2;
        replaceContent(col, currentSubstate - 1, "<");
      } else {
        replaceContent(col, currentSubstate - 1, " ");
        currentSubstate--;  
        if(currentSubstate > 0) replaceContent(col, currentSubstate - 1, "<");
      }
    }
  }
}

void Display::handleDirBtnClick(boolean dir) {
  Serial.print("Direction Button was clicked: "); Serial.println(dir ? "Left" : "Right");
  if (0 < currentState && currentState < 4) { //if we are in the settings scrollable zone
    if (!dir) { //if it is a right click
      if (currentState == 3) {
        currentState = 1; //reset the main page counter back to 1
      } else {
        currentState ++; //increment the page
      }
    } else {//it is a right click
      if (currentState == 1) {
        currentState = 3;
      } else {
        currentState --;
      }
    }
    switch (currentState) { //update the placeholders with their values
      case 1:
        placeholder1 = mySettings.getVolumeScaler();
        placeholder2 = CURRENT_NOTE;
        break;
      case 2:
        placeholder1 = notes[CURRENT_NOTE-mySettings.MIN_NOTE_ID].getMinVel();
        placeholder2 = notes[CURRENT_NOTE-mySettings.MIN_NOTE_ID].getMaxVel();
        break;
      case 3:
        placeholder1 = mySettings.getMinDPWM();
        placeholder2 = mySettings.getMaxDPWM();
        break;
    }
    showStateScreen(currentState); //update the screen
  }
}
void Display::handleEncdrClick() {
  Serial.println("Encoder was clicked");
  switch (currentState) {
    case 0: //Settings || Test
      if (currentSubstate == 1) {
        currentState = 1;
      } else {
        currentState = 4;
      }
      showStateScreen(currentState);
      break;
    case 1: //Volume || Note
      if (flashing) { //if we are done editing the settings
        if (currentSubstate == 1) {
          if (placeholder1 != mySettings.getVolumeScaler()) { //we do not write VOLUME into NVS because we don't care about saving the last used volume
            #ifdef MEMORY_DEBUG
            Serial.print("Current Volume set to: "); Serial.println(placeholder1);
            #endif
            mySettings.setVolumeScaler(placeholder1);
          }
        } else {
          if (placeholder2 != CURRENT_NOTE) { //we do not write CURRENT_NOTE into NVS (CURRENT_NOTE is used for the program to write other parameters into the NVS)
            CURRENT_NOTE = placeholder2;
            #ifdef MEMORY_DEBUG
            Serial.print("Current Note selection set to: "); Serial.println(placeholder2);
            #endif
          }
        }
        if(flash) replaceContent(7, currentSubstate - 1, "<"); //flash back on
      }
      flashing = !flashing; //toggle flashing
      break;
    case 2: //MinVel || MaxVel
      if (flashing) { //if we are done editing the settings
        if (currentSubstate == 1) {
          if (placeholder1 != notes[CURRENT_NOTE-mySettings.MIN_NOTE_ID].getMinVel()) { //we only write into our NVS if the data has changed
            //write into NVS
            notes[CURRENT_NOTE-mySettings.MIN_NOTE_ID].setMinVel((uint8_t)placeholder1);
            String noteKey = "MIN_" + String(CURRENT_NOTE-mySettings.MIN_NOTE_ID);
            if(mynvs.setInt(noteKey, placeholder1)){
              #ifdef MEMORY_DEBUG
                Serial.print("Writing MinVel of Note "); Serial.print(notes[CURRENT_NOTE-mySettings.MIN_NOTE_ID].getMidiId()); Serial.print(" into NVS as "); Serial.println(placeholder1);
              #endif
            }else{
              #ifdef MEMORY_DEBUG
                Serial.print("FAILED to write MinVel of Note "); Serial.print(notes[CURRENT_NOTE-mySettings.MIN_NOTE_ID].getMidiId()); Serial.print(" into NVS as "); Serial.println(placeholder1);
              #endif
            }
          }
        } else {
          if (placeholder2 != notes[CURRENT_NOTE-mySettings.MIN_NOTE_ID].getMaxVel()) { //we only write into our NVS if the data has changed
            //write into NVS
            notes[CURRENT_NOTE-mySettings.MIN_NOTE_ID].setMaxVel((uint8_t)placeholder2);
            String noteKey = "MAX_"+String(CURRENT_NOTE-mySettings.MIN_NOTE_ID);
            if(mynvs.setInt(noteKey,placeholder2)){
              #ifdef MEMORY_DEBUG
                Serial.print("Writing MaxVel of Note "); Serial.print(notes[CURRENT_NOTE-mySettings.MIN_NOTE_ID].getMidiId()); Serial.print(" into NVS as "); Serial.println(placeholder2);
              #endif
            }else{
              #ifdef MEMORY_DEBUG
                Serial.print("FAILED to write MaxVel of Note "); Serial.print(notes[CURRENT_NOTE-mySettings.MIN_NOTE_ID].getMidiId()); Serial.print(" into NVS as "); Serial.println(placeholder2);
              #endif
            }
          }
        }
        if(flash) replaceContent(7, currentSubstate - 1, "<"); //flash back on
      }
      flashing = !flashing; //toggle flashing
      break;
    case 3: //MinPWM || MaxPWM
      if (flashing) { //if we are done editing the settings
        if (currentSubstate == 1) {
          if (placeholder1 != mySettings.getMinDPWM()) { //we only write into our NVS if the data has changed
            //write into NVS
            Serial.println("Writing MinPWM into NVS as "); Serial.println(placeholder1);
            mySettings.setMinDPWM(placeholder1);
            String minPwm = "MIN_PWM";
            if(mynvs.setInt(minPwm,placeholder1)){
              #ifdef MEMORY_DEBUG
                Serial.print("Writing minPWM into NVS as "); Serial.println(placeholder1);
              #endif 
            }else{
              #ifdef MEMORY_DEBUG
                Serial.print("FAILED to write minPWM into NVS as "); Serial.println(placeholder1);
              #endif 
            }
          }
        } else {
          if (placeholder2 != mySettings.getMaxDPWM()) { //we only write into our NVS if the data has changed
            //write into NVS
            Serial.println("Writing MaxPWM into NVS as "); Serial.println(placeholder2);
            mySettings.setMaxDPWM(placeholder2);
            if(mynvs.setInt("MAX_PWM",placeholder1)){
              #ifdef MEMORY_DEBUG
                Serial.print("Writing maxPWM into NVS as "); Serial.println(placeholder2);
              #endif 
              #ifdef MEMORY_DEBUG
                Serial.print("FAILED to write maxPWM into NVS as "); Serial.println(placeholder2);
              #endif 
            } 
          }
        }
      }
      flashing = !flashing; //toggle flashing
      break;
     case 4: //Normal < || Bounce <
      uint8_t velocity = notes[CURRENT_NOTE-mySettings.MIN_NOTE_ID].getMaxVel();
      if(currentSubstate == 1){
        notes[CURRENT_NOTE-mySettings.MIN_NOTE_ID].scheduleOn(velocity,millis()+mySettings.DELAY_TIME);
        notes[CURRENT_NOTE-mySettings.MIN_NOTE_ID].scheduleOff(millis()+mySettings.DELAY_TIME+mySettings.ACTIVATION_DURATION);
      }else{
        uint8_t pwm = notes[CURRENT_NOTE-mySettings.MIN_NOTE_ID].calculatePWM(velocity);
        notes[CURRENT_NOTE-mySettings.MIN_NOTE_ID].scheduleBB(pwm,millis()+mySettings.DELAY_TIME);
      }
      break;
  }
}

void Display::handleHomeBtnClick() {
  Serial.println("Home Button was clicked");
  currentState = 0;
  currentSubstate = 0;
  showStateScreen(currentState);
}

void Display::replaceContent(byte posCol, byte posRow, String msg) {
  lcd.setCursor(posCol, posRow);
  for (uint8_t i = 0; i < msg.length(); i++) {
    lcd.print(" ");
  }
  lcd.setCursor(posCol, posRow);
  lcd.print(msg);
}
void Display::showStateScreen(byte screen) {
  Serial.print("Showing screen: "); Serial.println(screen);
  if(screen==1 || screen ==0) canSendBLE = true; 
  else canSendBLE = false; 
  if(!canSendBLE) digitalWrite(BLE_ALERT_LED, HIGH);
  else digitalWrite(BLE_ALERT_LED, LOW);
  currentSubstate = 0; //reset the substate
  flashing = false; 
  lcd.clear(); //clear the LCD
  String firstRow = "", secondRow = "";
  switch (screen) {
    case 0: //first page, Settings (<) || Test (<)
      firstRow = "Settings";
      secondRow = "Test";
      break;
    case 1: //second page, Volume:< XXX% || Note  :<
      firstRow = "Volume:  " + String(placeholder1)+"%";
      secondRow = "Note  :  " + String(placeholder2);
      break;
    case 2: //third page, MinVel:<XXXX|| MaxVel:<XXXX
      firstRow = "MinVel:  " + String(placeholder1);
      secondRow = "MaxVel:  " + String(placeholder2);
      break;
    case 3: //fourth page, MinPWM:<XXXX|| MaxPWM:<XXXX
      firstRow = "MinPWM:  " + String(placeholder1);
      secondRow = "MaxPWM:  " + String(placeholder2);
      break;
    case 4: //fifth page, Normal < || Bounce <
      firstRow = "Normal ";
      secondRow = "Bounce ";
      break;
  }
  lcd.setCursor(0, 0);
  lcd.print(firstRow);
  lcd.setCursor(0, 1);
  lcd.print(secondRow);
}
void Display::setupNVS(){
  mynvs.begin("settings");
  //create the vector of notes using EEPROM (if avail) or DEFAULT values
  for(int i = mySettings.MIN_NOTE_ID; i <= mySettings.MAX_NOTE_ID; i++){
    String maxKeyName = "MAX_"+String(i);
    String minKeyName = "MIN_"+String(i);
    //Serial.print("MaxKeyName: "); Serial.print(maxKeyName); Serial.print(", "); Serial.print("MinKeyName: "); Serial.println(minKeyName);
    //check if EEPROM data is initialized for the particular note
    uint8_t maxV = mySettings.MIDI_MAX_VELOCITY; 
    int savedMaxV = mynvs.getInt(maxKeyName);
    if(savedMaxV == 0){ //if there is currently no value stored in the NVS for this particular key name
      if(mynvs.setInt(maxKeyName, maxV)){
        #ifdef MEMORY_DEBUG
        Serial.print("Saved default value of maxV for note "); Serial.println(i);
        #endif
      }else{
        #ifdef MEMORY_DEBUG
        Serial.print("FAILED to save default value of maxV for note "); Serial.println(i);
        #endif
      }
    }else{
      maxV = savedMaxV; 
      #ifdef MEMORY_DEBUG
      Serial.print("RETRIEVED saved value of maxV of "); Serial.print(maxV); Serial.print(" for note "); Serial.println(i);
      #endif 
    }
    int savedMinV = mynvs.getInt(minKeyName); //if savedMinV == 0 it could either mean that the saved value is actually 0 or that there is no data being saved 
    //but in either case we don't care since the end outcome is the same: 0 is the standard minV
    #ifdef MEMORY_DEBUG
    Serial.print("SET minV to be "); Serial.print(savedMinV); Serial.print(" for note "); Serial.println(i); 
    #endif
    notes.push_back(Note(mySettings, i, maxV, savedMinV));
  }
  uint8_t savedMaxPWM = mynvs.getInt("MAX_PWM");
  if(savedMaxPWM != mySettings.MAX_PWM){ //we will save the value of MAX_PWM into the NVS if it is different from the one defined in our sketch
    if(mynvs.setInt("MAX_PWM", mySettings.MAX_PWM)){
      #ifdef MEMORY_DEBUG
        Serial.print("Saved default value of MaxPWM as "); Serial.println(mySettings.MAX_PWM);
      #endif
      }else{
       #ifdef MEMORY_DEBUG
        Serial.println("FAILED to save default value of MaxPWM");
       #endif 
      }
  }
  mySettings.setMaxDPWM(mynvs.getInt("MAX_PWM")); //in any case, set the max pwm parameter to the value retrieved from the NVS
  #ifdef MEMORY_DEBUG
  Serial.print("RETRIEVED saved value of MaxPWM "); Serial.println(mySettings.getMaxDPWM());
  #endif

  uint8_t savedMinPWM = mynvs.getInt("MIN_PWM");
  if(mySettings.MIN_PWM != savedMinPWM){ //the default mySettings.MIN_PWM is different from that in the NVS
    if(mynvs.setInt("MIN_PWM",mySettings.MIN_PWM)){ 
      #ifdef MEMORY_DEBUG
      Serial.print("Saved default value of MinPWM as "); Serial.println(mySettings.MIN_PWM);
      #endif
    }else{
      #ifdef MEMORY_DEBUG
      Serial.println("FAILED to save default value of MinPWM");
      #endif
    }
   }
   mySettings.setMinDPWM(mynvs.getInt("MIN_PWM")); //
   #ifdef MEMORY_DEBUG
     Serial.print("RETRIEVED saved value of MinPWM "); Serial.println(mySettings.getMinDPWM());
   #endif
}
