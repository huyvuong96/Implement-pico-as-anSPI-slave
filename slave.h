#ifndef _SLAVE_466__H_INCLUDED_
#define _SLAVE_466__H_INCLUDED_
#include <stdint.h>

#define LOW (0)
#define HIGH (1)
#define FEATURE_BITBANG_SPI

#ifdef FEATURE_BITBANG_SPI
#define CS_PIN   13  
#define CLK_PIN  10  
#define MOSI_PIN 11  
#define MISO_PIN 12  
#define RED_PIN 20
#define GREEN_PIN 19
#define BLUE_PIN 18
#define LED_PIN  25
#define SW1_PIN  14
#define SW2_PIN  15
#define INT_1 27
#define INT_2 26
#define CS_HIGH 0x00
#define CS_LOW 0x01
#define CLK_HIGH 0x03
#define CLK_LOW 0x04
#endif

#endif // _SLAVE_466__H_INCLUDED_