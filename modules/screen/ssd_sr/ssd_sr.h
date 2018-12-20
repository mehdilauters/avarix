#ifndef SCREEN_SSD_SR_
#define SCREEN_SSD_SR_

#include <avarix.h>
#include <avarix/portpin.h>

typedef struct {

  uint8_t n;

  portpin_t pp_input;
  portpin_t pp_clock;
  portpin_t pp_latch;

  uint8_t display[32];

}ssd_sr_t;


/** @brief Initialize internal structure for a daisy chain of Seven Segment Display (SSD) controlled by
 * Shift Registers (74HC595 or equivalent)
 *
 * @param n number of daisy chained 7 segment display
 * @param input portpin of GPIO connected to shift register input (GPIO will be configured as ouput)
 * @param clock portpin of GPIO connected to shift register clock input (GPIO will be configured as ouput)
 * @param latch portpin of GPIO connected to shift register latch input (GPIO will be configured as ouput)
 */
void ssd_sr_init(ssd_sr_t *s, uint8_t n, portpin_t input, portpin_t clock, portpin_t latch);

/** @brief Clear display */
void ssd_sr_clear_display(ssd_sr_t *s);

/** @brief Clear display at position offset, for length digits */
int ssd_sr_clear(ssd_sr_t *s, uint8_t offset, uint8_t length);

/** @brief Display integer value on SSDs starting at a position offset, limiting number of used SSDs to length*/
int ssd_sr_display_integer(ssd_sr_t *s, uint8_t offset, uint8_t length, int value);

/** @brief Display float value on SSDs using precision, starting at a position offset, limiting number of used SSDs to length*/
int ssd_sr_display_float(ssd_sr_t *s, uint8_t offset, uint8_t length, float value, uint8_t precision);
#endif//SCREEN_SSD_SR_
