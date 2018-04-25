#include "IRremote.h"
#include "FastLED.h"
#include "avr/pgmspace.h"

// Define constants
#define DATA_PIN_LED      3
#define DATA_PIN_IR       21
#define BRIGHTNESS_PIN    A0
#define SPEED_PIN         A1
#define DATA_PIN_AUDIO
#define INTERRUPT_PIN     19
#define LED_TYPE          WS2812B
#define COLOR_ORDER       GRB
#define WIDTH             16
#define HEIGHT            16
#define NUM_LEDS          WIDTH*HEIGHT
#define BRIGHTNESS        64 // 0-255


// Define objects
IRrecv                    irrecv(DATA_PIN_IR); // Object used for reading the IR signal
decode_results            results; // Struct to hold information from the IR signal
CRGB                      leds[NUM_LEDS]; // Array to hold the color values of all the LEDs, uses 3 bytes per assigned LED


// Define global variables
volatile unsigned long lastCode;
volatile bool progRunning; // MUST be volatile as it can be updated in an ISR. 
int brightness; // Changed by potentiometer in order to update the brightness on-the-go
int progSpeed;
// unsigned long codes[18] =
// {0xFF629D, 0xFF22DD, 0xFF02FD, 0xFFC23D, 0xFFA857, 0xFF6897, 0xFF9867, 0xFFB04F, 0xFF30CF, 0xFF18E7, 0xFF7A85, 0xFF10EF, 0xFF38C7, 0xFF5AA5, 0xFF42BD, 0xFF4AB5, 0xFF52AD, 0xFFFFFFFF};


// Save 16x16 pictures in Flash memory with the avr/pgmspace-library in order to save space in SRAM
const long DigDug01[] PROGMEM = {
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000, 0x000000,
  0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x0066cc, 0x000000, 0x0066cc, 0x000000, 0x0066cc, 0x0066cc, 0x0066cc, 0x0066cc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000,
  0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0x0066cc, 0x0066cc, 0x0066cc, 0x0066cc, 0x000000, 0x0066cc, 0x000000, 0x0066cc, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x0066cc, 0x0066cc, 0x0066cc, 0x0066cc, 0x0066cc, 0x0066cc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000,
  0x000000, 0x000000, 0x000000, 0x0066cc, 0x0066cc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000, 0x000000, 0xff0000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0xff0000, 0xff0000, 0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x0066cc, 0x0066cc, 0xcccccc, 0x000000, 0x000000,
  0x000000, 0xff0000, 0xff0000, 0xff0000, 0x0066cc, 0x0066cc, 0x0066cc, 0x0066cc, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0x000000,
  0x000000, 0x000000, 0xff0000, 0xff0000, 0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0x0066cc, 0x0066cc, 0x0066cc, 0xcccccc, 0xcccccc, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000, 0x000000, 0xff0000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xcccccc, 0xcccccc, 0x000000, 0x000000, 0x000000, 0xcccccc, 0xcccccc, 0x000000, 0x000000,
  0x000000, 0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000
};

const long DigDug02[] PROGMEM =
{
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000, 0x000000,
  0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x0066cc, 0x000000, 0x0066cc, 0x000000, 0x0066cc, 0x0066cc, 0x0066cc, 0x0066cc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000,
  0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0x0066cc, 0x0066cc, 0x0066cc, 0x0066cc, 0x000000, 0x0066cc, 0x000000, 0x0066cc, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x0066cc, 0x0066cc, 0x0066cc, 0x0066cc, 0x0066cc, 0x0066cc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000,
  0x000000, 0x000000, 0x000000, 0x0066cc, 0x0066cc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000, 0x000000, 0xff0000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0xff0000, 0xff0000, 0x000000, 0x000000, 0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x0066cc, 0x0066cc, 0xcccccc, 0x000000, 0x000000,
  0x000000, 0xff0000, 0xff0000, 0xff0000, 0x0066cc, 0x0066cc, 0x0066cc, 0x0066cc, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0x000000,
  0x000000, 0x000000, 0xff0000, 0xff0000, 0x000000, 0x000000, 0x000000, 0xcccccc, 0x0066cc, 0x0066cc, 0x0066cc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000, 0x000000, 0x000000, 0x000000, 0xff0000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xcccccc, 0xcccccc, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xcccccc, 0xcccccc, 0xcccccc, 0xcccccc, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000
};

