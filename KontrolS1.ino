#include <SPI.h>
#include <Encoder.h>

// 165
// Chip pin 2 (CP)   goes to SCK   (D13)
// Chip pin 9 (Q7)   goes to MISO  (D12)
// 595
// Chip pin 11 (SH)  goes to SCK   (D13)
// Chip pin 14 (DS)  goes to MOSI  (D11)

#define latchPinIn 8 // 74HC165
#define latchPinOut 9 // 74HC595

#define faderPin 19

#define debug 1
#define LEDS_SERIAL_OFF_STATE 0b01010101, 0b00000000, 0b11000000, 0b00000000, 0b11111111, 0b11111111
#define BUTTONS_SERIAL_OFF_STATE 0b00000000, 0b00000000, 0b00001100

const int channelNumber = 1; // MIDI channel

const byte ledsOffState[6] = { LEDS_SERIAL_OFF_STATE };
byte ledsState[6] = { LEDS_SERIAL_OFF_STATE };

const byte buttonsOffState[3] = { BUTTONS_SERIAL_OFF_STATE };
byte buttonsState[3] = { BUTTONS_SERIAL_OFF_STATE };

const byte digitStates[22] = {
  // .32
  0b00011000,
  0b10100100,
  // .16
  0b01111001,
  0b10010000,
  // .8
  0b11111011,
  0b10000000,
  // .4
  0b11111011,
  0b10001011,
  // .2
  0b11111011,
  0b10100100,
  // 1
  0b11111111,
  0b11001111,
  // 2
  0b11111111,
  0b10100100,
  // 4
  0b11111111,
  0b10001011,
  // 8
  0b11111111,
  0b10000000,
  // 16
  0b01111101,
  0b10010000,
  // 32
  0b00011100,
  0b10100100
};

// LED OFF STATES from least significant bit (last one so notation is in reverse)
// -- BYTE [0]: 0b01010101 --
// CUE1 green - CUE1 blue - CUE2 green - CUE2 blue - CUE3 green - CUE3 blue - CUE4 green - CUE4 blue 
// -- BYTE [1]: 0b00000000 --
// Sample 1 - Sample 2 - Sample 3 - Sample 4 - SYNC - CUE - PLAY - SHIFT
// -- BYTE [2]: 0b11000000 --
// ON AIR - SAMPLES - DECK A/B - DECK C/D - KEYLOCK - MASTER - TEMPO OFFSET UP - TEMPO OFFSET DOWN
// -- BYTE [3]: 0b00000000 --
// LOOP IN - LOOP OUT - TOGGLE DECK C/D - LOAD - NONE - NONE - NONE -NONE
// -- BYTE [4]: 0b11111111 -- LEFT DIGIT 
// UP-MIDDLE - UP-RIGHT - DOT - UP-LEFT - DOWN-LEFT - DONW-MIDDLE - MIDDLE-MIDDLE - DOWN-RIGHT
// -- BYTE [5]: 0b11111111 -- RIGHT DIGIT
// DM - DL - UL - UM - UR - DR - MM - DOT

Encoder encoderLeft(15, 16);
Encoder encoderRight(17, 18);

Encoder encoderJog(5, 6);

int faderState = 0;

void setup () {
  // setup SPI
  SPI.begin ();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
//  SPI.setClockDivider(SPI_CLOCK_DIV128);
//  SPI.setDataMode(SPI_MODE0);
//  SPI.setBitOrder(MSBFIRST);
  pinMode(latchPinIn, OUTPUT);
  pinMode(latchPinOut, OUTPUT);

  pinMode(faderPin, INPUT);

  digitalWrite (latchPinIn, HIGH);
  digitalWrite (latchPinOut, HIGH);
  // setup handlers for USB MIDI
  usbMIDI.setHandleNoteOn(handleNoteOn);
  usbMIDI.setHandleNoteOff(handleNoteOff);
  usbMIDI.setHandleControlChange(handleControlChange);
  // setup state
  clearLedsState();
  for (int i = 0; i < (6*8); i++) {
    // flip bit at position
    setLedState(i, 1);
    sendLedsState();
    delay(25);
  }
  clearLedsState();
  sendLedsState();
  readButtonState();
}

void loop () {
  readEncoders();
  readButtonState();
  readTempoFader();
  
  // read USB MIDI
  usbMIDI.read();
}

