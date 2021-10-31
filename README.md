![S1 Project](https://raw.githubusercontent.com/NickolasBoyer/KontrolS1/master/logo.svg)

The following info is all about the S4 MK1, the MK2 has a different (non-panelized) PCB design and thus doesn't allow for reusing the original PCB's without serious modifications. The MK3 also has a single PCB for the mixer and deck sections.

## Setup
- Use the `Kontrol_S1_LC_OLED` sketch
- Either `#include "unit_d.h"` or `#include "unit_c.h"` based on the unit
- Uncomment `#define SCREEN` if using the OLED module
- Edit the `MIDI_NAME` in the `name.c` file to the desired name (these cannot conflict with other S1 units you're using)
- Flash the firmware

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

## Button off states
The first four bits of the last packet are connected to GND trough some 330 R resistors. 

```
// BUTTON OFF STATES
// -- BYTE [0]: 0B00000000 --
// PLAY - CP4 - CUE - CP3 - SYNC - CP2 - SHIFT - CP1
// -- BYTE [1]: 0B00000000 --
// S4 - LOAD - S3 - LOOP OUT - S2 - LOOP IN - DECK C/D
// -- BYTE [2]: 0B00001100 --
// X - X - X - X - ENC-R BUT - ENC-L BUT - OFFSET DOWN - OFFSET UP
```

## Pinout
This is the pinout for the DECK A board the DECK B connector is almost identical (but mirrored). The two exceptions are pin 2 and 13, ~~I have no idea what pin 2 is~~ pin 2 is the 165's DS on the DECK B unit. Click [here](https://i.imgur.com/EdCKgI3.jpg) for the DECK B section.

> WARNING: Pinout changed, check below.

![pinout](https://i.imgur.com/A7mcbnp.png)
```
// 595’s output enable is connected to GND.
// 165’s CE (chip enable) is connected to GND.
// (13, 14, 15), (16, 17) and (18, 19) are connected together.

1. Q7 (serial out 165)
2. GND (DS from 165's on DECK B)
3. PL (latch pin 165)
4. CP (clock input 165)
5. ENC4-R (330 R)
6. ENC5-L (330 R)
7. ENC4-R (330 R)
8. ENC5-L (330 R)
9. Q7S (serial out 595) (for DECK B this is the serial out of the 4 upper 595's)
10. DS (serial in 595) (for DECK B this is the serial in of the 2 lower 595's)

11. ST_CP (latch pin 595)
12. SH_CP (clock input 595)
13. GND (For DECK B this is the serial in for the 4 upper 595's)
14. GND
15. GND
16. LED V1 (5v but can just be 3.v)
17. LED V1 (5v but can just be 3.v)
18. LED V2 (3.3v)
19. LED V2 (3.3v)
20. VCC

1. GND
2. VCC
3. POT
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

## Jog wheels

The rotation of the jogs is measured by two photo-interrupters ([KTIR0611S](http://www.farnell.com/datasheets/2307823.pdf)). The "push down" of the jog is measured by a photo sensor that gets blocked when you push the platter down on it.

### Connections
The 6 pin JST connector is connected to the circuit with the two photo-interrupters. There is also a connection to a third photo-interrupter but they did not end up mounting and integrating this third one into the final product.
```
1. VCC (3.3v)
2. VCC (3.3v) (this "enables" the optical readings?) 
3. GND
4. ENC R
5. ENC L
6. ENC M (not connected or mounted)
```
The 3 color push cable coming out of the jog:
```
BLUE. GND
RED. VCC (3.3V)
WHITE. SIGNAL (analog read: 800~900 off, 1023 on)
-. DUMMY (unused pin on 4-pin JST connector) 
```
> ## Warning
> The jog press is not yet working so the info about it is possibly wrong. The MK1 does seem to use a IR proximity sensor but so far no reliable readings have been produced. The MK2 uses an Ambient Light Sensor with an I2C Interface called the [VNCL4000](https://cdn-shop.adafruit.com/datasheets/vcnl4000.pdf). The current MK1 problems might just be calibration issues but could also be something more serious.


## Observations
- VCC should be 3.3v, not 5v. The 595 works with 5v and you will not fry the LED's but the 165 will not work and you might damage it.
- You can easily run the 165 and 595 chips with hardware SPI.
- The left and right circuit board are not identical at all though the connectors appear to be.

## Custom FX PCB

The custom FX PCB has the same layout on both sides so only one version is required (it has both the L&R mounting holes). The custom FX PCB provides a 165 and 595 interface for the buttons and LED's respectively and a separate connector for the potentiometer signals.

### Pinout

```
1. INH (165-15) [Clock enable]
2. SER (165-10) [Serial data input]
3. QH (595-9) [Serial data output]
4. LATCH 165 (165-1) [Parallel load input]
5. RCK (595-12) [storage register clock input]
6. QH (165-9) [Serial out]
7. SER (595-14)[Serial data input]
8. CLOCK
9. GND
10. VCC

11. VCC
12. GND
13. FX_3
14. FX_2
15. FX_1
16. FX_DW
```

## Parts

These simplified BOM's are for the MK4 releases of the brain and pot boards.

### Brain board

These are the parts needed to populate 1 brain board, to convert one S4 to two S1 units you'd need these parts x2.

| Part                                   | Amount | Value |
| -------------------------------------- | ------ | ----- |
| Molex Picoblade (1.25mm) 10-pin        | 3      |       |
| Molex Picoblade (1.25mm) 8-pin         | 1      |       |
| Molex Picoblade (1.25mm) 6-pin         | 3      |       |
| Molex Picoblade (1.25mm) 4-pin         | 2      |       |
| M3 metal standoffs                     | 4      | 15mm  |
| M3 metal screws                        | 8      | 6mm   |
| USB-B socket                           | 1      |       |
| Teensy LC                              | 1      |       |
| Capacitor US 1206                      | 1      | 100nF |
| Resistor US 1206 (0805 compatible)     | 1      | 1M    |

The 1M resistor might need a higher value.

### Pot board

These are the parts needed to populate 1 brain board, to convert one S4 to two S1 units you'd need these parts x2. The standoffs needed are already included with the brain BOM.

| Part                                   | Amount | Value |
| -------------------------------------- | ------ | ----- |
| Molex Picoblade (1.25mm) 10-pin        | 1      |       |
| Molex Picoblade (1.25mm) 6-pin         | 1      |       |
| Resistor US 0805                       | 5      | 82    |
| Resistor US 0805                       | 8      | 10K   |
| 74HC595D                               | 1      |       |
| 74HC165D                               | 1      |       |
| Capacitor US 1206                      | 6      | 100nF |

The four capacitors for the potentiometers might need a lower value.

## Custom OLED PCB

Contains adapter from 1.25mm JST to OLED, a step-up to 12V and some passive components the OLED driver needs.

### Pinout

```
1. GND
2. CLOCK
3. GND
4. MOSI
5. SS-SCREEN
6. VCC
7. DC
8. RST
```

# Screen

The screen uses https://github.com/ErikMinekus/traktor-api-client to get data from traktor + a local server to send it to the teensy over serial.