const long Qbert01[] PROGMEM =
{
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff6600, 0xff6600, 0xff6600, 0x000000, 0x000000,
  0x000000, 0xff0033, 0xff0033, 0xff6600, 0xff0033, 0xffffcc, 0xffffcc, 0xff0033, 0xffffcc, 0xffffcc, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xff0033, 0x000000, 0x000000, 0xff0033, 0xff6600, 0xff6600, 0xff0033, 0xff0033,
  0xff0033, 0xff0033, 0xff0033, 0xff6600, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff6600, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff0033, 0xff0033, 0xff6600, 0xff6600, 0xff6600, 0xff0033, 0xff0033,
  0xff0033, 0xff0033, 0xff0033, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0x000000, 0x000000,
  0x000000, 0xff6600, 0xff6600, 0xff6600, 0xff0033, 0xff0033, 0xff0033, 0xff6600, 0xff0033, 0xff6600, 0xff0033, 0xff6600, 0xff0033, 0xff6600, 0xff0033, 0xff0033,
  0x000000, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff6600, 0xff0033, 0xff0033, 0x000000, 0xff0033, 0xff0033, 0x000033, 0x000033, 0xff6600, 0x000000,
  0x000000, 0xff0033, 0x000033, 0x000033, 0xff0033, 0x000000, 0x000000, 0x000000, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0x000000, 0x000000, 0x000000, 0x000000, 0xff0033, 0xff0033, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xff0033, 0x000000, 0x000000, 0xff0033, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0xff6600, 0xff6600, 0xff6600, 0x000000, 0x000000, 0xff0033, 0xff6600, 0xff6600, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0xff6600, 0xff6600, 0xff0033, 0x000000, 0x000000, 0xff6600, 0xff6600, 0xff6600, 0xff0033, 0x000000, 0x000000, 0x000000
};

const long Qbert02[] PROGMEM =
{
  0x000000, 0x000000, 0x000000, 0x000000, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff6600, 0xff6600, 0xff6600, 0x000000, 0x000000,
  0x000000, 0xff0033, 0xff0033, 0xff6600, 0xff0033, 0xffffff, 0xffffff, 0xff0033, 0xffffff, 0xffffff, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xff0033, 0x000000, 0x000000, 0xff0033, 0xff6600, 0xff6600, 0xff0033, 0x000000,
  0xff0033, 0xff0033, 0xff0033, 0xff6600, 0xff0033, 0x000000, 0x000000, 0xff0033, 0x000000, 0x000000, 0xff6600, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff0033, 0xff0033, 0xff6600, 0xff6600, 0xff6600, 0xff0033, 0xff0033,
  0xff0033, 0xff0033, 0xff0033, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0xff6600, 0x000000, 0x000000,
  0x000000, 0xff6600, 0xff6600, 0xff6600, 0xff0033, 0xff0033, 0xff0033, 0xff6600, 0xff0033, 0xff6600, 0xff0033, 0xff6600, 0xff0033, 0xff6600, 0xff0033, 0xff0033,
  0x000000, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff6600, 0xff0033, 0xff0033, 0x000000, 0xff0033, 0xff0033, 0x000000, 0x000000, 0xff6600, 0x000000,
  0x000000, 0xff0033, 0x000000, 0x000000, 0xff0033, 0x000000, 0x000000, 0x000000, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0xff0033, 0x000000, 0x000000, 0x000000, 0x000000, 0xff0033, 0xff0033, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xff0033, 0x000000, 0x000000, 0x000000, 0xff0033, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0xff0033, 0x000000, 0x000000, 0x000000, 0xff0033, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xff0033, 0x000000, 0x000000, 0x000000, 0xff0033, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0xff6600, 0xff6600, 0xff6600, 0x000000, 0x000000, 0xff0033, 0xff6600, 0xff6600, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
  0x000000, 0x000000, 0x000000, 0x000000, 0xff6600, 0xff6600, 0xff0033, 0x000000, 0x000000, 0xff6600, 0xff6600, 0xff6600, 0xff0033, 0x000000, 0x000000, 0x000000
};

