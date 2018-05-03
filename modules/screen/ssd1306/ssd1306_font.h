#ifndef SSD1306_FONT_
#define SSD1306_FONT_
#include <stdint.h>
struct ssd1306;

typedef const uint8_t ssd1306_font_t;

typedef struct {
  uint8_t width;
  uint8_t height;
  uint8_t* data;
  uint8_t size;
  uint8_t code;
} ssd1306_font_char_t;


uint8_t ssd1306_font_get_width(ssd1306_font_t*);
uint8_t ssd1306_font_get_height(ssd1306_font_t*);
uint8_t ssd1306_font_get_first_char(ssd1306_font_t*);
uint8_t ssd1306_font_get_char_count(ssd1306_font_t*);
ssd1306_font_char_t ssd1306_font_char_get(ssd1306_font_t* _font, uint8_t _c);


#endif
