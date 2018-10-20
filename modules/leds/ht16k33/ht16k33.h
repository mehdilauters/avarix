#ifndef LEDS_HT16K33_H
#define LEDS_HT16K33_H
#include <stdint.h>
#include <stdbool.h>
#include <i2c/i2c.h>

#include "registers.h"

typedef enum {
  
  HT16K33_ERROR_SUCCESS = 0,
  HT16K33_ERROR_PEER_UNREACHABLE,
  HT16K33_ERROR_INVALID_POSITION,
  HT16K33_ERROR_INVALID_COLOR,

} ht16k33_error_t;

typedef enum {
  HT16K33_COLOR_OFF,
  HT16K33_COLOR_A,
  HT16K33_COLOR_B,
  HT16K33_COLOR_AB,
} ht16k33_color_t;

typedef enum {
  HT16K33_BLINK_NONE = 0,
  HT16K33_BLINK_2HZ,
  HT16K33_BLINK_1HZ,
  HT16K33_BLINK_0_5HZ,
} ht16k33_blink_t;

typedef struct {
  i2cm_t * i2c;

  uint8_t address;

  uint8_t segment_length;

  // ht16k33 internal RAM is 2x8 bytes long
  uint16_t buffer[8];

} ht16k33_t;

/** @brief Initialize HT16K33 chip
 * @param i2cm i2c bus master
 * @param address i2c chip address
 */
ht16k33_error_t ht16k33_init(ht16k33_t *b, i2cm_t *i2cm, uint8_t address,
                              uint8_t segment_length);

/** @brief Set HT16K33 blink mode */
ht16k33_error_t ht16k33_blink(ht16k33_t *b, ht16k33_blink_t blink);

/** @brief Set HT16K33 dimming 
 * @param dim dimming value where 0 is lowest illumination, 15 max illumination */
ht16k33_error_t ht16k33_dimming(ht16k33_t *b, uint8_t dim);

/** @brief Clear internal buffer */
void ht16k33_clear(ht16k33_t *b);

/** @brief Set LED in internal buffer (need to blit after) */ 
ht16k33_error_t ht16k33_set(ht16k33_t *b, uint8_t l, ht16k33_color_t c);

/** @brief Push internal buffer to chip */
ht16k33_error_t ht16k33_blit(ht16k33_t *b);

#endif