const long BombJack01[] PROGMEM =
{
  0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366,
  0x333366, 0x333366, 0x333366, 0x0099ff, 0x333366, 0x333366, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x333366, 0x333366, 0x0099ff, 0x333366, 0x333366,
  0x333366, 0x333366, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x333366, 0x333366, 0x333366,
  0x333366, 0x333366, 0x333366, 0x0099ff, 0x0099ff, 0x0099ff, 0xffffff, 0xffffff, 0x0099ff, 0xffffff, 0xffffff, 0x0099ff, 0x0099ff, 0x0099ff, 0x333366, 0x333366,
  0x333366, 0x333366, 0x333366, 0x0099ff, 0x0099ff, 0xffffff, 0x000000, 0x0099ff, 0x000000, 0xffffff, 0x0099ff, 0x0099ff, 0x333366, 0x333366, 0x333366, 0x333366,
  0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x0099ff, 0xffffff, 0x000000, 0x0099ff, 0x000000, 0xffffff, 0x0099ff, 0x333366, 0x333366, 0x333366, 0x333366,
  0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0xffcc99, 0xffcc99, 0xffcc99, 0xffcc99, 0xffcc99, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366,
  0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0xffffff, 0xffcc99, 0xffcc99, 0xffcc99, 0xffcc99, 0xffcc99, 0xffffff, 0x333366, 0x333366, 0x333366, 0x333366,
  0x333366, 0x333366, 0x333366, 0xff0000, 0xff0000, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xff0000, 0xff0000, 0x333366, 0x333366, 0x333366, 0x333366,
  0x333366, 0x333366, 0x333366, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0x000000, 0xff0000, 0x000000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0x333366, 0x333366,
  0x333366, 0x000000, 0x000000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0x000000, 0x000000, 0x333366, 0x333366,
  0x333366, 0x333366, 0x000000, 0x000000, 0xffffff, 0x0099ff, 0x0099ff, 0x0099ff, 0xffff00, 0x0099ff, 0x0099ff, 0x0099ff, 0xffffff, 0x000000, 0x000000, 0x333366,
  0x333366, 0xffffff, 0xffffff, 0xffffff, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xffffff, 0xffffff, 0xffffff, 0x333366, 0x333366,
  0x333366, 0x333366, 0xffffff, 0xffffff, 0xffffff, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xffffff, 0xffffff, 0xffffff, 0x333366,
  0x333366, 0x333366, 0xffffff, 0xffffff, 0xffffff, 0xff0000, 0xff0000, 0xffffff, 0xff0000, 0xff0000, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0x333366, 0x333366,
  0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x000000, 0x000000, 0x000000, 0x333366, 0x000000, 0x000000, 0x000000, 0x333366, 0x333366, 0x333366, 0x333366
};

const long BombJack02[] PROGMEM =
{
  0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x333366,
  0x333366, 0x333366, 0x333366, 0x0099ff, 0x333366, 0x333366, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x333366, 0x333366, 0x0099ff, 0x333366, 0x333366,
  0x333366, 0x333366, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x0099ff, 0x333366, 0x333366, 0x333366,
  0x333366, 0x333366, 0x333366, 0x0099ff, 0x0099ff, 0x0099ff, 0xffffff, 0x000000, 0x0099ff, 0x000000, 0xffffff, 0x0099ff, 0x0099ff, 0x0099ff, 0x333366, 0x333366,
  0x333366, 0x333366, 0x333366, 0x0099ff, 0x0099ff, 0xffffff, 0x000000, 0x0099ff, 0x000000, 0xffffff, 0x0099ff, 0x0099ff, 0x333366, 0x333366, 0x333366, 0x333366,
  0x333366, 0x333366, 0x000000, 0x000000, 0x333366, 0x0099ff, 0xffffff, 0xffffff, 0xffcc99, 0xffffff, 0xffffff, 0x0099ff, 0x333366, 0x000000, 0x000000, 0x333366,
  0x333366, 0x000000, 0x000000, 0xff0000, 0xffcc99, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xffcc99, 0xff0000, 0x000000, 0x000000, 0x333366, 0x333366,
  0x333366, 0x333366, 0x333366, 0xff0000, 0xff0000, 0xffffff, 0xffcc99, 0xffcc99, 0xffcc99, 0xffcc99, 0xffcc99, 0xffffff, 0xff0000, 0xff0000, 0x333366, 0x333366,
  0x333366, 0x333366, 0x333366, 0xff0000, 0xff0000, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xff0000, 0xff0000, 0xffffff, 0x333366, 0x333366, 0x333366,
  0x333366, 0x333366, 0xffffff, 0xffffff, 0xffffff, 0xff0000, 0xff0000, 0x000000, 0xff0000, 0x000000, 0xff0000, 0xff0000, 0xffffff, 0xffffff, 0x333366, 0x333366,
  0x333366, 0x333366, 0xffffff, 0xffffff, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xffffff, 0xffffff, 0xffffff, 0x333366, 0x333366,
  0x333366, 0x333366, 0xffffff, 0xffffff, 0xffffff, 0x0099ff, 0x0099ff, 0x0099ff, 0xffff00, 0x0099ff, 0x0099ff, 0x0099ff, 0xffffff, 0xffffff, 0xffffff, 0x333366,
  0x333366, 0xffffff, 0xffffff, 0xffffff, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0x333366,
  0x333366, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xffffff, 0xffffff, 0xffffff, 0xffffff,
  0x333366, 0x333366, 0x333366, 0xffffff, 0xffffff, 0xff0000, 0xff0000, 0xffffff, 0xff0000, 0xff0000, 0xffffff, 0x333366, 0xffffff, 0xffffff, 0x333366, 0x333366,
  0x333366, 0x333366, 0x333366, 0x333366, 0x333366, 0x000000, 0x000000, 0x000000, 0x333366, 0x000000, 0x000000, 0x000000, 0x333366, 0x333366, 0x333366, 0x333366
};

