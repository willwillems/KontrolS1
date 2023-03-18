
#include <SPI.h>
#include <Encoder.h>

#include "unit_c.h"

#define debug 0

#define latchPinIn 9 // 74HC165
#define latchPinOut 8 // 74HC595
#define cePin 10 // 74HC165 & 74HC595

#define faderPin 19

#define touchPin 14

#define FX_PINS 15, 16, 17, 18
#define FX_PINS_CC 10, 11, 12, 13

// Channel number for MIDI messagess
const int channelNumber = 1;

// IC ammount for serial data
const int ica595 = 7;
const int ica165 = 4;

const byte ledsOffState[ica595] = { LEDS_SERIAL_OFF_STATE };
byte ledsState[ica595] = { LEDS_SERIAL_OFF_STATE };

const byte buttonsOffState[ica165] = { BUTTONS_SERIAL_OFF_STATE };
byte buttonsState[ica165] = { BUTTONS_SERIAL_OFF_STATE };

const byte digitStates[22] = {
  DIGIT_STATE_1_32,
  DIGIT_STATE_1_16,
  DIGIT_STATE_1_8,
  DIGIT_STATE_1_4,
  DIGIT_STATE_1_2,
  DIGIT_STATE_1,
  DIGIT_STATE_2,
  DIGIT_STATE_4,
  DIGIT_STATE_8,
  DIGIT_STATE_16,
  DIGIT_STATE_32
};

//const int ledMap[64] = {
//  1,
//  2,
//  3,
//  4,
//  5,6,7,8,9,
//  (4*8+4 - 1), // 9
//  11,
//  (4*8+5 - 1), // 11
//  13,
//  (4*8+6 - 1) // 13
//  , 15,
//  (4*8+7 - 1) // 15
//};

const int ledMap[64] = {
  (FX_CHIP_NR*8 + 0), // 0,
  (FX_CHIP_NR*8 + 1), // 1,
  (FX_CHIP_NR*8 + 2), // 2,
  (FX_CHIP_NR*8 + 3), // 3,
  (FX_CHIP_NR*8 + 4), // 4,
  6,7,8, // not used on FX chip
  
  (-1),  // 8
  (CUP_CHIP_NR*8 + 7), // 9 - SHIFT
  (-1), // 10
  (CUP_CHIP_NR*8 + 4), // 11 - SYNC
  (-1), // 12
  (CUP_CHIP_NR*8 + 5), // 13 - CUE
  (-1), // 13
  (CUP_CHIP_NR*8 + 6), // 15 - PLAY
  
  (26), // 16 - C
  (CUP_CHIP_NR*8 + 0), // 17 - S1
  (24), // 18 - LOOP IN
  (CUP_CHIP_NR*8 + 1), // 19 - S2
  (25), // 20 - LOOP OUT
  (CUP_CHIP_NR*8 + 2), // 21 - S3
  (27), // 22 - LOAD
  (CUP_CHIP_NR*8 + 3), // 23 - S4
  
  (LP_CHIP_NR*8 + 8 - 1), // 24
  (LP_CHIP_NR*8 + 8 - 2), // 25
  (LP_CHIP_NR*8 + 8 - 3), // 26
  (LP_CHIP_NR*8 + 8 - 4), // 27
  (LP_CHIP_NR*8 + 8 - 5), // 28
  (LP_CHIP_NR*8 + 8 - 6), // 29
  (LP_CHIP_NR*8 + 8 - 7), // 30
  (LP_CHIP_NR*8 + 8 - 8), // 31
};

const int buttonMap[32] = {
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,

  (8 + 16), // OFFSRT UP NOY MAPPED
  (9 + 16), // OFFSET DOWN NOT MAPPED
  (10 + 16), // SET LOOP
  (11 + 16), // ACTIVE LOOP
  (12 + 16), // empty ??
  (13 + 16), // empty ??
  (14 + 16), // empty ??
  (15 + 16), // empty ??

  (16 + 0), // C
  (17 + 0), // S1
  (18 + 0), // LOOP IN
  (19 + 0), // S2
  (20 + 0), // LOOP OUT
  (21 + 0), // S3
  (22 + 0), // LOAD
  (23 + 0), // S4
  
  (24 - 16), // CUE1
  (25 - 16), // SHIFT
  (26 - 16), // CUE2
  (27 - 16), // SYNC
  (28 - 16), // CUE3
  (29 - 16), // CUE
  (30 - 16), // CUE4
  (31 - 16) // PLAY
};
// 13 CUE

