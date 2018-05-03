#include "ssd1306.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/pgmspace.h>

//General use bit manipulating commands
#define BitSet(x, y) (x |= (1UL<<y))
#define BitClear(x, y) ( x &= (~(1UL<<y)))
#define BitToggle(x, y) ( x ^= (1UL<<y) )
#define BitCheck(x, y) ( x & (1UL<<y) ? 1 : 0 )

#define SSD1306_DC 6
#define SSD1306_LINE_HEIGHT 8


static void ssd1306_send(ssd1306_t *_ssd1306, uint8_t *_buffer, uint8_t _len) {
  i2cm_send(_ssd1306->i2c, _ssd1306->address, _buffer, _len);
}

static void ssd1306_send_command(ssd1306_t *_ssd1306, ssd1306_command_t _command) {
  uint8_t buffer[2];
  buffer[0] = 0 << SSD1306_DC;
  buffer[1] = _command;
  ssd1306_send(_ssd1306, buffer, 2);
}

static void ssd1306_send_command_value(ssd1306_t *_ssd1306, ssd1306_command_t _command, uint8_t _value) {
  ssd1306_send_command(_ssd1306, _command);
  ssd1306_send_command(_ssd1306, (ssd1306_command_t)_value);
}

static void ssd1306_send_data(ssd1306_t *_ssd1306, uint8_t *_data, uint8_t _len) {
  uint8_t buffer[127];
  memset(buffer, 0, 127);
  buffer[0] = 1 << SSD1306_DC;
  for(uint8_t i=0; i < _len; i++) {
    buffer[i+1] = _data[i];
  }
  ssd1306_send(_ssd1306, buffer, _len+1);
}

uint16_t ssd1306_get_buffer_size(ssd1306_t *_ssd1306) {
  return _ssd1306->width * _ssd1306->height * sizeof(uint8_t) / 8;
}

void ssd1306_init(ssd1306_t *_ssd1306, i2cm_t * _i2c, uint8_t _address, uint16_t _width, uint16_t _height) {
  _ssd1306->i2c = _i2c;
  _ssd1306->address = _address;
  _ssd1306->width = _width;
  _ssd1306->height = _height;
  _ssd1306->font = NULL;
  uint16_t size = ssd1306_get_buffer_size(_ssd1306);
  _ssd1306->buffer = (uint8_t *)malloc(size);
  if( _ssd1306->buffer == NULL ) {
    return;
  }
  memset(_ssd1306->buffer, SSD1306_COLOR_BLACK, size);
  ssd1306_send_command(_ssd1306, SSD1306_COMMAND_DISPLAY_OFF);
  ssd1306_send_command_value(_ssd1306, SSD1306_COMMAND_DISPLAY_CLOCK_DIV_RADIO_SET, 0xF0); // suggest ratio
  
  ssd1306_send_command(_ssd1306, SSD1306_COMMAND_MULTIPLEX_RADIO_SET);
  
  ssd1306_send_command(_ssd1306, _ssd1306->height - 1);
  
  ssd1306_send_command_value(_ssd1306, SSD1306_COMMAND_DISPLAY_OFFSET_SET, 0x00); // no offset
  
  ssd1306_send_command_value(_ssd1306, SSD1306_COMMAND_CHARGE_PUMP_SET, 0x14); // enable charge pump
  
  ssd1306_send_command(_ssd1306, SSD1306_COMMAND_DISPLAY_START_LINE_SET | 0x00);
  
  ssd1306_send_command_value(_ssd1306, SSD1306_COMMAND_MEMORY_ADDRESSING_SET, 0x00);
  
  ssd1306_send_command(_ssd1306, SSD1306_COMMAND_SEGMENT_REMAP_SET | 0x01); // left towards right
  
  ssd1306_send_command(_ssd1306, SSD1306_COMMAND_COM_OUTPUT_SCAN_DEC);
  
//   #if (GLCD_Size == GLCD_128_64)
  ssd1306_send_command(_ssd1306, 0x12); //Sequential COM pin configuration
//   #elif (GLCD_Size == GLCD_128x32)
//   ssd1306_send_command_value(_ssd1306, SSD1306_COMMAND_COM_PINS_SET, 0x02); //Alternative COM pin configuration
//   #elif (GLCD_Size == GLCD_96x16)
  //   ssd1306_send_command(_ssd1306, SSD1306_COMMAND_COM_PINS_SET, 0x02); //Alternative COM pin configuration
//   #endif
  
  ssd1306_send_command_value(_ssd1306, SSD1306_COMMAND_CONTRAST_SET, 0xFF);
  
  ssd1306_send_command_value(_ssd1306, SSD1306_COMMAND_PRECHARGE_PERIOD_SET, 0xF1);
  
  ssd1306_send_command_value(_ssd1306, SSD1306_COMMAND_VCOMH_DESELECT_LEVEL_SET, 0x20);
  
  ssd1306_send_command(_ssd1306, SSD1306_COMMAND_DISPLAY_ALL_ON_RESUME);
  
  ssd1306_send_command(_ssd1306, SSD1306_COMMAND_DISPLAY_NORMAL);
  
  ssd1306_send_command(_ssd1306, SSD1306_COMMAND_SCROLL_DEACTIVATE);
  
  ssd1306_send_command(_ssd1306, SSD1306_COMMAND_DISPLAY_ON);
//   ssd1306_send_command(_ssd1306, SSD1306_COMMAND_DISPLAY_INVERSE);  
}

