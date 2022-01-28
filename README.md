# Arduino RF24 MIDI Transport
RF24 MIDI Transport for use with the Arduino MIDI Library and nrf24L01 radios.

This library implements a direct RF24 MIDI transport layer for the [FortySevenEffects Arduino MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library).

*Warning: This is a very draft implementation.  Use at your own risk!*

## Installation

This library depends on the [Arduino MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library) and the [Arduino RF24 Library](https://github.com/nRF24/RF24).

## Usage
### Basic / Default

MIDI RF24 Transmitter

```cpp
// MIDI Transmit over RF24
#include <MIDI.h>
#include <RF24MIDI.h>

#define RF24INADDR   42
#define RF24OUTADDR  43
RF24MIDI_CREATE_INSTANCE(RF24INADDR, RF24OUTADDR, RF24MIDI);

void setup()
{
   RF24MIDI.begin();
...

void loop()
{
   RF24MIDI.SendNoteOn(60, 127, 1);
...
```
MIDI RF24 Receiver

```cpp
// MIDI Receive over RF24
#include <MIDI.h>
#include <RF24MIDI.h>

#define RF24INADDR   43
#define RF24OUTADDR  42
RF24MIDI_CREATE_INSTANCE(RF24INADDR, RF24OUTADDR, RF24MIDI);

void setup()
{
   RF24MIDI.begin(1);
...

void loop()
{
   RF24MIDI.read();
   ...
```
These will create a instance named `RF24MIDI` using a RF24 exchange to a device at RF24 address 43, using MIDI channel 1.


### Provided Example 1

The included examples demonstrate a transmitter (MIDI OUT) and a receiver (MIDI IN).  This requires two Arduinos both connected up to nrf24L01 radios via SPI.  A simple loudspeaker can be connected to pin 8 of the receiver to hear the tones being played by the transmitter.  Either way the results will be printed out to the serial monitor.

For more details, refer to the following blog post: [Arduino RF24 MIDI Interface]()

### Provided Example 2

A second example provides code to show how an Arduino can act as a two-way Serial MIDI to RF24 MIDI relay.

For more details on this, once again refer to the above mentioned blog post: [Arduino RF24 MIDI Interface]()

Kevin
@diyelectromusic
