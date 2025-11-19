#define FCY 40000000UL
#include <libpic30.h>
#include "i2c_hal.h"

void I2C_Init(void) {
    AD1PCFGL = 0xFFFF;
    
    TRISBbits.TRISB8 = 1;
    TRISBbits.TRISB9 = 1;
    
    ODCBbits.ODCB8 = 1;
    ODCBbits.ODCB9 = 1;
    
    I2C1CONbits.I2CEN = 0;
    
    I2C1BRG = 48;  // 400kHz Fast Mode con FCY=40MHz
    
    I2C1CON = 0x0000;
    I2C1STAT = 0x0000;
    
    I2C1CONbits.DISSLW = 1;  // Slew rate control OFF para Fast Mode
    I2C1CONbits.A10M = 0;
    I2C1CONbits.SCLREL = 1;
    
    I2C1CONbits.I2CEN = 1;
    __delay_us(100);
}

void I2C_WaitIdle(void) {
    while(I2C1CONbits.SEN || I2C1CONbits.PEN || 
          I2C1CONbits.RSEN || I2C1CONbits.RCEN || 
          I2C1CONbits.ACKEN || I2C1STATbits.TRSTAT);
}

void I2C_Start(void) {
    I2C_WaitIdle();
    I2C1CONbits.SEN = 1;
    while(I2C1CONbits.SEN);
}

void I2C_Stop(void) {
    I2C_WaitIdle();
    I2C1CONbits.PEN = 1;
    while(I2C1CONbits.PEN);
}

void I2C_Restart(void) {
    I2C_WaitIdle();
    I2C1CONbits.RSEN = 1;
    while(I2C1CONbits.RSEN);
}

bool I2C_Write(uint8_t data) {
    I2C_WaitIdle();
    I2C1TRN = data;
    while(I2C1STATbits.TRSTAT);
    
    if(I2C1STATbits.IWCOL) {
        I2C1STATbits.IWCOL = 0;
        return false;
    }
    
    return !I2C1STATbits.ACKSTAT;
}

uint8_t I2C_Read(bool ack) {
    uint8_t data;
    I2C_WaitIdle();
    
    I2C1CONbits.RCEN = 1;
    while(!I2C1STATbits.RBF);
    data = I2C1RCV;
    
    I2C_WaitIdle();
    I2C1CONbits.ACKDT = ack ? 0 : 1;
    I2C1CONbits.ACKEN = 1;
    while(I2C1CONbits.ACKEN);
    
    return data;
}