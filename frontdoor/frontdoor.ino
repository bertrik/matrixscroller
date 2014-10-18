#include <stdint.h>

#include <SPI.h>
#include <RF24.h>

//We always have to include the library
#include "LedControl.h"

#include "font.h"

static LedControl lc = LedControl(/*data*/ 2, /*clk*/ 4, /*cs*/ 3, /*#devices*/ 2);
static uint64_t address = 0x66996699LL;  // So that's 0x0066996699

static RF24 rf(/*ce*/ 9, /*cs*/ 10);

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
  lc.shutdown(1,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,15);
  lc.setIntensity(1,15);
  /* and clear the display */
  lc.clearDisplay(0);
  lc.clearDisplay(1);
  
    Serial.begin(9600);
    Serial.println("Hello world!");
    
    /* NRF init */
    rf.begin();
    Serial.print("Is plus? ");
    Serial.println(rf.isPVariant() ? "yes" : "no");    

    rf.enableDynamicPayloads();
    rf.openReadingPipe(1, address);
    rf.startListening();
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
  
  uint8_t buf[2][8];
  int x,y,d;
  memset(buf, 0, sizeof(buf));
  for (y = 0; y < 8; y++) {
    for (d = 0; d < 2; d++) {
      for (x = 0; x < 8; x++) {
        int shift = 8 + (d * 8) + x;
        boolean bit = (frame[y] & (1L << shift)) != 0;
        if (bit) {
          buf[1 - d][7 - x] |= 1L << y;
        }
      }
    }
  }
  
  // send to display
  for (i = 0; i < 8; i++) {
    lc.setRow(0, i, buf[0][i]);
    lc.setRow(1, i, buf[1][i]);
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
  static char text[32] = " PIZZA ";
  static boolean avail = false;
  
  // prepare text
    while (rf.available()) {
        char buf[32];
        memset(buf, 0, sizeof(buf));
        rf.read(&buf, sizeof(buf));
        if (memcmp(buf + 1, "DOOR", 4) == 0) {
            uint8_t len = buf[0];
            if (len < 4) {
                len = 4;
            }
            if (len > 31) {
                len = 31;
            }
            strncpy(text, buf + 5, len - 4);
            text[len - 4] = '\0';
            Serial.print("Received: '");
            Serial.print(text);
            Serial.println("'");
        }
    }

  // display text
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