// SETUP
void setup() {
  // Serial monitor start, can be disabled to slightly improve efficiency of program
  Serial.begin(9600);
  Serial.println("Program test");

  // IR startup
  irrecv.enableIRIn();

  // LED startup
  FastLED.addLeds<LED_TYPE, DATA_PIN_LED, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  // Start interrupt
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), isr, RISING); // RISING, FALLING, CHANGE

  // Audio setup, not really used
  analogReference(DEFAULT); // Set aref voltage to default (5V)

  // Potentiometer setups
  pinMode(BRIGHTNESS_PIN, INPUT); // Set pin to input
  pinMode(SPEED_PIN, INPUT); // Set pin to input

  // Set random seed in order to randomize differently each time the Arduino is run
  randomSeed(analogRead(0));
  progRunning = true;
}

// LOOP
void loop() {
  progRunning = true;
  if (irrecv.decode(&results)) {
    translateIR();
    irrecv.resume();
  }
}

// Update brightness and program speed based on the potentiometers
void check() {
  FastLED.setBrightness(map(analogRead(BRIGHTNESS_PIN), 0, 1023, 255, 0));
  progSpeed = map(analogRead(SPEED_PIN), 0, 1023, 1, 20);
}

// Interrupt service routine activated by the push-button in order to stop currently running program
void isr() { 
  progRunning = false;
}

// Return index of pixel (x,y) in LED matrix
int pos(int x, int y) { 
  return WIDTH * constrain(y, 0, WIDTH - 1) + constrain(x, 0, HEIGHT - 1); // Make sure x and y values aren't out of bounds
}

// Run program based on which button is pushed on IR remote
void translateIR() { 
  irrecv.resume();
  switch (results.value) {

    case 0xFF629D: lastCode = 0xFF629D; break;                    // FORWARD
    case 0xFF22DD: lastCode = 0xFF22DD; break;                    // LEFT
    case 0xFF02FD: lastCode = 0xFF02FD; presentation(); break;    // OK
    case 0xFFC23D: lastCode = 0xFFC23D; break;                    // RIGHT
    case 0xFFA857: lastCode = 0xFFA857; break;                    // REVERSE
    case 0xFF6897: lastCode = 0xFF6897; testMatrixPot();   break; // 1
    case 0xFF9867: lastCode = 0xFF9867; sinePulse(); break;       // 2
    case 0xFFB04F: lastCode = 0xFFB04F; sparkleMatrix(); break;   // 3
    case 0xFF30CF: lastCode = 0xFF30CF; simplexNoise(); break;    // 4
    case 0xFF18E7: lastCode = 0xFF18E7; strobe2(); break;         // 5
    case 0xFF7A85: lastCode = 0xFF7A85; rain(); break;            // 6
    case 0xFF10EF: lastCode = 0xFF10EF; pictureAnim(); break;     // 7
    case 0xFF38C7: lastCode = 0xFF38C7; rainbow(); break;         // 8
    case 0xFF5AA5: lastCode = 0xFF5AA5; beatsinTest(); break;     // 9
    case 0xFF42BD: lastCode = 0xFF42BD; break;                    // *
    case 0xFF4AB5: lastCode = 0xFF4AB5; resetStrip();   break;    // 0
    case 0xFF52AD: lastCode = 0xFF52AD; break;                    // #
    case 0xFFFFFFFF: lastCode = 0xFFFFFFFF; break;                // REPEAT

    default:
      break;
  }
  FastLED.delay(20); // Sanity delay, can be removed
}

// Test the LED matrix display. The amount of LEDs lit up is based on the speed potentiometer
void testMatrixPot() {
  progRunning = true;
  while (progRunning) {
    FastLED.clear();
    int val = map(analogRead(SPEED_PIN), 0, 1023, NUM_LEDS, 0);
    for (int i = 0; i < val; ++i) {
      leds[i] = CRGB::White;
      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
    }
    if (irrecv.decode(&results) && results.value != lastCode) {
      progRunning = false;
      return;
    }
    else if (irrecv.isIdle()) {
      FastLED.show();
    }
    check();
  }
}

