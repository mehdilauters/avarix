#include "mfrc522.h"
#include "mfrc522_config.h"
#include "registers.h"
#include <string.h>

static void spi_init(void) {
  // setup SPI pins
  portpin_dirset(&PORTPIN_SPI_MOSI(&MFRC522_SPI));
  portpin_dirclr(&PORTPIN_SPI_MISO(&MFRC522_SPI));
  portpin_dirset(&PORTPIN_SPI_SCK(&MFRC522_SPI));
  portpin_dirset(&PORTPIN_SPI_SS(&MFRC522_SPI));
  portpin_outset(&PORTPIN_SPI_SS(&MFRC522_SPI));
  
  MFRC522_SPI.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc |
  #if MFRC522_SPI_PRESCALER == 2
  SPI_PRESCALER_DIV4_gc | SPI_CLK2X_bm
  #elif MFRC522_SPI_PRESCALER == 4
  SPI_PRESCALER_DIV4_gc
  #elif MFRC522_SPI_PRESCALER == 8
  SPI_PRESCALER_DIV16_gc | SPI_CLK2X_bm
  #elif MFRC522_SPI_PRESCALER == 16
  SPI_PRESCALER_DIV16_gc
  #elif MFRC522_SPI_PRESCALER == 32
  SPI_PRESCALER_DIV64_gc | SPI_CLK2X_bm
  #elif MFRC522_SPI_PRESCALER == 64
  SPI_PRESCALER_DIV64_gc
  #elif MFRC522_SPI_PRESCALER == 128
  SPI_PRESCALER_DIV128_gc
  #else
  # error Invalid MFRC522_SPI_PRESCALER value
  #endif
  ;
}

static uint8_t spi_transceive(uint8_t byte) {
  MFRC522_SPI.DATA = byte;
  while(!(MFRC522_SPI.STATUS & SPI_IF_bm));
  return MFRC522_SPI.DATA;
}

static void write_register(uint8_t reg, uint8_t data) {
  portpin_outclr(&PORTPIN_SPI_SS(&MFRC522_SPI));
  // LSB has to be set to 0, MSB has set to 0 for a write operation
  spi_transceive((reg<<1)&0x7E);
  spi_transceive(data);
  portpin_outset(&PORTPIN_SPI_SS(&MFRC522_SPI));
}

static uint8_t read_register(uint8_t reg) {
  // LSB has to be set to 0, MSB has set to 1 for a read operation
  portpin_outclr(&PORTPIN_SPI_SS(&MFRC522_SPI));
  spi_transceive(((reg<<1)&0x7E)|0x80);
  uint8_t byte = spi_transceive(0);
  portpin_outset(&PORTPIN_SPI_SS(&MFRC522_SPI));
  return byte;
}

static uint8_t crc_checksum(void *buffer, int len) {
  uint8_t crc = 0;
  for(int i=0;i<len;i++) {
    crc ^= ((char*)buffer)[i];
  }
  return crc;
}

static void reset(mfrc522_t *m, bool b) {
  if(b)
    portpin_outclr(m->reset);
  else
    portpin_outset(m->reset);
}

