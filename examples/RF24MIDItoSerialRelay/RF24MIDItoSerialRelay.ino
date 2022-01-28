/*
// Simple DIY Electronic Music Projects
//    diyelectromusic.wordpress.com
//
//  RF24MIDI to Serial MIDI Relay
//  https://diyelectromusic.wordpress.com/2022/01/09/arduino-i2c-midi-interface-part-3/
//
      MIT License
      
      Copyright (c) 2022 diyelectromusic (Kevin)
      
      Permission is hereby granted, free of charge, to any person obtaining a copy of
      this software and associated documentation files (the "Software"), to deal in
      the Software without restriction, including without limitation the rights to
      use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
      the Software, and to permit persons to whom the Software is furnished to do so,
      subject to the following conditions:
      
      The above copyright notice and this permission notice shall be included in all
      copies or substantial portions of the Software.
      
      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
      IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
      FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
      COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHERIN
      AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
      WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*
  Using principles from the following:
     Arduino MIDI Library: https://github.com/FortySevenEffects/arduino_midi_library
     RF24 Library: https://github.com/nRF24/RF24
*/
#include <MIDI.h>
#include <RF24MIDI.h>

#define RF24MIDIINADDR  49
#define RF24MIDIOUTADDR 50
RF24MIDI_CREATE_INSTANCE(RF24MIDIINADDR, RF24MIDIOUTADDR, RF24MIDI);

// Initialise the serial MIDI
MIDI_CREATE_DEFAULT_INSTANCE();

#define MIDI_LED LED_BUILTIN

void setup() {
   pinMode(MIDI_LED, OUTPUT);
   MIDI.begin(MIDI_CHANNEL_OMNI);
   MIDI.turnThruOff();
   RF24MIDI.begin(MIDI_CHANNEL_OMNI);
   RF24MIDI.turnThruOff();
}

int ledCount;
void ledOn() {
   digitalWrite(MIDI_LED, HIGH);
   ledCount = 1000;
}

void ledOff() {
  if (ledCount > 0) {
     ledCount--;
  } else if (ledCount == 0) {
     digitalWrite(MIDI_LED, LOW);
     ledCount = -1;
  }
}

void loop() {

  if (MIDI.read())
  {
     ledOn();
     RF24MIDI.send(MIDI.getType(),
                   MIDI.getData1(),
                   MIDI.getData2(),
                   MIDI.getChannel());
  }
  if (RF24MIDI.read())
  {
     ledOn();
     MIDI.send(RF24MIDI.getType(),
               RF24MIDI.getData1(),
               RF24MIDI.getData2(),
               RF24MIDI.getChannel());
  }
  ledOff();
}
