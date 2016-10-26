/******************************************************************************
# midiVelocity
 Use SparkFun MIDI Shield as a on the fly MIDI velocity modifier.

 Arnaud Fenioux - afenioux.fr
 October 2016
 
 This programe will change note velocity on channel 1
 depending on the values of the pots (pot0 is min velocity, pot1 in max velocity)
 all others messages are transmitied as is (Through).

 Press Buton 0 to enable the Min/Max velocity reduction (green led on):
  pot0 will set min velocity,
  pot1 will set max velocity.
  
  velocity received under pot0 value is set to pot0 value,
  velocity received above pot1 value is set to pot1 value.

 Press Buton 2 to enable the Range/Offset velocity reduction (red led on):
  pot0 will set velocity range (default is 127, can be reducte down to 1),
  pot1 will set the offset (if pot0 is set to 1 and port1 to 64, velocity returned will be 65).

  velocity is now reduced as a linear function : 
  velocity received with a value of 1 is set to pot1 value,
  velocity received with a value of 127 is set to pot1 + pot0 value.
 
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
