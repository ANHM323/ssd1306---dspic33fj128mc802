#define FCY 40000000UL
#include <libpic30.h>
#include "ssd1306.h"
#include "i2c_hal.h"
#include <stdlib.h>

static uint8_t ssd1306_buffer[SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8];

#define MAX_DIRTY_RECTS 8
typedef struct {
    uint8_t x, y, w, h;
} DirtyRect_t;

static DirtyRect_t dirty_rects[MAX_DIRTY_RECTS];
static uint8_t dirty_count = 0;
static uint8_t dirty_enabled = 1;

static const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, {0x00, 0x00, 0x5F, 0x00, 0x00},
    {0x00, 0x07, 0x00, 0x07, 0x00}, {0x14, 0x7F, 0x14, 0x7F, 0x14},
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, {0x23, 0x13, 0x08, 0x64, 0x62},
    {0x36, 0x49, 0x55, 0x22, 0x50}, {0x00, 0x05, 0x03, 0x00, 0x00},
    {0x00, 0x1C, 0x22, 0x41, 0x00}, {0x00, 0x41, 0x22, 0x1C, 0x00},
    {0x14, 0x08, 0x3E, 0x08, 0x14}, {0x08, 0x08, 0x3E, 0x08, 0x08},
    {0x00, 0x50, 0x30, 0x00, 0x00}, {0x08, 0x08, 0x08, 0x08, 0x08},
    {0x00, 0x60, 0x60, 0x00, 0x00}, {0x20, 0x10, 0x08, 0x04, 0x02},
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, {0x00, 0x42, 0x7F, 0x40, 0x00},
    {0x42, 0x61, 0x51, 0x49, 0x46}, {0x21, 0x41, 0x45, 0x4B, 0x31},
    {0x18, 0x14, 0x12, 0x7F, 0x10}, {0x27, 0x45, 0x45, 0x45, 0x39},
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, {0x01, 0x71, 0x09, 0x05, 0x03},
    {0x36, 0x49, 0x49, 0x49, 0x36}, {0x06, 0x49, 0x49, 0x29, 0x1E},
    {0x00, 0x36, 0x36, 0x00, 0x00}, {0x00, 0x56, 0x36, 0x00, 0x00},
    {0x08, 0x14, 0x22, 0x41, 0x00}, {0x14, 0x14, 0x14, 0x14, 0x14},
    {0x00, 0x41, 0x22, 0x14, 0x08}, {0x02, 0x01, 0x51, 0x09, 0x06},
    {0x32, 0x49, 0x79, 0x41, 0x3E}, {0x7E, 0x11, 0x11, 0x11, 0x7E},
    {0x7F, 0x49, 0x49, 0x49, 0x36}, {0x3E, 0x41, 0x41, 0x41, 0x22},
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, {0x7F, 0x49, 0x49, 0x49, 0x41},
    {0x7F, 0x09, 0x09, 0x09, 0x01}, {0x3E, 0x41, 0x49, 0x49, 0x7A},
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, {0x00, 0x41, 0x7F, 0x41, 0x00},
    {0x20, 0x40, 0x41, 0x3F, 0x01}, {0x7F, 0x08, 0x14, 0x22, 0x41},
    {0x7F, 0x40, 0x40, 0x40, 0x40}, {0x7F, 0x02, 0x0C, 0x02, 0x7F},
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, {0x3E, 0x41, 0x41, 0x41, 0x3E},
    {0x7F, 0x09, 0x09, 0x09, 0x06}, {0x3E, 0x41, 0x51, 0x21, 0x5E},
    {0x7F, 0x09, 0x19, 0x29, 0x46}, {0x46, 0x49, 0x49, 0x49, 0x31},
    {0x01, 0x01, 0x7F, 0x01, 0x01}, {0x3F, 0x40, 0x40, 0x40, 0x3F},
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, {0x3F, 0x40, 0x38, 0x40, 0x3F},
    {0x63, 0x14, 0x08, 0x14, 0x63}, {0x07, 0x08, 0x70, 0x08, 0x07},
    {0x61, 0x51, 0x49, 0x45, 0x43}, {0x00, 0x7F, 0x41, 0x41, 0x00},
    {0x02, 0x04, 0x08, 0x10, 0x20}, {0x00, 0x41, 0x41, 0x7F, 0x00},
    {0x04, 0x02, 0x01, 0x02, 0x04}, {0x40, 0x40, 0x40, 0x40, 0x40},
    {0x00, 0x01, 0x02, 0x04, 0x00}, {0x20, 0x54, 0x54, 0x54, 0x78},
    {0x7F, 0x48, 0x44, 0x44, 0x38}, {0x38, 0x44, 0x44, 0x44, 0x20},
    {0x38, 0x44, 0x44, 0x48, 0x7F}, {0x38, 0x54, 0x54, 0x54, 0x18},
    {0x08, 0x7E, 0x09, 0x01, 0x02}, {0x0C, 0x52, 0x52, 0x52, 0x3E},
    {0x7F, 0x08, 0x04, 0x04, 0x78}, {0x00, 0x44, 0x7D, 0x40, 0x00},
    {0x20, 0x40, 0x44, 0x3D, 0x00}, {0x7F, 0x10, 0x28, 0x44, 0x00},
    {0x00, 0x41, 0x7F, 0x40, 0x00}, {0x7C, 0x04, 0x18, 0x04, 0x78},
    {0x7C, 0x08, 0x04, 0x04, 0x78}, {0x38, 0x44, 0x44, 0x44, 0x38},
    {0x7C, 0x14, 0x14, 0x14, 0x08}, {0x08, 0x14, 0x14, 0x18, 0x7C},
    {0x7C, 0x08, 0x04, 0x04, 0x08}, {0x48, 0x54, 0x54, 0x54, 0x20},
    {0x04, 0x3F, 0x44, 0x40, 0x20}, {0x3C, 0x40, 0x40, 0x20, 0x7C},
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, {0x3C, 0x40, 0x30, 0x40, 0x3C},
    {0x44, 0x28, 0x10, 0x28, 0x44}, {0x0C, 0x50, 0x50, 0x50, 0x3C},
    {0x44, 0x64, 0x54, 0x4C, 0x44},
};

