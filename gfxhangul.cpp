class hangul {
  private:
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

    void print_char(char ascii, uint16_t x, uint16_t y, uint16_t pixel) {
      int ptr = ((int) ascii) * 16; // Pointer used to read font data

      for (int i=0; i<16; i++) {
        drawByte(font_8x16[ptr], x, y, pixel); // Hopefully self-explanatory
        ptr++; y++; // Increment data and rendering pointer
      }

      return;
    }

    uint8_t get_byte_type(unsigned char byte) {
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

    uint8_t render_beol(uint8_t col, uint8_t line, uint16_t x, uint16_t y, uint16_t pixel) {
      int i, off1, off2 = 0;
      
      for (i=0; i<32; i++) { // Size of hangul is 16 * 2 bytes
        off1 = 64 * (i % 16); // 512 / 8 (1byte) = 64, (i/2) [Increments every 2 cycles]
        off2 = i/16 * 8; // I can't believe this single error made me struggle for weeks and months... (actually more like bcs of me abandoning it lol)

        drawByte(font_han1[1024*line + col*2 + off1 + i/16], x+off2, y+i%16, pixel);
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
  public:
    uint16_t* convert_utf(char* byte) { // now more compact and efficient!
      unsigned int valtmp, val_list_ptr = 0;
      uint8_t b_type = 5;
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

      for (i=0; i<len; i++) { // Main loop
        byte_ = *(byte+i);

        b_type = get_byte_type(byte_); // Get type of current byte

        if (b_type == 0) // Redirect the raw value if the char is ordinary ascii char
          valtmp = byte_; // Write value to array

        if (b_type > 1 && b_type < 5) {
          valtmp = (byte_ & (0x1f >> (b_type - 2) ) ) << 6; // Bit mask lower 5 bit and shift 6 bits to left

          for (int j=0; j<b_type; j++) { // Shift temp val by 6 bits and increment it by value of extension bytes
            valtmp = valtmp << 6;
            valtmp += *(byte+i+j) & 0x3f;
          }

          i += b_type - 1; // Increment reading ptr
        }

        *(val_list+val_list_ptr) = valtmp; // Write value to array
        alloc_ptr += 2; // Increment allocation pointer
        val_list = (uint16_t*) realloc(val_list, alloc_ptr+2); // Reallocate dynamic var so it matches length of string
        val_list_ptr++; // Increment writing ptr
        valtmp = 0; // Reset temporary variable
      }

      *(val_list+val_list_ptr) = 0x0000; // Set last char to null so its length can be calculated

      return val_list;
    }

    uint8_t print_unicode(char* byte, uint16_t x, uint16_t y, uint16_t pixel) {
      uint16_t* str = convert_utf(byte); // Convert utf8 byte stream into utf16 string
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

        if (cur_char >= 0xac00 && cur_char <= 0xd7a3) { // Check if current character is hangul
          isHangul = true;   isAscii = false;
        } else if (cur_char < 0x100) { // Check if current character is ascii
          isHangul = false;  isAscii = true;
        } else
          isAscii = false;

        if (isHangul) {
          rb = get_renderer_beol(cur_char); // Calculate appropriate beol nums
          render_beol(*(rb+3), *rb, x+xptr, y+yptr, pixel); // Render beol 1
          render_beol(*(rb+4), 8 + *(rb+1), x+xptr, y+yptr, pixel); // Render beol 2
          render_beol(*(rb+5), 12 + *(rb+2), x+xptr, y+yptr, pixel); // Render beol 3
          xptr += 16; // Increment x ptr by 16 (equivalent to 2 letters)

          free(rb); // Free dynamic var to prevent memory leakage
        }

        if (isAscii && cur_char != 0x0a && cur_char != 0x0d) { // If current char is ascii and isn't cr or lf
          print_char(cur_char, x+xptr, y+yptr, pixel); // Print corresponding char
          xptr += 8; // and increment x ptr by 16
        } 

        if (cur_char == 0x0a) { // Line Feed
          yptr += 16;
        } else if (cur_char == 0x0d) { // Carriage return
          xptr = 0;
        }
      }
      
      free(str); // Free dynamic var to prevent mem. leak

      return 0; // Return to main routine to prevent it from looping by itself
    }
}