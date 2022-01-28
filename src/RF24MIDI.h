/*!
 *  @file       RF24MIDI.h
 *  @license    MIT - Copyright (c) 2022 Kevin (@diyelectromusic)
 *  @author     Kevin, Francois Best
 *  @date       01/25/22
 *
 *  Based on:
 *  @file       serialMIDI.h
 *  Project     Arduino MIDI Library
 *  @brief      MIDI Library for the Arduino - Platform
 *  @license    MIT - Copyright (c) 2015 Francois Best
 *  @author     lathoub, Francois Best
 *  @date       22/03/20
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once
#include <MIDI.h>
#include <RF24.h>
using namespace MIDI_NAMESPACE;

#include "RF24MIDI_Namespace.h"

BEGIN_RF24MIDI_NAMESPACE

// Only supports up to three byte messages at present
#define RF24MIDIPAYLOAD 3  // Allows radio to optimise a little rather than send a whole RF buffer at a time

//#define DBG 1

class RF24MIDI
{

public:
   RF24MIDI(int in_addr, int out_addr, int pin_ce=10, int pin_csn=9, bool debug=false)
   {
      mPinCE = pin_ce;
      mPinCSN = pin_csn;
      mRF24 = RF24(mPinCE, mPinCSN);
      mRF24ok = false;

      if ((in_addr < 1) || (in_addr > 255)) {
         in_addr = 42;
      }
      if ((out_addr < 1) || (out_addr > 255)) {
         out_addr = 44;
      }
      // RF24 Address is "MIDIn" where n = addr (1 to 255)
      mRF24InAddr[0] = 'M';
      mRF24InAddr[1] = 'I';
      mRF24InAddr[2] = 'D';
      mRF24InAddr[3] = 'I';
      mRF24InAddr[4] = in_addr;
      mRF24InAddr[5] = 0;
      mRF24OutAddr[0] = 'M';
      mRF24OutAddr[1] = 'I';
      mRF24OutAddr[2] = 'D';
      mRF24OutAddr[3] = 'I';
      mRF24OutAddr[4] = out_addr;
      mRF24OutAddr[5] = 0;
   };

public:
   static const bool thruActivated = false;
    
   void begin()
   {
#ifdef DBG
      Serial.print("RF24MIDI: Begin: In Address=");
      Serial.print((char *)mRF24InAddr);
      Serial.print("\tOut Address=");
      Serial.println((char *)mRF24OutAddr);
#endif
      if (!mRF24.begin()) {
#ifdef DBG
         Serial.print("Failed to talk to radio hardware!\n");
#endif
         return;
      }
      mRF24ok = true;
      mRF24.setPALevel(RF24_PA_LOW);
      mRF24.setPayloadSize(RF24MIDIPAYLOAD);
      mRF24.openWritingPipe(mRF24OutAddr);
      mRF24.openReadingPipe(1, mRF24InAddr); // Use "pipe 1" for reading
      mRF24.startListening();
   }

   bool beginTransmission(MidiType)
   {
      return true;
   };

   void write(byte value)
   {
      if (!mRF24ok) {
         return;
      }
      mRF24.stopListening();
      bool res = mRF24.write(&value, 1);
#ifdef DBG
      mAvCnt++; if (mAvCnt >= 16) {mAvCnt=0; Serial.print("\n"); }
      Serial.print(" ");
      if (res) {
         if (value < 16) {  Serial.print("0"); }
         Serial.print(value, HEX);
      } else {
         Serial.print ("(F)");
      }
#endif
      mRF24.startListening();
   };

   void endTransmission()
   {
   };

   byte read()
   {
      if (!mRF24ok) {
         return 0;
      }
      uint8_t val;
      if (mRF24.available()) {
         mRF24.read(&val, 1);
#ifdef DBG
         Serial.print("\nRF24MIDI: Read: Value=0x");
         Serial.println(val, HEX);
#endif
         return (byte)val;
      }
   }

   unsigned available()
   {
      if (!mRF24ok) {
         return 0;
      }
      unsigned av = mRF24.available();
#ifdef DBG
      mAvCnt++; if (mAvCnt >= 64) {mAvCnt=0; Serial.print(".");}
#endif
      return av;
   };

private:
   bool mRF24ok;
   uint8_t mRF24InAddr[6];
   uint8_t mRF24OutAddr[6];
   int mPinCE;
   int mPinCSN;
   RF24 mRF24;
   int mAvCnt;
};

/*! \brief Create an instance of the library attached to an nrf24L01.
 Example: MIDI_CREATE_INSTANCE(RF24InADDR, RF24OutADDR, rf24midi);
 Then call rf24midi.begin(), rf24midi.read() etc..
 */
#define RF24MIDI_CREATE_INSTANCE(RF24InAddr, RF24OutAddr, Name)  \
   RF24MIDI_NAMESPACE::RF24MIDI RF24##Name(RF24InAddr, RF24OutAddr);\
   MIDI_NAMESPACE::MidiInterface<RF24MIDI_NAMESPACE::RF24MIDI> Name((RF24MIDI_NAMESPACE::RF24MIDI&)RF24##Name);

END_RF24MIDI_NAMESPACE
