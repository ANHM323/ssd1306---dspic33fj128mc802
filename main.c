#include <xc.h>
#include <stdio.h>
#define FCY 40000000UL
#include <libpic30.h>
#include "ssd1306.h"
#include "i2c_hal.h"

// Configuration bits
#pragma config FNOSC = FRC      // Oscilador FRC interno
#pragma config FWDTEN = OFF     // Watchdog deshabilitado
#pragma config OSCIOFNC = OFF   // OSC2 como pin de reloj
#pragma config FCKSM = CSECMD   // Clock switching habilitado
#pragma config POSCMD = NONE    // Sin oscilador primario
#pragma config IESO = OFF       // Start-up con oscilador seleccionado
#pragma config ICS = PGD1       // ICSP por PGD1/PGC1
#pragma config JTAGEN = OFF     // JTAG deshabilitado

// Configurar oscilador para 40 MIPS
void ConfigureOscillator(void) {
    // FRC = 7.37MHz
    PLLFBD = 38;                // M = 40
    CLKDIVbits.PLLPOST = 0;     // N2 = 2
    CLKDIVbits.PLLPRE = 0;      // N1 = 2
    // Fosc = 7.37MHz * 40 / 2 / 2 = 73.7MHz
    // FCY = 73.7MHz / 2 = ~37MHz (cercano a 40MIPS)
    
    // Cambiar a FRC con PLL
    __builtin_write_OSCCONH(0x01);
    __builtin_write_OSCCONL(0x01);
    
    // Esperar PLL lock
    while(OSCCONbits.COSC != 1);
    while(OSCCONbits.LOCK != 1);
}

int main(void) {
    // Configurar sistema
    ConfigureOscillator();
    
    // Configurar todos los pines como digitales
    AD1PCFGL = 0xFFFF;
    
    // Inicializar I2C
    I2C_Init();
    __delay_ms(100);  // Estabilización extra
    
    // Inicializar display
    SSD1306_Init();
    
    // Limpiar pantalla
    SSD1306_Clear();
    
    // Mostrar texto de título
    SSD1306_DrawString(0, 0, "PIC33FJ128MC802", SSD1306_WHITE, SSD1306_BLACK);
    SSD1306_DrawString(0, 10, "SSD1306 OLED", SSD1306_WHITE, SSD1306_BLACK);
    SSD1306_DrawString(0, 20, "XC16 Driver OK", SSD1306_WHITE, SSD1306_BLACK);
    
    // Dibujar marco
    SSD1306_DrawRect(5, 35, 118, 24, SSD1306_WHITE);
    
    // Figuras geométricas
    SSD1306_DrawCircle(20, 47, 8, SSD1306_WHITE);
    SSD1306_FillCircle(40, 47, 6, SSD1306_WHITE);
    
    // Cruz
    SSD1306_DrawLine(55, 40, 75, 54, SSD1306_WHITE);
    SSD1306_DrawLine(75, 40, 55, 54, SSD1306_WHITE);
    
    // Rectángulo relleno
    SSD1306_FillRect(85, 42, 15, 10, SSD1306_WHITE);
    
    // Mostrar pantalla inicial
    SSD1306_Display();
    
    // Variables para contador
    uint8_t counter = 0;
    char buffer[16];
    
    // Bucle principal
    while(1) {
        __delay_ms(1000);  // Esperar 1 segundo
        
        // Limpiar área del contador
        SSD1306_FillRect(0, 56, 128, 8, SSD1306_BLACK);
        
        // Actualizar contador
        sprintf(buffer, "Count: %d", counter);
        SSD1306_DrawString(30, 56, buffer, SSD1306_WHITE, SSD1306_BLACK);
        
        // Actualizar display
        SSD1306_Display();
        
        // Incrementar contador
        counter++;
        if(counter > 99) counter = 0;
    }
    
    return 0;
}