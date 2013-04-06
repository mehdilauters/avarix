/** @defgroup quadra Quadra
 * @brief Quadrature decoder
 *
 * This module is intended to be used for motor enccoders.
 *
 * Decoded value is a 16-bit value which can overflow or underflow.
 */
//@{
/**
 * @file
 * @brief Quadrature module definitions
 */
#ifndef QUADRA_H__
#define QUADRA_H__

#include <avr/io.h>
#include <avarix/portpin.h>


/** Quadrature decoder data
 * @note Fields are private and should not be accessed directly.
 */
typedef struct {
  TC1_t *tc;  ///< timer used to decode the quadrature signal
} quadra_t;


/** @brief Initialize quadrature decoder
 *
 * @param q  quadrature decoder to initialize
 * @param tc  timer to use (can also be a pointer to a \e TC0_t)
 * @param evch  event channel to use (0, 2 or 4)
 * @param pp0  port pin of the reference encoder signal
 * @param pp90  port pin of the phase shifted encoder signal
 * @param samples  length of digital filtering (1 to 8)
 *
 * A quadrature decoder uses two timer and one event channel. Since it only
 * requires two channels it is better to use a timer of type 1. However, timers
 * of type 0 can be used too.
 *
 * Input signal must use pins valid as event source (port A to F).
 */
void quadra_init(quadra_t *q, TC1_t *tc, uint8_t evch, portpin_t pp0, portpin_t pp90, uint8_t samples);

/// Get the current decoder value
uint16_t quadra_get(quadra_t *q);

/// Reset the decoder value
void quadra_reset(quadra_t *q, uint16_t value);


#endif
//@}
