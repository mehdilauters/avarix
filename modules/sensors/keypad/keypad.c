#include "keypad.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <clock/clock.h>
#include <util/delay.h>

void keypad_init(keypad_t *_keypad, portpin_t *_rows, uint8_t _row_count, portpin_t *_columns, uint8_t _columns_count, uint8_t *_keymap) {
  _keypad->columns = _columns;
  _keypad->columns_count = _columns_count;
  _keypad->rows = _rows;
  _keypad->rows_count = _row_count;
  _keypad->keymap = _keymap;
  _keypad->current = malloc(sizeof(uint8_t*) * _keypad->rows_count* _keypad->columns_count);
  _keypad->current_count = 0;
  
  for(uint8_t i = 0; i < _keypad->columns_count; i++) {
    // set input + pullup
    portpin_dirclr(&_keypad->columns[i]);
    portpin_inpulldown(&_keypad->columns[i]);
  }
}

void keypad_update(keypad_t *_keypad) {
  // clear result
  _keypad->current_count = 0;
  memset(_keypad->current, 0, sizeof(uint8_t*) * _keypad->rows_count* _keypad->columns_count);
  for(uint8_t r = 0; r < _keypad->rows_count; r++) {
    // set row as output high
    portpin_dirset(&_keypad->rows[r]);
    portpin_outset(&(_keypad->rows[r]));
    for(uint8_t c = 0; c < _keypad->columns_count; c++) {
      // check each column
      if( portpin_in(&(_keypad->columns[c]))) {
        // get key from mapping
        uint8_t *key = _keypad->keymap + _keypad->columns_count * r + c ;
        //store it on the result array
        _keypad->current[_keypad->current_count] = *key;
        _keypad->current_count++;
      }
    }
    // clear row and set it to z
    portpin_outclr(&(_keypad->rows[r]));
    portpin_dirclr(&_keypad->rows[r]);
  }
}

uint8_t keypad_available(keypad_t *_keypad) {
  return _keypad->current_count;
}

uint8_t keypad_get(keypad_t *_keypad, uint8_t _pos) {
  return _keypad->current[_pos];
}