//const int buttonMap[32] = {
//  0,
//  1,
//  2,
//  3,
//  4,
//  5,
//  6,
//  7,
//
//  8,
//  9,
//  10,
//  11,
//  12,
//  13,
//  14,
//  15,
//
//  16,
//  17,
//  18,
//  19,
//  20,
//  21,
//  22,
//  23,
//  
//  24,
//  25,
//  26,
//  27,
//  28,
//  29,
//  30,
//  31
//};

// LED OFF STATES from least significant bit (last one so notation is in reverse)
// -- BYTE [5]: 0b01010101 --
// CUE1 green - CUE1 blue - CUE2 green - CUE2 blue - CUE3 green - CUE3 blue - CUE4 green - CUE4 blue 
// -- BYTE [4]: 0b00000000 --
// Sample 1 - Sample 2 - Sample 3 - Sample 4 - SYNC - CUE - PLAY - SHIFT
// -- BYTE [3]: 0b11000000 --
// ON AIR - SAMPLES - DECK A/B - DECK C/D - KEYLOCK - MASTER - TEMPO OFFSET UP - TEMPO OFFSET DOWN
// -- BYTE [2]: 0b00000000 --
// LOOP IN - LOOP OUT - TOGGLE DECK C/D - LOAD - NONE - NONE - NONE -NONE
// -- BYTE [1]: 0b11111111 -- LEFT DIGIT 
// UP-MIDDLE - UP-RIGHT - DOT - UP-LEFT - DOWN-LEFT - DONW-MIDDLE - MIDDLE-MIDDLE - DOWN-RIGHT
// -- BYTE [0]: 0b11111111 -- RIGHT DIGIT
// DM - DL - UL - UM - UR - DR - MM - DOT

Encoder encoderLeft(2, 3);
Encoder encoderRight(4, 5);

Encoder encoderJog(6, 7);

const int fxPins[4] = {FX_PINS};
const int fxPinsCC[4] = {FX_PINS_CC};
int fxPinsState[4] = {64, 64, 64, 64};

int faderState = 0;

// depends on resistor value 
const int jogPressThresh = 980;
uint jogPressWindow = 0b00000000000000000000000000000000;
bool jogPressState = false;

void setup () {
  // setup SPI
  SPI.begin ();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  // set latch pins as output
  pinMode(latchPinIn, OUTPUT);
  pinMode(latchPinOut, OUTPUT);
  
  pinMode(cePin, OUTPUT);
  
  // set fader pin as input
  pinMode(faderPin, INPUT);
  // set jog touch as input
  pinMode(touchPin, INPUT);
  // set fx pins as input
  for(int i = 0; i < 4; i++) {
    pinMode(fxPins[i], INPUT);
  }
  // set latch pins to initial state
  digitalWrite (latchPinIn, HIGH);
  digitalWrite (latchPinOut, HIGH);
  digitalWrite (cePin, HIGH);
  // setup handlers for USB MIDI
  usbMIDI.setHandleNoteOn(handleNoteOn);
  usbMIDI.setHandleNoteOff(handleNoteOff);
  usbMIDI.setHandleControlChange(handleControlChange);
  // setup led state and start-up animation
  clearLedsState();
  
  for (int i = 0; i < (ica595*8); i++) {
    // flip bit at position
    setLedState(i, 1);
    sendLedsState();
    delay(20);
  }

  clearLedsState();
  setDigitState(8); // DEBUG
  sendLedsState();
}

void loop () {
  readButtonState();
  readEncoders();
  readTempoFader();
  readFXKnobs();
  readJogPress();
  // read USB MIDI
  usbMIDI.read();
}

