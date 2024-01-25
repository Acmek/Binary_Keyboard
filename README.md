#Binary Keyboard

WARNING
=====
The USB-C's CC pins are attached to 5.1k resistors, allowing smart chargers to output 5v.

However, the is no onboard regulator! Please do not plug the keyboard into a source greater than 5v!


Parts
=====
The BOM file is formatted to be compatible with JLCPCB's PCBA, but it is possible to solder all components by hand.

They also contain links to other components such as the screen, keyboard switches, etc.


Programming
=====
You can use Arduino IDE's ISP programming feature and the SPI test points to burn the bootloader onto the Atmega32u4.

After burning the bootloader, uploading the .ino file can be done through the USB port.


Switching Modes
=====
By pressing both 0 and 1 keys down, you are able to change which direction you are able to type in.

The current input will reset and the new input will begin at the LSB (R-L Mode) or MSB (L-R Mode).

Which mode is in use is saved even after cutting power.
