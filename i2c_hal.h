#ifndef I2C_HAL_H
#define I2C_HAL_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_Restart(void);
bool I2C_Write(uint8_t data);
uint8_t I2C_Read(bool ack);
void I2C_WaitIdle(void);

#endif