static void SSD1306_Command(uint8_t cmd) {
    I2C_Start();
    I2C_Write(SSD1306_ADDRESS << 1);
    I2C_Write(0x00);
    I2C_Write(cmd);
    I2C_Stop();
}

void SSD1306_Init(void) {
    __delay_ms(150);
    SSD1306_Command(SSD1306_DISPLAYOFF);
    SSD1306_Command(SSD1306_SETDISPLAYCLOCKDIV);
    SSD1306_Command(0x80);
    SSD1306_Command(SSD1306_SETMULTIPLEX);
    SSD1306_Command(0x3F);
    SSD1306_Command(SSD1306_SETDISPLAYOFFSET);
    SSD1306_Command(0x00);
    SSD1306_Command(SSD1306_SETSTARTLINE | 0x00);
    SSD1306_Command(SSD1306_CHARGEPUMP);
    SSD1306_Command(0x14);
    SSD1306_Command(SSD1306_MEMORYMODE);
    SSD1306_Command(0x00);
    SSD1306_Command(SSD1306_SEGREMAP | 0x01);
    SSD1306_Command(SSD1306_COMSCANDEC);
    SSD1306_Command(SSD1306_SETCOMPINS);
    SSD1306_Command(0x12);
    SSD1306_Command(SSD1306_SETCONTRAST);
    SSD1306_Command(0xCF);
    SSD1306_Command(SSD1306_SETPRECHARGE);
    SSD1306_Command(0xF1);
    SSD1306_Command(SSD1306_SETVCOMDETECT);
    SSD1306_Command(0x40);
    SSD1306_Command(SSD1306_DISPLAYALLON_RESUME);
    SSD1306_Command(SSD1306_NORMALDISPLAY);
    SSD1306_Command(SSD1306_DISPLAYON);
    SSD1306_Clear();
    SSD1306_Display();
}

void SSD1306_Clear(void) {
    memset(ssd1306_buffer, 0x00, sizeof(ssd1306_buffer));
}

void SSD1306_Display(void) {
    SSD1306_Command(SSD1306_COLUMNADDR);
    SSD1306_Command(0);
    SSD1306_Command(127);
    SSD1306_Command(SSD1306_PAGEADDR);
    SSD1306_Command(0);
    SSD1306_Command(7);
    
    I2C_Start();
    I2C_Write(SSD1306_ADDRESS << 1);
    I2C_Write(0x40);
    for(uint16_t i = 0; i < sizeof(ssd1306_buffer); i++) {
        I2C_Write(ssd1306_buffer[i]);
    }
    I2C_Stop();
}

void SSD1306_UpdateRegion(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    uint8_t start_page = y / 8;
    uint8_t end_page = (y + h - 1) / 8;
    
    if(x >= SSD1306_LCDWIDTH) return;
    if(x + w > SSD1306_LCDWIDTH) w = SSD1306_LCDWIDTH - x;
    if(end_page > 7) end_page = 7;
    
    SSD1306_Command(SSD1306_COLUMNADDR);
    SSD1306_Command(x);
    SSD1306_Command(x + w - 1);
    SSD1306_Command(SSD1306_PAGEADDR);
    SSD1306_Command(start_page);
    SSD1306_Command(end_page);
    
    I2C_Start();
    I2C_Write(SSD1306_ADDRESS << 1);
    I2C_Write(0x40);
    for(uint8_t page = start_page; page <= end_page; page++) {
        for(uint8_t col = x; col < x + w; col++) {
            I2C_Write(ssd1306_buffer[col + page * SSD1306_LCDWIDTH]);
        }
    }
    I2C_Stop();
}

