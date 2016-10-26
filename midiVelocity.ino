/******************************************************************************
 Use SparkFun MIDI Shield as a on the fly MIDI velocity modifier.

 Arnaud Fenioux - afenioux.fr
 October 2016
 
 This programe will change note velocity on channel 1
 depending of the value pots (pot0 is min velocity, pot1 in max velocity)
 all others messages are transmitied as is (Through).

 Press Buton 0 to enable the velocity reduction (green led on):
  pot0 will set min velocity
  pot1 will set max velocity
  velocity received under pot0 value is set to pot0 value
  velocity received above pot1 value is set to pot1 value

 Press Buton 2 to enable the velocity reduction (red led on):
  pot0 will set min velocity
  pot1 will set max velocity
  velocity is now reduced as a linear function : 
  velocity received with a value of 1 is set to pot0 value
  velocity received with a value of 127 is set to pot1 value
 
  This code is dependent on the FortySevenEffects MIDI library for Arduino.
  https://github.com/FortySevenEffects/arduino_midi_library
  You'll need to install that library into the Arduino IDE before compiling.
  This code is more than inspired of repo :
  https://github.com/sparkfun/MIDI_Shield/
  
Development environment specifics:
  It was developed for the Arduino Uno compatible SparkFun RedBoard, with a  SparkFun
  MIDI Shield.

Distributed as-is; no warranty is given.
******************************************************************************/
#include <MIDI.h>

//MIDI_CREATE_INSTANCE(SoftwareSerial, SoftSerial, MIDI);
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);
MIDI_CREATE_DEFAULT_INSTANCE();

#define LED_ARD 13           // LED pin on Arduino Uno

#define BTN0 2
#define BTN1 3
#define BTN2 4

#define POT0 0
#define POT1 1

#define LED_GRN 6
#define LED_RED 7

#define DBG 0

int pot0;
int pot1;
int ticks = 0;

//TODO : using a struct would be a plus
boolean button_state[3];
boolean led_state[14];

void BlinkLed(byte led, byte num)         // Basic blink function
{
  // I'm only using this function when debug is enable
  // because delay() imply latency and non real time execution
    for (byte i=0;i<num;i++)
    {
        digitalWrite(led, LOW);
        delay(60);
        digitalWrite(led, HIGH);
        delay(60);
    }
}

void CheckButtonPushed()                    // Return true if button is pressed
{
  // A bit of a funny statement below.
  // digitalRead return HIGH or LOW.
  // We want boolean (true/false) indicators of whether the button are
  // pushed.
  // And button inputs are active low - when a button is pushed, it'll read "LOW"
  for(int i=0; i<3; i++) {
    button_state[i] = (digitalRead(i+2) == LOW);
   }
}

void SetLight(int led, bool state)                       //Light on/off led
{
  led_state[led] = state;
  //if state is true, set light up
  if (state)  digitalWrite(led, LOW);
  else        digitalWrite(led, HIGH);
}

void ToggleLight(int led)                    //Light on/off led
{
  //TODO This function needs to be tested
  if (led_state[led])  digitalWrite(led, LOW);
  else                 digitalWrite(led, HIGH);
  led_state[led] = !led_state[led];
}

void ButtonAction()                       //Light led if button was pressed
{
  // Light on the green led if BTN0 is pressed
  if (button_state[0]) {    
    SetLight(LED_GRN, true);
    //And light off red led
    SetLight(LED_RED, false);
  }
  // Light on the red led if BTN2 is pressed
  if (button_state[2]) {    
    SetLight(LED_RED, true);
    //And light off green led
    SetLight(LED_GRN, false);
  }
  // Middle button BTN1 clears the selection
  if (button_state[1]) {    
    SetLight(LED_GRN, false);
    SetLight(LED_RED, false);
  }
}

void MidiSendAsIs()                 //Send MIDI to out and blink once if debug is enabled
{
    if (DBG) BlinkLed(LED_RED, 1);
    
    MIDI.send(MIDI.getType(),
              MIDI.getData1(),
              MIDI.getData2(),
              MIDI.getChannel());
}

