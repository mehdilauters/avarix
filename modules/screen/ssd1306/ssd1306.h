#ifndef SCREEN_SSD1306_
#define SCREEN_SSD1306_
#include <stdint.h>
#include <stdbool.h>
#include <i2c/i2c.h>

#include "registers.h"

#define SSD1306_XY(a,b) ((ssd1306_position_t) {.x=(a),.y=(b)})

typedef enum {
  
  SSD1306_ERROR_SUCCESS = 0,
  SSD1306_ERROR_INVALID_SCREEN_SIZE,
  SSD1306_ERROR_PEER_UNREACHABLE,

} ssd1306_error_t;

typedef enum {
  SSD1306_SIZE_128_32,

} ssd1306_size_t;

typedef struct {
  int8_t x;
  int8_t y;
} ssd1306_position_t;

typedef struct {

  const uint8_t *p;

  char first_char;
  uint8_t number_of_chars;
  uint8_t height;

} ssd1306_font_t;

typedef struct {
  i2cm_t * i2c;
  uint8_t address;
  uint16_t width;
  uint16_t height;

  uint16_t blit_it;
  bool blit_in_progress;
  // NDJD: as we only support 128x32 screens
  // this size will remain static for now
  uint8_t screen[128*32/8];

  ssd1306_font_t font;

} ssd1306_t;

/** @brief Initialize internal structure for SSD1306 screen management
 * @param i2cm I2C-master controller on which the screen is located
 * @param address screen I2C address 
 * @param size screen size (see ssd1306_size_t enum for an exhaustive list)
 *
 * @retval SSD1306_ERROR_SUCCESS on success
 * @retval SSD1306_ERROR_INVALID_SCREEN_SIZE if given size does not match a known screen size
 *
 * @warning this function does not perform any communication with screen
 */
ssd1306_error_t ssd1306_init(ssd1306_t *s, i2cm_t *i2cm, uint8_t address, ssd1306_size_t size);


/** @brief Communication and configure SSD1306 chip
 *
 * @retval SSD1306_ERROR_SUCCESS on chip initialization success
 * @retval SSD1306_PEER_UNREACHBLE if communication with screen failure
 */
ssd1306_error_t ssd1306_configure(ssd1306_t *s);

/** @brief Load font 
 * @param p pointer to an uint8_t array located in PROGMEM following the format generated
 * by http://oleddisplay.squix.ch
 */
void ssd1306_load_font(ssd1306_t *s, const uint8_t * p);

/** @return true if a font is loaded, false otherwise */
bool ssd1306_font_loaded(ssd1306_t *s);

/** @brief Clear screen 
 * 
 * @warning This operation will silently fail if a blit is in progress, check ssd1306_blit_in_progress() to false
 * to ensure operation success
 *
 * @warning This function does not directly update screen, call ssd1306_blit() and wait for completion
 * with ssd1306_blit_in_progress() to ensure screen is updated.
 */
void ssd1306_clear(ssd1306_t *s);

/** @brief Draw one character using current font
 * 
 * @param p screen position where char will be drawn
 * @param c char to draw
 *
 * @return new cursor position which can be used to draw right after drawn char.
 *
 * @warning This operation will silently fail if a blit is in progress, check ssd1306_blit_in_progress() to false
 * to ensure operation success
 * 
 * @warning This function does not directly update screen, call ssd1306_blit() and wait for completion
 * with ssd1306_blit_in_progress() to ensure screen is updated.
 */
ssd1306_position_t ssd1306_draw_char(ssd1306_t *s, ssd1306_position_t p, char c);

/** @brief Draw a string of characters
 *
 * @param p screen position where char will be drawn
 * @param str a null-terminated string of characteres
 * @param letter_spacing_px horizontal letter spacing in pixels (zero will default to font original spacing)
 *
 * @warning This operation will silently fail if a blit is in progress, check ssd1306_blit_in_progress() to false
 * to ensure operation success
 *
 * @warning This function does not directly update screen, call ssd1306_blit() and wait for completion
 * with ssd1306_blit_in_progress() to ensure screen is updated.
*/
ssd1306_position_t ssd1306_draw_string(ssd1306_t *s, ssd1306_position_t p, const char* str, uint8_t len, uint8_t letter_spacing_px);

/** @brief Blit in-memory screen bitmap to real screen 
 *
 * This function will start an async transaction with screen, check ssd1306_blit_in_progress() to
 * false to ensure update completion.
 * */
void ssd1306_blit(ssd1306_t *s);

/** @return true if a blit operation is in progress, false otherwise */
bool ssd1306_blit_in_progress(ssd1306_t *s);

#endif
