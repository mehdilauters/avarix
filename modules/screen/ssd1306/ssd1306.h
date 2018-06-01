#ifndef SCREEN_SSD1306_
#define SCREEN_SSD1306_
#include <stdint.h>
#include <stdbool.h>
#include <i2c/i2c.h>

#include "registers.h"
#include "ssd1306_font.h"

typedef struct {
  i2cm_t * i2c;
  uint8_t address;
  uint16_t width;
  uint16_t height;
} ssd1306_t;

void ssd1306_init(ssd1306_t *s, i2cm_t *i2cm, uint8_t address, uint16_t width, uint16_t height);

void ssd1306_blit(ssd1306_t *s);

#endif
