#ifndef SSD1306_H
#define SSD1306_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Dirección I2C - probar ambas si no funciona
#define SSD1306_ADDRESS         0x3C   // También puede ser 0x3D

// Dimensiones del display
#define SSD1306_LCDWIDTH        128
#define SSD1306_LCDHEIGHT       64

// Comandos fundamentales
#define SSD1306_SETCONTRAST     0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON    0xA5
#define SSD1306_NORMALDISPLAY   0xA6
#define SSD1306_INVERTDISPLAY   0xA7
#define SSD1306_DISPLAYOFF      0xAE
#define SSD1306_DISPLAYON       0xAF

// Comandos de configuración
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS      0xDA
#define SSD1306_SETVCOMDETECT   0xDB
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE    0xD9
#define SSD1306_SETMULTIPLEX    0xA8
#define SSD1306_SETLOWCOLUMN    0x00
#define SSD1306_SETHIGHCOLUMN   0x10
#define SSD1306_SETSTARTLINE    0x40

// Comandos de memoria
#define SSD1306_MEMORYMODE      0x20
#define SSD1306_COLUMNADDR      0x21
#define SSD1306_PAGEADDR        0x22

// Comandos de hardware
#define SSD1306_COMSCANINC      0xC0
#define SSD1306_COMSCANDEC      0xC8
#define SSD1306_SEGREMAP        0xA0
#define SSD1306_CHARGEPUMP      0x8D

// Colores
#define SSD1306_BLACK           0
#define SSD1306_WHITE           1
#define SSD1306_INVERSE         2

// Funciones principales
void SSD1306_Init(void);
void SSD1306_Clear(void);
void SSD1306_Display(void);
void SSD1306_SetPixel(uint8_t x, uint8_t y, uint8_t color);

// Funciones gráficas
void SSD1306_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color);
void SSD1306_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void SSD1306_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void SSD1306_DrawCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color);
void SSD1306_FillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color);

// Funciones de texto
void SSD1306_DrawChar(uint8_t x, uint8_t y, char c, uint8_t color, uint8_t bg);
void SSD1306_DrawString(uint8_t x, uint8_t y, const char *str, uint8_t color, uint8_t bg);

// Funciones de control
void SSD1306_SetContrast(uint8_t contrast);
void SSD1306_InvertDisplay(bool invert);
void SSD1306_DrawBitmap(uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color);

#endif