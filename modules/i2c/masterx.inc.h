/** @defgroup i2c I2C
 * @brief I2C module
 */
//@{
/**
 * @file
 * @brief I2C master definitions
 */
#define i2cX(s) X_(i2c,s)

// I2cX master singleton
extern i2cm_t i2cX();

#undef i2cX
#undef X_
