// setup input mode
// 0 = pin mode (one key per hw pin)
// 1 = Adafruit NeoTrellis Keypad support
#define PINMODE 0
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
// define max elemets for keyconfig status arrays
#define MAXELEMENTS 30

#if PINMODE == 1
#include "Adafruit_NeoTrellis.h"
Adafruit_NeoTrellis trellis;
#define INT_PIN 5
#endif

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

/* Config for NeoTrellis LED buttons (only usable in PINMODE 1)
 *  Button { idle 0xRRGGBB, pressed 0xRRGGBB}
 *  Colors: 0..255
 */

static uint32_t buttonled[][2] = {{0x7F0000, 0x000000}, // button  1
                                  {0x007F00, 0x00FF00}, // button  2
                                  {0x00007F, 0x0000FF}, // button  3
                                  {0x000000, 0x0000FF}, // button  4
                                  {0x000000, 0x0000FF}, // button  5
                                  {0x000000, 0x0000FF}, // button  6
                                  {0x000000, 0x0000FF}, // button  7
                                  {0x000000, 0x0000FF}, // button  8
                                  {0x000000, 0x0000FF}, // button  9
                                  {0x000000, 0x0000FF}, // button 10
                                  {0x000000, 0x0000FF}, // button 11
                                  {0x000000, 0x0000FF}, // button 12
                                  {0x000000, 0x0000FF}, // button 13
                                  {0x000000, 0x0000FF}, // button 14
                                  {0x000000, 0x0000FF}, // button 15
                                  {0x000000, 0xFFFFFF}  // button 16
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

#ifdef DEBUG
void print_debug(char keyconfignum, char state) {
  Serial.print("Key ");
  if (state == 'P')
  {
    Serial.print("pressed");
  } else if (state == 'R')
  {
    Serial.print("released");
  }
  Serial.print(" Pin: ");
  Serial.print(keyconfig[keyconfignum][0], DEC);
  Serial.print(" Type: ");
  Serial.print(keyconfig[keyconfignum][1]);
  switch(keyconfig[keyconfignum][1])
  {
    case 'K': Serial.print(" Keyboard V1: "); break;
    case 'M': Serial.print(" MIDI USB Note: Channel: "); break;
    case 'm': Serial.print(" MIDI Note: Channel: "); break;
    case 'C': Serial.print(" MIDI USB Control: Channel: "); break;
    case 'c': Serial.print(" MIDI Control: Channel: "); break;
  }
  Serial.print(keyconfig[keyconfignum][2], DEC);
  switch(keyconfig[keyconfignum][1])
  {
    case 'K': Serial.print(" V2: "); break;
    case 'M': Serial.print(" Pitch: "); break;
    case 'm': Serial.print(" Pitch: "); break;
    case 'C': Serial.print(" Control: "); break;
    case 'c': Serial.print(" Control: "); break;
  }
  Serial.print(keyconfig[keyconfignum][3], DEC);
  switch(keyconfig[keyconfignum][1])
  {
    case 'K': Serial.print(" V3: "); break;
    case 'M': Serial.print(" Velocity: "); break;
    case 'm': Serial.print(" Velocity: "); break;
    case 'C': Serial.print(" Value: "); break;
    case 'c': Serial.print(" Value: "); break;
  }
  Serial.println(keyconfig[keyconfignum][4], DEC);
}
#endif

void keyPressed(char button) {
  char count, keycount;
  for (count=0; count < (sizeof(keyconfig)/sizeof(keyconfig[0])); count++)
  {
    if (button == keyconfig[count][0])
    {
#ifdef DEBUG
      print_debug(count, 'P');
#endif
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
        // switch MIDI USB note on. [2]=channel, [3]=pitch, [4]=velocity
        noteOnUSB(keyconfig[count][2], keyconfig[count][3], keyconfig[count][4]);
      }
      // keyconfig element [1] contains type of config (C=MIDI USB Control)
      else if (keyconfig[count][1] == 'C')
      {
        // switch MIDI USB control on. [2]=channel, [3]=control, [4]=value
        controlChangeUSB(keyconfig[count][2], keyconfig[count][3], keyconfig[count][4]);
      }
#endif
#if MIDI == 1
      // keyconfig element [1] contains type of config (m=MIDI)
      if (keyconfig[count][1] == 'm')
      {
        // switch MIDI note on. [2]=channel, [3]=pitch, [4]=velocity
        noteOn(keyconfig[count][2], keyconfig[count][3], keyconfig[count][4]);
      }
      // keyconfig element [1] contains type of config (c=MIDI Control)
      else if (keyconfig[count][1] == 'c')
      {
        // switch MIDI control on. [2]=channel, [3]=control, [4]=value
        controlChange(keyconfig[count][2], keyconfig[count][3], keyconfig[count][4]);
      }
#endif
    }
  }
}

