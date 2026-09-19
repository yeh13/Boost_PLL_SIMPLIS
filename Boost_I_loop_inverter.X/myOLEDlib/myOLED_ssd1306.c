/* 
 * File:   
 * Author:AdamSyu
 * Comments:
 * Revision history: 
 */

#include "./myOLED_ssd1306.h"
#include <xc.h>
#include "./../mcc_generated_files/delay.h"
#include "./../mcc_generated_files/pin_manager.h"
#include "./../mcc_generated_files/spi1.h"

extern const uint8_t Alphabet_Fonts[];
// 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz~!@#$%^&()[]{}_\|;:,.`'"<>+-*/=? 
char Alphabet[95] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz~!@#$%^&()[]{}_\\|;:,.`'\"<>+-*/=? ";
#define FONT_WIDTH  7
#define FONT_HEIGHT 8

void myOLED_write_command(uint8_t command)
{
    OLED_RS_SetLow(); /* RS  = 0, Instruction Mode */
    OLED_nCS_SetLow();
    SPI1_Exchange8bit(command);
    OLED_nCS_SetHigh();
}

void myOLED_write_data(uint8_t data)
{
    OLED_RS_SetHigh(); /* RS  = 1, Data Mode */
    OLED_nCS_SetLow();
    SPI1_Exchange8bit(data);
    OLED_nCS_SetHigh();
}

void myOLED_Init(void)
{
    /**** Reset Feature for OLED (WEA012864DWPP3N00002 ****/
    OLED_nRST_SetLow();
    DELAY_milliseconds(100);
    OLED_nRST_SetHigh();

    OLED_RS_SetHigh();
    OLED_nCS_SetHigh();

    // Display turn off
    myOLED_write_command(SSD1306_CMD_SET_DISPLAY_OFF);

    // Set mux ratio 1/64 Duty (0x0F~0x3F)
    myOLED_write_command(SSD1306_CMD_SET_MULTIPLEX_RATIO);
    myOLED_write_command(0x3F); //64

    // Shift Mapping RAM Counter (0x00~0x3F)
    myOLED_write_command(SSD1306_CMD_SET_DISPLAY_OFFSET);
    myOLED_write_command(0x00);
    // Set Mapping RAM Display Start Line (0x00~0x3F)
    myOLED_write_command(SSD1306_CMD_SET_DISPLAY_START_LINE(0x00));
    // Set Column Address 0 Mapped to SEG0
    myOLED_write_command(SSD1306_CMD_SET_SEGMENT_RE_MAP_COL127_SEG0);
    // Set COM/Row Scan Scan from COM63 to 0
    myOLED_write_command(SSD1306_CMD_SET_COM_OUTPUT_SCAN_DOWN);
    // Set COM Pins hardware configuration
    myOLED_write_command(SSD1306_CMD_SET_COM_PINS);
    myOLED_write_command(0x12);
    // Set Contrast
    myOLED_write_command(SSD1306_CMD_SET_CONTRAST_CONTROL_FOR_BANK0);
    myOLED_write_command(0xCF);
    // Disable Entire display On
    myOLED_write_command(SSD1306_CMD_ENTIRE_DISPLAY_AND_GDDRAM_ON);
    // Set display invert disable
    myOLED_write_command(SSD1306_CMD_SET_NORMAL_DISPLAY);
    // Set Display Clock Divide Ratio / Oscillator Frequency (Default => 0x80)
    myOLED_write_command(SSD1306_CMD_SET_DISPLAY_CLOCK_DIVIDE_RATIO);
    myOLED_write_command(0x80);
    // Enable charge pump regulator
    myOLED_write_command(SSD1306_CMD_SET_CHARGE_PUMP_SETTING);
    myOLED_write_command(0x14);
    // Set addressging mode
    myOLED_write_command(SSD1306_CMD_SET_MEMORY_ADDRESSING_MODE);
    myOLED_write_command(0x02); // horizontal addressing mode
    //Set VCOMH Deselect Level
    myOLED_write_command(SSD1306_CMD_SET_VCOMH_DESELECT_LEVEL);
    myOLED_write_command(0x40); // Default => 0x20 (0.77*VCC)
    // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    myOLED_write_command(SSD1306_CMD_SET_PRE_CHARGE_PERIOD);
    myOLED_write_command(0xF1);
    // Display turn on
    myOLED_write_command(SSD1306_CMD_SET_DISPLAY_ON);
}

