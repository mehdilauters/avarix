/** @defgroup sensors Keypad
 * @brief matrix keyboard
 */
//@{
/**
 * @file
 * @brief Keypad module definitions
 */
#ifndef KEYPAD_H__ 
#define KEYPAD_H__

#include <avarix/portpin.h>
#include <stdint.h>

/** Keypad data
 * @note Fields are private and should not be accessed directly.
 */
typedef struct {
  portpin_t *columns; /// portpin_t array wired to columns
  uint8_t columns_count; /// columns count
  portpin_t *rows; /// portpin_t array wired to rows
  uint8_t rows_count; /// rows count
  uint8_t *keymap; /// key mapping
  uint8_t *current;  /// array of current pressed keys
  uint8_t current_count; /// current pressed keys count
} keypad_t;


/** @brief Initialize keypad
 * 
 * @param _keyad  keypad to initialize
 * @param _rows  portpin_t array wired to lines
 * @param _rows_count  number of element in rows array
 * @param _columns  portpin_t array wired to columns
 * @param _columns_count  number of element in columns array
 * @param _keymap  keymap array linking row/col combination to key
 *
 * A Keypad uses _columns_count * _rows_count IO.
 * example:
 * 
 * uint8_t keymap[4][3] = {
 *  {'1', '2', '3'},
 *  {'4', '5', '6'},
 *  {'7', '8', '9'},
 *  {'*', '0', '#'}
 * } ;*
 * 
 * portpin_t rows[] = {
 *  PORTPIN(C,2),
 *  PORTPIN(C,3),
 *  PORTPIN(C,4),
 *  PORTPIN(C,5)
 * };
 * portpin_t columns[] = {
 *  PORTPIN(C,7),
 *  PORTPIN(B,2),
 *  PORTPIN(B,3)
 * };
 * keypad_t keypad;
 * keypad_init(&keypad, rows, rows_count, columns, columns_count, (uint8_t*)keymap);
 * keypad_update(&keypad);
 * uint8_t count = keypad_available(&keypad);
 * if(count > 0) {
 *   printf("Keypress (%d): ", count);
 *   for(uint8_t i=0; i < count; i++) {
 *     printf("%c-", keypad_get(&keypad, i));
 *   }
 *   printf("\n");
 *  }
 */
void keypad_init(keypad_t *_keypad, portpin_t *_rows, uint8_t _rows_count, portpin_t *_columns, uint8_t _columns_count, uint8_t *_keymap);

/// update keys status
void keypad_update(keypad_t *_keypad);

/// get the current number of pressed keys
uint8_t keypad_available(keypad_t *_keypad);

/// get the mapped key at _pos index
uint8_t keypad_get(keypad_t *_keypad, uint8_t _pos);

#endif
