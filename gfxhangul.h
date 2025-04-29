#ifndef GFX_HANGUL
#define GFX_HANGUL

#include "./font/font_ascii.h"
#include "./font/font_hangul.h"

class hangul {
  private:
    void drawByte(uint8_t byte, uint16_t x, uint16_t y, uint16_t pixel);
    void print_char(char ascii, uint16_t x, uint16_t y, uint16_t pixel);
    uint8_t get_byte_type(unsigned char byte);
    uint8_t get_first_beol(uint8_t joong, uint8_t jong);
    uint8_t get_second_beol(uint8_t cho, uint8_t jong);
    uint8_t get_last_beol(uint8_t joong);
    uint8_t* get_renderer_beol(uint16_t utf16);
    uint8_t render_beol(uint8_t col, uint8_t line, uint16_t x, uint16_t y, uint16_t pixel);
  
  public:
    uint16_t* convert_utf(char* byte);
    uint8_t print_unicode(char* byte, uint16_t x, uint16_t y, uint16_t pixel);
}