void readButtonState () {
  byte newButtonsState[ica165];
  // pulse the parallel load latch
  digitalWrite (latchPinIn, LOW);
  delayMicroseconds(5); // doesn't break when we remove this
  digitalWrite (latchPinIn, HIGH);
  delayMicroseconds(5); // doesn't break when we remove this

  digitalWrite (cePin, LOW);
  
  // get new button state
  for (int i = 0; i < ica165; i++) {
    newButtonsState[i] = SPI.transfer(0b00000000);
  }

  digitalWrite (cePin, HIGH);
  
  // compare to previous button state and send midi messages
  for (int i = 0; i < (ica165 * 8); i++) {
    const int arrayEl = i/8;
    const int byteEl = i%8;
    const bool offState = bitRead(buttonsOffState[arrayEl], byteEl);
    const bool oldState = bitRead(buttonsState[arrayEl], byteEl);
    const bool newState = bitRead(newButtonsState[arrayEl], byteEl);
    if (newState != oldState) {
      if (newState ? !offState : offState) {
        usbMIDI.sendNoteOn(buttonMap[i], 127, channelNumber);
      } else {
        usbMIDI.sendNoteOff(buttonMap[i], 127, channelNumber);
      }
      
      if (debug) {
        Serial.print("Button nr: ");
        Serial.print(i);
        Serial.print(" Map: ");
        Serial.print(buttonMap[i]);
        Serial.print(" now: ");
        Serial.println(newState ? !offState : offState);
      }
    }
  }

  for (int i = 0; i < ica165; i++) {
    buttonsState[i] = newButtonsState[i];
  }
}

void sendLedsState () {
  // set latch pin 595 to fill registers with state
  delayMicroseconds(5);
  digitalWrite (latchPinOut, LOW);
  delayMicroseconds(5);
  // delay(1); // not needed, or can be mS, EDIT: needed for C unit
  // send state
  for (int i = 0; i < ica595; i++) {
    SPI.transfer(ledsState[i]);
  }
  Serial.println("\n");
  // load state into outputs
  digitalWrite (latchPinOut, HIGH);
}

void setLedState (int ledNumber, bool newState) {
  if (ledNumber < 0) {
    if (debug) { Serial.println("Led number < 0"); }
    return;
  }
  int arrayEl = ledNumber/8;
  int byteEl  = ledNumber%8;
  bool offState = bitRead(ledsOffState[arrayEl], byteEl);
  bitWrite(ledsState[arrayEl], byteEl, (newState ? !offState : offState));

  if (debug) {
    Serial.print("Led nr: ");
    Serial.print(ledNumber);
    Serial.print(" State: ");
    Serial.println(newState);
  }
}

void setDigitState (int newState) {
  byte leftDigit = digitStates[(newState * 2)];
  byte rightDigit = digitStates[(newState * 2) + 1];
  ledsState[LDG_CHIP_NR] = leftDigit;
  ledsState[RDG_CHIP_NR] = rightDigit;

  if (debug) {
    Serial.print("Left digit: ");
    Serial.print(leftDigit, BIN);
    Serial.print(" right digit: ");
    Serial.println(rightDigit, BIN);
  }
}

void clearLedsState () {
  memcpy(ledsState, ledsOffState, sizeof ledsState);
}

void readEncoders () {
  int encoderLeftPosition = (encoderLeft.read() / 4);
  if (encoderLeftPosition != 0) {
    Serial.print("Encoder left position: ");
    Serial.println(encoderLeftPosition);
    // send encoder midi note
    if (encoderLeftPosition > 0) { usbMIDI.sendControlChange(0, 63, channelNumber); }
    else { usbMIDI.sendControlChange(0, 65, channelNumber); }
    // reset enncoder
    encoderLeft.write(0);
  }
  // read and proccess right encoder
  int encoderRightPosition = (encoderRight.read() / 4);
  if (encoderRightPosition != 0) {
    Serial.print("Encoder right position: ");
    Serial.println(encoderRightPosition);
    // send encoder midi note
    if (encoderRightPosition > 0) { usbMIDI.sendControlChange(1, 63, channelNumber); }
    else { usbMIDI.sendControlChange(1, 65, channelNumber); }
    // reset enncoder
    encoderRight.write(0);
  }
  // read and proccess jog encoder
  int encoderJogPosition = (encoderJog.read() / 4); // only does -4 and 4
  if (encoderJogPosition != 0) {
    Serial.print("Encoder jog position: ");
    Serial.println(encoderJogPosition);
    // send encoder midi note55
    if (encoderJogPosition > 0) { usbMIDI.sendControlChange(2, 63, channelNumber); }
    else { usbMIDI.sendControlChange(2, 65, channelNumber); }
    // reset enncoder
    encoderJog.write(0);
  }
}

