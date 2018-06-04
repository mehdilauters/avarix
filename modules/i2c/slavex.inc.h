/** @defgroup i2c I2C
 * @brief I2C module
 */
//@{
/**
 * @file
 * @brief I2C slave definitions
 */
#define i2cX(s) X_(i2c,s)

// I2cX slave singleton
extern i2cs_t i2cX();

#undef i2cX
#undef X_
