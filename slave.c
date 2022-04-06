#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <string.h>
#include <slave.h>

QueueHandle_t xQueue;
int available;
volatile uint8_t sendValue; 
volatile uint8_t receivedValue;

//data for registers
volatile uint8_t LED_REG = 0x00;
volatile uint8_t SW_REG = 0x03;
volatile uint8_t INT_REG = 0x00;

volatile uint8_t com_address = 0x01;

volatile uint8_t index_read = 0x80;
volatile int index_write = 0;
volatile int index_com = 0;

volatile uint8_t past_value = 0x00;

volatile bool begin_op = false;

volatile bool done_com = false;


uint32_t heartbeatDelay = 500;  // ms



void interupt(uint gpio, uint32_t events){
    uint32_t trigger = events & 0xFF;
    
    if(gpio == CS_PIN){

        if(trigger == 4){
            sendValue = CS_LOW;
            xQueueSendFromISR(xQueue, (void *) &sendValue, pdFALSE );
            //printf("CS_PIN low\n");            
        }

        else if(trigger == 8){
            sendValue = CS_HIGH;
            xQueueSendFromISR(xQueue, (void *) &sendValue, pdFALSE );
            //printf("CS_PIN high\n");           
        }
        
    }

    else if(gpio == CLK_PIN){
        if(trigger == 4){
            sendValue = CLK_LOW;
            xQueueSendFromISR(xQueue, (void *) &sendValue, pdFALSE );
            //printf("CLK_PIN low\n");
        }

        else if(trigger == 8){
            sendValue = CLK_HIGH;
            xQueueSendFromISR(xQueue, (void *) &sendValue, pdFALSE );
            //printf("CLK_PIN high\n");
        }
        
    }

    else if(gpio == SW1_PIN){
        if(trigger == 4){
            if(INT_REG & 0x3){
                gpio_put(INT_1, LOW);
                SW_REG = 0x05;
            }
        }

        else{
            gpio_put(INT_1, HIGH);
            SW_REG = 0x03;
        }
        
    }

    else if(gpio == SW2_PIN){
        if(trigger == 4){
            if(INT_REG & 0x3){
                gpio_put(INT_1, LOW);
                SW_REG = 0xa;
            }
        }

        else{
            gpio_put(INT_1, HIGH);
            SW_REG = 0x03;
        }
        
    }    
}

void hardware_init(void){
	gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(RED_PIN);
    gpio_set_dir(RED_PIN, GPIO_OUT);

    gpio_init(GREEN_PIN);
    gpio_set_dir(GREEN_PIN, GPIO_OUT);

    gpio_init(BLUE_PIN);
    gpio_set_dir(BLUE_PIN, GPIO_OUT);

    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_IN);
    //gpio_pull_up(CS_PIN);
    gpio_set_irq_enabled_with_callback(CS_PIN, GPIO_IRQ_EDGE_FALL,true, &interupt);
    gpio_set_irq_enabled_with_callback(CS_PIN, GPIO_IRQ_EDGE_RISE,true, &interupt);

    gpio_init(CLK_PIN);
    gpio_set_dir(CLK_PIN, GPIO_IN);
    gpio_pull_down(CLK_PIN);
    gpio_set_irq_enabled_with_callback(CLK_PIN, GPIO_IRQ_EDGE_FALL,true, &interupt);
    gpio_set_irq_enabled_with_callback(CLK_PIN, GPIO_IRQ_EDGE_RISE,true, &interupt);

    gpio_init(MOSI_PIN);
    gpio_set_dir(MOSI_PIN, GPIO_IN);
    //gpio_pull_up(MOSI_PIN);


    gpio_init(MISO_PIN);
    gpio_set_dir(MISO_PIN, GPIO_OUT);

    gpio_init(SW1_PIN);
    gpio_set_dir(SW1_PIN, GPIO_IN);
    gpio_pull_up(SW1_PIN);
    gpio_set_irq_enabled_with_callback(SW1_PIN, GPIO_IRQ_EDGE_FALL,true, &interupt);
    gpio_set_irq_enabled_with_callback(SW1_PIN, GPIO_IRQ_EDGE_RISE,true, &interupt);

    gpio_init(SW2_PIN);
    gpio_set_dir(SW2_PIN, GPIO_IN);
    gpio_pull_up(SW2_PIN);
    gpio_set_irq_enabled_with_callback(SW2_PIN, GPIO_IRQ_EDGE_FALL,true, &interupt);
    gpio_set_irq_enabled_with_callback(SW2_PIN, GPIO_IRQ_EDGE_RISE,true, &interupt);

    gpio_init(INT_1);
    gpio_set_dir(SW1_PIN, GPIO_OUT);

    gpio_init(INT_2);
    gpio_set_dir(SW2_PIN, GPIO_OUT);
}

