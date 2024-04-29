# pipboy3000
Code and data for an Arduino-based PIP-Boy 3000.

This was designed for an Arduino Pro Mini or Nano with an Adafruit 3.5" TFT 320x480 touchscreen (HXD8357D, product ID 2050). It doesn't use the touchscreen features; instead, it uses an encoder wheel and three buttons for input. The data files should be placed on a MicroSD card, which should be inserted into the reader on the display.

The code expects the Arduino pins to be connected to:

D4 - TFT CCS  
D6 - Rotary encoder (right)  
D7 - Rotary encoder (left)  
D9 - TFT D/C  
D10 - TFT CS  
D11 - TFT MOS1  
D12 - TFT MIS0  
D13 - TFT CLK  
A0 - Button 1  
A1 - Button 2  
A2 - Button 3  
A3 - LED 3  
A4 - LED 1  
A5 - LED 2  
5V - TFT VIN  
GND - TFT GND, buttons, LEDs (with resistor), and rotary encoder (center)  

The screen painting is not blindingly fast, but it's faster than loading static images on from the SD card.

When the buttons are pushed individually, they switch between STATS, INV, and DATA modes. Pressing the left and center buttons simultaneously will cycle through the four color choices offered by Fallout 3 (green, amber, blue, and white). Pressing the center and right buttons will engage Flashlight mode (press any button or move the encoder to exit). Pressing the left and right buttons will reset to the initial STATS:Status screen.
