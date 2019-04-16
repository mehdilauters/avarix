/** @defgroup bootloader Bootloader
 * @brief Bootloader module
 *
 * The bootloader module is not a real module. It is not intended to be put as
 * dependency. Special build rules are available to builde the bootloader for a
 * project.
 *
 * The device which runs the bootloader will be referred as the \e server. The
 * remote client will be referred as the \e client.
 *
 * All CRC computations use the CRC-16-CCITT.
 */
//@{
/** @file
 *  @brief Bootloader code
 */
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <util/crc16.h>
#include <clock/clock.h>
#include <util/delay.h>
#include <avarix/portpin.h>
#include <avarix/register.h>
#include <avarix/signature.h>
#include <i2c/i2c.h>

#include "bootloader_config.h"

typedef enum {

  REGISTER_ID_INFOS = 0xf0,
  REGISTER_ID_ANSWER_INFOS,

  REGISTER_ID_PUSH_PAGE,
  REGISTER_ID_COMMIT_PAGE,

  REGISTER_ID_REBOOT,

  REGISTER_ID_ANSWER_FAILURE = 0xff,

} protocol_register_t;

typedef enum {

  FAILURE_SUCCESS = 0,
  FAILURE_INVALID_CHECKSUM,
  FAILURE_INVALID_FRAME_SIZE,

  FAILURE_INVALID_REGISTER_ID = 0xf0,

} protocol_failure_t;

#if defined(RAMPZ)
# define pgm_read_byte_bootloader  pgm_read_byte_far
#else
# define pgm_read_byte_bootloader  pgm_read_byte_near
#endif


/// Wait completion of NVM command
#define boot_nvm_busy_wait() do{}while(NVM_STATUS & NVM_NVMBUSY_bm)

/// 
uint8_t bootloader_keep_alive;
bool reboot_asap;

/// Load a word into page buffer
static void boot_flash_page_fill(uint32_t address, uint16_t word)
{
  asm volatile (
    "movw r0, %[word]\n"          // set word data to r0:r1
    "sts %[rampz], %C[addr32]\n"  // set RAMPZ
    "sts %[nvmcmd], %[cmdval]\n"  // set NVM.CMD for load command
    "sts %[ccp], %[ccpspm]\n"     // disable CCP protection
    "spm\n"                       // execute SPM operation
    "clr r1\n"                    // clear r1, GCC's __zero_reg__
    "sts %[nvmcmd], %[cmdnop]\n"  // clear NVM.CMD
    :
    : [nvmcmd] "i" (_SFR_MEM_ADDR(NVM_CMD))
    , [cmdval] "r" ((uint8_t)(NVM_CMD_LOAD_FLASH_BUFFER_gc))
    , [cmdnop] "r" ((uint8_t)(NVM_CMD_NO_OPERATION_gc))
    , [ccp]    "i" (_SFR_MEM_ADDR(CCP))
    , [ccpspm] "r" ((uint8_t)(CCP_SPM_gc))
    ,          "z" ((uint16_t)(address))
    , [addr32] "r" ((uint32_t)(address))
    , [rampz]  "i" (_SFR_MEM_ADDR(RAMPZ))
    , [word]   "r" ((uint16_t)(word))
    : "r0", "r1"
  );
}


/// Erase then write a page
static void boot_app_page_erase_write(uint32_t address)
{
  asm volatile (
    "sts %[rampz], %C[addr32]\n"  // set RAMPZ
    "sts %[nvmcmd], %[cmdval]\n"  // set NVM.CMD for load command
    "sts %[ccp], %[ccpspm]\n"     // disable CCP protection
    "spm\n"                       // execute SPM operation
    "sts %[nvmcmd], %[cmdnop]\n"  // clear NVM.CMD
    :
    : [nvmcmd] "i" (_SFR_MEM_ADDR(NVM_CMD))
    , [cmdval] "r" ((uint8_t)(NVM_CMD_ERASE_WRITE_APP_PAGE_gc))
    , [cmdnop] "r" ((uint8_t)(NVM_CMD_NO_OPERATION_gc))
    , [ccp]    "i" (_SFR_MEM_ADDR(CCP))
    , [ccpspm] "r" ((uint8_t)(CCP_SPM_gc))
    ,          "z" ((uint16_t)(address))
    , [addr32] "r" ((uint32_t)(address))
    , [rampz]  "i" (_SFR_MEM_ADDR(RAMPZ))
  );
}


/// Erase user signature page
static void boot_user_sig_erase(void)
{
  asm volatile (
    "sts %[nvmcmd], %[cmdval]\n"  // set NVM.CMD for load command
    "sts %[ccp], %[ccpspm]\n"     // disable CCP protection
    "spm\n"                       // execute SPM operation
    "sts %[nvmcmd], %[cmdnop]\n"  // clear NVM.CMD
    :
    : [nvmcmd] "i" (_SFR_MEM_ADDR(NVM_CMD))
    , [cmdval] "r" ((uint8_t)(NVM_CMD_ERASE_USER_SIG_ROW_gc))
    , [cmdnop] "r" ((uint8_t)(NVM_CMD_NO_OPERATION_gc))
    , [ccp]    "i" (_SFR_MEM_ADDR(CCP))
    , [ccpspm] "r" ((uint8_t)(CCP_SPM_gc))
  );
}


