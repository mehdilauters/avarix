/** @defgroup i2c I2C
 * @brief I2C module
 */
//@{
/**
 * @file
 * @brief I2C definitions
 */
#ifndef I2C_H__
#define I2C_H__

#include <avr/io.h>
#include <avarix/intlvl.h>
#include <stdint.h>
#include "i2c_config.h"


#ifdef DOXYGEN

/// I2C master state
typedef struct i2cm_struct i2cm_t;
/// I2C slave state
typedef struct i2cs_struct i2cs_t;

#else

#include "i2cm.inc.h"

#include "i2cs.inc.h"

// Check for I2C enabled as both master and slave
// Define pointers to internal structures

// Command to duplicate I2CC code for each I2C.
// Vimmers will use it with " :*! ".
//   python -c 'import sys,re; s=sys.stdin.read(); print "\n".join(re.sub("(I2C\x7ci2c\x7cTWI)C", r"\1"+x, s) for x in "CDEF")'

#if (defined I2CC_MASTER) && (defined I2CC_SLAVE)
# error I2CC enabled as both master and slave
#elif (defined I2CC_MASTER)
# define X_(p,s)  p ## C ## s
# include "masterx.inc.h"
#elif (defined I2CC_SLAVE)
# define X_(p,s)  p ## C ## s
# include "slavex.inc.h"
#endif

#if (defined I2CD_MASTER) && (defined I2CD_SLAVE)
# error I2CD enabled as both master and slave
#elif (defined I2CD_MASTER)
# define X_(p,s)  p ## D ## s
# include "masterx.inc.h"
#elif (defined I2CD_SLAVE)
# define X_(p,s)  p ## D ## s
# include "slavex.inc.h"
#endif

#if (defined I2CE_MASTER) && (defined I2CE_SLAVE)
# error I2CE enabled as both master and slave
#elif (defined I2CE_MASTER)
# define X_(p,s)  p ## E ## s
# include "masterx.inc.h"
#elif (defined I2CE_SLAVE)
# define X_(p,s)  p ## E ## s
# include "slavex.inc.h"
#endif

#if (defined I2CF_MASTER) && (defined I2CF_SLAVE)
# error I2CF enabled as both master and slave
#elif (defined I2CF_MASTER)
# define X_(p,s)  p ## E ## s
# include "masterx.inc.h"
#elif (defined I2CF_SLAVE)
# define X_(p,s)  p ## F ## s
# include "slavex.inc.h"
#endif

#endif


/** @brief Initialize all enabled I2Cs
 */
void i2c_init(void);


/** @brief Deinitialize all I2Cs */
void i2c_deinit(void);

/** @brief Send a frame to a slave
 *
 * @param  m  I2C master to use
 * @param  addr  slave address
 * @param  n  size to send (max: 127)
 * @param  data  buffer to send
 *
 * @retval -1  error before first byte has been sent
 * @retval  0  NACK has been immediately received
 * @retval  n  size of sent data (even on error / NACK)
 */
int8_t i2cm_send(i2cm_t *m, uint8_t addr, const uint8_t *data, uint8_t n);

/** @brief Receive a frame from a slave
 *
 * @param  m  I2C master to use
 * @param  addr  slave address
 * @param  n  size to read (max: 127)
 * @param  data  buffer for read data
 *
 * @retval -1  error before first byte has been received
 * @retval  0  NACK has been immediately received
 * @retval  n  size of sent data
 */
int8_t i2cm_recv(i2cm_t *m, uint8_t addr, uint8_t *data, uint8_t n);

/** @brief Asynchonously send a frame to a slave, 
 * write completion will be notified using the write_completed callback function
 *
 * @param addr slave address
 * @param n size to send
 * @param data buffer to send
 * @param f function which will be called on write completion (success or failure)
 * @param payload pointer which will be passed as argument to callback function
 *
 * @retval -1 send size overflow internal buffer
 * @retval -2 another async send/recv is underway
 * @retval -3 i2c bus was not ready
 * @retval n  size of data which will be sent
 */
int8_t i2cm_async_send(i2cm_t *i2c, uint8_t addr, const uint8_t *data, uint8_t n,
                        i2cm_write_completed_callback f, void *payload);

/** @brief Asynchonously receive a frame from a slave,
 * read completion will be notified using the read_completed callback function
 * @param addr slave address
 * @param n receive buffer size
 * @param f function which will be called on read completion (success or failure)
 * @param payload pointer which will be passed as argument to callback function
 * @retval -1 recv size overflowed internal buffer
 * @retval -2 another async send/recv is underway
 * @retval -3 i2c bus was not ready
 * @retval 0 on success
 */
int8_t i2cm_async_recv(i2cm_t *i2c, uint8_t addr, uint8_t n,
                        i2cm_read_completed_callback f, void *payload);

/** @brief Set I2C slave peripheral own address */
void i2cs_set_address(i2cs_t *s, uint8_t address);

/** @brief Register f to be called whenever a master-read operation was requested and user need
 * to provision the send buffer */
void i2cs_register_prepare_send_callback(i2cs_t *, i2cs_prepare_send_callback_t f);

/** @brief Register f to be called whenever a master-write operation as finished */
void i2cs_register_recv_callback(i2cs_t *, i2cs_recv_callback_t f);

/** @brief Register f to be called whenever the i2c transaction was terminated (on STOP or ERROR) */
void i2cs_register_reset_callback(i2cs_t *, i2cs_reset_callback_t f);

#endif
//@}
