/**
 * @cond internal
 * @file
 */
#include "i2c.h"
#include "i2c_config.h"

#include <string.h>

// Apply default configuration values
// Include template for each enabled I2C
// Define i2cX_init()

// Command to duplicate I2CC code for each I2C.
// Vimmers will use it with " :*! ".
//   python -c 'import sys,re; s=sys.stdin.read(); print "\n".join(re.sub("(I2C\x7ci2c\x7c )C", r"\1"+x, s) for x in "CDEF")'

#if (defined I2CC_MASTER) || (defined I2CC_SLAVE)
# define X_(p,s)  p ## C ## s
# ifndef I2CC_BAUDRATE
#  define I2CC_BAUDRATE  I2C_BAUDRATE
# endif
# ifdef I2CC_MASTER
#  include "masterx.inc.c"
# else
#  include "slavex.inc.c"
# endif
#else
# define i2cC_init()
# define i2cC_deinit()
#endif

#if (defined I2CD_MASTER) || (defined I2CD_SLAVE)
# define X_(p,s)  p ## D ## s
# ifndef I2CD_BAUDRATE
#  define I2CD_BAUDRATE  I2C_BAUDRATE
# endif
# ifdef I2CD_MASTER
#  include "masterx.inc.c"
# else
#  include "slavex.inc.c"
# endif
#else
# define i2cD_init()
# define i2cD_deinit()
#endif

#if (defined I2CE_MASTER) || (defined I2CE_SLAVE)
# define X_(p,s)  p ## E ## s
# ifndef I2CE_BAUDRATE
#  define I2CE_BAUDRATE  I2C_BAUDRATE
# endif
# ifdef I2CE_MASTER
#  include "masterx.inc.c"
# else
#  include "slavex.inc.c"
# endif
#else
# define i2cE_init()
# define i2cE_deinit()
#endif

#if (defined I2CF_MASTER) || (defined I2CF_SLAVE)
# define X_(p,s)  p ## F ## s
# ifndef I2CF_BAUDRATE
#  define I2CF_BAUDRATE  I2C_BAUDRATE
# endif
# ifdef I2CF_MASTER
#  include "masterx.inc.c"
# else
#  include "slavex.inc.c"
# endif
#else
# define i2cF_init()
# define i2cF_deinit()
#endif



void i2c_init(void)
{
  i2cC_init();
  i2cD_init();
  i2cE_init();
  i2cF_init();
}

void i2c_deinit(void)
{
  i2cC_deinit();
  i2cD_deinit();
  i2cE_deinit();
  i2cF_deinit();
}

int8_t i2cm_send(i2cm_t *i2cm, uint8_t addr, const uint8_t *data, uint8_t n)
{
  TWI_MASTER_t *m = i2cm->master;

  uint8_t status;

  // slave address + Write bit (0)
  m->ADDR = addr << 1;
  while(!((status = m->STATUS) & (TWI_MASTER_RIF_bm|TWI_MASTER_WIF_bm))) ;
  if(!(status & TWI_MASTER_WIF_bm)) {
    return -1;
  } else if(status & TWI_MASTER_RXACK_bm) {
    // NACK
    m->CTRLC = TWI_MASTER_CMD_STOP_gc;
    return 0;
  }

  uint8_t i;
  for(i=0; i<n; ) {
    m->DATA = data[i++];
    while(!((status = m->STATUS) & (TWI_MASTER_RIF_bm|TWI_MASTER_WIF_bm))) ;
    if(!(status & TWI_MASTER_WIF_bm)) {
      return -1;
    } else if(status & TWI_MASTER_RXACK_bm) {
      break;
    }
  }
  m->CTRLC = TWI_MASTER_CMD_STOP_gc;

  return i;
}