// Random LEDs are lit up in a twinkling star formation with a random hue
void sparkleMatrix() {
  progRunning = true;
  while (progRunning) {
    EVERY_N_MILLISECONDS(50) {
      int x = random8(WIDTH);
      int y = random8(HEIGHT);
      int hue = random8();
      leds[pos(x - 1, y)] = CHSV(hue, 255, 128);
      leds[pos(x + 1, y)] = CHSV(hue, 255, 128);
      leds[pos(x, y - 1)] = CHSV(hue, 255, 128);
      leds[pos(x, y + 1)] = CHSV(hue, 255, 128);
      leds[pos(x, y)] = CHSV(hue, 255, 255);

      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
      else if (irrecv.isIdle()) {
        FastLED.show();
      }
    }
    EVERY_N_MILLISECONDS(20) {
      fadeToBlackBy(leds, NUM_LEDS, 7);

      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
      else if (irrecv.isIdle()) {
        FastLED.show();
      }
      check();
    }
    if (irrecv.decode(&results)) {
      progRunning = false;
    }
  }
}

// Full-screen lighting effect based upon a sin(x)sin(y)-scalar field
void sinePulse() {
  progRunning = true;
  float t = 0;
  while (progRunning) {
    for (int y = 0; y < HEIGHT; ++y) {
      for (int x = 0; x < WIDTH; ++x) {
        uint8_t hue = sin8(map(1.5 * y, 0, HEIGHT, 0, 2 * PI) + t) * sin8(map(1.5 * x, 0, WIDTH, 0, 2 * PI) + t);
        leds[pos(x, y)] = CHSV(hue, 255, 255);
      }
    }

    if (irrecv.decode(&results) && results.value != lastCode) {
      progRunning = false;
      return;
    }
    else if (irrecv.isIdle()) {
      FastLED.show();
    }
    check();
    t += 0.013 * progSpeed;
    if (t > 10 * PI) {
      t = 0;
    }
  }
}

// Randomized noise field based upon Ken Perlin's Simplex algorithm. A three-dimentional scalar field with the scalar values mapped to the hue, the three dimentions being x, y and time
void simplexNoise() {
  uint16_t t = 0;
  while (progRunning) {
    for (int y = 0; y < HEIGHT; ++y) {
      for (int x = 0; x < WIDTH; ++x) {
        uint8_t hue = inoise8(50 * x, 50 * y, t);
        leds[pos(x, y)] = CHSV(hue, 255, 255);
        if (irrecv.decode(&results) && results.value != lastCode) {
          progRunning = false;
          return;
        }
      }
    }

    if (irrecv.decode(&results) && results.value != lastCode) {
      progRunning = false;
      return;
    }
    else if (irrecv.isIdle()) {
      FastLED.show();
    }
    check();
    t += progSpeed;                  // How fast the Simplex noise / Perlin noise field changes

  }
}

// Old strobe program, depends on delay-statemens which is unfortunate
void strobe() {
  while (progRunning) {
    for (int i = 0; i < NUM_LEDS; ++i) {
      leds[i] = CRGB::White;
      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
    }
    FastLED.show();
    check();
    FastLED.delay(4 * progSpeed);
    for (int i = 0; i < NUM_LEDS; ++i) {
      leds[i] = CRGB::Black;
      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
    }
    if (irrecv.decode(&results) && results.value != lastCode) {
      progRunning = false;
      return;
    }
    else if (irrecv.isIdle()) {
      check();
      FastLED.show();
    }
    FastLED.delay(4 * progSpeed);
  }
}

// Strobe function not perfectly working as the light pulse duration cannot be changed on-the-fly
void strobe2() {
  progRunning = true;
  bool state = true;
  while (progRunning) {
    EVERY_N_MILLISECONDS(50) {
      if (state) {
        for (int i = 0; i < NUM_LEDS; ++i) {
          leds[i] = CRGB::White;
          if (irrecv.decode(&results) && results.value != lastCode) {
            progRunning = false;
            return;
          }
        }
      } else {
        for (int i = 0; i < NUM_LEDS; ++i) {
          leds[i] = CRGB::Black;
          if (irrecv.decode(&results) && results.value != lastCode) {
            progRunning = false;
            return;
          }
        }
      }

      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
      else if (irrecv.isIdle()) {
        check();
        FastLED.show();
      }
      state = !state;
    }
  }
}