byte SetVelocityMinMax(byte velocity) //Adjust Velocity Min on POT0 and Velocity Max on POT1
{
    if (DBG) BlinkLed(LED_GRN, 2);
  
    // velocity need to be between 0 and 127 -> /2
    int velocityMin = pot0/2;
    int velocityMax = pot1/2;
    int velocityModified = velocity;
    //set velocityMin to velocityMax when velocityMin > velocityMax
    if ( velocityMin > velocityMax ) velocityMin = velocityMax;
    if ( velocity < velocityMin ) velocityModified = velocityMin;
    if ( velocity > velocityMax ) velocityModified = velocityMax;
    if ( velocityModified > 127 ) velocityModified = 127;
    if ( velocityModified < 1   ) velocityModified = 1;
    
    return (byte)velocityModified;
}


byte SetVelocityRangeOff(byte velocity) //Adjust Velocity Range on POT0 and Velocity Offset on POT1
{
    if (DBG) BlinkLed(LED_RED, 1);
    if (DBG) BlinkLed(LED_GRN, 1);
  
    // velocity need to be between 0 and 127 -> /2
    int range  = pot0/2;
    int offset = pot1/2;
    
    // we avoid a "saturation"
    //the priority is to keep the velocity's range
    if (offset > 127-range)
      offset = 127-range;
      
    // Mathematical functin is a*x+b
    // Where :  a is range/127.0
    //          x is velocity
    //          b is offset
    float velocityModified = (range/127.0) * velocity + offset;
    if ( velocityModified > 127) velocityModified = 127;
    if ( velocityModified < 1) velocityModified = 1;
    
    return (byte)velocityModified;
}


void handleNoteOn(byte channel, byte pitch, byte velocity)
{
    // Do whatever you want when a note is pressed.
    // Try to keep your callbacks short (no delays etc)
    // otherwise it would slow down the loop() and have a bad impact
    // on real-time performance.
    
    if (MIDI.getChannel() == 1)
    {
      // Analog inputs have an LSB (out of 10 bits) or so noise, 
      // leading to "chatter" in the change detector logic.
      // Shifting off the 2 LSBs to remove it
      // value is now between 0 and 255
      pot0 = analogRead(POT0) >> 2;
      pot1 = analogRead(POT1) >> 2;
    
      byte velocityModified = velocity;
      
      //if green led is lighted use Min/Max mode
      if (!led_state[LED_RED] && led_state[LED_GRN])
        velocityModified = SetVelocityMinMax(velocity);
      //if red led is lighted use Range/Offset mode
      else if (led_state[LED_RED] && !led_state[LED_GRN])
        velocityModified = SetVelocityRangeOff(velocity);
      
      MIDI.sendNoteOn(pitch,velocityModified,channel);
      if (DBG) BlinkLed(LED_GRN, 1);
    }
    else
      MIDI.sendNoteOn(pitch,velocity,channel);
}

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_ARD, OUTPUT);

  pinMode(BTN0, INPUT_PULLUP);
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);

  pinMode(LED_GRN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  digitalWrite(LED_GRN, HIGH);
  digitalWrite(LED_RED, HIGH);

  for (byte i=0;i<3;i++)
  {
    BlinkLed(LED_GRN, 1);
    BlinkLed(LED_RED, 1);
  }
    
  // Connect the handleNoteOn function to the library,
  // so it is called upon reception of a NoteOn.
  MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function
  
  // We want to receive messages on all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);  
  //MIDI.begin(1);          // Launch MIDI and listen to channel 1
  
  MIDI.turnThruOff();    
  //MIDI.turnThruOn();
}

void loop() {
  // put your main code here, to run repeatedly:
  
  CheckButtonPushed();
  ButtonAction();
    
 if (MIDI.read())                // Is there a MIDI message incoming ?
  { 
    switch (MIDI.getType())
      {
        case midi::NoteOn :
          {
            // Do nothing (handler is set)
          }
          break;
          
        case midi::Clock :
          {
            ticks++;
            if (ticks == 3)
              SetLight(LED_ARD, true);
            else if (ticks == 24) {
              ticks = 0;
              SetLight(LED_ARD, false);
            }
          }
          break;

        default:
          {
            //send stuff
            MidiSendAsIs();
          }
          break;
      }
  }
}
