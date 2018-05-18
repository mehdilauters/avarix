#ifndef MFRC522_H__ 
#define MFRC522_H__

#include <avarix/portpin.h>

#define MFRC522_MAX_LEN 16

// https://github.com/ianuragit/MIFARE-RFID-with-AVR/blob/master/RFID-example.tar.gz

// mfrc522 is quite bugged using SPI: it doesn't respect chip select, so we have to also use its reset pin
typedef struct {
  portpin_t *reset;
  uint8_t tag[MFRC522_MAX_LEN];
  bool available;
} mfrc522_t;

typedef enum {
  MFRC522_VERSION_UNKNOWN,
  MFRC522_VERSION_RC522v1,
  MFRC522_VERSION_RC522v2,
} mfrc522_version_t;

typedef enum {
  MFRC522_STATUS_CARD_FOUND,
  MFRC522_STATUS_CARD_NOT_FOUND,
  MFRC522_STATUS_ERROR
} mfrc522_status_t;

typedef enum {
  MFRC522_COMMAND_IDLE = 0x00,
  MFRC522_COMMAND_MEM = 0x01,
  MFRC522_COMMAND_GENERATE_RANDOM_ID = 0x02,
  MFRC522_COMMAND_CALC_CRC = 0x03,
  MFRC522_COMMAND_TRANSMIT = 0x04,
  MFRC522_COMMAND_NO_COMMAND_CHANGE = 0x07,
  MFRC522_COMMAND_RECEIVE = 0x08,
  MFRC522_COMMAND_TRANSCEIVE = 0x0C,
  MFRC522_COMMAND_RESERVED = 0x0D,
  MFRC522_COMMAND_MF_AUTHENT = 0x0E,
  MFRC522_COMMAND_SOFT_RESET = 0x0F
} mfrc522_command_t;

