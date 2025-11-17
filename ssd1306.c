#define FCY 40000000UL
#include <libpic30.h>
#include "ssd1306.h"
#include "i2c_hal.h"
#include <stdlib.h>

// Buffer de pantalla
static uint8_t ssd1306_buffer[SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8];

// Fuente 5x7 píxeles
static const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // (space)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // backslash
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // f
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // z
};

// Enviar comando al SSD1306
static void SSD1306_Command(uint8_t cmd) {
    I2C_Start();
    I2C_Write(SSD1306_ADDRESS << 1);  // Dirección + bit escritura
    I2C_Write(0x00);                   // Co=0, D/C=0 (comando)
    I2C_Write(cmd);                    // Comando
    I2C_Stop();
}

// Inicializar display
void SSD1306_Init(void) {
    __delay_ms(150);  // Esperar power-on reset del display
    
    // Secuencia de inicialización
    SSD1306_Command(SSD1306_DISPLAYOFF);         // Display off
    SSD1306_Command(SSD1306_SETDISPLAYCLOCKDIV); // Clock divide
    SSD1306_Command(0x80);                       // Valor recomendado
    SSD1306_Command(SSD1306_SETMULTIPLEX);       // Multiplex ratio
    SSD1306_Command(0x3F);                       // 64 líneas
    SSD1306_Command(SSD1306_SETDISPLAYOFFSET);   // Display offset
    SSD1306_Command(0x00);                       // Sin offset
    SSD1306_Command(SSD1306_SETSTARTLINE | 0x00);// Start line 0
    SSD1306_Command(SSD1306_CHARGEPUMP);         // Charge pump
    SSD1306_Command(0x14);                       // Habilitar (0x14=on, 0x10=off)
    SSD1306_Command(SSD1306_MEMORYMODE);         // Memory mode
    SSD1306_Command(0x00);                       // Horizontal
    SSD1306_Command(SSD1306_SEGREMAP | 0x01);    // Seg remap
    SSD1306_Command(SSD1306_COMSCANDEC);         // COM scan direction
    SSD1306_Command(SSD1306_SETCOMPINS);         // COM pins
    SSD1306_Command(0x12);                       // Alternative COM, disable remap
    SSD1306_Command(SSD1306_SETCONTRAST);        // Contraste
    SSD1306_Command(0xCF);                       // Valor contraste
    SSD1306_Command(SSD1306_SETPRECHARGE);       // Precharge
    SSD1306_Command(0xF1);                       // Valor precharge
    SSD1306_Command(SSD1306_SETVCOMDETECT);      // VCOMH
    SSD1306_Command(0x40);                       // Valor VCOMH
    SSD1306_Command(SSD1306_DISPLAYALLON_RESUME);// Resume to RAM
    SSD1306_Command(SSD1306_NORMALDISPLAY);      // Display normal
    SSD1306_Command(SSD1306_DISPLAYON);          // Display on
    
    SSD1306_Clear();
    SSD1306_Display();
}

// Limpiar buffer
void SSD1306_Clear(void) {
    memset(ssd1306_buffer, 0x00, sizeof(ssd1306_buffer));
}

// Enviar buffer al display
void SSD1306_Display(void) {
    // Configurar ventana completa
    SSD1306_Command(SSD1306_COLUMNADDR);
    SSD1306_Command(0);    // Columna inicial
    SSD1306_Command(127);  // Columna final
    SSD1306_Command(SSD1306_PAGEADDR);
    SSD1306_Command(0);    // Página inicial
    SSD1306_Command(7);    // Página final
    
    // Enviar datos
    I2C_Start();
    I2C_Write(SSD1306_ADDRESS << 1);  // Dirección + escritura
    I2C_Write(0x40);                   // Co=0, D/C=1 (datos)
    
    // Enviar buffer completo
    for(uint16_t i = 0; i < sizeof(ssd1306_buffer); i++) {
        I2C_Write(ssd1306_buffer[i]);
    }
    
    I2C_Stop();
}

// Establecer pixel
void SSD1306_SetPixel(uint8_t x, uint8_t y, uint8_t color) {
    if(x >= SSD1306_LCDWIDTH || y >= SSD1306_LCDHEIGHT) return;
    
    switch(color) {
        case SSD1306_WHITE:
            ssd1306_buffer[x + (y/8)*SSD1306_LCDWIDTH] |= (1 << (y & 7));
            break;
        case SSD1306_BLACK:
            ssd1306_buffer[x + (y/8)*SSD1306_LCDWIDTH] &= ~(1 << (y & 7));
            break;
        case SSD1306_INVERSE:
            ssd1306_buffer[x + (y/8)*SSD1306_LCDWIDTH] ^= (1 << (y & 7));
            break;
    }
}