void keyReleased(char button) {
  char count, keycount;
  for (count=0; count < (sizeof(keyconfig)/sizeof(keyconfig[0])); count++)
  {
    if (button == keyconfig[count][0])
    {
#ifdef DEBUG
      print_debug(count, 'R');
#endif
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
  }
}

int cmpfunc (const void * a, const void * b) {
  return ( *(char*)a - *(char*)b );
}

#if PINMODE == 1
//define a callback for key presses
TrellisCallback blink(keyEvent evt){
  if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
    keyPressed(evt.bit.NUM); //on rising
    trellis.pixels.setPixelColor(evt.bit.NUM, buttonled[evt.bit.NUM][1]);
  }
  else if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
    keyReleased(evt.bit.NUM); //off falling
    trellis.pixels.setPixelColor(evt.bit.NUM, buttonled[evt.bit.NUM][0]);
  }
  trellis.pixels.show();
  return 0;
}
#endif

unsigned char switchstate[MAXELEMENTS];
unsigned char inputpins[MAXELEMENTS];

void setup() {
  char count;
#ifdef DEBUG
  Serial.begin(115200);
#endif
  // set pins to input and enable internal pullup; init switch state; init inputpins
  for (count=0; count < MAXELEMENTS; count++)
  {
#if PINMODE == 0
    pinMode(keyconfig[count][0], INPUT_PULLUP);
#endif
    switchstate[count] = 1;
    inputpins[count] = 255;
  }
  for (count=0; count < (sizeof(keyconfig)/sizeof(keyconfig[0])); count++)
  {
    inputpins[count] = keyconfig[count][0];
  }
  qsort(inputpins, (sizeof(keyconfig)/sizeof(keyconfig[0])), sizeof(char), cmpfunc);
#if KEYBOARD == 1
  // initialize Keyboard and set layout to de_DE
  Keyboard.begin(KeyboardLayout_de_DE);
#endif
#if MIDI == 1
  // initialize Serial1 for MIDI, 31250 baud
  Serial1.begin(31250);
#endif
#if PINMODE == 1
  pinMode(INT_PIN, INPUT);
  if(!trellis.begin()){
#ifdef DEBUG
    Serial.println("could not start trellis");
#endif
    while(1) delay(1);
  }
#ifdef DEBUG
  else{
    Serial.println("trellis started");
  }
#endif
  //activate all keys, set callbacks and init button led
  for(int i=0; i<NEO_TRELLIS_NUM_KEYS; i++){
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING);
    trellis.registerCallback(i, blink);
    trellis.pixels.setPixelColor(i, buttonled[i][0]);
  }
  delay(10);
  trellis.pixels.show();
#endif
}

void loop() {
  char count, keycount, currentswitchstate;
  char prevpin = 0;
#if PINMODE == 0
  // loop for all configured keys
  for (count=0; count < MAXELEMENTS; count++)
  {
    if ((inputpins[count] != prevpin) && (inputpins[count] != 255))
    {
      // read current pin state. pin number is stored in element [0]
      currentswitchstate = digitalRead(inputpins[count]);
      // switch has been pressed, when current state is 0 and was 1 before
      if ((currentswitchstate == 0) and (switchstate[count] == 1))
      {
        // Switch pressed!
        keyPressed(inputpins[count]);
      }
      // switch has been released, when current state is 1 and was 0 before
      else if ((currentswitchstate == 1) and (switchstate[count] == 0))
      {
        // Switch released!
        keyReleased(inputpins[count]);
      }
      // save state of switch
      switchstate[count] = currentswitchstate;
    }
    prevpin = inputpins[count];
  }
#endif
#if PINMODE == 1
  if(!digitalRead(INT_PIN)){
    trellis.read(false);
  }
#endif
  // delay in milliseconds
  delay(LOOPDELAY);
}