void readButtonState () {
  byte newButtonsState[3];
  digitalWrite (latchPinIn, LOW);    // pulse the parallel load latch
  delayMicroseconds(5); // doesn't break when we remove this
  digitalWrite (latchPinIn, HIGH);
  // delayMicroseconds(5); // doesn't break when we remove this
  // get new button state
  for (int i = 0; i <= 2; i++) {
    newButtonsState[i] = SPI.transfer(0b00000000);
  }
  
  // compare to previous button state and send midi messages
  for (int i = 0; i < (3 * 8); i++) {
    const int arrayEl = i/8;
    const int byteEl = i%8;
    const bool offState = bitRead(ledsOffState[arrayEl], byteEl);
    const bool oldState = bitRead(buttonsState[arrayEl], byteEl);
    const bool newState = bitRead(newButtonsState[arrayEl], byteEl);
    if (newState != oldState) {
      if (newState ? !offState : offState) {
        usbMIDI.sendNoteOn(i, 127, channelNumber);
      } else {
        usbMIDI.sendNoteOff(i, 127, channelNumber);
      }
      
      if (debug) {
        Serial.print(arrayEl);
        Serial.print(byteEl);
        Serial.print(offState);
        Serial.print(oldState);
        Serial.print(newState);
        Serial.print("Button ");
        Serial.print(i);
        Serial.print(" now ");
        Serial.println(newState ? !offState : offState);
      }
    }
  }

  for (int i = 0; i < 3; i++) {
    buttonsState[i] = newButtonsState[i];
  }
}

void sendLedsState () {
  // set latch pin 595 to fill registers with state
  digitalWrite (latchPinOut, LOW);
  // delay(1); // not needed, or can be mS
  // send state
  for (int i = 0; i <= 5; i++) {
    SPI.transfer(ledsState[i]);
  }
  // load state into outputs
  digitalWrite (latchPinOut, HIGH);
}

void setLedState (int ledNumber, bool newState) {
  int arrayEl = ledNumber/8;
  int byteEl  = ledNumber%8;
  bool offState = bitRead(ledsOffState[arrayEl], byteEl);
  bitWrite(ledsState[arrayEl], byteEl, (newState ? !offState : offState));

  if (debug) {
    Serial.print("Arry: ");
    Serial.print(arrayEl);
    Serial.print(" byte: ");
    Serial.print(byteEl);
    Serial.print(" message: ");
    Serial.println(ledsState[arrayEl], BIN);
  }
}

void setDigitState (int newState) {
  byte leftDigit = digitStates[(newState * 2)];
  byte rightDigit = digitStates[(newState * 2) + 1];
  ledsState[4] = leftDigit;
  ledsState[5] = rightDigit;

  if (debug) {
    Serial.print("Left digit: ");
    Serial.print(leftDigit, BIN);
    Serial.print(" right digit: ");
    Serial.println(rightDigit, BIN);
  }
}

void clearLedsState () {
  memcpy(ledsState, ledsOffState, 6);
}

void readEncoders () {
  int encoderLeftPosition = (encoderLeft.read() / 4);
  if (encoderLeftPosition != 0) {
    Serial.print("encoder left position: ");
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
    Serial.print("encoder right position: ");
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
    Serial.print("encoder jog position: ");
    Serial.println(encoderJogPosition);
    // send encoder midi note
    if (encoderJogPosition > 0) { usbMIDI.sendControlChange(2, 63, channelNumber); }
    else { usbMIDI.sendControlChange(2, 65, channelNumber); }
    // reset enncoder
    encoderJog.write(0);
  }
}

void readTempoFader () {
  int newFaderState = (analogRead(faderPin) / 8);
  if (newFaderState != faderState) {
    faderState = newFaderState;
    usbMIDI.sendControlChange(5, faderState, channelNumber);
  }
}


void handleNoteOn(byte channel, byte note, byte velocity) {
  if (channel != channelNumber) { return; } 
  setLedState(note, 1);
  sendLedsState();
  if (debug) {
    Serial.print("Note On, ch=");
    Serial.print(channel, DEC);
    Serial.print(", note=");
    Serial.print(note, DEC);
    Serial.print(", velocity=");
    Serial.println(velocity, DEC);
  }
}

void handleNoteOff(byte channel, byte note, byte velocity) {
  if (channel != channelNumber) { return; } 
  setLedState(note, 0);
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