void SSD1306_MarkDirty(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    if(!dirty_enabled || dirty_count >= MAX_DIRTY_RECTS) return;
    dirty_rects[dirty_count].x = x;
    dirty_rects[dirty_count].y = y;
    dirty_rects[dirty_count].w = w;
    dirty_rects[dirty_count].h = h;
    dirty_count++;
}

void SSD1306_DisplayDirty(void) {
    if(!dirty_enabled || dirty_count == 0) {
        SSD1306_Display();
        return;
    }
    for(uint8_t i = 0; i < dirty_count; i++) {
        SSD1306_UpdateRegion(dirty_rects[i].x, dirty_rects[i].y,
                            dirty_rects[i].w, dirty_rects[i].h);
    }
    dirty_count = 0;
}

void SSD1306_ClearDirty(void) {
    dirty_count = 0;
}

void SSD1306_SetDirtyTracking(uint8_t enable) {
    dirty_enabled = enable;
    if(!enable) dirty_count = 0;
}

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

void SSD1306_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color) {
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;
    
    uint8_t min_x = (x0 < x1) ? x0 : x1;
    uint8_t max_x = (x0 > x1) ? x0 : x1;
    uint8_t min_y = (y0 < y1) ? y0 : y1;
    uint8_t max_y = (y0 > y1) ? y0 : y1;
    
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
    SSD1306_MarkDirty(min_x, min_y, max_x - min_x + 1, max_y - min_y + 1);
}

void SSD1306_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
    SSD1306_DrawLine(x, y, x + w - 1, y, color);
    SSD1306_DrawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
    SSD1306_DrawLine(x + w - 1, y + h - 1, x, y + h - 1, color);
    SSD1306_DrawLine(x, y + h - 1, x, y, color);
}

void SSD1306_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
    for(uint8_t i = x; i < x + w && i < SSD1306_LCDWIDTH; i++) {
        for(uint8_t j = y; j < y + h && j < SSD1306_LCDHEIGHT; j++) {
            SSD1306_SetPixel(i, j, color);
        }
    }
    SSD1306_MarkDirty(x, y, w, h);
}

void SSD1306_DrawCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    
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
        
        SSD1306_SetPixel(x0 + x, y0 + y, color);
        SSD1306_SetPixel(x0 - x, y0 + y, color);
        SSD1306_SetPixel(x0 + x, y0 - y, color);
        SSD1306_SetPixel(x0 - x, y0 - y, color);
        SSD1306_SetPixel(x0 + y, y0 + x, color);
        SSD1306_SetPixel(x0 - y, y0 + x, color);
        SSD1306_SetPixel(x0 + y, y0 - x, color);
        SSD1306_SetPixel(x0 - y, y0 - x, color);
    }
    SSD1306_MarkDirty(x0 - r, y0 - r, 2 * r + 1, 2 * r + 1);
}

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
        
        SSD1306_DrawLine(x0 + x, y0 - y, x0 + x, y0 + y, color);
        SSD1306_DrawLine(x0 - x, y0 - y, x0 - x, y0 + y, color);
        SSD1306_DrawLine(x0 + y, y0 - x, x0 + y, y0 + x, color);
        SSD1306_DrawLine(x0 - y, y0 - x, x0 - y, y0 + x, color);
    }
}

void SSD1306_DrawChar(uint8_t x, uint8_t y, char c, uint8_t color, uint8_t bg) {
    if(c < 32 || c > 122) c = 32;
    c -= 32;
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
    SSD1306_MarkDirty(x, y, 5, 8);
}

void SSD1306_DrawString(uint8_t x, uint8_t y, const char *str, uint8_t color, uint8_t bg) {
    uint8_t start_x = x;
    while(*str) {
        if(*str == '\n') {
            y += 8;
            x = 0;
        } else if(*str == '\r') {
            x = 0;
        } else {
            SSD1306_DrawChar(x, y, *str, color, bg);
            x += 6;
            if(x + 5 >= SSD1306_LCDWIDTH) {
                x = 0;
                y += 8;
            }
        }
        str++;
    }
}

void SSD1306_SetContrast(uint8_t contrast) {
    SSD1306_Command(SSD1306_SETCONTRAST);
    SSD1306_Command(contrast);
}

void SSD1306_InvertDisplay(bool invert) {
    SSD1306_Command(invert ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
}

void SSD1306_DrawBitmap(uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color) {
    for(uint8_t j = 0; j < h; j++) {
        for(uint8_t i = 0; i < w; i++) {
            if(bitmap[j * ((w + 7) / 8) + i / 8] & (1 << (i % 8))) {
                SSD1306_SetPixel(x + i, y + j, color);
            }
        }
    }
    SSD1306_MarkDirty(x, y, w, h);
}