/** @addtogroup clock
 *
 * @par Configuration
 *
 * Clock configuration shares definitions exported by \ref defs.h.
 *
 * Values are redundant, for instance \ref CLOCK_PRESCALER_A_DIV can
 * be deduced from \ref CLOCK_SYS_FREQ and \ref CLOCK_PER4_FREQ.
 * As a result, values are often optional but enough information must be
 * provided to have an unambiguous configuration.
 *
 * Variables defined manually are not overwritten. This is not an issue to
 * define more values than strictly needed, as long as they do not mismatch.
 *
 * Thorough checks are made to detect invalid configurations such as invalid
 * prescaler ratios, frequency roundings, incompatible values, etc.
 *
 * See documentation of each value to know when it must be defined.
 */
//@{
/** @file
 * @brief Clock configuration
 *
 * See \ref clock module description for configuration details.
 */
///@cond internal

#define CLOCK_SOURCE  CLOCK_SOURCE_RC32M
#define CLOCK_SYS_FREQ  32000000
#define CLOCK_CPU_FREQ  32000000
#define CLOCK_PER2_FREQ  CLOCK_CPU_FREQ
#define CLOCK_PER4_FREQ  CLOCK_CPU_FREQ

// uncomment the following line to activate automatic run-time calibration of internal clock
// only usable with 2MHz or 32Mhz internal clocks
// it will use the 32,768kHz calibrated clock as the adjustement source
//#define CLOCK_AUTOMATIC_RUN_TIME_CALIBRATION

///@endcond
//@}
