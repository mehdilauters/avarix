#ifndef MFRC522_H__ 
#define MFRC522_H__

#include <avarix/portpin.h>

typedef enum {
  MFRC522_STATE_INIT,
  MFRC522_STATE_POWERON,
  MFRC522_STATE_CONFIGURE,
  MFRC522_STATE_REQUEST,
  MFRC522_STATE_POLL,
  MFRC522_STATE_REQUEST_UID,
  MFRC522_STATE_POLL_UID,
  MFRC522_STATE_POWEROFF,

} mfrc522_state_t;

typedef enum {
  MFRC522_ERROR_TRANSMISSION = 1,
  MFRC522_ERROR_LAST_BITS_NOT_NULL,
} mfrc522_error_t;

typedef struct {

  mfrc522_state_t state;
  const portpin_t *reset;

  int polling_counter;

  uint32_t serial;
  bool serial_validity;

} mfrc522_t;

/** @brief Initialize MFRC SPI and MFRC chip
 * @brief reset pin connected to chip RST
 */
void mfrc522_init(mfrc522_t *, const portpin_t *reset);

/** @brief Update communication with MFRC522, need to be called often
 * @return true when mfrc522 was just reseted
 * (and switching to another mfrc522 sharing the same SPI bus is possible)
 */
bool mfrc522_update(mfrc522_t *);

/** @brief Return pointer to tag UID if one is currently seen, NULL otherwise */
uint32_t* mfrc522_get_tag_uid(mfrc522_t *);

#endif
