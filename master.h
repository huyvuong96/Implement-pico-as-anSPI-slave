#ifndef _MASTER_466__H_INCLUDED_
#define _MASTER_466__H_INCLUDED_
#include <stdint.h>

#define LOW (0)
#define HIGH (1)
#define FEATURE_BITBANG_SPI

#ifdef FEATURE_BITBANG_SPI
#define CS_PIN   18  
#define CLK_PIN  21  
#define MOSI_PIN 20  
#define MISO_PIN 19  
#define SW1_PIN  14
#define SW2_PIN  15
#define INT_1 6
#define INT_2 7
#define LED_PIN  25
#define READ_CMD 0xA0
#define WRITE_CMD 0xB0
#define LED_REG 0x01
#define SW_REG 0x02
#define INT_REG 0x03
#endif

void    Start(void);
uint8_t SlaveTransfer( uint8_t outData);
uint8_t SlaveReadByte(uint8_t address);
void    SlaveWriteByte(uint8_t byte, uint8_t address);


#endif // _MASTER_466__H_INCLUDED_