/*
// Simple DIY Electronic Music Projects
//    diyelectromusic.wordpress.com
//
//  RF24MIDI MIDI Broadcast Relay
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

// This setup will "broadcast" all MIDI messages to consecutively numbered
// receivers, starting at RF24MIDIOUTADDR and finishing at RF24MIDIOUTADDR+RFMIDIBROADCAST-1.
//
// So for RF24MIDIBROADCAST = 3 and RF24MIDIOUTADDR = 0x31, this will result
// in transmitting to the following RF24 addresses:
//    0x31, 0x32, 0x33, 0x34
//
// If any of the nodes are not physically present, then the comms will stutter
// as the transmitter will end up timing out and retrying transmissions.
//
// Receiver nodes don't need to be configured for "broadcast" mode, they can
// just be set up as normal, just using individual IN addresses from the above range.
//
#define RF24MIDIBROADCAST 3
#define RF24MIDIINADDR    0x30
#define RF24MIDIOUTADDR   0x31
RF24MIDI_CREATE_BROADCAST_INSTANCE(RF24MIDIINADDR, RF24MIDIOUTADDR, RF24MIDIBROADCAST, RF24MIDI);

// Initialise the serial MIDI
MIDI_CREATE_DEFAULT_INSTANCE();

// NB: Cannot use LED_BUILTIN as it clashes with tbe RF24 Radio on the SPI pins
//#define MIDI_LED 4

void ledInit() {
#ifdef MIDI_LED
   pinMode(MIDI_LED, OUTPUT);
#endif
}

int ledCount;
void ledOn() {
#ifdef MIDI_LED
   digitalWrite(MIDI_LED, HIGH);
   ledCount = 1000;
#endif
}

void ledOff() {
#ifdef MIDI_LED
  if (ledCount > 0) {
     ledCount--;
  } else if (ledCount == 0) {
     digitalWrite(MIDI_LED, LOW);
     ledCount = -1;
  }
#endif
}

void setup() {
   ledInit();
   MIDI.begin(MIDI_CHANNEL_OMNI);
   MIDI.turnThruOff();
   RF24MIDI.begin(MIDI_CHANNEL_OMNI);
   RF24MIDI.turnThruOff();
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
  ledOff();
}