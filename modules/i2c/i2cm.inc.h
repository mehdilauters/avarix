#ifndef I2CM_INC_H__
#define I2CM_INC_H__

#ifndef I2CM_BUFFER_SIZE
# define I2CM_BUFFER_SIZE 32
#endif

/** @brief I2C master master-write transaction completed
 * @param n if positive number of byte successfully sent (ACKed) the slave,
 * if negative one of the following happened:
 *  -1 no ACK from slave on address 
 *  -2 bus or arbitration error
 */
typedef void (*i2cm_write_completed_callback)(int16_t n, void* payload);

/** @brief I2C master master-read transaction completed
 * @param buffer pointer to received bytes
 * @param n if positive number of byte successfully recv (ACKed) the slave,
 * if negative one of the following happened:
 *  -1 no ACK from slave on address 
 *  -2 bus or arbitration error
 */
typedef void (*i2cm_read_completed_callback)(uint8_t *buffer, int16_t n, void* payload);

typedef struct {

  TWI_MASTER_t *master;

  uint8_t buffer[I2CM_BUFFER_SIZE];
  uint8_t bytes_count;
  uint8_t bytes_to_send;
  uint8_t bytes_to_recv;

  i2cm_read_completed_callback read_completed_callback;
  void *read_completed_callback_payload;

  i2cm_write_completed_callback write_completed_callback;
  void *write_completed_callback_payload;

} i2cm_t;


#endif//I2CM_INC_H__
