/** @addtogroup bootloader */
//@{
/** @file
 * @brief Bootloader configuration
 */
/** @name Configuration
 *
 * For UART configuration, it is possible to use values from uart_config.h.
 */
//@{
#include "uart_config.h"

/// UART to use for bootloader
#define BOOTLOADER_I2C i2cE

/// LEDs used by the bootloader to signal bootloading mode
#define BOOTLOADER_PP_LEDA (&PORTPIN(E,2))
#define BOOTLOADER_PP_LEDB (&PORTPIN(E,3))

//@}
//@}
