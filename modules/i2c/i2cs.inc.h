#ifndef I2C_INC_H__
#define I2C_INC_H__

typedef enum {
  I2CS_STATE_NONE,
  I2CS_STATE_READ,
  I2CS_STATE_WRITE,

} i2cs_state_t;

#ifndef I2CS_RECV_BUFFER_SIZE
# define I2CS_RECV_BUFFER_SIZE  32
#endif

#ifndef I2CS_SEND_BUFFER_SIZE
# define I2CS_SEND_BUFFER_SIZE  32
#endif

/** @brief I2C slave master-write frame received
 *
 * @param buffer buffer containing the received bytes
 * @param n number of bytes received from master
 *
 * This function is called when a master-write operation has completed
 */
typedef void (*i2cs_recv_callback_t)(uint8_t *buffer, uint8_t n);

/** @brief I2C slave master-read operation was requested
 *
 * @param buffer buffer to provision
 * @param maxsz maximum number of bytes which can be written to buffer
 * @return number of bytes to send, returning 0 will result in a NACK from slave.
 *
 * This function is called when a master-read operation was requested by master
 * and ask user to provision the buffer which will be sent.
 */
typedef uint8_t (*i2cs_prepare_send_callback_t)(uint8_t *buffer, uint8_t maxsz);

/** @brief I2C slave transaction finished successfully or not
 *
 * This function is called when a STOP condition or any bus error has ended current transaction
 */
typedef void (*i2cs_reset_callback_t)(void);

typedef struct {

  i2cs_state_t state;

  uint8_t recvd_bytes;
  uint8_t recv_buffer[I2CS_RECV_BUFFER_SIZE];

  uint8_t sent_bytes;
  uint8_t bytes_to_send;
  uint8_t send_buffer[I2CS_SEND_BUFFER_SIZE];

  i2cs_recv_callback_t recv_callback;

  i2cs_prepare_send_callback_t prepare_send_callback;

  i2cs_reset_callback_t reset_callback;

} i2cs_t;

#endif//I2C_INC_H__