// Nothing to write home about, really not sure why we kept this program
void rain() {
  progRunning = true;
  while (progRunning) {
    EVERY_N_MILLISECONDS(50) {
      int limit = random8();
      if (limit > map(progSpeed, 1, 20, 100, 250)) {
        int x = random8(WIDTH);
        for (int y = HEIGHT - 1; y >= 0; --y) {
          leds[pos(x, y)] = CRGB::Aqua;

          if (irrecv.decode(&results) && results.value != lastCode) {
            progRunning = false;
            return;
          }
          else if (irrecv.isIdle()) {
            FastLED.show();
          }
        }
      }
    }
    EVERY_N_MILLISECONDS(10) {
      fadeToBlackBy(leds, NUM_LEDS, 25);

      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
      else if (irrecv.isIdle()) {
        FastLED.show();
      }
      check();
    }
  }
}

// Quick program to set all LEDs to black, could also just use build-in FastLED.clear();
void resetStrip() { 
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  check();
}

// Test-function to light the leds horizonally in order to check for wiring-issues and other possible problems
void testHoriz() {
  resetStrip();
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = CRGB::White;

    if (irrecv.decode(&results) && results.value != lastCode) {
      progRunning = false;
      return;
    }
    else if (irrecv.isIdle()) {
      FastLED.show();
    }
    check();
    FastLED.delay(2 * progSpeed);
  }
}

// Radial rainbow emerging from bottom-left, with the x^2+y^2-value mapped to the hue of the LED giving a circular rainbow-esque look
void rainbow() {
  progRunning = true;
  float t = 0;
  while (progRunning) {
    for (int x = 0; x < WIDTH; ++x) {
      for (int y = 0; y < HEIGHT; ++y) {
        leds[pos(x, y)] = CHSV(map(x * x + y * y, 0, WIDTH * WIDTH + HEIGHT * HEIGHT, 0, 255) + t, 255, 255);
        if (irrecv.decode(&results) && results.value != lastCode) {
          progRunning = false;
          return;
        }
      }
    }
    t += 0.75 * progSpeed;

    if (irrecv.decode(&results) && results.value != lastCode) {
      progRunning = false;
      return;
    }
    else if (irrecv.isIdle()) {
      FastLED.show();
    }
    check();
  }
}

// Three seperate particles moving in seemingly random patterns with variable speed. The hues are 120deg phase changed, and all iterating through hues in range 0 to 255
void beatsinTest() {
  progRunning = true;
  uint8_t x1, y1, x2, y2, x3, y3, hue = 0;
  while (progRunning) {
    x1 = beatsin8(3 * progSpeed, 0, WIDTH, 0, 60);
    y1 = beatsin8(2 * progSpeed, 0, HEIGHT, 0, 40);
    x2 = beatsin8(3.4 * progSpeed, 0, WIDTH, 0, 0);
    y2 = beatsin8(2.5 * progSpeed, 0, HEIGHT, 0, 0);
    x3 = beatsin8(3.5 * progSpeed, 0, WIDTH, 0, 25);
    y3 = beatsin8(1.8 * progSpeed, 0, HEIGHT, 0, 70);
    leds[pos(x1, y1)] = CHSV(hue, 255, 255);
    leds[pos(x2, y2)] = CHSV(hue + 85, 255, 255);
    leds[pos(x3, y3)] = CHSV(hue + 170, 255, 255);

    ++hue;

    if (irrecv.decode(&results) && results.value != lastCode) {
      progRunning = false;
      return;
    }
    else if (irrecv.isIdle()) {
      FastLED.show();
    }
    fadeToBlackBy(leds, NUM_LEDS, 15);
    check();
  }
}

