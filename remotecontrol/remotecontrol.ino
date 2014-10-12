#include <RF24.h>
#include <nRF24L01.h>
#include <RF24_config.h>

#include <SPI.h>

#include <stdint.h>
#include <stdbool.h>

#define MAX_TEXTSIZE    27

#define BELL    0x07
#define BS      0x08
#define LF      0x0A
#define CR      0x0D

static long int address = 0x66996699L;  // So that's 0x0066996699

static uint8_t buffer[5];
static RF24 rf(/*ce*/ 9, /*cs*/ 10);

void setup(void)
{
    // initialize serial port
    Serial.begin(115200);
    Serial.println("Hello world!\n");
    
    // init RF24
    rf.begin();
    rf.setRetries(15, 15);
    rf.enableDynamicPayloads();
    rf.openWritingPipe(address);
}

/* Processes a character into an edit buffer, returns true 
 * @param c the character to process
 * @param buf the edit buffer
 * @param size the size of the buffer
 * @return true if a full line was entered, false otherwise
 */
static bool process_char(char c, char *buf, int size)
{
    static int index = 0;
    switch (c) {
    case CR:
        // ignore
        break;
    case LF:
        // finish
        buf[index] = 0;
        Serial.print(c);
        index = 0;
        return true;
    case BS:
        // backspace
        if (index > 0) {
            Serial.print(BS);
            Serial.print(' ');
            Serial.print(BS);
            index--;
        } else {
            Serial.print(BELL);
        }
        break;
    default:
        // try to add character to buffer
        if (index < (size - 1)) {
            buf[index++] = c;
            Serial.print(c);
        } else {
            Serial.print(BELL);
        }
        break;
    }
    return false;
}


static void send_text(char *text)
{
    char buffer[32];
    
    // construct buffer
    int length = strlen(text);
    int idx = 0;
    buffer[idx++] = 4 + length;
    buffer[idx++] = 'D';
    buffer[idx++] = 'O';
    buffer[idx++] = 'O';
    buffer[idx++] = 'R';
    memcpy(buffer + idx, text, length);
    
    // send it
    rf.write(buffer, idx + length);
}

void loop(void)
{
    static char textbuffer[MAX_TEXTSIZE+1];

    if (Serial.available() > 0) {
        char c = Serial.read();
        if (process_char(c, textbuffer, MAX_TEXTSIZE)) {
            send_text(textbuffer);
        }
    }
}


