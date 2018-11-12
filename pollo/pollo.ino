#include "MIDI.h"

#include "MUX74HC4067.h"

#define SERIAL_RATE 9600

#define MUX_NUM_CHANNELS 16
#define MUX_BUTTONS 9
#define MUX_POTS 5

#define JOYSTICK_X A5
#define JOYSTICK_Y A4

bool TOGGLE_BUTTONS[MUX_BUTTONS];
// en s0 s1 s2 s3
MUX74HC4067 button_mux(7, 8, 9, 10, 11);
MUX74HC4067 encoder_mux(2, 3, 4, 5, 6);

#if defined(USBCON)
#include <midi_UsbTransport.h>

static const unsigned sUsbTransportBufferSize = 16;
typedef midi::UsbTransport<sUsbTransportBufferSize> UsbTransport;

UsbTransport sUsbTransport;

MIDI_CREATE_INSTANCE(UsbTransport, sUsbTransport, MIDI);

#else // No USB available, fallback to Serial
MIDI_CREATE_DEFAULT_INSTANCE();
#endif

void setup() {
  Serial.begin(SERIAL_RATE);
  while (!Serial);

  // setup 16-multiplexer as digital reader.
  button_mux.signalPin(12, INPUT, DIGITAL);
  // setup 16-multiplexer as analog reader.
  encoder_mux.signalPin(A1, INPUT, ANALOG);

  MIDI.begin();
}

void loop() {
  // read digital multiplexer
  for (byte muxChan = 0; muxChan < MUX_NUM_CHANNELS; ++muxChan) {
    digital16Mux(muxChan);
  }
  // read analog multiplexer
  for (byte muxChan = 0; muxChan < MUX_NUM_CHANNELS; ++muxChan) {
    analog16Mux(muxChan);
  }

  // read joystick
  readJoystick();
  delay(100);
}

void digital16Mux(int muxChan) {
  if (muxChan >= 0 && muxChan <= 8) {
    byte data = button_mux.read(muxChan);
    //  Button is pressed
    if (data == HIGH) {
      playMIDINote(muxChan + 60, data, muxChan + 1);
    }
  }
}

void printMuxData(char mux[10], int muxChan, int data) {
  Serial.print(mux);
  Serial.print(" Mux ");
  Serial.print(muxChan);
  Serial.print(" data: ");
  Serial.println(data);
}

void analog16Mux(int muxChan) {
  int data = encoder_mux.read(muxChan);
  byte mapping = map(data, 0, 1023, 0, 127);
  if (muxChan == 0) {
    playMIDINote(20, mapping, 20);
  } else if (muxChan == 1) {
    playMIDINote(21, mapping, 21);
  } else if (muxChan == 2) {
    playMIDINote(22, mapping, 22);
  } else if (muxChan == 3) {
    playMIDINote(23, mapping, 23);
  } else if (muxChan == 4) {
    playMIDINote(24, mapping, 24);
  }
}

void readJoystick() {
  int value1 = analogRead(JOYSTICK_X);
  int value2 = analogRead(JOYSTICK_Y);

  byte mapx = map(value1, 0, 1023, 0, 127);
  byte mapy = map(value2, 0, 1023, 0, 127);

  playMIDINote(25, mapx, 25);
  playMIDINote(26, mapy, 26);
}


void playMIDINote(byte channel, byte note, byte velocity)
{
    //MIDI channels 1-16 are really 0-15
    byte noteOnStatus = 0x90;

    //Transmit a Note-On message
    Serial.write(noteOnStatus);
    Serial.write(note);
    Serial.write(velocity);
} 