// Animates the previously declared pictures from Flash memory. 
// The included six pictures are a bit weirdly stored (every other line of colors is backwards) resulting in a rather inelegant coding solution
void pictureAnim() {
  progRunning = true;
  while (progRunning) {
    for (int iterate = 0; iterate < 8; ++iterate) {
      for (int y = HEIGHT - 1; y >= 0; --y) {
        if (y % 2 == 1) {
          for (int x = 0; x < WIDTH; ++x) {
            leds[pos(x, map(y, 0, 15, 15, 0))] = pgm_read_dword(&(Qbert01[pos(x, y)]));
            if (irrecv.decode(&results) && results.value != lastCode) {
              progRunning = false;
              return;
            }
            if (!progRunning) {
              return;
            }
          }
        }
        else {
          for (int x = WIDTH - 1; x >= 0; --x) {
            leds[pos(x, map(y, 0, 15, 15, 0))] = pgm_read_dword(&(Qbert01[pos(map(x, 15, 0, 0, 15), y)]));
            if (irrecv.decode(&results) && results.value != lastCode) {
              progRunning = false;
              return;
            }
            if (!progRunning) {
              return;
            }
          }
        }
      }

      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
      else if (irrecv.isIdle()) {
        FastLED.show();
        check();
      }
      FastLED.delay(500);

      for (int y = HEIGHT - 1; y >= 0; --y) {
        if (y % 2 == 1) {
          for (int x = 0; x < WIDTH; ++x) {
            leds[pos(x, map(y, 0, 15, 15, 0))] = pgm_read_dword(&(Qbert02[pos(x, y)]));
            if (irrecv.decode(&results) && results.value != lastCode) {
              progRunning = false;
              return;
            }
            if (!progRunning) {
              return;
            }
          }
        }
        else {
          for (int x = WIDTH - 1; x >= 0; --x) {
            leds[pos(x, map(y, 0, 15, 15, 0))] = pgm_read_dword(&(Qbert02[pos(map(x, 15, 0, 0, 15), y)]));
            if (irrecv.decode(&results) && results.value != lastCode) {
              progRunning = false;
              return;
            }
            if (!progRunning) {
              return;
            }
          }
        }
      }
      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
      else if (irrecv.isIdle()) {
        FastLED.show();
        check();
      }
      FastLED.delay(500);
    }
    for (int iterate = 0; iterate < 8; ++iterate) {
      for (int y = HEIGHT - 1; y >= 0; --y) {
        if (y % 2 == 1) {
          for (int x = 0; x < WIDTH; ++x) {
            leds[pos(x, map(y, 0, 15, 15, 0))] = pgm_read_dword(&(DigDug01[pos(x, y)]));
            if (irrecv.decode(&results) && results.value != lastCode) {
              progRunning = false;
              return;
            }
            if (!progRunning) {
              return;
            }
          }
        }
        else {
          for (int x = WIDTH - 1; x >= 0; --x) {
            leds[pos(x, map(y, 0, 15, 15, 0))] = pgm_read_dword(&(DigDug01[pos(map(x, 15, 0, 0, 15), y)]));
            if (irrecv.decode(&results) && results.value != lastCode) {
              progRunning = false;
              return;
            }
            if (!progRunning) {
              return;
            }
          }
        }
      }

      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
      else if (irrecv.isIdle()) {
        FastLED.show();
        check();
      }
      FastLED.delay(500);

      for (int y = HEIGHT - 1; y >= 0; --y) {
        if (y % 2 == 0) {
          for (int x = 0; x < HEIGHT; ++x) {
            leds[pos(x, map(y, 0, 15, 15, 0))] = pgm_read_dword(&(DigDug02[pos(x, y)]));
            if (irrecv.decode(&results) && results.value != lastCode) {
              progRunning = false;
              return;
            }
            if (!progRunning) {
              return;
            }
          }
        }
        else {
          for (int x = WIDTH - 1; x >= 0; --x) {
            leds[pos(x, map(y, 0, 15, 15, 0))] = pgm_read_dword(&(DigDug02[pos(map(x, 15, 0, 0, 15), y)]));
            if (irrecv.decode(&results) && results.value != lastCode) {
              progRunning = false;
              return;
            }
            if (!progRunning) {
              return;
            }
          }
        }
      }
      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
      else if (irrecv.isIdle()) {
        FastLED.show();
        check();
      }
      FastLED.delay(500);
    }
    for (int iterate = 0; iterate < 8; ++iterate) {
      for (int y = HEIGHT - 1; y >= 0; --y) {
        if (y % 2 == 0) {
          for (int x = 0; x < WIDTH; ++x) {
            leds[pos(x, map(y, 0, 15, 15, 0))] = pgm_read_dword(&(BombJack01[pos(x, y)]));
            if (irrecv.decode(&results) && results.value != lastCode) {
              progRunning = false;
              return;
            }
            if (!progRunning) {
              return;
            }
          }
        }
        else {
          for (int x = WIDTH - 1; x >= 0; --x) {
            leds[pos(x, map(y, 0, 15, 15, 0))] = pgm_read_dword(&(BombJack01[pos(map(x, 15, 0, 0, 15), y)]));
            if (irrecv.decode(&results) && results.value != lastCode) {
              progRunning = false;
              return;
            }
            if (!progRunning) {
              return;
            }
          }
        }
      }

      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
      else if (irrecv.isIdle()) {
        FastLED.show();
        check();
      }
      FastLED.delay(500);

      for (int y = HEIGHT - 1; y >= 0; --y) {
        if (y % 2 == 0) {
          for (int x = 0; x < HEIGHT; ++x) {
            leds[pos(x, map(y, 0, 15, 15, 0))] = pgm_read_dword(&(BombJack02[pos(x, y)]));
            if (irrecv.decode(&results) && results.value != lastCode) {
              progRunning = false;
              return;
            }
            if (!progRunning) {
              return;
            }
          }
        }
        else {
          for (int x = WIDTH - 1; x >= 0; --x) {
            leds[pos(x, map(y, 0, 15, 15, 0))] = pgm_read_dword(&(BombJack02[pos(map(x, 15, 0, 0, 15), y)]));
            if (irrecv.decode(&results) && results.value != lastCode) {
              progRunning = false;
              return;
            }
            if (!progRunning) {
              return;
            }
          }
        }
      }
      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
      else if (irrecv.isIdle()) {
        FastLED.show();
        check();
      }
      FastLED.delay(500);
    }
  }
}