/// Write user signature page
static void boot_user_sig_write(void)
{
  asm volatile (
    "sts %[nvmcmd], %[cmdval]\n"  // set NVM.CMD for load command
    "sts %[ccp], %[ccpspm]\n"     // disable CCP protection
    "spm\n"                       // execute SPM operation
    "sts %[nvmcmd], %[cmdnop]\n"  // clear NVM.CMD
    :
    : [nvmcmd] "i" (_SFR_MEM_ADDR(NVM_CMD))
    , [cmdval] "r" ((uint8_t)(NVM_CMD_WRITE_USER_SIG_ROW_gc))
    , [cmdnop] "r" ((uint8_t)(NVM_CMD_NO_OPERATION_gc))
    , [ccp]    "i" (_SFR_MEM_ADDR(CCP))
    , [ccpspm] "r" ((uint8_t)(CCP_SPM_gc))
  );
}

//XXX wdt_disable() is provided by avr-libc but fails in 1.8.1
/// Disable watchdog
static void wdt_disable_(void)
{
  ccp_io_write(&WDT.CTRL, (WDT.CTRL & ~WDT_ENABLE_bm) | WDT_CEN_bm);
}


/** @brief Run the application
 * @note Registers are not initialized.
 */
static void run_app(void)
{
  // switch vector table
  ccp_io_write(&PMIC.CTRL, PMIC.CTRL & ~PMIC_IVSEL_bm);
  asm volatile (
    "ldi r30, 0\n"
    "ldi r31, 0\n"
    "ijmp\n"
  );
  __builtin_unreachable();
}


/** @brief Run the application
 */
static void boot(void)
{
  run_app();
  __builtin_unreachable();
}

static uint16_t fletcher16_checksum(uint8_t *buffer, int len) {
  uint8_t lo=0,hi=0;
  for(int i=0; i<len; i++) {
    lo += buffer[i];
    hi += lo;
  }
  return hi << 8 | lo;
}

static uint32_t unpack_u32(uint8_t **buffer) {
  uint32_t value;
  memcpy(&value, *buffer, sizeof(uint32_t));
  (*buffer) += sizeof(uint32_t);
  return value;
}

static uint16_t unpack_u16(uint8_t **buffer) {
  uint16_t value;
  memcpy(&value, *buffer, sizeof(uint16_t));
  (*buffer) += sizeof(uint16_t);
  return value;
}

static uint8_t unpack_u8(uint8_t **buffer) {
  uint8_t value = *buffer[0];
  (*buffer)++;
  return value;
}

uint8_t i2cs_txbuffer[I2CS_SEND_BUFFER_SIZE];
uint16_t i2cs_txbuffer_len;

static void i2c_tx_answer(uint8_t* buffer, int len) {
  // maximum buffer length excluding checksum
  const int max_len = I2CS_SEND_BUFFER_SIZE - 2;
  // blindly limit buffer size
  if(len > max_len) {
    len = max_len;
  }

  // copy buffer
  memcpy(i2cs_txbuffer, buffer, len);
  // compute and enqueue checksum
  uint16_t csum = fletcher16_checksum(buffer, len);
  memcpy(i2cs_txbuffer + len, &csum, sizeof(csum));
  i2cs_txbuffer_len = len + sizeof(csum);
}

static void i2c_tx_answer_failure(protocol_failure_t failure) {
  uint8_t frame[] = {REGISTER_ID_ANSWER_FAILURE, failure};
  return i2c_tx_answer(frame, sizeof(frame));
}

static void i2c_reset_callback(void) {

}

static uint8_t i2c_prepare_send_callback(uint8_t *buffer, uint8_t maxsz) {
  uint8_t wsz = i2cs_txbuffer_len;
  if(wsz > 0) {
    if(wsz > maxsz) {
      wsz = maxsz;
    }
    memcpy(buffer, i2cs_txbuffer, wsz);
  }
  return wsz;
}

