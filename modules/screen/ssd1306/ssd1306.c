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

static void i2c_send_buffer(ssd1306_t *s, uint8_t *buffer, uint8_t len) {
  i2cm_send(s->i2c, s->address, buffer, len);
}

static void send_command(ssd1306_t *s, uint8_t command) {
  uint8_t buffer[] = {
    SSD1306_COMMAND_DC_COMMAND_bm,
    command
  };
  i2c_send_buffer(s, buffer, sizeof(buffer));
}

static void send_data(ssd1306_t *s, uint8_t *data, int len) {
  uint8_t buffer[len+1];
  buffer[0] = SSD1306_COMMAND_DC_DATA_bm;
  memcpy(buffer + 1, data, len);
  i2c_send_buffer(s, buffer, len+1);
}

void ssd1306_init(ssd1306_t *s, i2cm_t * i2c, uint8_t address, uint16_t width, uint16_t height) {
  s->i2c = i2c;
  s->address = address;
  s->width = width;
  s->height = height;

  // turn display off while configuring
  send_command(s, SSD1306_COMMAND_DISPLAY_OFF);

  send_command(s, SSD1306_COMMAND_DISPLAY_CLOCK_DIV_RATIO_SET);
  send_command(s, 0x80);

  send_command(s, SSD1306_COMMAND_MULTIPLEX_RATIO_SET);
  send_command(s, height-1);
  send_command(s, SSD1306_COMMAND_DISPLAY_START_LINE_SET);

  // enable internal charge pump circuitry
  send_command(s, SSD1306_COMMAND_CHARGE_PUMP_SET);
  send_command(s, SSD1306_COMMAND_CHARGE_PUMP_SET_ONE_bm
                  | SSD1306_COMMAND_CHARGE_PUMP_SET_ENABLE_bm );
  
  send_command(s, SSD1306_COMMAND_MEMORY_ADDRESSING_MODE);
  send_command(s, SSD1306_COMMAND_MEMORY_ADDRESSING_MODE_HORIZONTAL_bm);
  
  send_command(s, SSD1306_COMMAND_DISPLAY_OFFSET_SET);
  send_command(s, 0);

  send_command(s, SSD1306_COMMAND_COM_PINS_SET);
  send_command(s, 0);

  // left towards right
  send_command(s, SSD1306_COMMAND_SEGMENT_REMAP_SET_ADDRESS_127);
  
  send_command(s, SSD1306_COMMAND_COM_OUTPUT_SCAN_DEC);
  
  send_command(s, SSD1306_COMMAND_CONTRAST_SET);
  send_command(s, 0xFF);
  
  send_command(s, SSD1306_COMMAND_PRECHARGE_PERIOD_SET);
  send_command(s, 0xF1);
  
  send_command(s, SSD1306_COMMAND_SCROLL_DEACTIVATE);
  send_command(s, SSD1306_COMMAND_DISPLAY_NORMAL);
  send_command(s, SSD1306_COMMAND_DISPLAY_ON);

  (void)send_data;
}

void ssd1306_blit(ssd1306_t *s) {
  // prepare a full screen copy
  send_command(s, SSD1306_COMMAND_COLUMN_ADDRESS_SET);
  send_command(s, 0);
  send_command(s, s->width-1);

  uint8_t lines = s->height / 8;
  send_command(s, SSD1306_COMMAND_PAGE_ADDRESS_SET);
  send_command(s, 0);
  send_command(s, lines-1);

  static int v = 0; v++;
  const int page_size = 64;
  const int npages = s->width * lines / page_size;
  for(int i=0;i<npages;i++) {
    uint8_t buffer[page_size];// = {0};
    memset(buffer, v+i, sizeof(buffer));
    send_data(s, buffer, sizeof(buffer));
  }
}