typedef enum {
  //PAGE 0 ==> COMMAND AND STATUS
  MFRC522_REG_PAGE0_RESERVED_1 = 0x00,
  MFRC522_REG_COMMANDREG = 0x01,
  MFRC522_REG_COMIENREG = 0x02,
  MFRC522_REG_DIVIENREG = 0x03,
  MFRC522_REG_COMIRQREG = 0x04,
  MFRC522_REG_DIVIRQREG = 0x05,
  MFRC522_REG_ERRORREG = 0x06,
  MFRC522_REG_STATUS1REG = 0x07,
  MFRC522_REG_STATUS2REG = 0x08,
  MFRC522_REG_FIFODATAREG = 0x09,
  MFRC522_REG_FIFOLEVELREG = 0x0A,
  MFRC522_REG_WATERLEVELREG = 0x0B,
  MFRC522_REG_CONTROLREG = 0x0C,
  MFRC522_REG_BITFRAMINGREG = 0x0D,
  MFRC522_REG_COLLREG = 0x0E,
  MFRC522_REG_PAGE0_RESERVED_2 = 0x0F,
  
  //PAGE 1 ==> COMMAND
  MFRC522_REG_PAGE1_RESERVED_1 = 0x10,
  MFRC522_REG_MODEREG = 0x11,
  MFRC522_REG_TXMODEREG = 0x12,
  MFRC522_REG_RXMODEREG = 0x13,
  MFRC522_REG_TXCONTROLREG = 0x14,
  MFRC522_REG_TXASKREG = 0x15,
  MFRC522_REG_TXSELREG = 0x16,
  MFRC522_REG_RXSELREG = 0x17,
  MFRC522_REG_RXTHRESHOLDREG = 0x18,
  MFRC522_REG_DEMODREG = 0x19,
  MFRC522_REG_PAGE1_RESERVED_2 = 0x1A,
  MFRC522_REG_PAGE1_RESERVED_3 = 0x1B,
  MFRC522_REG_MFTXREG = 0x1C,
  MFRC522_REG_MFRXREG = 0x1D,
  MFRC522_REG_PAGE1_RESERVED_4 = 0x1E,
  MFRC522_REG_SERIALSPEEDREG = 0x1F,
  
  //PAGE 2 ==> CFG
  MFRC522_REG_PAGE2_RESERVED_1 = 0x20,
  MFRC522_REG_CRCRESULTREG_1 = 0x21,
  MFRC522_REG_CRCRESULTREG_2 = 0x22,
  MFRC522_REG_PAGE2_RESERVED_2 = 0x23,
  MFRC522_REG_MODWIDTHREG = 0x24,
  MFRC522_REG_PAGE2_RESERVED_3 = 0x25,
  MFRC522_REG_RFCFGREG = 0x26,
  MFRC522_REG_GSNREG = 0x27,
  MFRC522_REG_CWGSPREG = 0x28,
  MFRC522_REG_MODGSPREG = 0x29,
  MFRC522_REG_TMODEREG = 0x2A,
  MFRC522_REG_TPRESCALERREG = 0x2B,
  MFRC522_REG_TRELOADREG_1 = 0x2C,
  MFRC522_REG_TRELOADREG_2 = 0x2D,
  MFRC522_REG_TCOUNTERVALREG_1 = 0x2E,
  MFRC522_REG_TCOUNTERVALREG_2 = 0x2F,
  
  //PAGE 3 ==> TESTREGISTER
  MFRC522_REG_PAGE3_RESERVED_1 = 0x30,
  MFRC522_REG_TESTSEL1REG = 0x31,
  MFRC522_REG_TESTSEL2REG = 0x32,
  MFRC522_REG_TESTPINENREG = 0x33,
  MFRC522_REG_TESTPINVALUEREG = 0x34,
  MFRC522_REG_TESTBUSREG = 0x35,
  MFRC522_REG_AUTOTESTREG = 0x36,
  MFRC522_REG_VERSIONREG = 0x37,
  MFRC522_REG_ANALOGTESTREG = 0x38,
  MFRC522_REG_TESTDAC1REG = 0x39,
  MFRC522_REG_TESTDAC2REG = 0x3A,
  MFRC522_REG_TESTADCREG = 0x3B,
  MFRC522_REG_PAGE3_RESERVED_2 = 0x3C,
  MFRC522_REG_PAGE3_RESERVED_3 = 0x3D,
  MFRC522_REG_PAGE3_RESERVED_4 = 0x3E,
  MFRC522_REG_PAGE3_RESERVED_5 = 0x3F
} mfrc522_reg_t;

typedef enum {
  MFRC522_MIFAREONE_COMMAND_PICC_REQIDL = 0x26,
  MFRC522_MIFAREONE_COMMAND_PICC_REQALL = 0x52,
  MFRC522_MIFAREONE_COMMAND_PICC_ANTICOLL = 0x93,
  MFRC522_MIFAREONE_COMMAND_PICC_SELECTTAG = 0x93,
  MFRC522_MIFAREONE_COMMAND_PICC_AUTHENT1A = 0x60,
  MFRC522_MIFAREONE_COMMAND_PICC_AUTHENT1B = 0x61,
  MFRC522_MIFAREONE_COMMAND_READ = 0x30,
  MFRC522_MIFAREONE_COMMAND_WRITE = 0x10,
  MFRC522_MIFAREONE_COMMAND_DECREMENT = 0xC0,
  MFRC522_MIFAREONE_COMMAND_INCREMENT = 0xC1,
  MFRC522_MIFAREONE_COMMAND_RESTORE = 0xC2,
  MFRC522_MIFAREONE_COMMAND_TRANSFER = 0xB0,
  MFRC522_MIFAREONE_COMMAND_HALT = 0x50,
  
} mfrc522_mifareone_command_t;

void mfrc522_spi_init(void);
void mfrc522_init(mfrc522_t *_mfrc522, portpin_t *_reset);
void mfrc522_update(mfrc522_t *_mfrc522);
uint8_t * mfrc522_get_tag(mfrc522_t *_mfrc522);

mfrc522_version_t mfrc522_get_version(mfrc522_t *_mfrc522);
void mfrc522_power(mfrc522_t *_mfrc522, bool _on);

#endif
