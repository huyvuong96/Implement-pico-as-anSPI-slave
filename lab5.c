#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <string.h>
#include <master.h>


uint32_t heartbeatDelay = 500;  // ms

void hardware_init(void)
{
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(SW1_PIN);
    gpio_pull_up(SW1_PIN);
    gpio_set_dir(SW1_PIN, GPIO_IN);
    //gpio_set_irq_enabled_with_callback(SW1_PIN, GPIO_IRQ_EDGE_FALL,true, &gpio_int);
    
    gpio_init(SW2_PIN);
    gpio_pull_up(SW2_PIN);
    gpio_set_dir(SW2_PIN, GPIO_IN);
    
    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);

    gpio_init(CLK_PIN);
    gpio_set_dir(CLK_PIN, GPIO_OUT);

    gpio_init(MOSI_PIN);
    gpio_set_dir(MOSI_PIN, GPIO_OUT);

    gpio_init(MISO_PIN);
    gpio_set_dir(MISO_PIN, GPIO_IN);


    gpio_put(CS_PIN, HIGH);
    gpio_put(CLK_PIN, LOW);
}

void heartbeat(void * notUsed)
{   
    while (true) {
       	//printf("hb-tick: %d\n", heartbeatDelay);	          
	    gpio_put(LED_PIN, 1);
	    vTaskDelay(heartbeatDelay);
	    gpio_put(LED_PIN, 0);
	    vTaskDelay(heartbeatDelay);
        
    }
}

void gpioReadWrite(void* notUsed){
    const uint32_t queryDelay = 100;
    uint8_t regValue;
    uint8_t count = 0;

    while(true){
        SlaveWriteByte(0x01, LED_REG);
        vTaskDelay(heartbeatDelay);
        SlaveWriteByte(0x02, LED_REG);
        vTaskDelay(heartbeatDelay);
        SlaveWriteByte(0x04, LED_REG);
        vTaskDelay(heartbeatDelay);
        SlaveWriteByte(0x00, LED_REG);
        vTaskDelay(heartbeatDelay);

        // SlaveWriteByte(count++, LED_REG);
        // regValue = SlaveReadByte(LED_REG);
        // printf("LED_REG: %02x, ", regValue);

        // regValue = SlaveReadByte(INT_REG);
        // printf("INT_REG: %02x, ", regValue);

        // regValue = SlaveReadByte(SW_REG);
        // printf("SW_REG: %02x\n", regValue);

        // vTaskDelay(queryDelay);
    }
}

int main()
{
    stdio_init_all();
    
    hardware_init();
  
    xTaskCreate(heartbeat, "LED_Task", 256, NULL, 0, NULL);

    xTaskCreate(gpioReadWrite, "BYTE_Task", 256, NULL, 1, NULL);
    
    vTaskStartScheduler();

    while(1){};
    
}