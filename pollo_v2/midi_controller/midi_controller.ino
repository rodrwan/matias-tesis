// #include <MIDI.h>
#include "MIDIUSB.h"

#include "MUX74HC4067.h"

#define SERIAL_RATE 9600

#define MUX_NUM_CHANNELS 16
#define MUX_BUTTONS 9
#define MUX_POTS 5

#define MUX_ANALOG_COM A3
#define MUX_DIGITAL_COM 3

#define NOTE_OFF       0x80
#define NOTE_ON        0x90
#define CC             0xB0

bool TOGGLE_BUTTONS[MUX_BUTTONS];

int newAnalog[MUX_POTS];
int oldAnalog[MUX_POTS];
// s0 s1 s2 s3
MUX74HC4067 mux(7, 8, 9, 10, 11);

// Created and binds the MIDI interface to the default hardware Serial port
// MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
  Serial.begin(SERIAL_RATE);
  while (!Serial);

  // setup 16-multiplexer as digital reader on pin 3.
  mux.signalPin(3, INPUT, DIGITAL);
  // MIDI.begin(MIDI_CHANNEL_OFF);
}

void loop() {
  // read digital multiplexer
  for(byte muxChan = 0; muxChan < MUX_NUM_CHANNELS; ++muxChan) {
    digital16Mux(muxChan);
  }

  // read analog multiplexer
  // for(byte muxChan = 0; muxChan < MUX_NUM_CHANNELS; ++muxChan) {
  //   analog16Mux(muxChan);
  // }
}

void digital16Mux(int muxChan) {
  if (muxChan >= 0 && muxChan <= 8) {
    byte data = mux.read(muxChan);
    //  Button is pressed
    if (data == 0) {
      if (!TOGGLE_BUTTONS[muxChan]) {
        // control, value, channel
        // MIDI.sendControlChange(data, 0, muxChan+1); // off
        noteOn(0, muxChan+1, 0);
        MidiUSB.flush();
        TOGGLE_BUTTONS[muxChan] = true;
      } else {
        // MIDI.sendControlChange(data, 127, muxChan+1); // on
        TOGGLE_BUTTONS[muxChan] = false;
        noteOff(0, muxChan+1, 127);
        MidiUSB.flush();
      }
    }
  }
}

void analog16Mux(int muxChan) {
  if (muxChan >= 0 && muxChan <= 5) {
    // check if pot value has changed
    if (newAnalog[muxChan] != oldAnalog[muxChan]) {
      byte data = mux.read(muxChan);
      newAnalog[muxChan] = data
      uint8_t intensity = (uint8_t) (map(data, 0, 1023, 0, 127));

      // MIDI.sendControlChange(60, intensity, muxChan+1);
      noteOn(0, muxChan+1, intensity);
      oldAnalog[muxChan] = newAnalog[muxChan]
    }
  }
}

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, NOTE_ON | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, NOTE_OFF | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}
// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).
void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, CC | channel, control, value};
  MidiUSB.sendMIDI(event);
}