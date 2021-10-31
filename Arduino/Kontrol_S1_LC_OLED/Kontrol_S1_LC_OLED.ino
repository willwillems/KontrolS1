#include <SPI.h>
#include <Encoder.h>

#include "unit_c.h"

// #define DEBUG

// #define SCREEN

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
unsigned short buttonReadings[ica165 * 8];

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

// Mappes note numbers to LED's
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
  
  (LP_CHIP_NR*8 + 2),  // 16      // load (formula might be bs)
  (CUP_CHIP_NR*8 + 0), // 17 - S1
  (-1), // 18
  (CUP_CHIP_NR*8 + 1), // 19 - S2
  (-1), // 20
  (CUP_CHIP_NR*8 + 2), // 21 - S3
  (LP_CHIP_NR*8 + 3),  // 22      // C button (formula might be bs)
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

const int buttonMap[32] = { BUTTON_MAP };

Encoder encoderLeft(2, 3);
Encoder encoderRight(4, 5);

Encoder encoderJog(6, 7);

const int fxPins[4] = {FX_PINS};
const int fxPinsCC[4] = {FX_PINS_CC};
int fxPinsState[4] = {64, 64, 64, 64};

int faderState = 0;

// depends on resistor value 
const int jogPressThresh = 525;
bool jogPressState = false;

void setup () {
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  #ifdef SCREEN
  screenSetup();
  #endif
  
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

  // fill the history array (better to #define this during compile?)
  for (int i=0;i<(ica165 * 8);i++) {buttonReadings[i] = bitRead(buttonsOffState[i/8], i%8);}
  
  // setup led state and start-up animation
  clearLedsState();
  
  for (int i = 0; i < (ica595*8); i++) {
    // flip bit at position
    setLedState(i, 1);
    sendLedsState();
    delay(10);
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
  
  byte newButtonReadings[ica165];
  // pulse the parallel load latch
  digitalWrite (latchPinIn, LOW);
  delayMicroseconds(5); // doesn't break when we remove this
  digitalWrite (latchPinIn, HIGH);
  delayMicroseconds(5); // doesn't break when we remove this

  digitalWrite (cePin, LOW);
  delayMicroseconds(100);
  // delay(1); // C BREAKS ELSE (seems false)
  
  // get new button state
  for (int i = 0; i < ica165; i++) {
    newButtonReadings[i] = SPI.transfer(0b00000000);
  }

  digitalWrite (cePin, HIGH);
  
  // compare to previous button state and send midi messages
  for (int i = 0; i < (ica165 * 8); i++) {
    const int arrayEl = i/8;
    const int byteEl = i%8;
    const bool offState = bitRead(buttonsOffState[arrayEl], byteEl);
    const bool oldState = bitRead(buttonsState[arrayEl], byteEl);
    const bool newReading = bitRead(newButtonReadings[arrayEl], byteEl);

    // add new reading
    // thiss might be inverse
    if (newReading == 1 ? !offState : offState) { buttonReadings[i] = (~((~buttonReadings[i]) << 1)); }
    else { buttonReadings[i] = (buttonReadings[i] << 1); }

    // detect falling edge (too many readings?)
    if (buttonReadings[i] == 0b1000000000000000 || buttonReadings[i] == 0b0000000000000001) {
      // falling edge
      if (buttonReadings[i] == 0b1000000000000000) { usbMIDI.sendNoteOff(buttonMap[i], 127, channelNumber); }
      else if (buttonReadings[i] == 0b0000000000000001) { usbMIDI.sendNoteOn(buttonMap[i], 127, channelNumber); }
//      Serial.print(buttonReadings[i], BIN);
//      Serial.print(" ");
//      Serial.println(millis());
      #ifdef DEBUG
        Serial.print("Button nr: ");
        Serial.print(i);
        Serial.print(" Button map: ");
        Serial.print(buttonMap[i]);
        Serial.print(" now: ");
        Serial.println(buttonReadings[i] ? !offState : offState);
      #endif
    }
    
  }

  for (int i = 0; i < ica165; i++) {
    buttonsState[i] = newButtonReadings[i];
  }
}

void sendLedsState () {
  // set latch pin 595 to fill registers with state
  delayMicroseconds(5);
  digitalWrite (latchPinOut, LOW);
  delayMicroseconds(5);
  // delay(1); // not needed, or can be mS, EDIT: needed for C unit (or nah?)
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
    #ifdef DEBUG
      Serial.println("Led number < 0");
    #endif
    return;
  }
  int arrayEl = ledNumber/8;
  int byteEl  = ledNumber%8;
  bool offState = bitRead(ledsOffState[arrayEl], byteEl);
  bitWrite(ledsState[arrayEl], byteEl, (newState ? !offState : offState));

  #ifdef DEBUG
    Serial.print("Set LED state: nr: ");
    Serial.print(ledNumber);
    Serial.print(", state: ");
    Serial.println(newState);
  #endif
}

