#include "ssd1306.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <clock/clock.h>
#include <util/delay.h>

int8_t i2cm_async_send_retry(i2cm_t *i2c, 
                            uint8_t addr, const uint8_t *data, uint8_t n,
                            i2cm_write_completed_callback f,
                            void *payload) {
  int r;
  // try to send frame multiple times
  for(int i=0; i<10; i++) {
    r = i2cm_async_send(i2c, addr, data, n, f, payload);
    if(r>0) {
      // successfull send
      return r;
    }
    
    if(r <= -2) {
      // bus was not ready
      _delay_us(100);
      continue;
    }
    
    // something else
    return r;
  }

  return r;
}

static bool send_command_retry(ssd1306_t *s, uint8_t command, uint8_t max_retries) {
  uint8_t buffer[] = {
    SSD1306_COMMAND_DC_COMMAND_bm,
    command
  };
  // try to send frame multiple times
  for(int retries=0; retries<max_retries; retries++) {
    int rv = i2cm_send(s->i2c, s->address, buffer, sizeof(buffer));
    if(rv > 0) {
      // success
      return true;
    }
    // i2cm_send is blocking, give peers some time to recover before next try
    _delay_us(100);
  }
  return false;
}

static bool send_command(ssd1306_t *s, uint8_t command) {
  return send_command_retry(s,command,10);
}

static void send_next_page(int16_t success, void* payload) {
  ssd1306_t *s = (ssd1306_t*)payload;

  // on frame failure, abort blit
  if(success < 0) {
    s->blit_in_progress = false;
    return;
  }

  const int page_size = 64;
  uint8_t nlines = s->height / 8;
  const int npages = s->width * nlines / page_size;
  (void)npages;

  // on frame success increment blit iterator through buffer
  if(success > 0) {
    s->blit_it += page_size;
    // we reached the end of the buffer
    if(s->blit_it >= sizeof(s->screen)) {
      s->blit_in_progress = false;
      return;
    }
  }

  // prepare send buffer
  uint8_t buffer[page_size+1];
  buffer[0] = SSD1306_COMMAND_DC_DATA_bm;
  memcpy(buffer + 1, s->screen + s->blit_it, page_size);
  int r = i2cm_async_send_retry(s->i2c, s->address, buffer, page_size+1,
    send_next_page, s);
  if(r<0) {
    s->blit_in_progress = false;
    return;
  }
}

ssd1306_error_t ssd1306_init(ssd1306_t *s, i2cm_t * i2c, uint8_t address, ssd1306_size_t size) {

  s->i2c = i2c;
  s->address = address;

  memset(&s->font, 0, sizeof(s->font));

  switch(size) {
    case SSD1306_SIZE_128_32:
      s->width = 128;
      s->height = 32;
      break;

    default:
      return SSD1306_ERROR_INVALID_SCREEN_SIZE;
  }

  s->blit_in_progress = false;
  s->blit_it = 0;

  memset(s->screen, 0, sizeof(s->screen));


  if(send_command_retry(s, SSD1306_COMMAND_DISPLAY_OFF, 1)) {
    return SSD1306_ERROR_SUCCESS;
  }
  else {
    return SSD1306_ERROR_PEER_UNREACHABLE;
  }
}

ssd1306_error_t ssd1306_configure(ssd1306_t *s) {

  // turn display off while configuring
  uint8_t commands[] = {
    SSD1306_COMMAND_DISPLAY_OFF,

    SSD1306_COMMAND_DISPLAY_CLOCK_DIV_RATIO_SET,
    0x80,

    SSD1306_COMMAND_MULTIPLEX_RATIO_SET,
    s->height-1,
    SSD1306_COMMAND_DISPLAY_START_LINE_SET,

    // enable internal charge pump circuitry
    SSD1306_COMMAND_CHARGE_PUMP_SET,
    SSD1306_COMMAND_CHARGE_PUMP_SET_ONE_bm | SSD1306_COMMAND_CHARGE_PUMP_SET_ENABLE_bm,
    
    SSD1306_COMMAND_MEMORY_ADDRESSING_MODE,
    SSD1306_COMMAND_MEMORY_ADDRESSING_MODE_VERTICAL_bm,
    
    SSD1306_COMMAND_DISPLAY_OFFSET_SET,
    0,

    SSD1306_COMMAND_COM_PINS_SET,
    0,

    // left towards right
    SSD1306_COMMAND_SEGMENT_REMAP_SET_ADDRESS_127,
    
    SSD1306_COMMAND_COM_OUTPUT_SCAN_DEC,
    
    SSD1306_COMMAND_CONTRAST_SET,
    0xFF,
    
    SSD1306_COMMAND_PRECHARGE_PERIOD_SET,
    0xF1,
    
    SSD1306_COMMAND_SCROLL_DEACTIVATE,
    SSD1306_COMMAND_DISPLAY_NORMAL,
    SSD1306_COMMAND_DISPLAY_ON,
  };

  for(unsigned int i=0; i<sizeof(commands); i++) {
    if(!send_command(s, commands[i])) {
      // send_command already retry 10 times 
      return SSD1306_ERROR_PEER_UNREACHABLE;
    }
  }

  return SSD1306_ERROR_SUCCESS;
}

