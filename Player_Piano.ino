#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>
#include <algorithm>
#include <vector> 
#include <iterator>

#include "Note.h"
#include "Display.h"
#include "Settings.h"

Settings mySettings; //create a settings object 

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(0x40, Wire); //create a PCA9635 object
//add more PCA boards here

#define SERIAL_DEBUG_RECEIVED
#define SERIAL_DEBUG_SCHEDULE
#define SERIAL_DEBUG_COMMAND
#define SERIAL_DEBUG_NOTECOUNTER

BLEMIDI_CREATE_INSTANCE("Player Piano", MIDI)

std::vector<Note> notes; //create a vector of type Note object. This stores the STATE OF INDIVIDUAL NOTES

Display LCDisplay(mySettings, notes); //create a Display object

uint8_t counter = 0; //initialize a counter that holds the total number of notes that are ON
uint8_t oldCounter = 0; 
unsigned long commandTimer = 0;

void setup() {
  Serial.begin(115200);
  
  //update other misc variables:
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  //set the Solenoids to OFF to prevent them from going ON on power up
  pinMode(mySettings.SOLENOID_ON_PIN, OUTPUT);
  digitalWrite(mySettings.SOLENOID_ON_PIN, LOW);
  
  MIDI.begin(MIDI_CHANNEL_OMNI);

  BLEMIDI.setHandleConnected(OnConnected);
  BLEMIDI.setHandleDisconnected(OnDisconnected);
  MIDI.setHandleNoteOn(OnNoteOn);
  MIDI.setHandleNoteOff(OnNoteOff);

  board1.begin(); 
  board1.setPWMFreq(mySettings.PWM_FREQ);
  //add more boards

  //set all the channels to 0 first
  for (int channel = 0; channel < 16; channel++) {
    board1.setPin(channel, 0);
  }//add more boards
  LCDisplay.LCDBegin(); //initialize the Display
  delay(500); //wait for everything to stabilize first

  //now that we have set all the solenoids to low, we can turn on the solenoids
  mySettings.setSolenoidState(true); 
}
void loop() {
  // put your main code here, to run repeatedly:
  MIDI.read();
  counter = 0; //reset the counter
  for(auto it = notes.begin(); it != notes.end(); it++){ //iterate through the vector of notes
    //declare variables we will be using
    uint8_t midiId = it->getMidiId(); //we need the midiId for a bunch of things
    boolean noteState = it->getNoteState();
    boolean lastScheduledState = it->getLastScheduledState();
    unsigned long lastScheduledAt = it->getLastScheduledAt();

    //count current number of notes that are on and check for notes that have timed out
    if(noteState){
    counter ++;
      if(lastScheduledState && millis() > lastScheduledAt && (millis() - lastScheduledAt) > mySettings.NOTE_TIMEOUT){
        //if the last scheduled state is ON i.e it is not scheduled to turn off AND the last time it was turned on was more than 10s ago:
        it->scheduleOff(millis()+mySettings.DELAY_TIME); //schedule an off command to turn the note off
        #ifdef SERIAL_DEBUG_COMMAND
          Serial.print("Note: "); Serial.print(midiId); Serial.print(" was last schedAt: "); Serial.print(millis()+mySettings.DELAY_TIME); Serial.println(", auto off"); 
        #endif
      }      
    }

    std::vector<Commands> &noteCommands = it->returnCommands();
    for(auto i = noteCommands.begin(); i != noteCommands.end(); i++){ //iterate through vector of commands pertaining to that note
      unsigned long runAt = i->getRunAt();
      int pwm = i->getPwm();
      if(runAt <= millis()){
      if (midiId >= mySettings.BOARD_1_MIN_ID && midiId <= mySettings.BOARD_1_MAX_ID) {
        Serial.print("Writing PWM: "); Serial.print(pwm); Serial.print(" to Board 1 Channel: "); Serial.println(midiId - mySettings.BOARD_1_MIN_ID);
        board1.setPin(midiId - mySettings.BOARD_1_MIN_ID, pwm);
      } //add more conditions here
      it->setNoteState(pwm > mySettings.MIN_PWM);
      #ifdef SERIAL_DEBUG_COMMAND
        Serial.print(millis()); Serial.print(" - RUN CMD: midiId - "); Serial.print(midiId); Serial.print(" , pwm - "); Serial.println(pwm);
        Serial.print(millis()); Serial.print(" - NOTE STATE: Id - "); Serial.print(midiId); Serial.print(" State - "); Serial.println(it->getNoteState()); 
      #endif
      noteCommands.erase(i--); //erase the command that has just been executed
      }
    }   
  }
  
  #ifdef SERIAL_DEBUG_NOTECOUNTER
    if(counter != oldCounter){
      Serial.print("Current Notes ON: "); 
      Serial.println(counter);
      oldCounter = counter;
    }
  #endif

  //display 
  LCDisplay.runProcesses();

  //check to see if the solenoid state has changed, and update the state accordingly
  boolean solenoidState = mySettings.getSolenoidState();
  if(mySettings.getSolenoidLastState() != solenoidState){
    digitalWrite(mySettings.SOLENOID_ON_PIN, solenoidState);
    Serial.print("Turning solenoid "); Serial.println(solenoidState?"ON":"OFF");
    mySettings.setSolenoidLastState(solenoidState);
  }
}