void setDigitState (int newState) {
  byte leftDigit = digitStates[(newState * 2)];
  byte rightDigit = digitStates[(newState * 2) + 1];
  ledsState[LDG_CHIP_NR] = leftDigit;
  ledsState[RDG_CHIP_NR] = rightDigit;

  #ifdef DEBUG
    Serial.print("Left digit: ");
    Serial.print(leftDigit, BIN);
    Serial.print(" right digit: ");
    Serial.println(rightDigit, BIN);
  #endif
}

void clearLedsState () {
  memcpy(ledsState, ledsOffState, sizeof ledsState);
}

void readEncoders () {
  int encoderLeftPosition = (encoderLeft.read() / 4);
  if (encoderLeftPosition != 0) {
    #ifdef DEBUG
      Serial.print("Encoder left position: ");
      Serial.println(encoderLeftPosition);
    #endif
    // send encoder midi note
    if (encoderLeftPosition > 0) { usbMIDI.sendControlChange(0, 63, channelNumber); }
    else { usbMIDI.sendControlChange(0, 65, channelNumber); }
    // reset enncoder
    encoderLeft.write(0);
  }
  // read and proccess right encoder
  int encoderRightPosition = (encoderRight.read() / 4);
  if (encoderRightPosition != 0) {
    #ifdef DEBUG
      Serial.print("Encoder right position: ");
      Serial.println(encoderRightPosition);
    #endif
    // send encoder midi note
    if (encoderRightPosition > 0) { usbMIDI.sendControlChange(1, 63, channelNumber); }
    else { usbMIDI.sendControlChange(1, 65, channelNumber); }
    // reset enncoder
    encoderRight.write(0);
  }
  // read and proccess jog encoder
  int encoderJogPosition = (encoderJog.read() / 4); // only does -4 and 4
  if (encoderJogPosition != 0) {
    #ifdef DEBUG
      Serial.print("Encoder jog position: ");
      Serial.println(encoderJogPosition);
    #endif
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

//  Serial.print(jogRead);
//  Serial.print("\t");
//  Serial.println((jogRead <= jogPressThresh) ? 100 : 0);
  bool newJogPressState = (jogRead <= jogPressThresh);  // jog push
  if (newJogPressState != jogPressState) {
    jogPressState = newJogPressState;
    usbMIDI.sendControlChange(55, jogPressState ? 127 : 0, channelNumber);
  }
}

void handleNoteOn(byte channel, byte note, byte velocity) {
  if (channel != channelNumber) { return; } 
  int led = getNoteLed(note, velocity, true);
  setLedState(led, 1);
  sendLedsState();
  #ifdef DEBUG
    Serial.print("Note On, ch=");
    Serial.print(channel, DEC);
    Serial.print(", note=");
    Serial.print(note, DEC);
    Serial.print(", led=");
    Serial.print(led, DEC);
    Serial.print(", velocity=");
    Serial.println(velocity, DEC);
  #endif
}

void handleNoteOff(byte channel, byte note, byte velocity) {
  if (channel != channelNumber) { return; }
  int led = getNoteLed(note, velocity, false);
  setLedState(led, 0);
  sendLedsState();
  #ifdef DEBUG
    Serial.print("Note Off, ch=");
    Serial.print(channel, DEC);
    Serial.print(", note=");
    Serial.print(note, DEC);
    Serial.print(", velocity=");
    Serial.println(velocity, DEC);
  #endif
}

void handleControlChange(byte channel, byte control, byte value) {
  if (channel != channelNumber) { return; }
  if (control == 0) { 
    setDigitState(value);
    sendLedsState(); 
  }
  #ifdef DEBUG
    Serial.print("Control Change, ch=");
    Serial.print(channel, DEC);
    Serial.print(", control=");
    Serial.print(control, DEC);
    Serial.print(", value=");
    Serial.println(value, DEC);
  #endif
}

int getNoteLed(int note, int velocity, bool on) {
  if (note == 8 || note == 10 || note == 12 || note == 14) {
    #ifdef DEBUG
      Serial.print("CUE");
      Serial.print(note - 8);
      Serial.println((velocity == 6) ? 0 : 1);
    #endif
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
