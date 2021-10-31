#ifndef UNIT_C
#define UNIT_C

#define CUE_CHIP_NR 0
#define CUP_CHIP_NR 1
#define DISP_CHIP_NR 2
#define LP_CHIP_NR 3
#define LDG_CHIP_NR 4
#define RDG_CHIP_NR 5
#define FX_CHIP_NR 6

#define BUTTONS_SERIAL_OFF_STATE 0b00000000, 0b00000000, 0b00000000, 0b00000000
#define LEDS_SERIAL_OFF_STATE 0b00000000, 0b00000000, 0b11111111, 0b11000000, 0b00000000, 0b01010101, 0b00000000
// for newer models: #define LEDS_SERIAL_OFF_STATE 0b01010101, 0b00000000, 0b11111111, 0b11000000, 0b00000000, 0b01010101, 0b00000000


#define DIGIT_STATE_1_32 0b00011000, 0b10100100
#define DIGIT_STATE_1_16 0b01111001, 0b10010000
#define DIGIT_STATE_1_8 0b11111011, 0b10000000
#define DIGIT_STATE_1_4 0b11111011, 0b10001011
#define DIGIT_STATE_1_2 0b11111011, 0b10100100
#define DIGIT_STATE_1 0b11111111, 0b11001111
#define DIGIT_STATE_2 0b11111111, 0b10100100
#define DIGIT_STATE_4 0b11111111, 0b10001011
#define DIGIT_STATE_8 0b11111111, 0b10000000
#define DIGIT_STATE_16 0b01111101, 0b10010000
#define DIGIT_STATE_32 0b00011100, 0b10100100

#define BUTTON_MAP 0, 1, 2, 3, 4, 5, 6, 7, 24, 25, 26, 27, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 8, 9, 10, 11, 12, 13, 14, 15

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
//  24,       // tempo range up
//  25,       // tempo range down
//  26,       // encr
//  27,       // encl
//  12,
//  13,
//  14,
//  15,
//
//  16,       // Deck shift toggle
//  17,       // S1
//  18,       // Loop in
//  19,       // S2
//  20,       // loop out
//  21,       // S3
//  22,       // Load
//  23,       // S4
//  
//  (8 + 0),  // CUE1
//  9,        // shift
//  (8 + 2),  // CUE2
//  11,       // sync
//  (8 + 4),  // CUE3
//  13,       // cue
//  (8 + 6),  // CUE4
//  15        // play
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

#endif