static void configure(mfrc522_t *m) {
  write_register(MFRC522_REG_COMMANDREG, MFRC522_COMMAND_SOFT_RESET);
  write_register(MFRC522_REG_MODEREG,
    MFRC522_REG_MODEREG_MSBFIRST_bm
    | MFRC522_REG_MODEREG_POLMFIN_bm
    | MFRC522_REG_MODEREG_CRCPRESET_01_bm);
  write_register(MFRC522_REG_TPRESCALERREG, 0x3E);
  write_register(MFRC522_REG_TRELOADREG_1, 30);
  write_register(MFRC522_REG_TRELOADREG_2, 0);
  write_register(MFRC522_REG_TXASKREG,
    MFRC522_REG_TXASKREG_FORCE100ASK_bm);
  write_register(MFRC522_REG_TMODEREG,
    MFRC522_REG_TMODEREG_TGATED_MFIN_bm
    | MFRC522_REG_TMODEREG_TAUTORESTART_bm
    | (0x0D << MFRC522_REG_TMODEREG_TPRESCALERHI_bp));

  // check if both TX antenna are emiting
  uint8_t byte = read_register(MFRC522_REG_TXCONTROLREG);
  (void)byte;
  bool txen = (byte & MFRC522_REG_TXCONTROLREG_TX2RFEN_bm) && (byte & MFRC522_REG_TXCONTROLREG_TX1RFEN_bm);
  if(!txen) {
    write_register(MFRC522_REG_TXCONTROLREG, byte
      | MFRC522_REG_TXCONTROLREG_TX1RFEN_bm
      | MFRC522_REG_TXCONTROLREG_TX2RFEN_bm);
  }
}

static void start_transceive(mfrc522_t *m, uint8_t *send_buffer, int send_buffer_len) {
  uint8_t v;
  // clear all interrupts bits
  v = read_register(MFRC522_REG_COMIRQREG);
  //write_register(MFRC522_REG_COMIRQREG, v & (~MFRC522_REG_COMIRQREG_SET1_bm)); 
  write_register(MFRC522_REG_COMIRQREG, v & (~0x80)); 
  // flush fifo
  v = read_register(MFRC522_REG_FIFOLEVELREG);
  //write_register(MFRC522_REG_FIFOLEVELREG, v|MFRC522_REG_FIFOLEVELREG_FLUSHBUFFER_bm);
  write_register(MFRC522_REG_FIFOLEVELREG, v|0x80);
  // cancel current actions
  write_register(MFRC522_REG_COMMANDREG, MFRC522_COMMAND_IDLE);

  // write data to FIFO
  for(int i=0; i<send_buffer_len; i++) {
    write_register(MFRC522_REG_FIFODATAREG, send_buffer[i]);
  }

  // transceive data
  write_register(MFRC522_REG_COMMANDREG, MFRC522_COMMAND_TRANSCEIVE);
  // begin transmission
  v = read_register(MFRC522_REG_BITFRAMINGREG);
  write_register(MFRC522_REG_BITFRAMINGREG, v|0x80);

}

bool poll_transceive(mfrc522_t *m) {
  uint8_t v = read_register(MFRC522_REG_COMIRQREG);

  if(v&MFRC522_REG_COMIRQREG_ERRIRQ_bm) {
    return true;
  }

  if((v&MFRC522_REG_COMIRQREG_RXIRQ_bm) || (v&MFRC522_REG_COMIRQREG_IDLEIRQ_bm)) {
    return true;
  }

  return false;
}

int stop_transceive(mfrc522_t *m) {
  // end transmission
  uint8_t v = read_register(MFRC522_REG_BITFRAMINGREG);
  write_register(MFRC522_REG_BITFRAMINGREG, v & 0x7f);

  return -1;
}

int read_transceived(mfrc522_t *m, uint8_t *buffer, int buffer_len) {
  uint8_t v;

  // read error register
  v = read_register(MFRC522_REG_ERRORREG);
  const uint8_t error_mask = MFRC522_REG_ERRORREG_BUFFEROVFL_bm
    | MFRC522_REG_ERRORREG_COLLERR_bm
    | MFRC522_REG_ERRORREG_PROTOCOLERR_bm
    | MFRC522_REG_ERRORREG_PARITYERR_bm;
  if(v & error_mask) {
    return -MFRC522_ERROR_TRANSMISSION;
  }

  // fetch data from FIFO
  uint8_t n = read_register(MFRC522_REG_FIFOLEVELREG);
  uint8_t last_bits = read_register(MFRC522_REG_CONTROLREG) & 0x07;

  if(last_bits != 0x00) {
    // NDJD: this code currently does not handle last_bits != 0
    return -MFRC522_ERROR_LAST_BITS_NOT_NULL;
  }
  
  // limit read size
  if(n > buffer_len) {
    n = buffer_len;
  }

  int i=0;
  for(; i<n; i++) {
    buffer[i] = read_register(MFRC522_REG_FIFODATAREG);
  }
  return i;
}

