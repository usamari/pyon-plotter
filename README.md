License: firmware GPL-3.0-or-later, print files CC BY-NC-SA 4.0, docs CC BY 4.0. Details at the end.

This is a kawaii pen plotter that draws within a 60 mm square.
It runs on its own with three buttons. Each button has a short press and a long press.

This machine was developed to turn leftover guide rails at home into a pen plotter, using inexpensive extra parts. A later version with a more deliberate parts selection is also planned.
A larger version, able to draw up to about 170 mm, is in development.

If you find a problem in the files, please use the contact at the end of this README.
Tips are welcome and do not change the license: https://ko-fi.com/usamari

## BOM

- Arduino Uno × 1
- CNC Shield V3 × 1
- Stepper driver A4988 × 2
- NEMA 17 stepper motor (42 × 42 × 23 mm) × 2
- V-groove bearing pulley (M3 × 12 × 4 mm) × 8
- SG90 servo motor × 1
- MGN9H linear rail (100 mm) × 3
- MGN7C linear rail (55 mm) × 1
- Push button × 3

Quantities are small and are omitted.

- M2 × 6 mm screw
- M3 × 6 mm screw
- M3 × 8 mm screw
- M3 × 20 mm screw
- Washer (M3 × 9 × 1 mm)
- Insert nut (M3, 3–4 mm)
- Set screw (M3, 3–4 mm)
- Spring (we recommend having several sizes available to test)
- Prepare wiring components as necessary.

The design assumes PLA filament.
See the images for the belt routing.


## Wiring

The firmware expects an Arduino Uno and a CNC Shield V3.
Motor A uses the X driver socket. Motor B uses the Y driver socket.
Enable is active low: the motors are energized when D8 is LOW.

| Function | Arduino | CNC Shield V3 | Connect to |
| --- | --- | --- | --- |
| Motor A step | D2 | X STEP | A4988 in X socket |
| Motor A dir | D5 | X DIR | A4988 in X socket |
| Motor B step | D3 | Y STEP | A4988 in Y socket |
| Motor B dir | D6 | Y DIR | A4988 in Y socket |
| Enable | D8 | EN | both drivers, active low |
| Servo signal | D11 | Z limit header | SG90 signal wire |
| Blue Abort | A0 | Abort | button to GND |
| Yellow Hold | A1 | Hold | button to GND |
| Red Resume | A2 | Resume | button to GND |

Buttons use the internal pull-ups. Wire each button between its pin and GND.
Do not add an external pull-up.

The SG90 needs 5 V and GND as well as the signal on D11.
If the servo resets the Arduino, power the servo from a separate 5 V supply and join the grounds.

A long press is 800 ms.
Blue: short = 55 mm square, long = 25 mm circle.
Yellow: short = pattern 1, long = pattern 2.
Red: short = pen up, or stop if a drawing is running. Long = pen down.


## License

Copyright (c) 2026 USAMARI 宇佐まり

These parts are licensed separately:

- Firmware: GPL-3.0-or-later. See LICENSE.
  It uses AccelStepper (Mike McCauley, GPL) and the Arduino Servo library (LGPL-2.1).
- Print files and mechanical design (STL, STEP): CC BY-NC-SA 4.0. See LICENSE-hardware.
  Non-commercial use, modification, and sharing are allowed. Sale is not allowed without permission.
- Documentation, including this README and images: CC BY 4.0. See LICENSE-docs.

A license applies only to the files it names. The GPL does not apply to the print files, and CC BY-NC-SA 4.0 does not apply to the firmware.
This is a summary. If it differs from the license files, those files prevail.

## Contact

ukabupukapuka@gmail.com
https://x.com/8_senkou

## Tips

https://ko-fi.com/usamari
