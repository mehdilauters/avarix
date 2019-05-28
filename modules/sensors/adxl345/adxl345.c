#include <avarix.h>
#include <clock/clock.h>
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>
#include "adxl345.h" 

static bool write_read(adxl345_t *s, adxl345_register_t r, uint8_t * value) {
    int rv = i2cm_send(s->i2c, s->address, &r, sizeof(uint8_t));
    if(rv > 0) {
      if(i2cm_recv(s->i2c,s->address, value, sizeof(uint8_t)) == 1) {
        return true;
      }
    }
    return false;
}

static bool read_register(adxl345_t *s, adxl345_register_t r, uint8_t * value) {
  return write_read(s, r, value);
}

static bool write_register(adxl345_t *s, adxl345_register_t r, uint8_t value) {
    uint8_t buffer[2] = {r,value};
    if(i2cm_send(s->i2c, s->address, buffer, sizeof(buffer)) == sizeof(buffer)) {
      return true;
    }
    return false;
}

bool adxl345_set_powermode(adxl345_t *b, adxl345_power_t mode) {
  return write_register(b, ADXL345_REGISTER_POWER_CTL, ADXL345_POWER_MEASURE);  
}

bool adxl345_set_rate(adxl345_t *b, uint8_t rate) {
  return write_register(b, ADXL345_REGISTER_BW_RATE, rate << ADXL345_RATE_BIT);
}

bool adxl345_set_data_format(adxl345_t *b, bool full_res, adxl345_range_t range) {
  uint8_t format = 0;
  if(full_res) {
      format |= 1 << ADXL345_DATA_FORMAT_FULL_RES_BIT;
  }
  format |= range << ADXL345_DATA_FORMAT_RANGE_BIT;
  return write_register(b, ADXL345_REGISTER_DATA_FORMAT, format);
}

bool adxl345_set_interrupts(adxl345_t *b, adxl345_interrupt_t interrupt) {
  return write_register(b, ADXL345_REGISTER_INT_ENABLE, interrupt);  
}

bool adxl345_set_tap_axes(adxl345_t *b, adxl345_tap_axes_t axes) {
  return write_register(b, ADXL345_REGISTER_TAP_AXES, axes);
}

bool adxl345_set_tap_threshold(adxl345_t *b, uint8_t threshold) {
  return write_register(b, ADXL345_REGISTER_THRESH_TAP, threshold);
}

bool adxl345_set_tap_duration(adxl345_t *b, uint8_t duration) {
  return write_register(b, ADXL345_REGISTER_DUR, duration);
}

bool adxl345_set_tap_parameters(adxl345_t *b, adxl345_tap_axes_t axes, uint8_t threshold, uint8_t duration) {
  if(!adxl345_set_tap_axes(b, axes)) {
    return false;
  }
  if(!adxl345_set_tap_threshold(b, threshold)) {
    return false;
  }
  if(!adxl345_set_tap_duration(b, duration)) {
    return false;
  }
  return true;
}

adxl345_error_t adxl345_init(adxl345_t *b, i2cm_t *i2cm, uint8_t address) {
  b->i2c = i2cm;
  b->address = address;
  b->interrupt = 0;
  uint8_t devid;
  if(read_register(b, ADXL345_REGISTER_DEVID, &devid)) {
    if(devid == ADXL345_DEVID_CODE) {
        return ADXL345_ERROR_SUCCESS;
    } else {
        return ADXL345_ERROR_INVALID_DEVID;
    }
  } else {
    return ADXL345_ERROR_PEER_UNREACHABLE;
  }
}

bool adxl345_is_tap_interrupt(adxl345_t *b) {
  if(b->interrupt & ADXL345_INTERRUPT_TAP) {
      b->interrupt = 0;
      return true;
  }
  return false;
}

bool adxl345_update(adxl345_t *b) {
  // do not reset interrupt flag if not already checked by adxl345_is_tap_interrupt
  if(b->interrupt & ADXL345_INTERRUPT_TAP) return true;
  return read_register(b, ADXL345_REGISTER_INT_SOURCE, &(b->interrupt));
}
