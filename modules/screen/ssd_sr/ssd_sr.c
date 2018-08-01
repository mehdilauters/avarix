#include "ssd_sr.h"

#include <math.h>
#include <string.h>

#include <clock/clock.h>
#include <util/delay.h>

#define SSD_SR_BITMAP_DASH (0x02)
#define SSD_SR_BITMAP_POINT (0x01)
#define SSD_SR_BITMAP_CLEAR (0x00)
#define SSD_SR_BITMAP_E (0x9E)
const uint8_t ssd_sr_bitmap[] = {
    0xfc, // 0
    0x60,
    0xda,
    0xf2,
    0x66,
    0xb6,
    0xbe,
    0xe0,
    0xfe,
    0xf6, // 9
};

static inline void set_output_low(portpin_t *pp) {
  portpin_dirset(pp);
  portpin_outclr(pp);
}

/// push one byte through shift register
static inline void push_byte(ssd_sr_t *s, uint8_t byte) {

  for(int i=0; i<8; i++) {
    // set input according to current bit
    if(byte & 0x01) {
      portpin_outclr(&s->pp_input);
    }
    else {
      portpin_outset(&s->pp_input);
    }

    // do a clock tick
    portpin_outset(&s->pp_clock);
    portpin_outclr(&s->pp_clock);

    // shift byte
    byte = byte >> 1;
  }

}

/// latch shift registers
static inline void latch_display(ssd_sr_t *s) {
  // do a latch tick
  portpin_outset(&s->pp_latch);
  portpin_outclr(&s->pp_latch);
}

/// shift a buffer of len bytes to SSDs
static inline void push_buffer(ssd_sr_t *s, uint8_t buffer[], uint8_t len) {
  for(unsigned i=0; i<len; i++) {
    push_byte(s, buffer[i]);
  }
}

/// push internal display buffer to display
static inline void show_display(ssd_sr_t *s) {
  push_buffer(s, s->display, s->n);
  latch_display(s);
}

int blit_integer(ssd_sr_t *s, uint8_t offset, uint8_t length, int value, bool display_zeroes) {
  
  // limit offset and length according to internal buffer size
  const unsigned max_sz = LENGTHOF(s->display);
  if(offset > max_sz) {
    return -1;
  }

  if(offset + length > max_sz) {
    return -2;
  };

  // make value positive and remember sign
  bool is_negative = value < 0;
  if(is_negative) {
    value = -value;
  }

  // erase all digits before writing
  uint8_t b = display_zeroes ? ssd_sr_bitmap[0] : SSD_SR_BITMAP_CLEAR;
  for(uint8_t i=0; i<length; i++) {
    s->display[offset+i] = b;
  }

  bool overflow = false;
  uint8_t widx = offset;
  uint8_t wend = widx + length;
  for(; value > 0; widx++){

    if(widx >= wend) {
      overflow = true;
      break;
    }
    uint8_t digit = value % 10;
    value /= 10;
    
    // set value in display
    s->display[widx] = ssd_sr_bitmap[digit];
  }

  // if value is negative a dash is need before number and may cause 
  // display to overflow
  if(is_negative && widx >= wend) {
    overflow = true;
  }

  // on overflow display "EEEE"
  if(overflow) {
    for(uint8_t i=0; i<length; i++) {
      s->display[offset+i] = SSD_SR_BITMAP_E;
    }
    return length;
  }

  // blit dash for negative numbers
  if(is_negative) {
    uint8_t i = display_zeroes ? offset + length - 1 : widx;
    s->display[i] = SSD_SR_BITMAP_DASH; 
    return i;
  }

  return widx;
}

void ssd_sr_init(ssd_sr_t *s, uint8_t n, portpin_t input, portpin_t clock, portpin_t latch) {

  // limit size according to internal buffer
  const unsigned max_sz = LENGTHOF(s->display);
  if(n > max_sz) {
    n = max_sz;
  }
  s->n = n;

  // configure GPIOs
  s->pp_input = input;
  set_output_low(&input);

  s->pp_clock = clock;
  set_output_low(&clock);

  s->pp_latch = latch;
  set_output_low(&latch);

  // clear all SSDs
  ssd_sr_clear_display(s);
}

void ssd_sr_clear_display(ssd_sr_t *s) {
  memset(s->display, SSD_SR_BITMAP_CLEAR, s->n);
  show_display(s);
}

int ssd_sr_display_integer(ssd_sr_t *s, uint8_t offset, uint8_t length, int value) {

  int rv = blit_integer(s, offset, length, value, false);
  if(rv < 0) {
    return rv;
  }

  show_display(s);
  return 0;
}

int ssd_sr_display_float(ssd_sr_t *s, uint8_t offset, uint8_t length, float value, uint8_t precision) {

  int ivalue = roundf( value * powf(10, precision) );

  int n = blit_integer(s, offset, length, ivalue, true);
  if(n < 0) {
    return n;
  }
  
  const int max_sz = LENGTHOF(s->display);

  // blit decimal point
  uint8_t iend = offset + precision;
  if(iend >= max_sz - 1) {
    return -1;
  }
  s->display[iend] |= SSD_SR_BITMAP_POINT;

  // remove zeroes
  uint8_t idx = n;
  for(; idx<offset+length; idx++) {
    s->display[idx] = SSD_SR_BITMAP_CLEAR;
  }
  
  show_display(s);
  return 0;
}