// Dibujar línea - Bresenham
void SSD1306_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color) {
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;
    
    while(1) {
        SSD1306_SetPixel(x0, y0, color);
        if(x0 == x1 && y0 == y1) break;
        
        int16_t e2 = 2 * err;
        if(e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if(e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Dibujar rectángulo
void SSD1306_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
    SSD1306_DrawLine(x, y, x + w - 1, y, color);
    SSD1306_DrawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
    SSD1306_DrawLine(x + w - 1, y + h - 1, x, y + h - 1, color);
    SSD1306_DrawLine(x, y + h - 1, x, y, color);
}

// Rellenar rectángulo
void SSD1306_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
    for(uint8_t i = x; i < x + w; i++) {
        for(uint8_t j = y; j < y + h; j++) {
            SSD1306_SetPixel(i, j, color);
        }
    }
}

// Dibujar círculo - Midpoint
void SSD1306_DrawCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    
    // Puntos cardinales
    SSD1306_SetPixel(x0, y0 + r, color);
    SSD1306_SetPixel(x0, y0 - r, color);
    SSD1306_SetPixel(x0 + r, y0, color);
    SSD1306_SetPixel(x0 - r, y0, color);
    
    while(x < y) {
        if(f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        
        // 8 puntos por simetría
        SSD1306_SetPixel(x0 + x, y0 + y, color);
        SSD1306_SetPixel(x0 - x, y0 + y, color);
        SSD1306_SetPixel(x0 + x, y0 - y, color);
        SSD1306_SetPixel(x0 - x, y0 - y, color);
        SSD1306_SetPixel(x0 + y, y0 + x, color);
        SSD1306_SetPixel(x0 - y, y0 + x, color);
        SSD1306_SetPixel(x0 + y, y0 - x, color);
        SSD1306_SetPixel(x0 - y, y0 - x, color);
    }
}

// Rellenar círculo
void SSD1306_FillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color) {
    SSD1306_DrawLine(x0, y0 - r, x0, y0 + r, color);
    
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    
    while(x < y) {
        if(f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        
        // Líneas verticales para rellenar
        SSD1306_DrawLine(x0 + x, y0 - y, x0 + x, y0 + y, color);
        SSD1306_DrawLine(x0 - x, y0 - y, x0 - x, y0 + y, color);
        SSD1306_DrawLine(x0 + y, y0 - x, x0 + y, y0 + x, color);
        SSD1306_DrawLine(x0 - y, y0 - x, x0 - y, y0 + x, color);
    }
}

// Dibujar carácter
void SSD1306_DrawChar(uint8_t x, uint8_t y, char c, uint8_t color, uint8_t bg) {
    // Limitar a caracteres válidos
    if(c < 32 || c > 122) c = 32;
    c -= 32;
    
    // Dibujar carácter de 5 columnas
    for(uint8_t i = 0; i < 5; i++) {
        uint8_t line = font5x7[(uint8_t)c][i];
        for(uint8_t j = 0; j < 8; j++, line >>= 1) {
            if(line & 1) {
                SSD1306_SetPixel(x + i, y + j, color);
            } else if(bg != color) {
                SSD1306_SetPixel(x + i, y + j, bg);
            }
        }
    }
}

// Dibujar cadena de texto
void SSD1306_DrawString(uint8_t x, uint8_t y, const char *str, uint8_t color, uint8_t bg) {
    while(*str) {
        if(*str == '\n') {
            y += 8;  // Salto de línea
            x = 0;
        } else if(*str == '\r') {
            x = 0;   // Retorno de carro
        } else {
            SSD1306_DrawChar(x, y, *str, color, bg);
            x += 6;  // Ancho de carácter + espacio
            if(x + 5 >= SSD1306_LCDWIDTH) {
                x = 0;
                y += 8;
            }
        }
        str++;
    }
}

// Establecer contraste
void SSD1306_SetContrast(uint8_t contrast) {
    SSD1306_Command(SSD1306_SETCONTRAST);
    SSD1306_Command(contrast);
}

// Invertir display
void SSD1306_InvertDisplay(bool invert) {
    SSD1306_Command(invert ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
}

// Dibujar bitmap
void SSD1306_DrawBitmap(uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color) {
    for(uint8_t j = 0; j < h; j++) {
        for(uint8_t i = 0; i < w; i++) {
            if(bitmap[j * ((w + 7) / 8) + i / 8] & (1 << (i % 8))) {
                SSD1306_SetPixel(x + i, y + j, color);
            }
        }
    }
}    