void myOLED_SetColumnPage(uint8_t column, uint8_t page)
{
    // Column 0-127
    myOLED_write_command(0x10 | (0x0f & (column >> 4)));
    myOLED_write_command(0x0f & column);

    // Page 0-7
    myOLED_write_command(0xB0 | (0x0f & page));
}

void myOLED_ClearScreen(void)
{
    uint8_t columnIndex = 0;
    uint8_t pageIndex = 0;

    for (pageIndex = 0; pageIndex < 8; pageIndex++)
    {
        for (columnIndex = 0; columnIndex < 128; columnIndex++)
        {
            myOLED_SetColumnPage(columnIndex, pageIndex);
            OLED_RS_SetHigh(); // Data
            OLED_nCS_SetLow();
            myOLED_write_data(0x00);
            OLED_nCS_SetHigh();
        }
    }
}

void myOLED_DrawBitmap(uint8_t column, uint8_t page, uint8_t width, uint8_t height, const uint8_t *byte)
{
    uint8_t columnIndex = column;
    uint8_t pageIndex = page;
    uint8_t bitmapcolumnIndex = 0;
    uint8_t bitmappageIndex = 0;

    for (bitmappageIndex = 0; bitmappageIndex < (height / 8); bitmappageIndex++)
    {
        for (bitmapcolumnIndex = 0; bitmapcolumnIndex < width; bitmapcolumnIndex++)
        {
            myOLED_SetColumnPage(columnIndex, pageIndex);
            OLED_RS_SetHigh(); // Data
            OLED_nCS_SetLow();
            //myOLED_write_data(*(byte + (bitmappageIndex * width) + bitmapcolumnIndex++));
            myOLED_write_data(*(byte++));
            OLED_nCS_SetHigh();
            columnIndex++;
        }
        pageIndex++;
        columnIndex = column;
    }
}

void myOLED_DrawChar(uint8_t column, uint8_t page, uint8_t byte)
{
    uint8_t Index = 0;
    while (byte != Alphabet[Index++])
    {
        //if(Index > 100)
    }
    myOLED_DrawBitmap(column, page, FONT_WIDTH, FONT_HEIGHT, Alphabet_Fonts + ((Index - 1) * FONT_WIDTH));
}

void myOLED_DrawStr(uint8_t column, uint8_t page, const uint8_t * byte)
{
    uint8_t columnindex = column;
    uint8_t pageindex = page;

    while ((*byte != '\0') &&(*byte != '\r') &&(*byte != '\n'))
    {
        myOLED_DrawChar(columnindex, pageindex, *byte);
        columnindex += FONT_WIDTH;
        byte++;
    }
}

/*
 0xAE, // DISPLAY OFF
 0xD5, // SET OSC FREQUENY
 0x80, // divide ratio = 1 (bit 3-0), OSC (bit 7-4)
 0xA8, // SET MUX RATIO
 0x3F, // 64MUX
 0xD3, // SET DISPLAY OFFSET
 0x00, // offset = 0
 0x40, // set display start line, start line = 0
 0x8D, // ENABLE CHARGE PUMP REGULATOR
 0x14, //
 0x20, // SET MEMORY ADDRESSING MODE
 0x02, // horizontal addressing mode
 0xA1, // set segment re-map, column address 127 is mapped to SEG0
 0xC8, // set COM/Output scan direction, remapped mode (COM[N-1] to COM0)
 0xDA, // SET COM PINS HARDWARE CONFIGURATION
 0x12, // alternative COM pin configuration
 0x81, // SET CONTRAST CONTROL
 0xCF, //
 0xD9, // SET PRE CHARGE PERIOD
 0xF1, //
 0xDB, // SET V_COMH DESELECT LEVEL
 0x40, //
 0xA4, // DISABLE ENTIRE DISPLAY ON
 0xA6, // NORMAL MODE (A7 for inverse display)
 0xAF // DISPLAY ON
 */
