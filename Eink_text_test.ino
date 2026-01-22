#include "Adafruit_ThinkInk.h"
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

#ifdef ARDUINO_ADAFRUIT_FEATHER_RP2040_THINKINK // detects if compiling for
                                                // Feather RP2040 ThinkInk
#define EPD_DC PIN_EPD_DC       // ThinkInk 24-pin connector DC
#define EPD_CS PIN_EPD_CS       // ThinkInk 24-pin connector CS
#define EPD_BUSY PIN_EPD_BUSY   // ThinkInk 24-pin connector Busy
#define SRAM_CS -1              // use onboard RAM
#define EPD_RESET PIN_EPD_RESET // ThinkInk 24-pin connector Reset
#define EPD_SPI &SPI1           // secondary SPI for ThinkInk
#else
#define EPD_DC 9
#define EPD_CS 6
#define EPD_BUSY 7 // can set to -1 to not use a pin (will wait a fixed delay)
#define SRAM_CS -1
#define EPD_RESET 8  // can set to -1 and share with microcontroller Reset!
#define EPD_SPI &SPI // primary SPI
#endif

 ThinkInk_213_Mono_GDEY0213B74 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY, EPD_SPI);



void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println("Eink text test started");
  display.begin(THINKINK_MONO);
}

void loop() {

  Serial.println("Text demo");
  // large block of text
  display.clearBuffer();

  display.setFont(&FreeSansBold9pt7b);
  display.setTextSize(1);
  display.setTextWrap(true);
  display.setTextColor(EPD_BLACK);
  display.setCursor(30, 30); // Start location of text
  display.println("Next bus in 0:35 Mins");

  display.setFont(&FreeSans9pt7b);
  display.setTextColor(EPD_DARK);
  display.setCursor(60, 50);
  display.println("at 4:33:02PM");

  display.setFont(&FreeSansBold9pt7b);
  display.setTextColor(EPD_BLACK);
  display.setCursor(40, 80);
  display.println("Oxt bus in 12 Mins");

  display.setFont(&FreeSans9pt7b);
  display.setTextColor(EPD_DARK);
  display.setCursor(60, 100);
  display.println("at 4:45:16PM");
  display.display();

  delay(2000);

  display.clearBuffer();
  display.display();

  delay(2000);
}


