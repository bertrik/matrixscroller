matrixscroller
==============

Arduino project showing scrolling text on a MAX7219-based 8x8 LED matrix.

Requirements:
    * LedControl library for controlling the hardware
    * a nice bitmapped monospaced font of size 8x8 or smaller
    * imagemagick for creating a binary font file
    * xxd utility to convert a binary font file into a C-style include file.
    
To build a new font:
    * run 'make' to convert the font (check how it looks by opening font.png)
    * update source code to reflect the effective height and width of the glyphs

To run:
    * make sure a MAX7219-based 8x8 LED matrix is connected to the arduino
    * make sure the LedControl constructor matches your hardware connections (for DIN, CS, CLOCK)
    * compile and upload the .ino file into an Arduino (arduino nano for example)