void heartbeat(void * notUsed)
{   
    while (true) {
       	printf("slave-tick: %d\n", heartbeatDelay);	          
	    gpio_put(LED_PIN, 1);
        
        
	    vTaskDelay(heartbeatDelay);
	    gpio_put(LED_PIN, 0);

	    vTaskDelay(heartbeatDelay);
        
    }
}

void slave_transfer(void * notUsed){
    while(true){
    
        xQueueReceive(xQueue, &receivedValue, (TickType_t) 10);

        if(receivedValue == CS_LOW){
            begin_op = true;

        }

        else if(receivedValue == CS_HIGH){
            begin_op = false;
            
            gpio_put(RED_PIN, (((LED_REG & 0x02) == 0x02 )? HIGH : LOW));
            gpio_put(GREEN_PIN, (((LED_REG & 0x04) == 0x04) ? HIGH : LOW));
            gpio_put(BLUE_PIN, (((LED_REG & 0x01) == 0x01) ? HIGH : LOW));

            printf("LED_REG: %02x\n", LED_REG);
            
                       
        }

        else if((receivedValue == CLK_HIGH) && (past_value == CLK_LOW) && begin_op == true && done_com){
            
            //write command
            if(done_com && ((com_address & 0xb0) == 0xb0)){
                if((com_address & 0x01) == 0x01 ){
                    LED_REG <<= 1;
                    LED_REG += gpio_get(MOSI_PIN);

                    index_write += 1;
                    
                    if(index_write >= 8){
                        index_write = 0;
                        com_address = 0x01;
                        done_com = false;                   
                    }
                }

                else if((com_address & 0x02) == 0x02){
                    SW_REG <<= 1;
                    SW_REG += gpio_get(MOSI_PIN);

                    index_write += 1;
                    
                    if(index_write >= 8){
                        index_write = 0;
                        com_address = 0x01;
                        done_com = false;                   
                    }
                }

                else if((com_address & 0x03) == 0x03){
                    INT_REG <<= 1;
                    INT_REG += gpio_get(MOSI_PIN);

                    index_write += 1;
                    
                    if(index_write >= 8){
                        index_write = 0;
                        com_address = 0x01;
                        done_com = false;                   
                    }
                }
            }

            //read command
            else if(done_com && ((com_address & 0xa0) == 0xa0)){
                if((com_address & 0x01) == 0x01 ){
                    gpio_put(MISO_PIN, (LED_REG & index_read) ? HIGH : LOW);
                    index_read >>= 1;
                    if(index_read == 0x00){                   
                        index_read = 0x80;
                        com_address = 0x01;
                        done_com = false;
                    }
                }

                else if((com_address & 0x02) == 0x02){
                    gpio_put(MISO_PIN, (SW_REG & index_read) ? HIGH : LOW);
                    index_read >>= 1;
                    if(index_read >= 0x00){                   
                        index_read = 0x80;
                        com_address = 0x01;
                        done_com = false;
                    }
                }

                else if((com_address & 0x03) == 0x03){
                    gpio_put(MISO_PIN, (INT_REG & index_read) ? HIGH : LOW);
                    index_read >>= 1;
                    if(index_read >= 0x00){                   
                        index_read = 0x80;
                        com_address = 0x01;
                        done_com = false;
                    }
                }
                
            }

        }

        else if((receivedValue == CLK_HIGH) && (past_value == CLK_LOW) && begin_op == true && done_com == false){
            //get command and address
            com_address <<= 1;         
            com_address += (gpio_get(MOSI_PIN) ? 1:0);
            
            index_com+=1;

            if(index_com >= 7){                
                done_com = true;
                index_com = 0;                 
            }                      
        }

        
        past_value = receivedValue;

        printf("com_address: %02x\n", com_address);
        printf("done_com: %d\n", done_com);        
        // printf("SW_REG: %02x\n", SW_REG);
        // printf("INT_REG: %02x\n", INT_REG);
        // printf("past_value: %02x\n", past_value);            
                 
    }
    
}


int main()
{
    stdio_init_all();
    
    hardware_init();

    xQueue = xQueueCreate(10000, sizeof(uint8_t));
  
    

    available = uxQueueSpacesAvailable(xQueue); 
    xTaskCreate(slave_transfer, "Slave_Task", 256, NULL, 1, NULL);
    //xTaskCreate(slave_write, "write_Task", 256, NULL, 2, NULL);
    
    vTaskStartScheduler();

    while(1){};       
    
    
}