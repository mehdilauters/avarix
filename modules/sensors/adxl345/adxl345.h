#ifndef ADXL345_H__ 
#define ADXL345_H__

#include <stdint.h>
#include <stdbool.h>
#include <i2c/i2c.h>
#include <avarix/portpin.h>
#include "registers.h"

typedef enum {
  
  ADXL345_ERROR_SUCCESS = 0,
  ADXL345_ERROR_PEER_UNREACHABLE,
  ADXL345_ERROR_INVALID_DEVID,
  ADXL345_ERROR_UNKNOWN_ERROR
} adxl345_error_t;

typedef struct {
  i2cm_t * i2c;
  uint8_t address;
  uint8_t tap_count;
} adxl345_t;

/** @brief Initialize the adxl345_t structure 
 */
adxl345_error_t adxl345_init(adxl345_t *b, i2cm_t *i2cm, uint8_t address);

/** @brief set adxl345 power mode
 */
bool adxl345_set_powermode(adxl345_t *b, adxl345_power_t mode);

/** @brief set adxl345 data rate
 */
bool adxl345_set_rate(adxl345_t *b, uint8_t rate);

/** @brief set adxl345 data format
 */
bool adxl345_set_data_format(adxl345_t *b, bool full_res, adxl345_range_t range);

/** @brief set adxl345 interrupts types
 */
bool adxl345_set_interrupts(adxl345_t *b, adxl345_interrupt_t interrupt);

/** @brief set adxl345 tap axes to consider
 */
bool adxl345_set_tap_axes(adxl345_t *b, adxl345_tap_axes_t axes);

/** @brief set adxl345 tap threshold
 */
bool adxl345_set_tap_threshold(adxl345_t *b, uint8_t threshold);

/** @brief set adxl345 tap duration
 */
bool adxl345_set_tap_duration(adxl345_t *b, uint8_t duration);

/** @brief set adxl345 tap parameters
 */
bool adxl345_set_tap_parameters(adxl345_t *b, adxl345_tap_axes_t axes, uint8_t threshold, uint8_t duration);

/** @brief get tap interrupt count
 */
uint8_t adxl345_get_tap_count(adxl345_t *b);

/** @brief update adxl345 structure
 */
bool adxl345_update(adxl345_t *b);

#endif
