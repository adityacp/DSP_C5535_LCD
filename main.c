#include "stdio.h"
#include "string.h"
#include "usbstk5515.h"
#include "usbstk5515_i2c.h"
#include "display.h"

void LiquidCrystal_I2C(Uint8 lcd_Addr, Uint8 lcd_cols, Uint8 lcd_rows, Uint8 dotsize);
void clear();
void home();
void noDisplay();
void display();
void noBlink();
void blink();
void noCursor();
void cursor();
void scrollDisplayLeft();
void scrollDisplayRight();
void leftToRight();
void rightToLeft();
void noBacklight();
void backlight();
void autoscroll();
void noAutoscroll();
void write_bytes(char* value);
void write_byte(Uint8 value);
void command(Uint8 value);
void setBacklight(Bool new_val);
void send(Uint8 value, Uint8 mode);
void write4bits(Uint8 value);
void expanderWrite(Uint8 _data);
void pulseEnable(Uint8 _data);
void write_str_at_position(char* value, Uint8 col, Uint8 row);
void setCursor(Uint8 col, Uint8 row);

Uint16 _Addr;
Uint8 _displayfunction;
Uint8 _displaycontrol;
Uint8 _displaymode;
Uint8 _numlines;
Uint8 _cols;
Uint8 _rows;
Uint8 _backlightval;

void main() {
    USBSTK5515_init();
    USBSTK5515_I2C_init();
    LiquidCrystal_I2C(0x27, 16, 2, LCD_5x8DOTS);
    write_str_at_position("Hello World", 0, 0);
    write_str_at_position("I2C interface", 0, 1);
    setBacklight(TRUE);
}

void LiquidCrystal_I2C(Uint8 lcd_Addr, Uint8 lcd_cols, Uint8 lcd_rows, Uint8 dotsize) {
    _Addr = lcd_Addr;
    _cols = lcd_cols;
    _rows = lcd_rows;
    _backlightval = LCD_NOBACKLIGHT;

    _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

    if (lcd_rows > 1) {
        _displayfunction |= LCD_2LINE;
    }
    _numlines = lcd_rows;

    if ((dotsize != 0) && (lcd_rows == 1))
    {
        _displayfunction |= LCD_5x10DOTS;
    }


    // Now we pull both RS and R/W low to begin commands
    expanderWrite(_backlightval); // reset expander and turn backlight off (Bit 8 =1)


    //put the LCD into 4 bit mode
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // finally, set to 4-bit interface
    write4bits(0x02 << 4);

    // set # lines, font size, etc.
    command((Uint8)(LCD_FUNCTIONSET | _displayfunction));

    // turn the display on with no cursor or blinking default
    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();

    // clear it off
    clear();

    // Initialize to default text direction (for roman languages)
    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

    // set the entry mode
    command((Uint8)(LCD_ENTRYMODESET | _displaymode));

    home();
}

void setCursor(Uint8 col, Uint8 row)
{
    int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
    if (row > _numlines)
    {
        row = (Uint8)(_numlines - 1); // we count rows starting w/0
    }
    command((Uint8)(LCD_SETDDRAMADDR | (col + row_offsets[row])));
}


void write_str_at_position(char* value, Uint8 col, Uint8 row) {
    setCursor(col, row);
    write_bytes(value);
}


void clear() {
    command(LCD_CLEARDISPLAY);
}


void home() {
    command(LCD_RETURNHOME);
}

void noDisplay() {
    Uint8 display = LCD_DISPLAYON;
    _displaycontrol &= (Uint8)~display;
    command((Uint8)(LCD_DISPLAYCONTROL | _displaycontrol));
}

void display() {
    _displaycontrol |= LCD_DISPLAYON;
    command((Uint8)(LCD_DISPLAYCONTROL | _displaycontrol));
}

void noBlink() {
    Uint8 blink = LCD_BLINKON;
    _displaycontrol &= (Uint8)~blink;
    command((Uint8)(LCD_DISPLAYCONTROL | _displaycontrol));
}

void blink() {
    _displaycontrol |= LCD_BLINKON;
    command((Uint8)(LCD_DISPLAYCONTROL | _displaycontrol));
}

void noCursor() {
    Uint8 cursor = LCD_CURSORON;
    _displaycontrol &= (Uint8)~cursor;
    command((Uint8)(LCD_DISPLAYCONTROL | _displaycontrol));

}

void cursor() {
    _displaycontrol |= LCD_CURSORON;
    command((Uint8)(LCD_DISPLAYCONTROL | _displaycontrol));
}

void scrollDisplayLeft() {
     command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void scrollDisplayRight()
{
     command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void leftToRight() {
    _displaymode |= LCD_ENTRYLEFT;
    command((Uint8)(LCD_ENTRYMODESET | _displaymode));
}

void rightToLeft() {
    Uint8 entry = LCD_ENTRYLEFT;
    _displaymode &= (Uint8)~entry;
    command((Uint8)(LCD_ENTRYMODESET | _displaymode));
}

void noBacklight() {
    _backlightval = LCD_NOBACKLIGHT;
    expanderWrite(0);
}

void backlight()
{
    _backlightval = LCD_BACKLIGHT;
    expanderWrite(0);
}

void autoscroll() {
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    command((Uint8)(LCD_ENTRYMODESET | _displaymode));

}

void noAutoscroll() {
    Uint8 entry = LCD_ENTRYSHIFTINCREMENT;
    _displaymode &= (Uint8)~entry;
    command((Uint8)(LCD_ENTRYMODESET | _displaymode));
}

void write_bytes(char* value) {
    int position;
    int str_len = strlen(value);
    for (position = 0; position < str_len; position++) {
        write_byte((Uint8) value[position]);
    }
}

void write_byte(Uint8 value)
{
    send(value, 0x01);
}

void command(Uint8 value)
{
    send(value, 0);
}

void setBacklight(Bool new_val)
{
    if (new_val) backlight(); // turn backlight on
    else noBacklight(); // turn backlight off
}

void send(Uint8 value, Uint8 mode) {
    Uint8 highnib = (Uint8)(value & 0xf0);
    Uint8 lownib = (Uint8)((value << 4) & 0xf0);

    write4bits((Uint8)((highnib) | mode));
    write4bits((Uint8)((lownib) | mode));
}

void write4bits(Uint8 value) {
    expanderWrite(value);
    pulseEnable(value);
}

void expanderWrite(Uint8 _data) {
    Uint8 data[1];
    data[0] = (Uint8)(_data | _backlightval);
    USBSTK5515_I2C_write(_Addr, data, 1);
}

void pulseEnable(Uint8 _data)
{
    expanderWrite((Uint8)(_data | 0x04)); // En high
    expanderWrite((Uint8)(_data & ~0x04)); // En low
}

