#define KEYBOARD 1
#define MIDI 0
                          
#if KEYBOARD == 1
#include "Keyboard.h"
#include "Keyboard_de_DE.h"
#endif

#if MIDI == 1
#include "MIDIUSB.h"
#endif

static const char pins[] = {2,   // Switch 0 Pin
                            3,   // Switch 1 Pin
                            4,   // Switch 2 Pin
                            5,   // Switch 3 Pin
                            6,   // Switch 4 Pin
                            7,   // Switch 5 Pin
                            8,   // Switch 6 Pin
                            9,   // Switch 7 Pin
                            10,  // Switch 8 Pin
                            11}; // Switch 9 Pin

static const char keys[] = {0,                 // Switch 0 Key
                            0,                 // Switch 1 Key
                            0,                 // Switch 2 Key
                            KEY_UP_ARROW,      // Switch 3 Key
                            KEY_DOWN_ARROW,    // Switch 4 Key
                            0,                 // Switch 5 Key
                            0,                 // Switch 6 Key
                            '+',               // Switch 7 Key
                            '-',               // Switch 8 Key
                            0};                // Switch 9 Key

#if MIDI == 1
// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}
#endif

char switchstate[10];

void setup() {
  char count;
  // set pins to input and enable internal pullup; init switch state
  for (count=0; count < sizeof(pins); count++)
  {
    pinMode(pins[count], INPUT_PULLUP);
    switchstate[count] == 0;
  }
  #if KEYBOARD == 1
  Keyboard.begin(KeyboardLayout_de_DE);
  #endif
}

void loop() {
  char count, currentswitchstate;
  for (count=0; count < sizeof(pins); count++)
  {
    currentswitchstate = digitalRead(pins[count]);
    if ((currentswitchstate == 0) and (switchstate[count] == 1))
    {
      // Switch pressed!
      if (keys[count] != 0)
      {
        Keyboard.press(keys[count]);
      }
    }
    else if ((currentswitchstate == 1) and (switchstate[count] == 0))
    {
      // Switch released!
      if (keys[count] != 0)
      {
        Keyboard.release(keys[count]);
      }
    }
    // save state of switch
    switchstate[count] = currentswitchstate;
  }
  delay(100);
}
