//We always have to include the library
#include "LedControl.h"

#include "font.h"

static LedControl lc = LedControl(/*data*/ 11, /*clk*/ 13, /*cs*/ 10, /*#devices*/ 1);

#define GLYPH_WIDTH     6
#define GLYPH_HEIGHT    8

static uint32_t frame[8];

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

// reverses bit order in a byte
static uint8_t revbits(uint8_t b)
{
    uint8_t r;
    r = (b >> 4) | (b << 4);
    r = ((r & 0xCC) >> 2) | ((r & 0x33) << 2);
    r = ((r & 0xAA) >> 1) | ((r & 0x55) << 1);
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

static void draw_bitmap(const uint8_t *bitmap)
{
    int i;
    for (i = 0; i < 8; i++) {
        frame[i] = (uint32_t)bitmap[i] << 24;
    }
}

static void alarm_cycle(void)
{
    static int phase = 0;
    static const uint8_t radio[8] = {0x3C, 0x5A, 0x99, 0x99, 0xFF, 0xE7, 0x42, 0x3C};
    static const uint8_t mask[8] =  {0x7C, 0xFE, 0x92, 0xFE, 0x7C, 0x38, 0x7C, 0x7C};
    static const uint8_t skull[8] = {0x7C, 0xFE, 0xFE, 0x92, 0x92, 0x6C, 0x38, 0x38};
    
    phase = (phase + 1) % 64;
    if (phase < 32) {
        draw_bitmap(radio);
    } else {
        draw_bitmap(mask);
    }
    update_display();
    
    int level = phase % 32;
    if (level < 16) {
        lc.setIntensity(0, level);
    } else {
        lc.setIntensity(0, 31 - level);
    }
}


void loop(void) 
{
  
  int i;
  int j;
  
#if 1
    alarm_cycle();
    delay(40);
#else
  static const char text[] = "  56 cpm  ";

  lc.setIntensity(0, 4);
  for (i = 0; text[i] != 0; i++) {
    draw_char(text[i]);
    
    int j;
    for (j = 0; j < GLYPH_WIDTH; j++) {
      scroll();
      update_display();
      delay(40);
    }
  }
#endif
}
