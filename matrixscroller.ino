//We always have to include the library
#include "LedControl.h"

#include "font.h"

static LedControl lc = LedControl(/*data*/ 11, /*clk*/ 13, /*cs*/ 10, /*#devices*/ 1);

#define GLYPH_WIDTH     6
#define GLYPH_HEIGHT    8

void setup(void)
{
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,1);
  /* and clear the display */
  lc.clearDisplay(0);
}

static uint32_t frame[8];

// reverses bit order in a byte
static uint8_t revbits(uint8_t b)
{
    uint8_t r = 0;
    int i;
    for (i = 0; i < 8; i++) {
        if ((b >> i) & 1) {
            r |= 128 >> i;
        }
    }
    return r;
}

// draw a glyph by index
static void draw_glyph(int index)
{
  int i;
  int pos = 8 * index;
  for (i = 0; i < GLYPH_HEIGHT; i++) {
    frame[i] |= revbits(pgm_read_byte(&font_bin[pos]));
    pos += 1;
  }
}

// draw a glyph by character
static void draw_char(char c)
{
  int i;
  for (i = 0; chars[i] != 0; i++) {
    if (c == chars[i]) {
      draw_glyph(i);
      return;
    }
  }
}

// updates the display
static void update_display(void)
{
  int i;
  for (i = 0; i < 8; i++) {
    lc.setRow(0, i, frame[i] >> 24);
  }
}

// scroll the framebuffer one pixel to the left
static void scroll(void)
{
    int i;
    for (i = 0; i < 8; i++) {
        frame[i] <<= 1;
    }
}

void loop(void) 
{
  static const char text[] = "Spoorlaan 5d   PIZZA   ";
  
  int i;
  for (i = 0; text[i] != 0; i++) {
    draw_char(text[i]);
    
    int j;
    for (j = 0; j < GLYPH_WIDTH; j++) {
      scroll();
      update_display();
      delay(25);
    }
  }
}
