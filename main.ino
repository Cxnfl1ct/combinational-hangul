#include "font_hangul.h"
#include "font_ascii.h"
#include <Arduino_GFX_Library.h>
#include <Arduino_GFX.h>

// Library and font includes

// Hardware specific part (Remove it and replace it with your own configuration)

#define GFX_BL DF_GFX_BL
#define TFT_BL 2

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

// Used beol rules from https://bakyeono.net/post/2013-12-03-clojure-hangul-bitmap-font.html
// 제작하는 데 위의 글을 참고하였습니다.

// End of hardware specific part

void drawByte(uint8_t byte, uint16_t x, uint16_t y, uint16_t pixel) {
  uint8_t dat = byte;
  if ( (dat & 0x80) == 0) gfx->drawPixel(x, y, pixel); // Most significant bit (2^7)
  if ( (dat & 0x40) == 0) gfx->drawPixel(x+1, y, pixel);
  if ( (dat & 0x20) == 0) gfx->drawPixel(x+2, y, pixel);
  if ( (dat & 0x10) == 0) gfx->drawPixel(x+3, y, pixel);
  if ( (dat & 0x08) == 0) gfx->drawPixel(x+4, y, pixel);
  if ( (dat & 0x04) == 0) gfx->drawPixel(x+5, y, pixel);
  if ( (dat & 0x02) == 0) gfx->drawPixel(x+6, y, pixel);
  if ( (dat & 0x01) == 0) gfx->drawPixel(x+7, y, pixel); // Least significant bit (2^0)

  return;
}

void print_char(char ascii, uint16_t x, uint16_t y, uint16_t color)
{
  int ptr = ((int) ascii) * 16; // Pointer used to read font data

  for (int i=0; i<16; i++) {
    drawByte(font_8x16[ptr], x, y, color); // Hopefully self-explanatory
    ptr++; y++; // Increment data and rendering pointer
  }

  return;
}

uint16_t get_byte_type(unsigned char byte) {
	if (byte < 0x80) // Regular ascii char
		return 0;
	if (byte < 0xc0) // Extension byte
		return 1;
	if (byte < 0xe0) // U+0080 - U+07FF Starting byte
		return 2;
	if (byte < 0xf0) // U+0800 - U+FFFF Starting byte
		return 3;
	if (byte <= 0xf0) // U+010000 - U+10FFFF Starting byte
		return 4;

	return 5; // Invalid byte
}

uint16_t* utf8_to_utf16(char* byte) { // this kinda feels like cisc instruction decoder...
	unsigned int valtmp, load_cnt, val_list_ptr = 0;
	uint16_t current_type = 5;
	int i, len, alloc_ptr = 0;
	char byte_;
	uint16_t* val_list; // Init variables

	for (i=0; ;i++)
		if (*(byte+i) == 0) { // If current char is 0 (nul char) copy current cnter and break
			len = i;
			break;
		}

	val_list = (uint16_t*)malloc(2); // Init dynamic list

	if (val_list == NULL) {
		return val_list; // If memory allocation fails, throw NULL and end
	}

	for (i=0; i<len; i++) {
		byte_ = *(byte+i); // Load dynamic str onto var

		if (current_type == 5) // If this is beginning of decoding, set load counter to temp value
			load_cnt = 4;

		current_type = get_byte_type(byte_); // Get type of current byte

		if (current_type > 1 && load_cnt == 4) // If it's beginning of character initialize load cnt
			load_cnt = current_type - 1;

		if (current_type == 0) { // Redirect the raw value if the char is ordinary ascii char
      valtmp = byte_; // Output the char to working var
    }

		if (current_type == 1) { // If current byte is ext. byte
			valtmp += byte_ & 0x3F; // Load lower 6 bits into temp var

			if (load_cnt > 1)
				valtmp = valtmp << 6; // shift 6 bits to left unless it's last byte of char.

			load_cnt--; // Decrease load cnt.
		} else {
      alloc_ptr += 2; // Increment allocation pointer
      val_list = (uint16_t*) realloc(val_list, alloc_ptr+2); // Reallocate dynamic var so it matches length of string
    }

		if (current_type == 2)
			valtmp = (byte_ & 0x1f) << 6; // Bit mask lower 5 bit and shift 6 bits to left

		if (current_type == 3)
			valtmp = (byte_ & 0x0f) << 6; // `` 4 bit ``

		if (current_type == 4)
			valtmp = (byte_ & 0x07) << 6; // `` 3 bit ``

		if (load_cnt < 1 || current_type == 0) { // If this is last byte of char
			*(val_list+val_list_ptr) = valtmp; // Output the char to array
			val_list_ptr++; // and increase output ptr
			load_cnt = 4; // then reset load cnt
			valtmp = 0; // finally, erase temporary var
		}
	}

  *(val_list+val_list_ptr) = 0x0000; // Set last char to null so its length can be calculated

	return val_list;
}

