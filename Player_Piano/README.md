# Player-Piano
THIS IS A WORK IN PROGRESS

**WARNING**
Make sure you properly adjust the settings under ``Settings.h`` to suit your needs: 

Specifically, edit these lines:
**These lines pertain to the voltage that will be supplied to your solenoids**. Failure to calculate appropriate values could see your solenoids **destroyed**.

```
  const int HOLD_PWM = 1023; //this must be the solenoid's nominal voltage, 6V
  const int ON_PWM = 4095; //this supplies the pulse of high voltage that allows the solenoid to move quickly.
  
  //MIN_PWM and MAX_PWM are for mapping the velocity of the notes to PWM for the VELOCITY PHASE. As such the maximum PWM
  //should be calculated based off a reasonable voltage for the solenoid.
  const int MIN_PWM = 0;
  const int MAX_PWM = 2570; //corresponds to 15V
  
  ... as well as...
  
  const int BB_ON_PWM = 100; //Pulse PWM for BounceBack
  const int BB_STARTUP_DURATION = 15; //Duration of Pulse PWM for BounceBack
  const int BB_VELOCITY_DURATION = 25; //Duration of Velocity for BounceBack
  const int BB_HOLD_DURATION = 10; //Hold Duration for Bounceback (can be 0)
  const int BB_TOTAL_DURATION = BB_STARTUP_DURATION + BB_VELOCITY_DURATION + BB_HOLD_DURATION;
  
  ```
  and these lines pertain to the "lookahead" duration (essentially how far into the future the code will look to make amendments to existing scheduling). A larger lookahead = better "response" to curveballs but also a more noticeable lag. 
  Also bear in mind that DELAY_TIME should be 1.5x larger than ACTIVATION_DURATION + DEACTIVATION_DURATION (this is because the code should naturally look further than the immediate time it takes to activate and deactivate a note.
  ```
  //derived empirically
  const int ACTIVATION_DURATION = 500; //the time taken for the note to become fully activated (this can be thought of as the
  //actual time it takes for the note to fully depress. Changing this value will affect the CRITERIA for when BounceBack / rescheduling
  //is implemented but not the actual implementation of these features themselves.
  const int DEACTIVATION_DURATION = 500; //similar to ACTIVATION_DURATION, the time taken for a note to become fully deactivated
  //Does not affect how BounceBack / rescheduling is implemented but instead WHEN these are implemented. 
  const int BOUNCEBACK_DURATION = 100; //time taken for note to complete a bounce back cycle
  
  const int NOTE_TIMEOUT = 10000; //After x amount of time, any note left on will be auto turned off
  const uint8_t MAX_NOTES = 10; //maximum number of notes that can be played at any point in time
  ```
  **Note: These values are set to ridiculously huge numbers for testing**. For actual implementation the empirically-derived values will be a lot smaller. 
  
  Wherever and whenever possible, I have made detailed annotations and comments to assist anyone in understanding my code. 
  
  This code has **NOT** been bench-tested so please bear that in mind when using it. 
  
  Thank you!
  
