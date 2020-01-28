# Kontrol S1 Project

## Led off states
All leds are controlled by a transistor activated by the 595 chip. However not all are connected the same way, sending 6 all-off byte packages (`0b00000000`) will turn some of the LED's on, these specific bits to be set to HIGH instead of LOW in order to be turned off.

```
// LED OFF STATES from least significant bit (so notation is in reverse)
// -- BYTE [0]: 0b01010101 --
// CUE1 green - CUE1 blue - CUE2 green - CUE2 blue - CUE3 green - CUE3 blue - CUE4 green - CUE4 blue 
// -- BYTE [1]: 0b00000000 --
// Sample 1 - Sample 2 - Sample 3 - Sample 4 - SYNC - CUE - PLAY - SHIFT
// -- BYTE [2]: 0b11000000 --
// ON AIR - SAMPLES - DECK A/B - DECK C/D - KEYLOCK - MASTER - TEMPO OFFSET UP - TEMPO OFFSET DOWN
// -- BYTE [3]: 0b00000000 --
// LOOP IN - LOOP OUT - TOGGLE DECK C/D - LOAD - NONE - NONE - NONE -NONE
// -- BYTE [4]: 0b11111111 -- LEFT DIGIT
// UP-MIDDLE - UP-RIGHT - DOT - UP-LEFT - DOWN-LEFT - DOWN-MIDDLE - MIDDLE-MIDDLE - DOWN-RIGHT
// -- BYTE [5]: 0b11111111 -- RIGHT DIGIT
// DM - DL - UL - UM - UR - DR - MM - DOT
```

## Pinout

![pinout](https://i.imgur.com/A7mcbnp.png)
```
// 595’s output enable is connected to GND.
// 165’s CE (chip enable) is connected to GND.
// (13, 14, 15), (16, 17) and (18, 19) are connected together.

1. Q7 (serial out 165)
2. GND
3. PL (latch pin 165)
4. CP (clock input 165)
5. ENC4-R (330 R)
6. ENC5-L (330 R)
7. ENC4-R (330 R)
8. ENC5-L (330 R)
9. Q7S (serial out 595)
10. DS (serial in 595)

11. ST_CP (latch pin 595)
12. SH_CP (clock input 595)
13. GND
14. GND
15. GND
16. LED V1 (5v but can just be 3.v)
17. LED V1 (5v but can just be 3.v)
18. LED V2 (3.3v)
19. LED V2 (3.3v)
20. VCC

1. GND
2. POT
3. VCC
4. GND
```

## Chip connections

### 74HC595

There are 6 595 chips chained together (the serial out(`Q7S`) for the last 595 is exposed as pin 9 on the connector).

- `ST_CP` and `SH_CP` of every chip are connected to connector pin 11 and 12.
- `MR` is connected to `VCC` on every chip.
- Output enable (`OE`) is connected to `GND` on every chip.

### 74HC165

There are 3 165 chips chained together.

- Clock enable(`CE`) is connect to `GND` on every chip.
- `PL` and `CP` of every chip are connected to connector pin 3 and 4.
