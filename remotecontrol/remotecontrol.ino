#include <stdint.h>
#include <stdbool.h>

#include <SPI.h>
#include <RF24.h>

#define MAX_TEXTSIZE    27

#define BELL    0x07
#define BS      0x08
#define LF      0x0A
#define CR      0x0D

static uint64_t address = 0x66996699L;  // So that's 0x0066996699

static RF24 rf( /*ce */ 9, /*cs */ 10);

void setup(void)
{
    // initialize serial port
    Serial.begin(9600);
    Serial.println("Hello world!\n");

    // init RF24
    rf.begin();
    rf.openWritingPipe(address);
    rf.enableDynamicPayloads();
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

    case LF:
        // ignore
        break;

    case CR:
        // finish
        buf[index] = 0;
        Serial.write(c);
        index = 0;
        return true;

    case BS:
    case 127:
        // backspace
        if (index > 0) {
            Serial.write(BS);
            Serial.write(' ');
            Serial.write(BS);
            index--;
        } else {
            Serial.write(BELL);
        }
        break;

    default:
        // try to add character to buffer
        if (index < (size - 1)) {
            buf[index++] = c;
            Serial.write(c);
        } else {
            Serial.write(BELL);
        }
        break;
    }
    return false;
}

/* Sends a text over NRF24
 * @param text the text to send (0-terminated), should be at most 27 characters long
 */
static void send_text(char *text)
{
    char sndbuf[32];

    Serial.print("Sending: '");
    Serial.print(text);
    Serial.println("'");

    // construct buffer
    int length = strlen(text);
    int idx = 0;
    sndbuf[idx++] = 4 + length;
    sndbuf[idx++] = 'D';
    sndbuf[idx++] = 'O';
    sndbuf[idx++] = 'O';
    sndbuf[idx++] = 'R';
    memcpy(sndbuf + idx, text, length);

    // send it
    rf.write(sndbuf, idx + length);
}

void loop(void)
{
    static char textbuffer[MAX_TEXTSIZE + 1];

    if (Serial.available() > 0) {
        char c = Serial.read();
        if (process_char(c, textbuffer, MAX_TEXTSIZE)) {
            send_text(textbuffer);
        }
    }
}
