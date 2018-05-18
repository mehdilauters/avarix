#include "mfrc522.h" 
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <util/atomic.h>
#include <clock/defs.h>
#include "mfrc522_config.h"

//delay
#include <util/delay.h>

#if (CLOCK_PER_FREQ / MFRC522_SPI_PRESCALER) > 1000000
# error MFRC522_SPI_PRESCALER is too low, max MFRC522 SPI frequency is 1MHz
#endif

void mfrc522_spi_init(void)
{
 
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

// Receive a single byte from SPI
static inline uint8_t mfrc522_spi_recv(void)
{
  MFRC522_SPI.DATA = 0;
  while(!(MFRC522_SPI.STATUS & SPI_IF_bm)) ;
  return MFRC522_SPI.DATA;
}

// Receive a single byte from SPI
static inline uint8_t mfrc522_spi_send(uint8_t _data)
{
  MFRC522_SPI.DATA = _data;
  while(!(MFRC522_SPI.STATUS & SPI_IF_bm)) ;
  return MFRC522_SPI.DATA;
}

static void mfrc522_write(mfrc522_t *_mfrc522, uint8_t reg, uint8_t data)
{
  portpin_outclr(&PORTPIN_SPI_SS(&MFRC522_SPI));
  mfrc522_spi_send((reg<<1)&0x7E);
  mfrc522_spi_send(data);
  portpin_outset(&PORTPIN_SPI_SS(&MFRC522_SPI));
}

static uint8_t mfrc522_read(mfrc522_t *_mfrc522, uint8_t reg)
{
  portpin_outclr(&PORTPIN_SPI_SS(&MFRC522_SPI));
  uint8_t data;	
  mfrc522_spi_send(((reg<<1)&0x7E)|0x80);
  data = mfrc522_spi_send(0x00);
  portpin_outset(&PORTPIN_SPI_SS(&MFRC522_SPI));
  return data;
}

static void mfrc522_reset(mfrc522_t *_mfrc522) {
  mfrc522_write(_mfrc522, MFRC522_REG_COMMANDREG, MFRC522_COMMAND_SOFT_RESET);
}

mfrc522_status_t mfrc522_to_card(mfrc522_t *_mfrc522, uint8_t cmd, uint8_t *send_data, uint8_t send_data_len, uint8_t *back_data, uint32_t *back_data_len)
{
  mfrc522_status_t status = MFRC522_STATUS_ERROR;
  uint8_t irqEn = 0x00;
  uint8_t waitIRq = 0x00;
  uint8_t lastBits;
  uint8_t n;
  uint8_t	tmp;
  uint32_t i;
  
  switch (cmd) {
    case MFRC522_COMMAND_MF_AUTHENT: { //Certification cards close
      irqEn = 0x12;
      waitIRq = 0x10;
      break;
    }
    case MFRC522_COMMAND_TRANSCEIVE: { //Transmit FIFO data
      irqEn = 0x77;
      waitIRq = 0x30;
      break;
    }
    default:
      break;
  }
  
  //mfrc522_write(ComIEnReg, irqEn|0x80);	//Interrupt request
  n = mfrc522_read(_mfrc522, MFRC522_REG_COMIRQREG);
  mfrc522_write(_mfrc522, MFRC522_REG_COMIRQREG,n&(~0x80));//clear all interrupt bits
  n = mfrc522_read(_mfrc522, MFRC522_REG_FIFOLEVELREG);
  mfrc522_write(_mfrc522, MFRC522_REG_FIFOLEVELREG,n|0x80);//flush FIFO data
  
  mfrc522_write(_mfrc522, MFRC522_REG_COMMANDREG, MFRC522_COMMAND_IDLE);	//NO action; Cancel the current cmd???
  
  //Writing data to the FIFO
  for (i=0; i<send_data_len; i++) {
    mfrc522_write(_mfrc522, MFRC522_REG_FIFODATAREG, send_data[i]); 
  }
  
  //Execute the cmd
  mfrc522_write(_mfrc522, MFRC522_REG_COMMANDREG, cmd);
  if (cmd == MFRC522_COMMAND_TRANSCEIVE) { 
    n = mfrc522_read(_mfrc522, MFRC522_REG_BITFRAMINGREG);
    mfrc522_write(_mfrc522, MFRC522_REG_BITFRAMINGREG,n|0x80);  
  }
  
  //Waiting to receive data to complete
  i = 2000;	//i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms???
  do {
    //CommIrqReg[7..0]
    //Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
    n = mfrc522_read(_mfrc522, MFRC522_REG_COMIRQREG);
    i--;
  }
  while ((i!=0) && !(n&0x01) && !(n&waitIRq));
  
  tmp=mfrc522_read(_mfrc522, MFRC522_REG_BITFRAMINGREG);
  mfrc522_write(_mfrc522, MFRC522_REG_BITFRAMINGREG,tmp&(~0x80));
  
  if (i != 0) { 
    if(!(mfrc522_read(_mfrc522, MFRC522_REG_ERRORREG) & 0x1B)) { //BufferOvfl Collerr CRCErr ProtecolErr
      status = MFRC522_STATUS_CARD_FOUND;
      if (n & irqEn & 0x01) {
        status = MFRC522_STATUS_CARD_NOT_FOUND;			//??
      }
      
      if (cmd == MFRC522_COMMAND_TRANSCEIVE) {
        n = mfrc522_read(_mfrc522, MFRC522_REG_FIFOLEVELREG);
        lastBits = mfrc522_read(_mfrc522, MFRC522_REG_CONTROLREG) & 0x07;
        if (lastBits) {
          *back_data_len = (n-1)*8 + lastBits;
        } else {
          *back_data_len = n*8;
        }
        
        if (n == 0) {
          n = 1; 
        }
        if (n > MFRC522_MAX_LEN) {
          n = MFRC522_MAX_LEN;
        }
        
        //Reading the received data in FIFO
        for (i=0; i<n; i++) {
          back_data[i] = mfrc522_read(_mfrc522, MFRC522_REG_FIFODATAREG); 
        }
      }
    } else {
      status = MFRC522_STATUS_ERROR;  
    }
    
  }
  
  //SetBitMask(ControlReg,0x80);  //timer stops
  //mfrc522_write(cmdReg, PCD_IDLE); 
  
  return status;
}

mfrc522_status_t mfrc522_request(mfrc522_t *_mfrc522, uint8_t req_mode, uint8_t * tag_type)
{
  mfrc522_status_t  status;  
  uint32_t backBits;//The received data bits
  
  mfrc522_write(_mfrc522, MFRC522_REG_BITFRAMINGREG, 0x07);//TxLastBists = MFRC522_REG_BITFRAMINGREG[2..0]	???
  
  tag_type[0] = req_mode;
  status = mfrc522_to_card(_mfrc522, MFRC522_COMMAND_TRANSCEIVE, tag_type, 1, tag_type, &backBits);
  
  if ((status != MFRC522_STATUS_CARD_FOUND) || (backBits != 0x10)) { 
    status = MFRC522_STATUS_ERROR;
  }
  
  return status;
}

void mfrc522_init(mfrc522_t *_mfrc522, portpin_t *_reset) {
  
  _mfrc522->reset = _reset;
  _mfrc522->available = false;
  portpin_dirset(_mfrc522->reset);
  
  uint8_t byte;
  mfrc522_reset(_mfrc522);
  mfrc522_write(_mfrc522, MFRC522_REG_MODEREG, 0x8D);
  mfrc522_write(_mfrc522, MFRC522_REG_TPRESCALERREG, 0x3E);
  mfrc522_write(_mfrc522, MFRC522_REG_TRELOADREG_1, 30);
  mfrc522_write(_mfrc522, MFRC522_REG_TRELOADREG_2, 0);	
  mfrc522_write(_mfrc522, MFRC522_REG_TXASKREG, 0x40);	
  mfrc522_write(_mfrc522, MFRC522_REG_TMODEREG, 0x3D);
  byte = mfrc522_read(_mfrc522, MFRC522_REG_TXCONTROLREG);
  if(!(byte&0x03)) {
    mfrc522_write(_mfrc522, MFRC522_REG_TXCONTROLREG,byte|0x03);
  }
}

mfrc522_status_t mfrc522_get_card_serial(mfrc522_t *_mfrc522, uint8_t * serial_out)
{
  mfrc522_status_t status;
  uint8_t i;
  uint8_t serNumCheck=0;
  uint32_t unLen;
  
  mfrc522_write(_mfrc522, MFRC522_REG_BITFRAMINGREG, 0x00);		//TxLastBists = MFRC522_REG_BITFRAMINGREG[2..0]
  
  serial_out[0] = MFRC522_MIFAREONE_COMMAND_PICC_ANTICOLL;
  serial_out[1] = 0x20;
  status = mfrc522_to_card(_mfrc522, MFRC522_COMMAND_TRANSCEIVE, serial_out, 2, serial_out, &unLen);
  
  if (status == MFRC522_STATUS_CARD_FOUND) {
    //Check card serial number
    for (i=0; i<4; i++) {
      serNumCheck ^= serial_out[i];
    }
    if (serNumCheck != serial_out[i]) {
      status = MFRC522_STATUS_ERROR; 
    }
  }
  return status;
}

mfrc522_version_t mfrc522_get_version(mfrc522_t *_mfrc522) {
  mfrc522_version_t version;
  //check version of the reader
  uint8_t byte = mfrc522_read(_mfrc522, MFRC522_REG_VERSIONREG);
  if(byte == 0x92) {
    version = MFRC522_VERSION_RC522v2;
  } else if(byte == 0x91 || byte==0x90) {
    version = MFRC522_VERSION_RC522v1;
  } else {
    version = MFRC522_VERSION_UNKNOWN;
  }
  return version;
}


void mfrc522_update(mfrc522_t *_mfrc522) {
  _mfrc522->available = false;
  uint8_t str[MFRC522_MAX_LEN];
  mfrc522_status_t res = mfrc522_request(_mfrc522,  MFRC522_MIFAREONE_COMMAND_PICC_REQALL,str);
  switch(res) {
    case MFRC522_STATUS_CARD_FOUND:
    {
      memset(_mfrc522->tag, 0, MFRC522_MAX_LEN);
      mfrc522_status_t idres = mfrc522_get_card_serial(_mfrc522, _mfrc522->tag);
      if( idres != MFRC522_STATUS_ERROR ) {
        _mfrc522->available = true;
      } else {
        // error
      }
      break;
    }
    case MFRC522_STATUS_CARD_NOT_FOUND:
      break;
    case MFRC522_STATUS_ERROR:
      break;
  }
}

uint8_t * mfrc522_get_tag(mfrc522_t *_mfrc522) {
  if(_mfrc522->available) {
    return _mfrc522->tag;
  }
  return NULL;
}

void mfrc522_power(mfrc522_t *_mfrc522, bool _on) {
  if(_on) {
    portpin_outset(_mfrc522->reset);
  } else {
    portpin_outclr(_mfrc522->reset);
  }
}