// Quick program to run four of the above lighting programs. This is perhaps the definition of spaghetti-code
void presentation() {
  unsigned long start = millis();

  progRunning = true;
  while (progRunning) {
    unsigned long start = millis();
    long duration = 8000;

    int tSimp = 0;
    while (millis() < start + 1 * duration && progRunning) { // Simplex noise
      for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
          uint8_t hue = inoise8(50 * x, 50 * y, tSimp);
          leds[pos(x, y)] = CHSV(hue, 255, 255);
          if (irrecv.decode(&results) && results.value != lastCode) {
            progRunning = false;
            return;
          }
        }
      }
      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
      else if (irrecv.isIdle()) {
        FastLED.show();
      }
      check();
      tSimp += progSpeed;
    }


    uint8_t x1, y1, x2, y2, x3, y3, hue = 0;
    while (millis() < start + 2 * duration && progRunning) { // Beatsin
      x1 = beatsin8(3 * progSpeed, 0, WIDTH, 0, 60);
      y1 = beatsin8(2 * progSpeed, 0, HEIGHT, 0, 40);
      x2 = beatsin8(3.4 * progSpeed, 0, WIDTH, 0, 0);
      y2 = beatsin8(2.5 * progSpeed, 0, HEIGHT, 0, 0);
      x3 = beatsin8(3.5 * progSpeed, 0, WIDTH, 0, 25);
      y3 = beatsin8(1.8 * progSpeed, 0, HEIGHT, 0, 70);
      leds[pos(x1, y1)] = CHSV(hue, 255, 255);
      leds[pos(x2, y2)] = CHSV(hue + 85, 255, 255);
      leds[pos(x3, y3)] = CHSV(hue + 170, 255, 255);

      ++hue;

      // FastLED.delay(2 * progSpeed);
      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
      else if (irrecv.isIdle()) {
        FastLED.show();
      }
      fadeToBlackBy(leds, NUM_LEDS, 15);
      check();
    }


    float tSine = 0;
    while (millis() < start + 3 * duration && progRunning) { // Sine pulse
      for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
          uint8_t hue = sin8(map(1.5 * y, 0, HEIGHT, 0, 2 * PI) + tSine) * sin8(map(1.5 * x, 0, WIDTH, 0, 2 * PI) + tSine);
          leds[pos(x, y)] = CHSV(hue, 255, 255);
          if (irrecv.decode(&results) && results.value != lastCode) {
            progRunning = false;
            return;
          }
        }
      }
      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
      else if (irrecv.isIdle()) {
        FastLED.show();
      }
      check();
      tSine += 0.013 * progSpeed;
      if (tSine > 10 * PI) {
        tSine = 0;
      }
    }


    float tRainbow = 0;
    while (millis() < start + 4 * duration && progRunning) { // Radial rainbow
      for (int x = 0; x < WIDTH; ++x) {
        for (int y = 0; y < HEIGHT; ++y) {
          leds[pos(x, y)] = CHSV(map(x * x + y * y, 0, WIDTH * WIDTH + HEIGHT * HEIGHT, 0, 255) + tRainbow, 255, 255);
          if (irrecv.decode(&results) && results.value != lastCode) {
            progRunning = false;
            return;
          }
        }
      }
      tRainbow += 0.75 * progSpeed;
      check();
      if (irrecv.decode(&results) && results.value != lastCode) {
        progRunning = false;
        return;
      }
      else if (irrecv.isIdle()) {
        FastLED.show();
      }
    }
  }
}