uint8_t get_first_beol(uint8_t joong, uint8_t jong) {
  if (joong < 8 || joong == 20) { // Comparison and comparison, over and over...
    if (jong == 0)
      return 0; // beol #1
    else
      return 5; // beol #6
  }

  if (joong == 8 || joong == 12 || joong == 18) {
    if (jong == 0)
      return 1; // beol #2
    else
      return 6; // beol #7
  }

  if (joong == 13 || joong == 17) {
    if (jong == 0)
      return 2; // beol #3
    else
      return 6; // beol #7
  }

  if ( (joong > 8 && joong < 12) || joong == 19) {
    if (jong == 0)
      return 3; // beol #4
    else
      return 7; // beol #8
  }

  if (joong > 13 && joong < 17) {
    if (jong == 0)
      return 4; // beol #5
    else
      return 7; // beol #8
  }
}

uint8_t get_second_beol(uint8_t cho, uint8_t jong) {
  if (cho == 0 || cho == 15) {
    if (jong == 0)
      return 0; // beol #1
    else
      return 2; // beol #3
  } else {
    if (jong == 0)
      return 1; // beol #2
    else
      return 3; // beol #4
  }
}

uint8_t get_last_beol(uint8_t joong) {
  if (joong == 0 || joong == 2 || joong == 9) return 0; // beol #1
  if (joong == 4 || joong == 6 || joong == 11 
   || joong == 14 || joong == 16 || joong == 19 || joong == 20)
    return 1; // beol 2
  if ((joong % 2 == 1 && joong < 8) || joong == 10 || joong == 15)
    return 2; // beol 3
  if (joong == 8 || joong == 12 || joong == 13 || joong == 17 || joong == 18)
    return 3; // beol 4
}

uint8_t* get_renderer_beol(uint16_t utf16) {
  uint8_t *lst = (uint8_t*)malloc(6); // Init dynamic var, 6 (total cnt of vals) * 1 (8 bit val)
  uint16_t pure_off = utf16 - 0xac00; // Calculate pure offset of hangul
  uint8_t cho = pure_off / 588; // Get choseong (first sound)
  uint8_t joong = (pure_off % 588) / 28; // Get joongseong (middle sound)
  uint8_t jong = pure_off % 28; // Get jongseong (ending sound)

  *lst     = get_first_beol(joong, jong); // Prepare to return dynamic variable
  *(lst+1) = get_second_beol(cho, jong);
  *(lst+2) = get_last_beol(joong);
  *(lst+3) = cho;
  *(lst+4) = joong;
  *(lst+5) = jong;

  return lst;
}

uint8_t render_beol(uint8_t col, uint8_t line, uint16_t x, uint16_t y, uint16_t color) {
  int i, off1 = 0;
  
  for (i=0; i<32; i++) { // Size of hangul is 16 * 2 bytes
    off1 = 64 * (i/2); // 512 / 8 (1byte) = 64, (i/2) [Increments every 2 cycles]
    drawByte(font_han1[1024*line + off1 + col*2 + i%2], x+(i%2) * 8, y+i/2, color); 
    /* Explanation
    
      Selection offsets:
      512 (width of font image) * 16 = 8192, 8192 / 8 (1byte) = 1024 
      col * 2 (char with 16 pixel has width of 2 bytes)
      i % 2 > Creates zig zag pattern 

      Coords:
      Minimal unit of X and Y coord is 1
      so, you'll have to multiply 8 for x field

      To create zig-zag pattern, the y field needs to be incremented every 2 cycles 
      */
  }

  return 0;
}

uint8_t print_unicode(char* byte, uint16_t x, uint16_t y, uint16_t color) {
  uint16_t* str = utf8_to_utf16(byte); // Convert utf8 byte stream into utf16 string
  uint8_t* rb;
  int i, len;
  int xptr = 0, yptr = 0;
  bool isHangul, isAscii;
  uint16_t cur_char = 0; // Init vars

  for (i=0; ;i++)
    if (*(str+i) == 0) {
      len = i; i=0;
      break;
    } // Find end of UTF16 string

  for (i=0; i<len; i++) {
    cur_char = *(str+i); // Load char from dynamic var.

    if (cur_char >= 0xac00 && cur_char <= 0xd7a3) // Check if current character is hangul
      isHangul = true;
    else
      isHangul = false;

    if (cur_char < 0x100) // Check if current character is ascii
      isAscii = true;
    else
      isAscii = false;

    if (isHangul) {
      rb = get_renderer_beol(cur_char); // Calculate appropriate beol nums
      render_beol(*(rb+3), *rb, x+xptr, y+yptr, color); // Render beol 1
      render_beol(*(rb+4), 8 + *(rb+1), x+xptr, y+yptr, color); // Render beol 2
      render_beol(*(rb+5), 12 + *(rb+2), x+xptr, y+yptr, color); // Render beol 3
      xptr += 16; // Increment x ptr by 16 (equivalent to 2 letters)

      free(rb); // Free dynamic var to prevent mem. leak
    }

    if (isAscii && cur_char != 0x0a && cur_char != 0x0d) { // If current char is ascii and isn't cr or lf
      print_char(cur_char, x+xptr, y+yptr, color); // Print corresponding char
      xptr += 8; // and increment x ptr by 16
    } 

    if (cur_char == 0x0a) { // Line Feed
      yptr += 16;
    } else if (cur_char == 0x0d) { // Carriage return
      xptr = 0;
    }
  }
  
  free(str); // Free dynamic var to prevent mem. leak

  return 0; // Return to main routine to prevent it from looping
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

  print_unicode("한글 안녕하세요 1231a\r\n테스트", 20, 20, 0xFFFF);
}

void loop() {
  // put your main code here, to run repeatedly:

}
