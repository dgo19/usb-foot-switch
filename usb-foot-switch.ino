// Keyboard support (enabled=1, disabled=0)
#define KEYBOARD 1
// MIDI USB support (enabled=1, disabled=0)
#define MIDIUSB 1
// MIDI support (enabled=1, disabled=0)
#define MIDI 1
// define delay in global loop
#define LOOPDELAY 100
// send MIDI Control Value 0 on release
#define MIDICRELEASE 0
// uncomment to activate serial debug output
//#define DEBUG

#if KEYBOARD == 1
#include "Keyboard.h"
// set keyboard layout to de_DE
#include "Keyboard_de_DE.h"
#endif

#if MIDIUSB == 1
#include "MIDIUSB.h"
#endif

/* Config for keys. Pins can be configured multiple times, to press multiple keys or midi commands
 *  Keyboard { Pin number, 'K', 'Key1', 'Key2', 'Key3' }
 *    set emtpy key fields to 0
 *  or
 *  MIDI { Pin number, 'M', MIDI channel, MIDI pitch, MIDI velocity
 */

static char keyconfig[][5] = {{2, 'K', KEY_UP_ARROW, 0, 0},   // Switch Pin 5 Keyboard, press KEY_UP_ARROW
                              {3, 'K', KEY_DOWN_ARROW, 0, 0}, // Switch Pin 6 Keyboard, press KEY_DOWN_ARROW
                              {4, 'K', '+', 0, 0},            // Switch Pin 9 Keyboard, press +
                              {5, 'K', '-', 0, 0},            // Switch Pin 10 Keyboard, press -
                              {6, 'M', 0, 48, 64},            // Switch Pin 11 MIDI USB Note Channel 1, middle C, normal velocity
                              {7, 'C', 1, 20, 127},           // Switch Pin 12 MIDI USB Control Channel 2, Control 20, Value 127
                              {6, 'm', 0, 48, 64},            // Switch Pin 11 MIDI Note Channel 1, middle C, normal velocity
                              {7, 'c', 1, 20, 127}            // Switch Pin 12 MIDI Control Channel 2, Control 20, Value 127
                             };

#if MIDIUSB == 1
// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOnUSB(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
}

void noteOffUSB(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChangeUSB(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}
#endif

#if MIDI == 1
// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  Serial1.write(0x09);
  Serial1.write(0x90 | channel);
  Serial1.write(pitch);
  Serial1.write(velocity);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  Serial1.write(0x08);
  Serial1.write(0x80 | channel);
  Serial1.write(pitch);
  Serial1.write(velocity);
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  Serial1.write(0x0B);
  Serial1.write(0xB0 | channel);
  Serial1.write(control);
  Serial1.write(value);
}
#endif

char switchstate[30];

void setup() {
  char count;
#ifdef DEBUG
  Serial.begin(115200);
#endif
  // set pins to input and enable internal pullup; init switch state
  for (count=0; count < (sizeof(keyconfig)/sizeof(keyconfig[0])); count++)
  {
    pinMode(keyconfig[count][0], INPUT_PULLUP);
    switchstate[count] == 1;
  }
#if KEYBOARD == 1
  // initialize Keyboard and set layout to de_DE
  Keyboard.begin(KeyboardLayout_de_DE);
#endif
#if MIDI == 1
  // initialize Serial1 for MIDI, 31250 baud
  Serial1.begin(31250);
#endif
}