void OnConnected(){
  digitalWrite(LED_BUILTIN, HIGH);
}

void OnDisconnected(){
  digitalWrite(LED_BUILTIN, LOW);
}

void OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity){
  #ifdef SERIAL_DEBUG_RECEIVED
    Serial.print(millis()); Serial.print(" - Received ON: Note "); Serial.print(note); Serial.print(" on Channel "); Serial.print(channel); Serial.print(" with velocity "); Serial.println(velocity);
  #endif
  //when a NoteOn is detected, we want to attempt to schedule it
  //DO NOT SCHEDULE if canSendBLE is false
  if(LCDisplay.canSendBLE){
    ScheduleOn(note, velocity);
  }else{
    #ifdef SERIAL_DEBUG_SCHEDULE
      Serial.println("canSendBLE is false, CANNOT schedule!");
    #endif
  }
  
}

void OnNoteOff(uint8_t channel, uint8_t note, uint8_t velocity){
  #ifdef SERIAL_DEBUG_RECEIVED
    Serial.print(millis()); Serial.print(" - Received OFF: Note "); Serial.print(note); Serial.print(" on Channel "); Serial.print(channel); Serial.print(" with velocity "); Serial.println(velocity);
  #endif
  if(LCDisplay.canSendBLE){
    ScheduleOff(note, velocity);
  }else{
    #ifdef SERIAL_DEBUG_SCHEDULE
      Serial.println("canSendBLE is false, CANNOT schedule!");
    #endif
  }
}

