/**
 * @internal
 * @file
 * @brief Include template code for i2c.c
 *
 * The X_(p,s) macro must be defined before including.
 * It is automatically undefined at the end of this file.
 */
#include <avarix.h>
#include <clock/defs.h>
#include <avr/interrupt.h>

#define I2CX(s) X_(I2C,s)
#define i2cX(s) X_(i2c,s)
#define TWIX X_(TWI,)
#define twiX(s) X_(TWI,s)

// declare i2cX singleton
i2cm_t i2cX();

// check configuration
#if I2CX(_BAUDRATE) > 400000
# error I2C baudrate must not be higher than 400kHz
#endif

void i2cX(_init)(void)
{
  i2cX().master = &TWIX.MASTER;

  i2cX().bytes_sent = 0;
  i2cX().bytes_to_send = 0;
  i2cX().write_completed_callback = NULL;

  // write baudrate first (master should be disabled)
  TWIX.MASTER.BAUD = ((CLOCK_SYS_FREQ)/(2*(I2CX(_BAUDRATE)))) - 5;

  TWIX.MASTER.CTRLA =
      TWI_MASTER_ENABLE_bm |
      (((I2C_INTLVL) << TWI_MASTER_INTLVL_gp) & TWI_MASTER_INTLVL_gm)
      ;
  TWIX.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
}

void i2cX(_deinit)(void)
{
  TWIX.MASTER.BAUD = 0;
  TWIX.MASTER.CTRLA = 0;
  TWIX.MASTER.STATUS = 0;
}

// TWI MASTER interruption handler
ISR(twiX(_TWIM_vect)) 
{
  i2cm_t *i2cm = &i2cX();
  uint8_t status = TWIX.MASTER.STATUS;

  // on write interrupt
  if(status & TWI_MASTER_WIF_bm) {

    if(status & (TWI_MASTER_ARBLOST_bm | TWI_MASTER_BUSERR_bm)) {
      // bus error
      if(i2cm->write_completed_callback) {
        i2cm->write_completed_callback(-2, i2cm->write_completed_callback_payload);
      }

      i2cm->bytes_to_send = 0;
      return;
    }

    if(status & TWI_MASTER_RXACK_bm) {
      // NACK received
      TWIX.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;

      if(i2cm->write_completed_callback) {
        i2cm->write_completed_callback(-1, i2cm->write_completed_callback_payload);
      }

      i2cm->bytes_to_send = 0;
      return;
    }

    if(i2cm->bytes_sent >= i2cm->bytes_to_send) {
      // finish transaction
      TWIX.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
      // clear write interruption
      TWIX.MASTER.CTRLA &= ~TWI_MASTER_WIEN_bm;

      i2cm->bytes_to_send = 0;

      if(i2cm->write_completed_callback) {
        i2cm->write_completed_callback(i2cm->bytes_sent, i2cm->write_completed_callback_payload);
      }
      return;
    }
    else {
      char byte = i2cm->send_buffer[i2cm->bytes_sent++];
      TWIX.MASTER.DATA = byte;
    }
  }
}

#undef I2CX
#undef i2cX
#undef TWIX
#undef X_
