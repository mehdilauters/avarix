#include <avarix.h>
#include <clock/clock.h>
#include <util/delay.h>
#include <string.h>

#include "ht16k33.h" 

#ifndef _BV
  #define _BV(bit) (1<<(bit))
#endif

static int8_t i2cm_async_send_retry(i2cm_t *i2c, 
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

static bool send_command_retry(ht16k33_t *s, uint8_t command, uint8_t max_retries) {
  // try to send frame multiple times
  for(int retries=0; retries<max_retries; retries++) {
    int rv = i2cm_send(s->i2c, s->address, &command, sizeof(uint8_t));
    if(rv > 0) {
      // success
      return true;
    }
    // i2cm_send is blocking, give peers some time to recover before next try
    _delay_us(100);
  }
  return false;
}

static bool send_command(ht16k33_t *s, uint8_t command) {
  return send_command_retry(s,command,10);
}

ht16k33_error_t ht16k33_init(ht16k33_t *b, i2cm_t *i2cm, uint8_t address,
                              uint8_t segment_length) {

  b->i2c = i2cm;
  b->address = address;
  memset(b->buffer, 0, sizeof(b->buffer));

  b->segment_length = segment_length;

  // turn chip clock on
  if ( ! send_command(b, HT16K33_SYSTEM_SETUP_ON) ) {
    return HT16K33_ERROR_PEER_UNREACHABLE;
  }
  
  // default to always on
  if( ht16k33_blink(b, HT16K33_BLINK_NONE)  != HT16K33_ERROR_SUCCESS ) {
    return HT16K33_ERROR_PEER_UNREACHABLE;
  }
  
  // default to full brightness
  if( ht16k33_dimming(b, 15) != HT16K33_ERROR_SUCCESS ) {
    return HT16K33_ERROR_PEER_UNREACHABLE;
  }
  
  return HT16K33_ERROR_SUCCESS;
}

ht16k33_error_t ht16k33_dimming(ht16k33_t *b, uint8_t dim) {

  // limit dimming value
  if(dim > 15) {
    dim = 15;
  }
  
  if(!send_command(b, HT16K33_DIMING_SET | dim)) {
    return HT16K33_ERROR_PEER_UNREACHABLE;
  }
  return HT16K33_ERROR_SUCCESS;
}

void ht16k33_clear(ht16k33_t *b) {
  for (uint8_t i=0; i<8; i++) {
    b->buffer[i] = 0;
  }
}

ht16k33_error_t ht16k33_blink(ht16k33_t *b, ht16k33_blink_t blink) {
  if( send_command(b, HT16K33_DISPLAY_BLINK | blink) ) {
    return HT16K33_ERROR_SUCCESS;
  } else {
    return HT16K33_ERROR_PEER_UNREACHABLE;
  }
}

ht16k33_error_t ht16k33_set(ht16k33_t *b, uint8_t pos, ht16k33_color_t color) {
  bool upper = false;
  if(pos >= b->segment_length) {
    upper = true;
    pos -= b->segment_length;
  }
  uint8_t nrow = pos%4;
  uint8_t ncom = pos/4;

  if(upper) {
    nrow += 4;
  }

  if(ncom >= LENGTHOF(b->buffer)) {
    return HT16K33_ERROR_INVALID_POSITION;
  }

  uint16_t *com = b->buffer + ncom;
  uint16_t a_mask = _BV(nrow);
  uint16_t b_mask = _BV(nrow + 8);
  switch(color) {
    case HT16K33_COLOR_OFF:
      *com &= ~a_mask;
      *com &= ~b_mask;
      break;

    case HT16K33_COLOR_A:
      *com &= ~b_mask;
      *com |= a_mask;
      break;

    case HT16K33_COLOR_B:
      *com &= ~a_mask;
      *com |= b_mask;
      break;

    case HT16K33_COLOR_AB:
      *com |= a_mask;
      *com |= b_mask;
      break;

    default:
      return HT16K33_ERROR_INVALID_COLOR;
    }

  return HT16K33_ERROR_SUCCESS;
}

ht16k33_error_t ht16k33_blit(ht16k33_t *b) {
  const size_t n = sizeof(b->buffer) + 1;
  uint8_t buffer[n];

  // we will always update the full buffer (starting at 0 in chip RAM)
  buffer[0] = 0;
  memcpy(buffer + 1, b->buffer, sizeof(b->buffer));
  
  int r = i2cm_async_send_retry(b->i2c, b->address, buffer, sizeof(buffer), NULL, b);
  if(r<0) {
    return HT16K33_ERROR_SUCCESS;
  }

  return HT16K33_ERROR_SUCCESS;
}
