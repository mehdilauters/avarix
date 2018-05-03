#include "ssd1306_font.h"
#include "ssd1306.h"
#include <avr/pgmspace.h>
#include <string.h>

#define JUMPTABLE_BYTES 4
#define JUMPTABLE_LSB   1
#define JUMPTABLE_SIZE  2
#define JUMPTABLE_WIDTH 3
#define JUMPTABLE_START 4
#define CHAR_NUM_POS 3

uint8_t ssd1306_font_get_width(ssd1306_font_t* _font) {
  return pgm_read_byte(_font + 0);
}

uint8_t ssd1306_font_get_height(ssd1306_font_t* _font) {
  return pgm_read_byte(_font + 1);
}

uint8_t ssd1306_font_get_first_char(ssd1306_font_t* _font) {
  return pgm_read_byte(_font + 2);
}

uint8_t ssd1306_font_get_char_count(ssd1306_font_t* _font) {
  return pgm_read_byte(_font + 3);
}

ssd1306_font_char_t ssd1306_font_char_get(ssd1306_font_t* _font, uint8_t _c) {
  ssd1306_font_char_t char_data;
  char_data.width = 0;
  char_data.height = ssd1306_font_get_height(_font);
  char_data.data = NULL;
  char_data.size = 0;
  char_data.code = _c;
  
  uint16_t sizeOfJumpTable = pgm_read_byte(_font + CHAR_NUM_POS) * JUMPTABLE_BYTES;
  
  if (_c >= ssd1306_font_get_first_char(_font)) {
    _c = _c - ssd1306_font_get_first_char(_font)-1;
    
    // 4 Bytes per char code
        
    uint8_t msb_jump_to_char    = pgm_read_byte( (uint8_t*)_font + JUMPTABLE_START + _c * JUMPTABLE_BYTES );                  // MSB  \ JumpAddress
    uint8_t lsb_jump_to_char    = pgm_read_byte( (uint8_t*)_font + JUMPTABLE_START + _c * JUMPTABLE_BYTES + JUMPTABLE_LSB);   // LSB /
    uint8_t char_byte_size     = pgm_read_byte( (uint8_t*)_font + JUMPTABLE_START + _c * JUMPTABLE_BYTES + JUMPTABLE_SIZE);  // Size
    uint8_t current_char_width = pgm_read_byte( (uint8_t*)_font + JUMPTABLE_START + _c * JUMPTABLE_BYTES + JUMPTABLE_WIDTH); // Width
    
    // Test if the char is drawable
    if (!(msb_jump_to_char == 255 && lsb_jump_to_char == 255)) {
      uint16_t char_data_position = JUMPTABLE_START + sizeOfJumpTable + ((msb_jump_to_char << 8) + lsb_jump_to_char);
      char_data.data = (uint8_t*)_font + char_data_position;
      char_data.width = current_char_width;
      char_data.size = char_byte_size;
    }
  }
  return char_data;
}
