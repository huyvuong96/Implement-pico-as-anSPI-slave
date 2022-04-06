#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <string.h>
#include "master.h"
#include "stdio.h"

void    Start(void){
	gpio_put(CS_PIN,LOW);
}
void    Complete(void){
	gpio_put(CS_PIN, HIGH);
}

uint8_t SlaveTransfer( uint8_t outData){
	uint8_t count, in = 0;
	
	gpio_put(CLK_PIN,LOW);

	vTaskDelay(10);

	for(count = 0; count <8; count++){
		in <<= 1;		
		int pull = (outData & 0x80);
		gpio_put(MOSI_PIN, pull);
		
		vTaskDelay(1);	
		gpio_put(CLK_PIN, HIGH);

		vTaskDelay(1);
		
		in += gpio_get(MISO_PIN);

		vTaskDelay(1);

		gpio_put(CLK_PIN, LOW);
		
		outData <<= 1;
	}

	vTaskDelay(1);
	gpio_put(MOSI_PIN, LOW);

	return in;
}

uint8_t SlaveReadByte(uint8_t address){
	uint8_t value; 
	uint8_t preRead = READ_CMD + address;

	Start();
	SlaveTransfer(preRead);
	value = SlaveTransfer(0);
	Complete();

	return value;
}

void    SlaveWriteByte(uint8_t byte, uint8_t address){
	uint8_t preWrite = WRITE_CMD + address;

	Start();
	vTaskDelay(1);
	SlaveTransfer(preWrite);
	vTaskDelay(1);	
	SlaveTransfer(byte);
	vTaskDelay(1);
	Complete();
}