void ssd1306_set_font(ssd1306_t *_ssd1306, const uint8_t *_font) {
  _ssd1306->font = _font;
}

void ssd1306_render(ssd1306_t *_ssd1306) {
  // set columns
  ssd1306_send_command_value(_ssd1306, SSD1306_COMMAND_COLUMN_ADDRESS_SET, 0x0);
  ssd1306_send_command(_ssd1306, _ssd1306->width -1);
  
  // set rows
  ssd1306_send_command_value(_ssd1306, SSD1306_COMMAND_PAGE_ADDRESS_SET, 0x0);
  uint8_t lines = _ssd1306->height / SSD1306_LINE_HEIGHT;
  ssd1306_send_command(_ssd1306, lines -1);
  
  uint16_t count = (_ssd1306->width>>4) * (_ssd1306->height>>3);
  for(uint16_t i=0; i<count; i++) {
    ssd1306_send_data(_ssd1306, &_ssd1306->buffer[i<<4], 16);
  }
}

static uint16_t ssd1306_get_pixel_index(ssd1306_t *_ssd1306, const uint16_t _x, const uint16_t _y) {
  return ( _x + ((_y / SSD1306_LINE_HEIGHT) * _ssd1306->width));
}

static uint16_t ssd1306_get_pixel_data(ssd1306_t *_ssd1306, const uint16_t _x, const uint16_t _y) {
  return _ssd1306->buffer[ ssd1306_get_pixel_index(_ssd1306, _x, _y) ];
}

static void ssd1306_set_pixel_data(ssd1306_t *_ssd1306, const uint16_t _x, const uint16_t _y, uint8_t _data) {
  _ssd1306->buffer[ ssd1306_get_pixel_index(_ssd1306, _x, _y) ] = _data;
}

void ssd1306_fill(ssd1306_t *_ssd1306, ssd1306_color_t _color) {
  for(uint16_t y=0; y < _ssd1306->height; y++) {
    for(uint16_t x=0; x < _ssd1306->width; x++) {
      ssd1306_set_pixel_data(_ssd1306, x, y, _color);
    }
  }
}

void ssd1306_clear(ssd1306_t *_ssd1306) {
  ssd1306_fill(_ssd1306, SSD1306_COLOR_BLACK);
}

void ssd1306_set_pixel(ssd1306_t *_ssd1306, const uint16_t _x, const uint16_t _y, const ssd1306_color_t _color) {
  uint8_t data = ssd1306_get_pixel_data(_ssd1306, _x, _y);
  if( _color == SSD1306_COLOR_BLACK ) {
    BitSet(data, _y % 8);
  } else {
    BitClear(data, _y % 8);
  }
  ssd1306_set_pixel_data(_ssd1306, _x, _y, data);
}


