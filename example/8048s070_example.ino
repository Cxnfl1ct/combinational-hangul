#include <gfxhangul.h>
#include <Arduino_GFX_Library.h>
#include <Arduino_GFX.h>

#define GFX_BL DF_GFX_BL
#define TFT_BL 2

hangul hr;

// Included from example code for ESP32-8048S070

Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    GFX_NOT_DEFINED /* CS */, GFX_NOT_DEFINED /* SCK */, GFX_NOT_DEFINED /* SDA */,
    41 /* DE */, 40 /* VSYNC */, 39 /* HSYNC */, 42 /* PCLK */,
    14 /* R0 */, 21 /* R1 */, 47 /* R2 */, 48 /* R3 */, 45 /* R4 */,
    9 /* G0 */, 46 /* G1 */, 3 /* G2 */, 8 /* G3 */, 16 /* G4 */, 1 /* G5 */,
    15 /* B0 */, 7 /* B1 */, 6 /* B2 */, 5 /* B3 */, 4 /* B4 */
);

Arduino_RPi_DPI_RGBPanel *gfx = new Arduino_RPi_DPI_RGBPanel(bus,
//  800 /* width */, 0 /* hsync_polarity */, 8/* hsync_front_porch */, 2 /* hsync_pulse_width */, 43/* hsync_back_porch */,
//  480 /* height */, 0 /* vsync_polarity */, 8 /* vsync_front_porch */, 2/* vsync_pulse_width */, 12 /* vsync_back_porch */,
//  1 /* pclk_active_neg */, 16000000 /* prefer_speed */, true /* auto_flush */);

    800 /* width */, 0 /* hsync_polarity */, 210 /* hsync_front_porch */, 30 /* hsync_pulse_width */, 16 /* hsync_back_porch */,
    480 /* height */, 0 /* vsync_polarity */, 22 /* vsync_front_porch */, 13 /* vsync_pulse_width */, 10 /* vsync_back_porch */,
    1 /* pclk_active_neg */, 16000000 /* prefer_speed */, true /* auto_flush */);

void main_prog() {
  int i;
  char* str = (char*)malloc(2);

  for (i=0; ;i++) {
    *str = i % 128;
    *(str+1) = 0;
    hr.print("한글 안녕하세요 1231a\r\n테스트 asdf가나다라\r\nüber alles", 20, 20, i);
    hr.print(str, 156 % 800, 36, i);
  }
}

void setup() {
  gfx->begin();
  gfx->fillScreen(0x0000); // Reset screen

  Serial.begin(115200);

  #ifdef TFT_BL
    pinMode(TFT_BL, OUTPUT); // Turn on TFT backlight if it is defined
    digitalWrite(TFT_BL, HIGH);
  #endif

  Serial.printf("System init\n");

  main_prog();
}

void loop() {}