void mfrc522_init(mfrc522_t *m, portpin_t *rpp) {
  // configure SPI
  spi_init();

  m->reset = rpp;
  portpin_dirset(rpp);
  reset(m,true);

  m->state = MFRC522_STATE_INIT;
}

bool mfrc522_update(mfrc522_t *m) {

  switch(m->state) {
    case MFRC522_STATE_INIT:
      m->state = MFRC522_STATE_POWERON;
      // no break
    case MFRC522_STATE_POWERON:
      reset(m,false);
      m->state = MFRC522_STATE_CONFIGURE;
      break;

    case MFRC522_STATE_CONFIGURE:
      configure(m);
      m->state = MFRC522_STATE_REQUEST;
      break;

    case MFRC522_STATE_REQUEST: {
      write_register(MFRC522_REG_BITFRAMINGREG,
        0x7 << MFRC522_REG_BITFRAMINGREG_TXLASTBITS_bp);

      uint8_t mode = MFRC522_MIFAREONE_COMMAND_PICC_REQALL;
      start_transceive(m,&mode,1);

      m->state = MFRC522_STATE_POLL;
      m->polling_counter = 0;
      break;
    }

    case MFRC522_STATE_POLL: {
      m->polling_counter++;
      if(poll_transceive(m)) {
        stop_transceive(m);
        m->state = MFRC522_STATE_REQUEST_UID;
      }
      else {
        if(m->polling_counter > 10) {
          stop_transceive(m);
          m->serial_validity = false;
          m->state = MFRC522_STATE_POWEROFF;
        }
      }
      break;
    }
  
    case MFRC522_STATE_REQUEST_UID: {
      uint8_t buffer[16];
      int rlen = read_transceived(m,buffer,sizeof(buffer)); 
      
      if(rlen > 0) {
        // something is on the line !
        // prepare a request asking tag for his UID
        write_register(MFRC522_REG_BITFRAMINGREG, 0);
        uint8_t request[] = {
          MFRC522_MIFAREONE_COMMAND_PICC_ANTICOLL,
          0x20
        };

        start_transceive(m, request, sizeof(request));
        m->polling_counter = 0;
        m->state = MFRC522_STATE_POLL_UID;
        break;
      }

      m->serial_validity = false;
      m->state = MFRC522_STATE_POWEROFF;
      break;
    }
    
    case MFRC522_STATE_POLL_UID: {
      uint8_t buffer[5];
      m->polling_counter++;
      if(poll_transceive(m)) {
        stop_transceive(m);
        int n = read_transceived(m,buffer,sizeof(buffer));
        
        // we expect 5 bytes for an UID
        if(n == 5) {
          uint32_t serial;
          memcpy(&serial, buffer, sizeof(uint32_t));
          uint8_t crc = buffer[4];

          if(crc_checksum(&serial, sizeof(uint32_t)) == crc) {
            m->serial = serial;
            m->serial_validity = true;
            m->state = MFRC522_STATE_POWEROFF;
            break;
          }
        }
        m->serial_validity = false;
        m->state = MFRC522_STATE_POWEROFF;
        break;
      }
      else {
        if(m->polling_counter > 10) {
          stop_transceive(m);
          m->serial_validity = false;
          m->state = MFRC522_STATE_POWEROFF;
        }
      }
    }

    case MFRC522_STATE_POWEROFF:
      reset(m,true);
      m->state = MFRC522_STATE_INIT;
      break;

    default:
      break;
  }

  return m->state == MFRC522_STATE_INIT;
}

uint32_t *mfrc522_get_tag_uid(mfrc522_t *m) {
  if(m->serial_validity) {
    return &m->serial;
  }
  return NULL;
}
