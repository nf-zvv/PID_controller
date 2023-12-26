#include "lcd.h"

#include <stdarg.h>
#include <stdio.h>
#include <util/delay.h>

void lcd_send(uint8_t value, uint8_t mode);
void lcd_write_nibble(uint8_t nibble);
void lcd_port_in(void);
void lcd_port_out(void);
void lcd_busywait(void);

static uint8_t lcd_displayparams;
static char lcd_buffer[LCD_COL_COUNT + 1];

void lcd_command(uint8_t command) {
  lcd_send(command, 0);
}

void lcd_write(uint8_t value) {
  lcd_send(value, 1);
}

void lcd_port_in(void) {
// Configure pins as input
  LCD_DATA_DDR = LCD_DATA_DDR
    & ~(1 << LCD_D0)
    & ~(1 << LCD_D1)
    & ~(1 << LCD_D2)
    & ~(1 << LCD_D3);

  // Turn on pull-up resistors
  LCD_DATA_PORT = LCD_DATA_PORT
    | (1 << LCD_D0)
    | (1 << LCD_D1)
    | (1 << LCD_D2)
    | (1 << LCD_D3);
}

void lcd_port_out(void) {
  // Data port to output
  LCD_DATA_DDR = LCD_DATA_DDR
    | (1 << LCD_D0)
    | (1 << LCD_D1)
    | (1 << LCD_D2)
    | (1 << LCD_D3);
}

void lcd_busywait(void) {
  uint8_t status = (1 << LCD_D3);

  lcd_port_in();

  LCD_CMD_PORT &= ~(1 << LCD_RS); // command mode
  LCD_CMD_PORT |= (1 << LCD_RW); // reading
  
  while (status & (1 << LCD_D3))
  {
    LCD_CMD_PORT |= (1 << LCD_EN);
    _delay_us(1);
    status = LCD_DATA_PIN;
    LCD_CMD_PORT &= ~(1 << LCD_EN);
    _delay_us(37);
    LCD_CMD_PORT |= (1 << LCD_EN);
    _delay_us(1);
    LCD_CMD_PORT &= ~(1 << LCD_EN);
    _delay_us(37);
  }

  lcd_port_out();
}

void lcd_send(uint8_t value, uint8_t mode) {
  lcd_busywait();
  if (mode) {
    LCD_CMD_PORT |= (1 << LCD_RS);
  } else {
    LCD_CMD_PORT &= ~(1 << LCD_RS);
  }

  LCD_CMD_PORT &= ~(1 << LCD_RW);

  lcd_write_nibble(value >> 4);
  lcd_write_nibble(value);
}

void lcd_write_nibble(uint8_t nibble) {
  LCD_DATA_PORT = (LCD_DATA_PORT & ~(0x0f << LCD_D0)) | ((nibble & 0x0f) << LCD_D0);
  LCD_CMD_PORT &= ~(1 << LCD_EN);
  LCD_CMD_PORT |= (1 << LCD_EN);
  _delay_us(1);
  LCD_CMD_PORT &= ~(1 << LCD_EN);
  _delay_us(37);	// If delay less than this value, the data is not correctly displayed  
}

void lcd_init(void) {
  LCD_CMD_PORT = LCD_CMD_PORT
    & ~(1 << LCD_EN)
    & ~(1 << LCD_RS)
    & ~(1 << LCD_RW);

  // Configure pins as output
  LCD_CMD_DDR = LCD_CMD_DDR
    | (1 << LCD_RS)
    | (1 << LCD_RW)
    | (1 << LCD_EN);
	
  // Data port to output
  lcd_port_out();

  // Wait for LCD to become ready (docs say 15ms+)
  _delay_ms(15);

  lcd_write_nibble(0x03); // Switch to 4 bit mode
  _delay_ms(4.1);

  lcd_write_nibble(0x03); // 2nd time
  _delay_ms(4.1);

  lcd_write_nibble(0x03); // 3rd time
  _delay_ms(4.1);

  lcd_write_nibble(0x02); // Set 8-bit mode (?)

  lcd_command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);

  lcd_displayparams = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  lcd_command(LCD_DISPLAYCONTROL | lcd_displayparams);
}

void lcd_on(void) {
  lcd_displayparams |= LCD_DISPLAYON;
  lcd_command(LCD_DISPLAYCONTROL | lcd_displayparams);
}

void lcd_off(void) {
  lcd_displayparams &= ~LCD_DISPLAYON;
  lcd_command(LCD_DISPLAYCONTROL | lcd_displayparams);
}

void lcd_clear(void) {
  lcd_command(LCD_CLEARDISPLAY);
  _delay_ms(2);
}

void lcd_return_home(void) {
  lcd_command(LCD_RETURNHOME);
  _delay_ms(2);
}

void lcd_enable_blinking(void) {
  lcd_displayparams |= LCD_BLINKON;
  lcd_command(LCD_DISPLAYCONTROL | lcd_displayparams);
}

void lcd_disable_blinking(void) {
  lcd_displayparams &= ~LCD_BLINKON;
  lcd_command(LCD_DISPLAYCONTROL | lcd_displayparams);
}

void lcd_enable_cursor(void) {
  lcd_displayparams |= LCD_CURSORON;
  lcd_command(LCD_DISPLAYCONTROL | lcd_displayparams);
}

void lcd_disable_cursor(void) {
  lcd_displayparams &= ~LCD_CURSORON;
  lcd_command(LCD_DISPLAYCONTROL | lcd_displayparams);
}

void lcd_scroll_left(void) {
  lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void lcd_scroll_right(void) {
  lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void lcd_set_left_to_right(void) {
  lcd_displayparams |= LCD_ENTRYLEFT;
  lcd_command(LCD_ENTRYMODESET | lcd_displayparams);
}

void lcd_set_right_to_left(void) {
  lcd_displayparams &= ~LCD_ENTRYLEFT;
  lcd_command(LCD_ENTRYMODESET | lcd_displayparams);
}

void lcd_enable_autoscroll(void) {
  lcd_displayparams |= LCD_ENTRYSHIFTINCREMENT;
  lcd_command(LCD_ENTRYMODESET | lcd_displayparams);
}

void lcd_disable_autoscroll(void) {
  lcd_displayparams &= ~LCD_ENTRYSHIFTINCREMENT;
  lcd_command(LCD_ENTRYMODESET | lcd_displayparams);
}

void lcd_create_char(uint8_t location, uint8_t *charmap) {
  lcd_command(LCD_SETCGRAMADDR | ((location & 0x7) << 3));
  for (int i = 0; i < 8; i++) {
    lcd_write(charmap[i]);
  }
  lcd_command(LCD_SETDDRAMADDR);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
  static uint8_t offsets[] = { 0x00, 0x40, 0x14, 0x54 };

  lcd_command(LCD_SETDDRAMADDR | (col + offsets[row]));
}

void lcd_puts(char *string) {
  for (char *it = string; *it; it++) {
    lcd_write(*it);
  }
}

void lcd_puts_P(const char *string) {
for(uint8_t i=0;(uint8_t)pgm_read_byte(&string[i]);i++)
	{
		lcd_write((uint8_t)pgm_read_byte(&string[i]));
	}
}

void lcd_printf(char *format, ...) {
  va_list args;

  va_start(args, format);
  vsnprintf(lcd_buffer, LCD_COL_COUNT + 1, format, args);
  va_end(args);

  lcd_puts(lcd_buffer);
}
