#ifndef SCREEN_SSD1306_
#define SCREEN_SSD1306_
#include <stdint.h>
#include <stdbool.h>
#include <i2c/i2c.h>

#include "ssd1306_font.h"
typedef struct {
  i2cm_t * i2c;
  uint8_t address;
  uint16_t width;
  uint16_t height;
  uint8_t *buffer;
  const uint8_t *font;
} ssd1306_t;


typedef enum {
  // fundamental
  SSD1306_COMMAND_DISPLAY_ON = 0xAF,
  SSD1306_COMMAND_DISPLAY_OFF = 0xAE,
  SSD1306_COMMAND_CONTRAST_SET = 0x81,
  SSD1306_COMMAND_DISPLAY_ALL_ON_RESUME = 0xA4,
  SSD1306_COMMAND_DISPLAY_ALL_ON = 0xA5,
  SSD1306_COMMAND_DISPLAY_NORMAL = 0xA6,
  SSD1306_COMMAND_DISPLAY_INVERSE = 0xA7,
  
  // scrolling
  SSD1306_COMMAND_SCROLL_ACTIVATE = 0x2F,
  SSD1306_COMMAND_SCROLL_DEACTIVATE = 0x2E,
  SSD1306_COMMAND_SCROLL_LEFT = 0x27,
  SSD1306_COMMAND_SCROLL_RIGHT = 0x26,
  SSD1306_COMMAND_SCROLL_VERTICAL_LEFT = 0x2A,
  SSD1306_COMMAND_SCROLL_VERTICAL_RIGHT = 0x29,
  SSD1306_COMMAND_SCROLL_AREA_SET = 0xA3,
  
  // addressing setting
  SSD1306_COMMAND_PAGE_ADDRESSING_COLUMN_LOWER_SET = 0x00,
  SSD1306_COMMAND_PAGE_ADDRESSING_COLUMN_HIGHER_SET = 0x10,
  SSD1306_COMMAND_PAGE_ADDRESSING_PAGE_START_SET = 0xB0,
  SSD1306_COMMAND_PAGE_ADDRESS_SET = 0x22,
  SSD1306_COMMAND_MEMORY_ADDRESSING_SET = 0x20,
  SSD1306_COMMAND_COLUMN_ADDRESS_SET = 0x21,
  
  // hardware configuration
  SSD1306_COMMAND_DISPLAY_START_LINE_SET = 0x40,
  SSD1306_COMMAND_DISPLAY_OFFSET_SET = 0xD3,
  SSD1306_COMMAND_SEGMENT_REMAP_SET = 0xA0,
  SSD1306_COMMAND_MULTIPLEX_RADIO_SET = 0xA8,
  SSD1306_COMMAND_COM_OUTPUT_SCAN_INC = 0xC0,
  SSD1306_COMMAND_COM_OUTPUT_SCAN_DEC = 0xC8,
  SSD1306_COMMAND_COM_PINS_SET = 0xDA,
  
  // timing and driving scheme setting
  SSD1306_COMMAND_DISPLAY_CLOCK_DIV_RADIO_SET = 0xD5,
  SSD1306_COMMAND_DISPLAY_OSCILLATOR_FREQUENCY_SET = 0xD5,
  SSD1306_COMMAND_PRECHARGE_PERIOD_SET = 0xD9,
  SSD1306_COMMAND_VCOMH_DESELECT_LEVEL_SET = 0xDB,
  SSD1306_COMMAND_NOP = 0xE3,
  
  // charge pump
  SSD1306_COMMAND_CHARGE_PUMP_SET = 0x8D,
} ssd1306_command_t;

typedef enum {
  SSD1306_COLOR_WHITE = 0x00,
  SSD1306_COLOR_BLACK = 0xFF
} ssd1306_color_t;

void ssd1306_init(ssd1306_t *_ssd1306, i2cm_t * _i2c, uint8_t _address, uint16_t _width, uint16_t _height);
void ssd1306_set_font(ssd1306_t *_ssd1306, const uint8_t *_font);
bool ssd1306_font_available(ssd1306_t *_ssd1306);
void ssd1306_clear(ssd1306_t *_ssd1306);
void ssd1306_set_pixel(ssd1306_t *_ssd1306, const uint16_t _x, const uint16_t _y, const ssd1306_color_t _color);
void ssd1306_fill(ssd1306_t *_ssd1306, ssd1306_color_t _color);

void ssd1306_printchar(ssd1306_t *_ssd1306, ssd1306_color_t _color, uint16_t _x, uint16_t _y, char _c);

void ssd1306_printstring(ssd1306_t *_ssd1306, ssd1306_color_t _color, uint16_t _x, uint16_t _y, char *_text);

void ssd1306_render(ssd1306_t *_ssd1306);

#endif
