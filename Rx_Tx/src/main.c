#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "RX_TX.h"
#include <string.h>

int main(){
/*
    rx_tx_init(spi1, 9, 8);
    
    config();
    modeRX();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_init(15);
    gpio_init(16);

    gpio_set_dir(PICO_DEFAULT_LED_PIN,GPIO_OUT);
    gpio_set_dir(15,0);
    gpio_set_dir(16,0);

    char bufferOut[32] = {0};
    char bufferIn[32] = {0};

    while(1){

        if(!gpio_get(15)){
        modeTX();
        printf(bufferOut, "ON");
        sendMessage(bufferOut);
        modeRX();
       }
        else if(!gpio_get(16)){
            modeTX();
            printf(bufferOut, "OFF");
            sendMessage(bufferOut);
            modeRX();
        }

        if(newMessage() == 1){
            receiveMessage(bufferIn);
            if(strcmp(bufferIn, "ON") == 0){
                gpio_put(PICO_DEFAULT_LED_PIN,1);
            }
            else if(strcmp(bufferIn, "OFF") == 0){
                gpio_put(PICO_DEFAULT_LED_PIN,0);
            }
        }
    }

    return 0;*/

#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    rx_tx_init(spi1, 9, 8);
    
    config();
    modeRX();

    gpio_init(LED_PIN);
    gpio_init(15);
    gpio_init(16);

    gpio_set_dir(LED_PIN,GPIO_OUT);
    gpio_set_dir(15,0);
    gpio_set_dir(16,0);

    char bufferOut[32] = {0};
    char bufferIn[32] = {0};

    while(1){
 
        if(!gpio_get(15)){
            modeTX();
            printf(bufferOut, "ON");
            sendMessage(bufferOut);
            modeRX();
       }
        else if(!gpio_get(16)){
            modeTX();
            printf(bufferOut, "OFF");
            sendMessage(bufferOut);
            modeRX();
        }

        if(newMessage() == 1){
            receiveMessage(bufferIn);
            if(strcmp(bufferIn, "ON") == 0){
                gpio_put(LED_PIN,1);
            }
            else if(strcmp(bufferIn, "OFF") == 0){
                gpio_put(LED_PIN,0);
            }
        }
    }
#endif
}