#define SSD1306_FONT_HEADER_SIZE 4
#define SSD1306_FONT_CHAR_HEADER_SIZE 4

typedef struct {
  uint16_t jump;
  uint8_t size;
  uint8_t width;

} ssd1306_font_char_header_t;


static uint32_t font_table_read_dword(ssd1306_t *s, int offset) {
  return pgm_read_dword(s->font.p + offset);
}

#if 0
static uint16_t font_table_read_word(ssd1306_t *s, int offset) {
  return pgm_read_word(s->font.p + offset);
}
#endif

static uint8_t font_table_read_byte(ssd1306_t *s, int offset) {
  return pgm_read_byte(s->font.p + offset);
}

static uint16_t swap16(uint16_t x) {
  return (x >> 8) | (x << 8);
}

static ssd1306_font_char_header_t 
  font_table_read_char_header(ssd1306_t *s, char c) {

  // compute font char header offset in table
  char fc = s->font.first_char;
  int offset = SSD1306_FONT_HEADER_SIZE + (c-fc)*SSD1306_FONT_CHAR_HEADER_SIZE;
  // access 32 bits from flash
  uint32_t dword = font_table_read_dword(s, offset);
  char *p = (char*)&dword;

  // unpack and return header
  ssd1306_font_char_header_t header;

  uint16_t jump;
  memcpy(&jump, p, sizeof(header.jump));

  // compute jump address useable in font_table_read_byte()
  header.jump = SSD1306_FONT_HEADER_SIZE 
    + s->font.number_of_chars * SSD1306_FONT_CHAR_HEADER_SIZE
    + swap16(jump);

  memcpy(&header.size, p+2, sizeof(header.size));
  memcpy(&header.width, p+3, sizeof(header.width));

  return header;
}

void ssd1306_load_font(ssd1306_t *s, const uint8_t *p) {
  s->font.p = p;
  s->font.height = font_table_read_byte(s,1);
  s->font.first_char = font_table_read_byte(s,2);
  s->font.number_of_chars = font_table_read_byte(s,3);
}

bool ssd1306_font_loaded(ssd1306_t *s) {
  return s->font.p != NULL;
}

ssd1306_position_t ssd1306_draw_char(ssd1306_t *s, ssd1306_position_t p, char c)
{
  if(!ssd1306_font_loaded(s)) {
    return p;
  }
  
  // check if a screen blit is already underway
  INTLVL_DISABLE_BLOCK(I2C_INTLVL) {
    if(s->blit_in_progress)
      return p;
  }

  ssd1306_font_char_header_t header = font_table_read_char_header(s,c);
  
  int x=0,y=0;
  const uint8_t yh = s->font.height;
  for(unsigned int i=0; i<header.size; i++) {
    unsigned int offset = 4*(p.x + x) + (p.y + y);
    if(offset >= sizeof(s->screen)) {
      // out of screen
      return p;
    }
    uint8_t byte = font_table_read_byte(s,header.jump+i);
    uint8_t *pixel = s->screen + offset;

    // draw over, do not erase
    *pixel |= byte;

    y++;
    if(8*y > yh) {
      x++;
      y=0;
    }

  }

  // update cursor position, return it
  p.x += header.width;
  return p;
}

ssd1306_position_t ssd1306_draw_string(ssd1306_t *s, ssd1306_position_t p, const char* str, uint8_t len, uint8_t letter_spacing_px) {

  if(!ssd1306_font_loaded(s)) {
    return p;
  }
  
  for(int i=0; (i<len) && (*str != 0); i++) {
    p = ssd1306_draw_char(s, p, *(str++));
    p.x += letter_spacing_px;
  }

  return p;
}

void ssd1306_clear(ssd1306_t *s) {
 // check if a screen blit is already underway
  INTLVL_DISABLE_BLOCK(I2C_INTLVL) {
    if(s->blit_in_progress)
      return;
  }

  memset(s->screen, 0, sizeof(s->screen));
}

void ssd1306_blit(ssd1306_t *s) {

  // check if a screen blit is already underway
  INTLVL_DISABLE_BLOCK(I2C_INTLVL) {
    if(s->blit_in_progress)
      return;
  }

  // prepare a full screen copy
  send_command(s, SSD1306_COMMAND_COLUMN_ADDRESS_SET);
  send_command(s, 0);
  send_command(s, s->width-1);

  uint8_t nlines = s->height / 8;
  send_command(s, SSD1306_COMMAND_PAGE_ADDRESS_SET);
  send_command(s, 0);
  send_command(s, nlines-1);

  // reset blit iterator and send page
  INTLVL_DISABLE_BLOCK(I2C_INTLVL) {
    s->blit_it = 0;
    s->blit_in_progress = true;
    send_next_page(0, s);
  }
}

bool ssd1306_blit_in_progress(ssd1306_t *s) {
  INTLVL_DISABLE_BLOCK(I2C_INTLVL) {
    return s->blit_in_progress;
  }
  return false;
}