static void i2c_recv_callback(uint8_t *rx, uint8_t rxlen) {

  if(rxlen == 0)
    return;

  // compute frame checksum
  uint16_t csum = fletcher16_checksum(rx,rxlen-2);
  // first byte is register ID
  uint8_t rid = unpack_u8(&rx);

  switch(rid) {

    case REGISTER_ID_INFOS: {
      // unpack frame
      uint16_t checksum = unpack_u16(&rx);

      // validate frame
      if(csum != checksum) {
        i2c_tx_answer_failure(FAILURE_INVALID_CHECKSUM);
      }

      // keep bootloader awake
      bootloader_keep_alive = 30;

      // reply with informations
      uint16_t pagesize = APP_SECTION_PAGE_SIZE;
      // pack answer frame
      uint8_t frame[1 + sizeof(uint16_t)];
      frame[0] = REGISTER_ID_ANSWER_INFOS;
      memcpy(frame+1, &pagesize, sizeof(pagesize));
      // tx frame
      i2c_tx_answer(frame, sizeof(frame));
      break;
    }

    case REGISTER_ID_REBOOT: {
      // unpack frame
      uint16_t checksum = unpack_u16(&rx);

      // validate frame
      if(csum != checksum) {
        i2c_tx_answer_failure(FAILURE_INVALID_CHECKSUM);
      }

      // keep bootloader awake
      bootloader_keep_alive = 30;

      // reboot asap
      reboot_asap = true;

      // tx frame
      i2c_tx_answer_failure(FAILURE_SUCCESS);
      break;
    }

    case REGISTER_ID_COMMIT_PAGE: {
      // unpack frame
      uint32_t address = unpack_u32(&rx);
      uint16_t checksum = unpack_u16(&rx);

      // validate frame
      if(csum != checksum) {
        i2c_tx_answer_failure(FAILURE_INVALID_CHECKSUM);
      }

      // keep bootloader awake
      bootloader_keep_alive = 30;

      boot_app_page_erase_write(address);

      // tx frame
      i2c_tx_answer_failure(FAILURE_SUCCESS);
      break;
    }

    case REGISTER_ID_PUSH_PAGE: {
      // frame is:
      // - 1 byte register ID
      // - 4 bytes address
      // - n bytes data
      // - 2 bytes checksum
      if(rxlen < 7) {
        i2c_tx_answer_failure(FAILURE_INVALID_FRAME_SIZE);
        break;
      }

      const unsigned int frame_length = rxlen - 7;

      // a valid frame contain an even number of bytes
      if(frame_length%2 != 0) {
        i2c_tx_answer_failure(FAILURE_INVALID_FRAME_SIZE);
        break;
      }

      // keep bootloader awake
      bootloader_keep_alive = 30;

      // unpack frame
      uint32_t address = unpack_u32(&rx);

      // store frame in memory
      uint8_t data[I2CS_RECV_BUFFER_SIZE];
      for(uint8_t i=0; i<frame_length; i++) {
        data[i] = unpack_u8(&rx);
      }

      uint16_t checksum = unpack_u16(&rx);

      // validate frame
      if(csum != checksum) {
        i2c_tx_answer_failure(FAILURE_INVALID_CHECKSUM);
        break;
      }

      for(uint8_t i=0; i<frame_length; i+=2) {
        uint16_t word;
        memcpy(&word, data+i, sizeof(word));
        boot_flash_page_fill(address + i, word);
      }

      i2c_tx_answer_failure(FAILURE_SUCCESS);
      break;
    }

    default:
      i2c_tx_answer_failure(FAILURE_INVALID_REGISTER_ID);
      break;
  }

  return;
}

int main(void) __attribute__ ((OS_main));
int main(void)
{
  bootloader_keep_alive = 20;
  reboot_asap = false;

  // switch vector table
  ccp_io_write(&PMIC.CTRL, PMIC.CTRL | PMIC_IVSEL_bm);
  wdt_disable_();

  clock_init();

  i2cs_t* i2c = &BOOTLOADER_I2C;
  i2c_init();
  i2cs_register_reset_callback(i2c, i2c_reset_callback);
  i2cs_register_prepare_send_callback(i2c, i2c_prepare_send_callback);
  i2cs_register_recv_callback(i2c, i2c_recv_callback);

  INTLVL_ENABLE_ALL();
  __asm__("sei");

  (void)boot_user_sig_erase;
  (void)boot_user_sig_write;
  (void)boot;

  for(;;) {

    // continue boot in 1s
    if(bootloader_keep_alive == 0) {
      break;
    }
    else {
      bootloader_keep_alive--;
    }

    if(reboot_asap) {
      PORTPIN_CTRL(BOOTLOADER_PP_LEDA) = 0;
      PORTPIN_CTRL(BOOTLOADER_PP_LEDB) = 0;

      wdt_enable(WDTO_60MS);
      for(;;);
    }
    
    if(bootloader_keep_alive%2) {
      PORTPIN_CTRL(BOOTLOADER_PP_LEDA) = PORT_OPC_PULLUP_gc;
      PORTPIN_CTRL(BOOTLOADER_PP_LEDB) = PORT_OPC_PULLDOWN_gc;
    }
    else {
      PORTPIN_CTRL(BOOTLOADER_PP_LEDA) = PORT_OPC_PULLDOWN_gc;
      PORTPIN_CTRL(BOOTLOADER_PP_LEDB) = PORT_OPC_PULLUP_gc;
    }

    _delay_ms(50);
  }

  PORTPIN_CTRL(BOOTLOADER_PP_LEDA) = 0;
  PORTPIN_CTRL(BOOTLOADER_PP_LEDB) = 0;

  // unconfigure I2C
  i2c_deinit();

  // run app
  boot();

}

//@}
