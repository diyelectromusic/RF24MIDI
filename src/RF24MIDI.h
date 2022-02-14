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

// There are two possible broadcast modes:
//     Multiple writes: reliable, but requires one transaction per receiving node
//                      and will timeout/become unreliable if a node isn't there.
//     No-auto ACK: not reliable, but for any number of nodes and a genuine radio
//                  broadcast "on-air".
//
// The multiple-writes method is the only method reliable enough
// not to be missing messages. The no-ACK method is not recommended.
//
// But if you have an application where you don't care if a node misses
// messages (e.g. for some kind of streaming/real-time control broadcast)
// then the no-ACK method might have some utility.
//
//#define NOACK_BROADCAST 1

class RF24MIDI
{

public:
   RF24MIDI(int in_addr, int out_addr, int broadcast=0, int pin_ce=10, int pin_csn=9)
   {
      mPinCE = pin_ce;
      mPinCSN = pin_csn;
      mRF24 = RF24(mPinCE, mPinCSN);
      mRF24ok = false;

      // Highest address is 250, but will also need consecutive
      // addresses if we are in "broadcast" mode
      if ((in_addr < 1) || (in_addr > 250+broadcast)) {
         in_addr = 42;
      }
      if ((out_addr < 1) || (out_addr > 250+broadcast)) {
         out_addr = 44;
      }
      // RF24 Address is "MIDIn" where n = addr (1 to 250)
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

#ifdef NOACK_BROADCAST
      // If the addresses are the same, then assume we are working
      // in broadcast/multicast mode.  This is less reliable and requires
      // that the auto-acknowledgement of packets is disabled otherwise
      // things won't be working properly as several nodes will try to ACK
      // messages.
      //
      // Note: this will almost certainly end up lossing messages though,
      //       so this mode is not recommended unless your MIDI system
      //       doesn't care (e.g. you are just streaming CC messages or
      //       something).
      //
      // Also note that this is configured by having the IN and OUT
      // addresses the same. In this mode the "broadcast" parameter
      // mentioned above is ignored.
      //
      if (in_addr == out_addr) {
         mRF24Broadcast = 1;
          
      } else {
         mRF24Broadcast = 0;
      }
#else
      // Using "multiple writes" as a pseudo broadcast mechanism.
      // This will use consecutive addresses after the "out" addresses.
      //
      // Broadcast mode does not need to be set for the receiver, only
      // the transmitter, but there needs to be one receiver on each of
      // the "broadcast" addresses otherwise the transmitter will timeout
      // waiting for responses which introduces jitter and unreliability
      // into the message stream (timing wise).
      //
      mRF24Broadcast = broadcast;

      // Duplicate the OUT address to mess around with it later...
      for (int i=0; i<6; i++) {
         mRF24BCastAddr[i] = mRF24OutAddr[i];
      }
#endif
   };

public:
   static const bool thruActivated = false;
    
   void begin()
   {
#ifdef DBG
      Serial.print("RF24MIDI: Begin: In Address=");
      Serial.print((char *)mRF24InAddr);
      Serial.print("\tOut Address=");
      Serial.print((char *)mRF24OutAddr);
      Serial.print("\tBroadcasts=");
      Serial.print(mRF24Broadcast);
      Serial.print("\tPins=(");
      Serial.print(mPinCE);
      Serial.print(",");
      Serial.print(mPinCSN);
      Serial.print(")\n");
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
      
      // Open a writing pipe for transmission.
      mRF24.openWritingPipe(mRF24OutAddr);
      
      // Setup "pipe 1" (of the six, 0 to 5 pipes) for reading.
      //
      // Note: "pipe 0" is special and is used for acknowledgements for the transmissions.
      //       For full details, see:
      //       http://maniacalbits.blogspot.com/2013/04/rf24-avoiding-rx-pipe-0-for-enhanced.html
      //
      //       "Pipe 0 is special because, whenever you transmit, RX pipe 0 is changed
      //       to that of the writing pipe's address. The fact this occurs is obscured,
      //       by design, by the RF24 driver. This is because the RF24 driver has
      //       specific logic for pipe 0."
      //
      //       This means there is the possibility of losing ACKs or data if the pipe
      //       was being used as a "normal" receiving pipe too.
      //
      // Also, from the comments for the write() function in RF24.h:
      //
      //       "This function will overwrite the address set to reading pipe 0
      //       as stipulated by the datasheet for proper auto-ack functionality in TX
      //       mode."
      //
      // For this reason pipe 0 is not used for receiving.
      //
      mRF24.openReadingPipe(1, mRF24InAddr);
#ifdef NOACK_BROADCAST
      if (mRF24Broadcast != 0) {
          // Turn off auto acknowledgements if in broadcast/multicast mode
          mRF24.setAutoAck(0);
      }
#else
      if (mRF24Broadcast != 0) {
#ifdef DBG
          Serial.print("RF24MIDI: Begin: BCasts: ");
          for (int i=0; i<mRF24Broadcast; i++) {
              mRF24BCastAddr[4] = mRF24OutAddr[4]+1+i;
              Serial.print((char *)mRF24BCastAddr);
              Serial.print(" ");
          }
          Serial.print("\n");
#endif
      }
#endif
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
#ifdef NOACK_BROADCAST
      // Nothing else to do for no-ACK broadcast mode.
      // We just keep our fingers crossed that the message makes it...
#else
      if (mRF24Broadcast != 0) {
          // Send to all other nodes on consecutive addresses too
          for (int i=0; i<mRF24Broadcast; i++) {
              mRF24BCastAddr[4] = mRF24OutAddr[4]+1+i;
              mRF24.openWritingPipe(mRF24BCastAddr);
              mRF24.write(&value, 1);
          }
          // Reset back to original OUT pipe
          mRF24.openWritingPipe(mRF24OutAddr);
        }
#endif
#ifdef DBG
      mAvCnt++; if (mAvCnt > 16) {mAvCnt=0; Serial.print("\n"); }
      Serial.print(" ");
      if (res) {
         if (value < 16) {  Serial.print("0"); }
         Serial.print(value, HEX);
      } else {
         Serial.print ("xx");
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
   int mRF24Broadcast;
   uint8_t mRF24InAddr[6];
   uint8_t mRF24OutAddr[6];
   uint8_t mRF24BCastAddr[6];
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

#define RF24MIDI_CREATE_BROADCAST_INSTANCE(RF24InAddr, RF24OutAddr, RF24Broadcast, Name)  \
   RF24MIDI_NAMESPACE::RF24MIDI RF24##Name(RF24InAddr, RF24OutAddr, RF24Broadcast);\
   MIDI_NAMESPACE::MidiInterface<RF24MIDI_NAMESPACE::RF24MIDI> Name((RF24MIDI_NAMESPACE::RF24MIDI&)RF24##Name);

END_RF24MIDI_NAMESPACE