void loop() {
  char count, keycount, currentswitchstate;
  // loop for all configured keys
  for (count=0; count < (sizeof(keyconfig)/sizeof(keyconfig[0])); count++)
  {
    // read current pin state. pin number is stored in element [0]
    currentswitchstate = digitalRead(keyconfig[count][0]);
    // switch has been pressed, when current state is 0 and was 1 before
    if ((currentswitchstate == 0) and (switchstate[count] == 1))
    {
      // Switch pressed!
#ifdef DEBUG
      Serial.print("Pin: ");
      Serial.print(keyconfig[count][0], DEC);
      Serial.print(" Type: ");
      Serial.print(keyconfig[count][1]);
      Serial.print(" V1: ");
      Serial.print(keyconfig[count][2], DEC);
      Serial.print(" V2: ");
      Serial.print(keyconfig[count][3], DEC);
      Serial.print(" V3: ");
      Serial.println(keyconfig[count][4], DEC);
#endif
#if KEYBOARD == 1
      // keyconfig element [1] contains type of config (K=Keyboard)
      if (keyconfig[count][1] == 'K')
      {
        // loop for 3 keys per pin (start at pin 2)
        for (keycount = 2; keycount < sizeof(keyconfig[count]); keycount++)
        {
          // key is configured, when its not 0
          if (keyconfig[count][keycount] != 0)
          {
#ifdef DEBUG
            Serial.print("Keyboard: ");
            Serial.println(keyconfig[count][keycount], DEC);
#endif
            // press the key on the keyboard
            Keyboard.press(keyconfig[count][keycount]);
          }
        }
      }
#endif
#if MIDIUSB == 1
      // keyconfig element [1] contains type of config (M=MIDI USB)
      if (keyconfig[count][1] == 'M')
      {
#ifdef DEBUG
        Serial.print("MIDI USB Note: ");
        Serial.print(" Channel: ");
        Serial.print(keyconfig[count][2], DEC);
        Serial.print(" Pitch: ");
        Serial.print(keyconfig[count][3], DEC);
        Serial.print(" Velocity: ");
        Serial.println(keyconfig[count][4], DEC);
#endif
        // switch MIDI USB note on. [2]=channel, [3]=pitch, [4]=velocity
        noteOnUSB(keyconfig[count][2], keyconfig[count][3], keyconfig[count][4]);
      }
      // keyconfig element [1] contains type of config (C=MIDI USB Control)
      else if (keyconfig[count][1] == 'C')
      {
#ifdef DEBUG
        Serial.print("MIDI USB Control: ");
        Serial.print(" Channel: ");
        Serial.print(keyconfig[count][2], DEC);
        Serial.print(" Control: ");
        Serial.print(keyconfig[count][3], DEC);
        Serial.print(" Value: ");
        Serial.println(keyconfig[count][4], DEC);
#endif
        // switch MIDI USB control on. [2]=channel, [3]=control, [4]=value
        controlChangeUSB(keyconfig[count][2], keyconfig[count][3], keyconfig[count][4]);
      }
#endif
#if MIDI == 1
      // keyconfig element [1] contains type of config (m=MIDI)
      if (keyconfig[count][1] == 'm')
      {
#ifdef DEBUG
        Serial.print("MIDI Note: ");
        Serial.print(" Channel: ");
        Serial.print(keyconfig[count][2], DEC);
        Serial.print(" Pitch: ");
        Serial.print(keyconfig[count][3], DEC);
        Serial.print(" Velocity: ");
        Serial.println(keyconfig[count][4], DEC);
#endif
        // switch MIDI note on. [2]=channel, [3]=pitch, [4]=velocity
        noteOn(keyconfig[count][2], keyconfig[count][3], keyconfig[count][4]);
      }
      // keyconfig element [1] contains type of config (c=MIDI Control)
      else if (keyconfig[count][1] == 'c')
      {
#ifdef DEBUG
        Serial.print("MIDI Control: ");
        Serial.print(" Channel: ");
        Serial.print(keyconfig[count][2], DEC);
        Serial.print(" Control: ");
        Serial.print(keyconfig[count][3], DEC);
        Serial.print(" Value: ");
        Serial.println(keyconfig[count][4], DEC);
#endif
        // switch MIDI control on. [2]=channel, [3]=control, [4]=value
        controlChange(keyconfig[count][2], keyconfig[count][3], keyconfig[count][4]);
      }
#endif
    }
    // switch has been released, when current state is 1 and was 0 before
    else if ((currentswitchstate == 1) and (switchstate[count] == 0))
    {
      // Switch released!
#if KEYBOARD == 1
      // keyconfig element [1] contains type of config (K=Keyboard)
      if (keyconfig[count][1] == 'K')
      {
        // loop for 3 keys per pin (start at element 2)
        for (keycount = 2; keycount < sizeof(keyconfig[count]); keycount++)
        {
          // key is configured, when its not 0
          if (keyconfig[count][keycount] != 0)
          {
            // release the key on the keyboard
            Keyboard.release(keyconfig[count][keycount]);
          }
        }
      }
#endif
#if MIDIUSB == 1
      // keyconfig element [1] contains type of config (M=MIDIUSB)
      if (keyconfig[count][1] == 'M')
      {
#ifdef DEBUG
        Serial.print("MIDI USB Note release: ");
        Serial.print(" Channel: ");
        Serial.print(keyconfig[count][2], DEC);
        Serial.print(" Pitch: ");
        Serial.print(keyconfig[count][3], DEC);
        Serial.print(" Velocity: ");
        Serial.println(keyconfig[count][4], DEC);
#endif
        // switch MIDI USB note off. [2]=channel, [3]=pitch, [4]=velocity
        noteOffUSB(keyconfig[count][2], keyconfig[count][3], keyconfig[count][4]);
      }
#if MIDICRELEASE == 1
      // keyconfig element [1] contains type of config (C=MIDI USB Control)
      else if (keyconfig[count][1] == 'C')
      {
        // switch MIDI USB control off. [2]=channel, [3]=control, value 0=off
        controlChangeUSB(keyconfig[count][2], keyconfig[count][3], 0);
      }
#endif
#endif
#if MIDI == 1
      // keyconfig element [1] contains type of config (m=MIDI)
      if (keyconfig[count][1] == 'm')
      {
#ifdef DEBUG
        Serial.print("MIDI Note release: ");
        Serial.print(" Channel: ");
        Serial.print(keyconfig[count][2], DEC);
        Serial.print(" Pitch: ");
        Serial.print(keyconfig[count][3], DEC);
        Serial.print(" Velocity: ");
        Serial.println(keyconfig[count][4], DEC);
#endif
        // switch MIDI note off. [2]=channel, [3]=pitch, [4]=velocity
        noteOff(keyconfig[count][2], keyconfig[count][3], keyconfig[count][4]);
      }
#if MIDICRELEASE == 1
      // keyconfig element [1] contains type of config (c=MIDI Control)
      else if (keyconfig[count][1] == 'c')
      {
        // switch MIDI USB control off. [2]=channel, [3]=control, value 0=off
        controlChange(keyconfig[count][2], keyconfig[count][3], 0);
      }
#endif
#endif
    }
    // save state of switch
    switchstate[count] = currentswitchstate;
  }
  // delay in milliseconds
  delay(LOOPDELAY);
}