void readTempoFader () {
  int newFaderState = 127 - (analogRead(faderPin) / 8); // inverted
  if (newFaderState != faderState) {
    faderState = newFaderState;
    usbMIDI.sendControlChange(5, faderState, channelNumber);
  }
}

void readFXKnobs () {
  for (int i = 0; i < 4; i++) {
    int newKnobState = (analogRead(fxPins[i]) / 8);
    if (newKnobState != fxPinsState[i]) {
      fxPinsState[i] = newKnobState;
      usbMIDI.sendControlChange(fxPinsCC[i], fxPinsState[i], channelNumber);
    }
  }
}

void readJogPress () {
  int jogRead = analogRead(touchPin);
  jogPressWindow = jogPressWindow << 1; // shift bit
  bitWrite(jogPressWindow, 0, jogRead <= jogPressThresh); // write bit

  Serial.print(jogRead);
  Serial.print("\t");
  Serial.println((jogRead <= jogPressThresh) ? 100 : 0);

  if (jogPressState == HIGH && jogPressWindow == 0b00000000000000000000000000000000) {
    jogPressState = LOW;
    usbMIDI.sendControlChange(55, jogPressState ? 127 : 0, channelNumber);
  } else if (jogPressState == LOW && jogPressWindow == 0b11111111111111111111111111111111){
    jogPressState = HIGH;
    usbMIDI.sendControlChange(55, jogPressState ? 127 : 0, channelNumber);
  }
}

void handleNoteOn(byte channel, byte note, byte velocity) {
  if (channel != channelNumber) { return; } 
  int led = getNoteLed(note, velocity, true);
  setLedState(led, 1);
  sendLedsState();
  if (debug) {
    Serial.print("Note On, ch=");
    Serial.print(channel, DEC);
    Serial.print(", note=");
    Serial.print(note, DEC);
    Serial.print(", led=");
    Serial.print(led, DEC);
    Serial.print(", velocity=");
    Serial.println(velocity, DEC);
  }
}

void handleNoteOff(byte channel, byte note, byte velocity) {
  if (channel != channelNumber) { return; }
  int led = getNoteLed(note, velocity, false);
  setLedState(led, 0);
  sendLedsState();
  if (debug) {
    Serial.print("Note Off, ch=");
    Serial.print(channel, DEC);
    Serial.print(", note=");
    Serial.print(note, DEC);
    Serial.print(", velocity=");
    Serial.println(velocity, DEC);
  }
}

void handleControlChange(byte channel, byte control, byte value) {
  if (channel != channelNumber) { return; }
  if (control == 0) { 
    setDigitState(value);
    sendLedsState(); 
  }
  if (debug) {
    Serial.print("Control Change, ch=");
    Serial.print(channel, DEC);
    Serial.print(", control=");
    Serial.print(control, DEC);
    Serial.print(", value=");
    Serial.println(value, DEC);
  }
}

int getNoteLed(int note, int velocity, bool on) {
  if (note == 8 || note == 10 || note == 12 || note == 14) {
    Serial.print("CUE");
    Serial.print(note - 8);
    Serial.println(" ");
    // Serial.print((velocity == 6) ? 0 : 1);
    if (true) {
      // Loop
      if      (velocity == 6) {
        setLedState((CUE_CHIP_NR*8) + (note - 8) + 0, 1);
        setLedState((CUE_CHIP_NR*8) + (note - 8) + 1, 0);
      }
      // Load 
      else if (velocity == 4) { 
        setLedState((CUE_CHIP_NR*8) + (note - 8) + 0, 1); 
        setLedState((CUE_CHIP_NR*8) + (note - 8) + 1, 1); 
      }
      else if (velocity == 0) {
        setLedState((CUE_CHIP_NR*8) + (note - 8) + 0, 0);
        setLedState((CUE_CHIP_NR*8) + (note - 8) + 1, 0);
      }
      // Cue or anything else
      else {
        setLedState((CUE_CHIP_NR*8) + (note - 8) + 0, 0);
        setLedState((CUE_CHIP_NR*8) + (note - 8) + 1, 1);
      }
    }
    sendLedsState();
    return -1;
  }
  return ledMap[note];
}