void ScheduleOn(uint8_t id, uint8_t velocity){
    if(mySettings.MIN_NOTE_ID <= id && id <= mySettings.MAX_NOTE_ID){ //check to see if the note id is within range, else do nothing
    Note &note =  *(find_if(begin(notes), end(notes), [&id](Note note) { return note.getMidiId() == id; })); //find the note by ID
    bool noteState = note.getNoteState();
    bool lastScheduledState = note.getLastScheduledState();
    unsigned long lastScheduledAt = note.getLastScheduledAt();
    unsigned long TD = millis() + mySettings.DELAY_TIME;

    #ifdef SERIAL_DEBUG_SCHEDULE
      Serial.print(millis()); Serial.print(" - ScheduleOn: ID - "); Serial.print(id); Serial.print(", Note state - "); Serial.print(noteState); Serial.print(", Last Sched State - "); Serial.print(lastScheduledState); Serial.print(", Last Sched At - "); Serial.print(lastScheduledAt); 
      Serial.print(", calc Vel - "); Serial.println(velocity);
    #endif
    
    if(noteState){ //if note is ON (note is being played)
      if(lastScheduledState){ //if last scheduled state is ON
        if(lastScheduledAt < millis()){ //the last scheduled state has already been executed
          note.scheduleOff(TD-mySettings.DEACTIVATION_DURATION); //schedule a deactivation just in time for the new activation
          note.scheduleOn(velocity,TD); //schedule the activation for the new note normally 
          #ifdef SERIAL_DEBUG_SCHEDULE
            Serial.println("ON: NoteState = ON, LastSched = ON, lastSched command has been executed");
          #endif
        }else if(lastScheduledAt < TD - mySettings.DEACTIVATION_DURATION - mySettings.ACTIVATION_DURATION){
          //there is sufficient time for the note to activate and deactivate
          note.scheduleOff(TD-mySettings.DEACTIVATION_DURATION); //schedule a deactivation just in time for the new activation
          note.scheduleOn(velocity,TD); //schedule the activation for the new note normally
          #ifdef SERIAL_DEBUG_SCHEDULE
            Serial.println("ON: NoteState = ON, LastSched = ON, sufficient time for note to act and deact");
          #endif
        }else{
          //there is insufficient time for note to activate and deactivate
          if(lastScheduledAt < TD - mySettings.BOUNCEBACK_DURATION){
            //there is sufficient time for a bounceback
            int oldPwm = note.getOldPWM(2); //get the old activation PWM; note that we substract 2 because the velocity phase is 2nd last command
            note.eraseCommands(3); //erase the activation command
            note.scheduleBB(oldPwm, lastScheduledAt);           
            note.scheduleOn(velocity,TD);
            #ifdef SERIAL_DEBUG_SCHEDULE
            Serial.println("ON: NoteState = ON, LastSched = ON, sufficient time for bounceback");
            #endif
          }else{
            //do nothing, let the original activation carry on
            #ifdef SERIAL_DEBUG_SCHEDULE
            Serial.println("ON: NoteState = ON, LastSched = ON, no time for bounceback");
            #endif
          }
        }
      
      }else{ //if last scheduled state is OFF
        if(lastScheduledAt > TD - mySettings.DEACTIVATION_DURATION){ //if there is insufficient time for the note to deactivate before the new note
          note.eraseCommands(1); //delete the latest deactivation command
          note.scheduleOff(TD-mySettings.DEACTIVATION_DURATION); //bring forward the deactivation JIT for the new note by creating a new deactivation command
          #ifdef SERIAL_DEBUG_SCHEDULE
            Serial.println("ON: NoteState = ON, LastSched = OFF, no time for deact");
          #endif
        }
        else{
          #ifdef SERIAL_DEBUG_SCHEDULE
            Serial.println("ON: NoteState = ON, LastSched = OFF, sufficient time for deact");
          #endif
        }
        note.scheduleOn(velocity, TD); //schedule the activation for the new note normally
      }
    }else{ //if the note is OFF
      if(lastScheduledState){ //if last scheduled state is ON
        if(lastScheduledAt < TD - mySettings.ACTIVATION_DURATION- mySettings.DEACTIVATION_DURATION){//there is sufficient time to schedule the activation and deactivation
          note.scheduleOff(TD-mySettings.DEACTIVATION_DURATION);//create a JIT deactivation command
          note.scheduleOn(velocity,TD);
          #ifdef SERIAL_DEBUG_SCHEDULE
            Serial.println("ON: NoteState = OFF, LastSched = ON, sufficient time for activ and deact");
          #endif
        }else{//there is insufficient time to schedule the activation and deactivation
          if(lastScheduledAt > TD - mySettings.BOUNCEBACK_DURATION){ //AND there is insufficient time to schedule a bounceback
            //do nothing, we leave the current activation to carry on, treating this new note as impossible to schedule
            #ifdef SERIAL_DEBUG_SCHEDULE
            Serial.println("ON: NoteState = OFF, LastSched = ON, insufficient time to schedule a bounceback");
            #endif
          }else{//there is sufficient time to schedule a bounceback
            //we go ahead and schedule a bounceback
            int oldPwm = note.getOldPWM(2); //we save out the old PWM value before we delete it
            note.eraseCommands(3); //delete the latest activation command (delete the last 3 -> Surge, Velocity, Holding)
            note.scheduleBB(oldPwm,lastScheduledAt); //schedule a bounceback command to play the note quickly at lastScheduledAt
            note.scheduleOn(velocity,TD);
            #ifdef SERIAL_DEBUG_SCHEDULE
            Serial.println("ON: NoteState = OFF, LastSched = ON, sufficient time to schedule a bounceback");
            #endif
          }
        }
      }else{//the last scheduled state is OFF 
        if(lastScheduledAt < millis()){ //if the off command has already been executed (i.e there is no further off command scheduled)
          #ifdef SERIAL_DEBUG_SCHEDULE
              Serial.println("ON: NoteState = OFF, LastSched = OFF, last scheduled cmd has already been executed");
            #endif
          //do nothing
        }else{
          if(lastScheduledAt > TD - mySettings.DEACTIVATION_DURATION){//there is insufficient time for the note to deactivate
            int oldPwm = note.getOldPWM(3); //we save out the old PWM value before we delete it
            note.eraseCommands(4); //delete the latest deactivation command AND the activation command
            note.scheduleBB(oldPwm,lastScheduledAt - mySettings.ACTIVATION_DURATION); //schedule a bounceback to replace the initial activation
            #ifdef SERIAL_DEBUG_SCHEDULE
              Serial.println("ON: NoteState = OFF, LastSched = OFF, act+deact command scheduled, no time for note to deactivate");
            #endif
          }else{
            #ifdef SERIAL_DEBUG_SCHEDULE
              Serial.println("ON: NoteState = OFF, LastSched = OFF, act+deact command scheduled, sufficient time for note to deactivate");
            #endif
          }
        }
        note.scheduleOn(velocity,TD);
      } 
    }
  }else{
    #ifdef SERIAL_DEBUG_SCHEDULE
      Serial.println("Note is not in range, no schedule!");
    #endif
  }

}
void ScheduleOff(uint8_t id, uint8_t velocity){
    if(mySettings.MIN_NOTE_ID <= id && id <= mySettings.MAX_NOTE_ID){ //check to see if the note id is within range, else do nothing
    Note &note =  *(find_if(begin(notes), end(notes), [&id](Note note) { return note.getMidiId() == id; })); //find the note by ID
    bool noteState = note.getNoteState();
    bool lastScheduledState = note.getLastScheduledState();
    unsigned long lastScheduledAt = note.getLastScheduledAt();
    unsigned long TD = millis() + mySettings.DELAY_TIME;

    #ifdef SERIAL_DEBUG_SCHEDULE
      Serial.print(millis()); Serial.print(" - ScheduleOff: ID - "); Serial.print(id); Serial.print(", Note state - "); Serial.print(noteState); Serial.print(", Last Sched State - "); Serial.print(lastScheduledState); Serial.print(", Last Sched At - "); Serial.print(lastScheduledAt); 
      Serial.print(", calc Vel - "); Serial.print(velocity); Serial.println();
    #endif
    
    if(noteState){ //if note is ON (note is being played)
      if(lastScheduledState){ //if last scheduled state is ON
        if(lastScheduledAt < millis()){
          #ifdef SERIAL_DEBUG_SCHEDULE
            Serial.println("OFF: NoteState = ON, LastSched = ON, last scheduled cmd has alr been executed");
          #endif
          note.scheduleOff(TD); //schedule a deactivation normally
        }else if(lastScheduledAt > TD - mySettings.BOUNCEBACK_DURATION){ //there is insufficient time to schedule a bounceback
          note.eraseCommands(3); //erase the last activation command; there is no time for it to happen
          #ifdef SERIAL_DEBUG_SCHEDULE
            Serial.println("OFF: NoteState = ON, LastSched = ON, insufficient time for bounceback");
          #endif
        }else{
          int oldPwm = note.getOldPWM(2); //we save out the old PWM value before we delete it
          note.eraseCommands(3);
          note.scheduleBB(oldPwm,lastScheduledAt - mySettings.ACTIVATION_DURATION); //schedule a bounceback to replace the initial activation
          #ifdef SERIAL_DEBUG_SCHEDULE
             Serial.println("OFF: NoteState = ON, LastSched = ON, sufficient time for bounceback");
          #endif          
        }


      }else{ //if last scheduled state is OFF
        #ifdef SERIAL_DEBUG_SCHEDULE
            Serial.println("OFF: NoteState = ON, LastSched = OFF, letting deactivation carry out");
        #endif
      }
    }else{ //if the note is OFF
      if(lastScheduledState){ //if last scheduled state is ON
        if(lastScheduledAt < TD - mySettings.ACTIVATION_DURATION){//there is sufficient time to schedule the activation of the note
          note.scheduleOff(TD); //schedule the deactivation normally
          #ifdef SERIAL_DEBUG_SCHEDULE
            Serial.println("OFF: NoteState = OFF, LastSched = ON, sufficient time for activation");
          #endif
        }else{ //there is insufficient time to schedule the activation
          if(lastScheduledAt < TD - mySettings.BOUNCEBACK_DURATION){//if there is sufficient time to schedule a bounceback
            int oldPwm = note.getOldPWM(2); //we save out the oldPwm value from the activation command before we delete it 
            note.eraseCommands(3); //delete the latest activation command
            note.scheduleBB(oldPwm,lastScheduledAt); //schedule a bounceback command to play the note quickly
            #ifdef SERIAL_DEBUG_SCHEDULE
              Serial.println("OFF: NoteState = OFF, LastSched = ON, sufficient time for bounceback");
            #endif
          }else{//there is insufficient time for bounceback
            note.eraseCommands(3); //delete the latest activation command 
            //we will ignore the previous activation command
            #ifdef SERIAL_DEBUG_SCHEDULE
              Serial.println("OFF: NoteState = OFF, LastSched = ON, insufficient time for bounceback");
            #endif
          }
          
        }
      }else{//the last scheduled state is OFF 
        //do nothing. We will let the deactivation follow through
        #ifdef SERIAL_DEBUG_SCHEDULE
          Serial.println("OFF: NoteState = OFF, LastSched = OFF, letting deactivation proceed");
        #endif
      }
    }
  }else{
    #ifdef SERIAL_DEBUG_SCHEDULE
      Serial.println("Note is not in range, not schedule!");
    #endif
  }
}