int8_t i2cm_recv(i2cm_t *i2cm, uint8_t addr, uint8_t *data, uint8_t n)
{
  TWI_MASTER_t *m = i2cm->master;

  uint8_t status;

  // slave address + Read bit (1)
  m->ADDR = (addr << 1) + 1;
  while(!((status = m->STATUS) & (TWI_MASTER_RIF_bm|TWI_MASTER_WIF_bm))) ;
  if(status & (TWI_MASTER_ARBLOST_bm|TWI_MASTER_BUSERR_bm)) {
    return -1;
  } else if(!(status & TWI_MASTER_RIF_bm)) {
    // NACK
    m->CTRLC = TWI_MASTER_CMD_STOP_gc;
    return 0;
  }

  uint8_t i;
  for(i=0; i<n-1; i++) {
    data[i] = m->DATA;
    m->CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
    while(!((status = m->STATUS) & TWI_MASTER_RIF_bm)) ;
  }
  data[i++] = m->DATA;
  m->CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;

  return i;
}

int8_t i2cm_async_send(i2cm_t *i2cm, uint8_t addr, const uint8_t *data, uint8_t n,
                        i2cm_write_completed_callback f, void* payload)
{
  TWI_MASTER_t *m = i2cm->master;

  // check if send size overflow internal buffer
  if(n > sizeof(i2cm->buffer)) {
    return -1;
  }

  INTLVL_DISABLE_BLOCK(I2C_INTLVL) {

    // check if an async read or write is currently under way
    if(i2cm->bytes_to_send != 0) {
      return -2;
    }

    // check bus is idle, return otherwise
    if((m->STATUS & TWI_MASTER_BUSSTATE_gm) != TWI_MASTER_BUSSTATE_IDLE_gc) {
      return -3;
    }

    // prepare send buffer
    memcpy(i2cm->buffer, data, n);
    i2cm->bytes_count = 0;
    i2cm->bytes_to_send = n;
    i2cm->bytes_to_recv = 0;

    // register callback
    i2cm->write_completed_callback = f;
    i2cm->write_completed_callback_payload = payload;

    // enable write interruption
    m->CTRLA |= TWI_MASTER_WIEN_bm;
    // push slave address + write bit (0);
    m->ADDR = (addr << 1) + 0;

  }
  return n;
}

int8_t i2cm_async_recv(i2cm_t *i2cm, uint8_t addr, uint8_t n,
                        i2cm_read_completed_callback f, void* payload)
{
  TWI_MASTER_t *m = i2cm->master;

  // check if recv size overflow internal buffer
  if(n > sizeof(i2cm->buffer)) {
    return -1;
  }

  INTLVL_DISABLE_BLOCK(I2C_INTLVL) {

    // check if an async read or write is currently under way
    if(i2cm->bytes_to_send != 0) {
      return -2;
    }

    // check bus is idle, return otherwise
    if((m->STATUS & TWI_MASTER_BUSSTATE_gm) != TWI_MASTER_BUSSTATE_IDLE_gc) {
      return -3;
    }

    // prepare read 
    i2cm->bytes_count = 0;
    i2cm->bytes_to_send = 0;
    i2cm->bytes_to_recv = n;

    // register callback
    i2cm->read_completed_callback = f;
    i2cm->read_completed_callback_payload = payload;

    // enable interruptions
    m->CTRLA |= TWI_MASTER_WIEN_bm;
    m->CTRLA |= TWI_MASTER_RIEN_bm;
    // push slave address + read bit (1)
    m->ADDR = (addr<<1) | 1;
  }

  return 0;
}


void i2cs_set_address(i2cs_t *i2cs, uint8_t address) {

  TWI_SLAVE_t *slave = i2cs->slave;
  INTLVL_DISABLE_BLOCK(I2C_INTLVL) {
    slave->ADDR = address << 1;
  }
}

void i2cs_register_reset_callback(i2cs_t *i2c, i2cs_reset_callback_t f)
{
  INTLVL_DISABLE_BLOCK(I2C_INTLVL) {
    i2c->reset_callback = f;
  }
}

void i2cs_register_recv_callback(i2cs_t *i2c, i2cs_recv_callback_t f)
{
  INTLVL_DISABLE_BLOCK(I2C_INTLVL) {
    i2c->recv_callback = f;
  }
}

void i2cs_register_prepare_send_callback(i2cs_t *i2c, i2cs_prepare_send_callback_t f)
{
  INTLVL_DISABLE_BLOCK(I2C_INTLVL) {
    i2c->prepare_send_callback = f;
  }
}



///@endcond
