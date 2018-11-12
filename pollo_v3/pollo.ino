#include <MIDI.h>

#define SERIAL_RATE 115200
#define BUTTON_PIN 2
#define LED_PIN 13

#define CC             0xB0
#define NOTE_OFF       0x80
#define NOTE_ON        0x90

int state = HIGH;      // the current state of the output pin
int reading;           // the current reading from the input pin
int previous = LOW;    // the previous reading from the input pin

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

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
}

const byte note = 0x1F; // G1

void loop() {

  reading = digitalRead(BUTTON_PIN);
  if (reading == HIGH && previous == LOW) {
    if (state == HIGH){
       state = LOW;
       // noteOff(0, note, 0);
       MIDI.sendNoteOff(note, 0, 1);
     } else {
       state = HIGH;
       // noteOn(0, note, 127);
       MIDI.sendNoteOn(note, 127, 1);

     }
  }

  digitalWrite(LED_PIN, state);
  previous = reading;
  delay(1);
}

void noteOn(byte channel, byte key, byte intencity) {
  Serial.write(NOTE_ON|channel);
  Serial.write(key);
  Serial.write(intencity);
}

void noteOff(byte channel, byte key, byte intencity) {
  Serial.write(NOTE_OFF|channel);
  Serial.write(key);
  Serial.write(intencity);
}

void changeControl(byte channel, byte key, byte intencity) {
  Serial.write(CC|channel);
  Serial.write(key);
  Serial.write(intencity);
}