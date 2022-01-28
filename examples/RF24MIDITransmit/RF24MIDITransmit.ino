/*
// Simple DIY Electronic Music Projects
//    diyelectromusic.wordpress.com
//
//  RF24MIDI Transmit
//  https://diyelectromusic.wordpress.com/
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

#define MIDI_CHANNEL 1
#define MIDI_LED LED_BUILTIN

void setup() {
   Serial.begin(9600);
   Serial.print("Sending MIDI data on channel ");
   Serial.println(MIDI_CHANNEL);
   pinMode(MIDI_LED, OUTPUT);
   RF24MIDI.begin(MIDI_CHANNEL_OFF);
}

void loop() {
   for (int i=0; i<13; i++) {
      digitalWrite(MIDI_LED, HIGH);
      RF24MIDI.sendNoteOn(60+i, 127, MIDI_CHANNEL);
      delay(100);
      digitalWrite(MIDI_LED, LOW);
      RF24MIDI.sendNoteOff(60+i, 0, MIDI_CHANNEL);
      delay(100);
   }
   delay(500);
   for (int i=12; i>=0; i--) {
      digitalWrite(MIDI_LED, HIGH);
      RF24MIDI.sendNoteOn(60+i, 127, MIDI_CHANNEL);
      delay(30);
      digitalWrite(MIDI_LED, LOW);
      RF24MIDI.sendNoteOff(60+i, 0, MIDI_CHANNEL);
   }
   delay(500);
}
