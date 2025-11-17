#define FCY 40000000UL
#include <libpic30.h>
#include "i2c_hal.h"

void I2C_Init(void) {
    // Pines como digitales
    AD1PCFGL = 0xFFFF;
    
    // Pines como entrada
    TRISBbits.TRISB8 = 1;   // SCL
    TRISBbits.TRISB9 = 1;   // SDA
    
    // Open Drain obligatorio para I2C
    ODCBbits.ODCB8 = 1;     // SCL open drain
    ODCBbits.ODCB9 = 1;     // SDA open drain
    
    // Deshabilitar I2C para configurar
    I2C1CONbits.I2CEN = 0;
    
    // Baud rate 100kHz: (FCY/FSCL - FCY/10000000) - 1
    I2C1BRG = 395;          // ((40MHz/100kHz) - (40MHz/10MHz)) - 1
    
    // Limpiar registros
    I2C1CON = 0x0000;
    I2C1STAT = 0x0000;
    
    // Habilitar I2C
    I2C1CONbits.I2CEN = 1;
    
    // Tiempo para estabilizar
    __delay_ms(50);
}

void I2C_WaitIdle(void) {
    // Esperar que todos los bits de estado estén inactivos
    while(I2C1CONbits.SEN || I2C1CONbits.PEN || 
          I2C1CONbits.RSEN || I2C1CONbits.RCEN || 
          I2C1CONbits.ACKEN || I2C1STATbits.TRSTAT);
}

void I2C_Start(void) {
    I2C_WaitIdle();
    I2C1CONbits.SEN = 1;        // Iniciar condición START
    while(I2C1CONbits.SEN);     // Esperar que termine
}

void I2C_Stop(void) {
    I2C_WaitIdle();
    I2C1CONbits.PEN = 1;        // Iniciar condición STOP
    while(I2C1CONbits.PEN);     // Esperar que termine
}

void I2C_Restart(void) {
    I2C_WaitIdle();
    I2C1CONbits.RSEN = 1;       // Iniciar RESTART
    while(I2C1CONbits.RSEN);    // Esperar que termine
}

bool I2C_Write(uint8_t data) {
    I2C_WaitIdle();
    I2C1TRN = data;             // Cargar dato a transmitir
    while(I2C1STATbits.TRSTAT); // Esperar transmisión
    return !I2C1STATbits.ACKSTAT; // Retornar true si hay ACK
}

uint8_t I2C_Read(bool ack) {
    uint8_t data;
    I2C_WaitIdle();
    I2C1CONbits.RCEN = 1;       // Habilitar recepción
    while(I2C1CONbits.RCEN);    // Esperar dato
    data = I2C1RCV;             // Leer dato recibido
    
    I2C_WaitIdle();
    I2C1CONbits.ACKDT = ack ? 0 : 1; // 0=ACK, 1=NACK
    I2C1CONbits.ACKEN = 1;      // Enviar ACK/NACK
    while(I2C1CONbits.ACKEN);   // Esperar que termine
    
    return data;
}