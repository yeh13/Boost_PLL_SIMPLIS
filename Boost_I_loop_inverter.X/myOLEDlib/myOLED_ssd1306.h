/* 
 * File:   
 * Author:AdamSyu
 * Comments:
 * Revision history: 
 */

#ifndef _MYOLED_SSD1306_H
    #define _MYOLED_SSD1306_H

    #include "xc.h"

void myOLED_Init(void);
void myOLED_ClearScreen(void);
void myOLED_DrawBitmap(uint8_t column, uint8_t page, uint8_t width, uint8_t height, const uint8_t *byte);
void myOLED_DrawChar(uint8_t column, uint8_t page, uint8_t byte);
void myOLED_DrawStr(uint8_t column, uint8_t page, const uint8_t *byte);

    #define SSD1306_CMD_SET_DISPLAY_ON 0xAF
    #define SSD1306_CMD_SET_DISPLAY_OFF 0xAE
    #define SSD1306_CMD_SET_DISPLAY_START_LINE(line) (0x40 | (line))
    #define SSD1306_CMD_SET_DISPLAY_OFFSET 0xD3
    #define SSD1306_CMD_SET_SEGMENT_RE_MAP_COL0_SEG0 0xA0
    #define SSD1306_CMD_SET_SEGMENT_RE_MAP_COL127_SEG0 0xA1
    #define SSD1306_CMD_SET_COM_OUTPUT_SCAN_UP 0xC0
    #define SSD1306_CMD_SET_COM_OUTPUT_SCAN_DOWN 0xC8
    #define SSD1306_CMD_SET_COM_PINS 0xDA
    #define SSD1306_CMD_SET_CONTRAST_CONTROL_FOR_BANK0 0x81
    #define SSD1306_CMD_ENTIRE_DISPLAY_AND_GDDRAM_ON 0xA4
    #define SSD1306_CMD_ENTIRE_DISPLAY_ON 0xA5
    #define SSD1306_CMD_SET_NORMAL_DISPLAY 0xA6
    #define SSD1306_CMD_SET_INVERSE_DISPLAY 0xA7
    #define SSD1306_CMD_SET_MULTIPLEX_RATIO 0xA8
    #define SSD1306_CMD_SET_DISPLAY_CLOCK_DIVIDE_RATIO 0xD5
    #define SSD1306_CMD_SET_CHARGE_PUMP_SETTING 0x8D
    #define SSD1306_CMD_SET_VCOMH_DESELECT_LEVEL 0xDB
    #define SSD1306_CMD_SET_PRE_CHARGE_PERIOD 0xD9
    #define SSD1306_CMD_SET_MEMORY_ADDRESSING_MODE 0x20

//#define SSD1306_CMD_COL_ADD_SET_LSB(column) (0x00 | (column))
//#define SSD1306_CMD_COL_ADD_SET_MSB(column) (0x10 | (column))
//#define SSD1306_CMD_SET_COLUMN_ADDRESS 0x21
//#define SSD1306_CMD_SET_PAGE_ADDRESS 0x22
//#define SSD1306_CMD_SET_PAGE_START_ADDRESS(page) (0xB0 | (page))
//#define SSD1306_CMD_NOP 0xE3
////@}
////! \name Graphic Acceleration Command defines
////@{
//#define SSD1306_CMD_SCROLL_H_RIGHT 0x26
//#define SSD1306_CMD_SCROLL_H_LEFT 0x27
//#define SSD1306_CMD_CONTINUOUS_SCROLL_V_AND_H_RIGHT 0x29
//#define SSD1306_CMD_CONTINUOUS_SCROLL_V_AND_H_LEFT 0x2A
//#define SSD1306_CMD_DEACTIVATE_SCROLL 0x2E
//#define SSD1306_CMD_ACTIVATE_SCROLL 0x2F
//#define SSD1306_CMD_SET_VERTICAL_SCROLL_AREA 0xA3

#endif	/* _MYOLED_SSD1306_H */
