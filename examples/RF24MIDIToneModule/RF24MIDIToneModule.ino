/*
// Simple DIY Electronic Music Projects
//    diyelectromusic.wordpress.com
//
//  RF24 MIDI Tone Module
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
     Arduino Tone Melody: https://www.arduino.cc/en/Tutorial/BuiltInExamples/toneMelody
     RF24MIDI Transport: https://github.com/diyelectromusic/RF24MIDI
*/
#include <MIDI.h>
#include <RF24MIDI.h>
#include "pitches.h"

#define MIDI_CHANNEL  1     // MIDI Receive channel (1-16)

// The RF24 base receive address is the same for each receiving node, but
// the actual address used depends on the MIDI channel for this demo.
//
// This is because the transmitter/relay is acting in "broadcast" mode where
// it will be sending out on consecutive addresses, starting with the "base"
// address above.
//
// Note the "OUT" address here isn't actually used, but configured for completeness.
//
#define RF24MIDIINADDR  0x31
#define RF24MIDIOUTADDR 0x30
RF24MIDI_CREATE_INSTANCE(RF24MIDIINADDR+MIDI_CHANNEL-1, RF24MIDIOUTADDR, RF24MIDI);

// NB: Cannot use LED_BUILTIN as it clashes with tbe RF24 Radio on the SPI pins
//#define MIDI_LED 4

void ledInit() {
#ifdef MIDI_LED
   pinMode(MIDI_LED, OUTPUT);
   digitalWrite(MIDI_LED, LOW);
#endif
}

int ledCount;
void ledOn() {
#ifdef MIDI_LED
   digitalWrite(MIDI_LED, HIGH);
#endif
}

void ledOff() {
#ifdef MIDI_LED
   digitalWrite(MIDI_LED, LOW);
#endif
}


// Attach a speaker to one of the Arduino digital pins.
// See: https://www.arduino.cc/en/Tutorial/BuiltInExamples/toneMelody
#define SPEAKER 8

// Set up the MIDI codes to respond to by listing the lowest note
#define MIDI_NOTE_START 23   // B0

// Set the notes to be played by each key
int notes[] = {
  NOTE_B0,
  NOTE_C1, NOTE_CS1, NOTE_D1, NOTE_DS1, NOTE_E1, NOTE_F1, NOTE_FS1, NOTE_G1, NOTE_GS1, NOTE_A1, NOTE_AS1, NOTE_B1,
  NOTE_C2, NOTE_CS2, NOTE_D2, NOTE_DS2, NOTE_E2, NOTE_F2, NOTE_FS2, NOTE_G2, NOTE_GS2, NOTE_A2, NOTE_AS2, NOTE_B2,
  NOTE_C3, NOTE_CS3, NOTE_D3, NOTE_DS3, NOTE_E3, NOTE_F3, NOTE_FS3, NOTE_G3, NOTE_GS3, NOTE_A3, NOTE_AS3, NOTE_B3,
  NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4,
  NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5,
  NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6,
  NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7,
  NOTE_C8, NOTE_CS8, NOTE_D8, NOTE_DS8
};
int numnotes;

// These are the functions to be called on recieving certain MIDI events.
// Full documentation of how to do this is provided here:
// https://github.com/FortySevenEffects/arduino_midi_library/wiki/Using-Callbacks

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  if (velocity == 0) {
    // Handle this as a "note off" event
    handleNoteOff(channel, pitch, velocity);
    return;
  }

  if ((pitch < MIDI_NOTE_START) || (pitch >= MIDI_NOTE_START+numnotes)) {
    // The note is out of range for us so do nothing
    return;
  }

  ledOn();
  tone (SPEAKER, notes[pitch-MIDI_NOTE_START]);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  // There is no special handling here.
  // If we receive a note off event just turn off all notes.
  noTone(SPEAKER);
  ledOff();
}

void setup() {
  ledInit();
  //Serial.begin(9600);
  
  // Set up the functions to be called on specific MIDI events
  RF24MIDI.setHandleNoteOn(handleNoteOn);
  RF24MIDI.setHandleNoteOff(handleNoteOff);

  // This listens to the specified MIDI channel
  RF24MIDI.begin(MIDI_CHANNEL);

  numnotes = sizeof(notes)/sizeof(notes[0]);
}

void loop() {
  // All the loop has to do is call the MIDI read function
  RF24MIDI.read();
}