static void ssd1306_drawcharinternal(ssd1306_t *_ssd1306, ssd1306_color_t _color, uint16_t _x, uint16_t _y, ssd1306_font_char_t *_char) {
  if ( _y > _ssd1306->height)  return;
  if ( _x > _ssd1306->width)   return;
  uint16_t  rasterHeight = 1 + ((_char->height - 1) >> 3); // fast ceil(height / 8.0)
  uint8_t   yOffset      = _y & 7;
  
//   bytesInData = bytesInData == 0 ? _char->width * rasterHeight : bytesInData;
  uint16_t bytesInData = _char->width * rasterHeight;
  
  int16_t initYMove   = _y;
  uint8_t  initYOffset = yOffset;
  
  
  for (uint16_t i = 0; i < bytesInData; i++) {
    // Reset if next horizontal drawing phase is started.
    if ( i % rasterHeight == 0) {
      _y   = initYMove;
      yOffset = initYOffset;
    }
    uint8_t currentByte = pgm_read_byte(_char->data + _char->size + i);
    
    uint16_t xPos = _x + (i / rasterHeight);
    uint16_t yPos = ((_y >> 3) + (i % rasterHeight)) * _ssd1306->width;
    
    //    int16_t yScreenPos = _y + yOffset;
    uint16_t dataPos    = xPos  + yPos;
    
    if (dataPos < ssd1306_get_buffer_size(_ssd1306) && xPos    < _ssd1306->width ) {
      
      if (yOffset > 0) {
        switch (_color) {
          case SSD1306_COLOR_WHITE:   _ssd1306->buffer[dataPos] |= currentByte << yOffset; break;
          case SSD1306_COLOR_BLACK:   _ssd1306->buffer[dataPos] &= ~(currentByte << yOffset); break;
        }
        
        if (dataPos < (ssd1306_get_buffer_size(_ssd1306) - _ssd1306->width)) {
          switch (_color) {
            case SSD1306_COLOR_WHITE:   _ssd1306->buffer[dataPos + _ssd1306->width] |= currentByte >> (8 - yOffset); break;
            case SSD1306_COLOR_BLACK:   _ssd1306->buffer[dataPos + _ssd1306->width] &= ~(currentByte >> (8 - yOffset)); break;
          }
        }
      } else {
        // Make new offset position
        yOffset = -yOffset;
        
        switch (_color) {
          case SSD1306_COLOR_WHITE:   _ssd1306->buffer[dataPos] |= currentByte >> yOffset; break;
          case SSD1306_COLOR_BLACK:   _ssd1306->buffer[dataPos] &= ~(currentByte >> yOffset); break;
        }
        
        // Prepare for next iteration by moving one block up
        _y -= 8;
        
        // and setting the new yOffset
        yOffset = 8 - yOffset;
      }
    }
  }
}

bool ssd1306_font_available(ssd1306_t *_ssd1306) {
  return _ssd1306->font != NULL;
}

void ssd1306_printchar(ssd1306_t *_ssd1306, ssd1306_color_t _color, uint16_t _x, uint16_t _y, char _c) {
  if( !ssd1306_font_available(_ssd1306) ) {
    return;
  }
  ssd1306_font_char_t c = ssd1306_font_char_get(_ssd1306->font, _c);
  ssd1306_drawcharinternal(_ssd1306, _color, _x, _y, &c);
}

void ssd1306_printstring(ssd1306_t *_ssd1306, ssd1306_color_t _color, uint16_t _x, uint16_t _y, char *_text) {
  if( !ssd1306_font_available(_ssd1306) ) {
    return;
  }
  uint8_t len = strlen(_text);
  for(int i=0; i<len; i++) {
    ssd1306_font_char_t c = ssd1306_font_char_get(_ssd1306->font, _text[i]);
    ssd1306_drawcharinternal(_ssd1306, _color, _x, _y, &c);
    _x += c.width;
  }
}
