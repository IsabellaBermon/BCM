#include "rx_tx.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include "pico/time.h"
#include <string.h>

#define SCK_pin 10
#define MOSI_pin 11
#define MISO_pin 12

/*struct rx_tx
{
    spi_inst_t *spi_port;
    uint16_t CNS;
    uint16_t CE;

};*/

struct rx_tx r1;

void rx_tx_init(spi_inst_t *spi_port, uint16_t CNS, uint16_t CE){

    spi_init(spi_port, 1000000 );
    gpio_set_function(SCK_pin, GPIO_FUNC_SPI);
    gpio_set_function(MOSI_pin, GPIO_FUNC_SPI);
    gpio_set_function(MISO_pin, GPIO_FUNC_SPI);

    gpio_init(CNS);
    gpio_init(CE);

    gpio_set_dir(CNS, 1);
    gpio_set_dir(CE, 1);

    ceLow();
    cnsHigh();
}

void cnsLow(){
    gpio_put(r1.CNS, 0);
}
void cnsHigh(){
    gpio_put(r1.CNS, 1);
}
void ceLow(){
    gpio_put(r1.CE, 0);
}
void ceHigh(){
    gpio_put(r1.CE, 1);
}

uint8_t readReg(uint8_t reg){
    uint8_t result = 0;
    reg = (0x1F & reg);
    cnsLow();
    spi_write_blocking(r1.spi_port, &reg, 1);
    spi_read_blocking(r1.spi_port, 0xff,&result, 1);
    cnsHigh();

    return result;
}

void write(uint8_t reg, uint8_t data){
    write2(reg, &data, 1);
}

void write2(uint8_t reg, uint8_t *data, uint8_t size){
    reg = 0x20 | (0x1F & reg);
    cnsLow();
    spi_write_blocking(r1.spi_port, &reg, 1);
    spi_write_blocking(r1.spi_port, (uint8_t*)data, size);
    cnsHigh();
}

void config(){
    cnsHigh();
    ceLow();
    sleep_ms(11);
    write(0, 0x0A); // config.
    sleep_us(1500);

    write(1,0); // no ack.

    write(5, 60); // channel.

    write2(0x0a, (uint8_t*)"gyroc",5);
    write2(0x10, (uint8_t*)"gyroc",5);

    write(0x11, 32);
}

void modeTX(){
    uint8_t reg = readReg(0);
    reg &= ~(1<<0);
    write(0, reg);
    
    ceLow(); // Correction from video, thansk schuhmann.
    
    sleep_us(130);
}

void modeRX(){
    uint8_t reg = readReg(0);
    reg |= (1<<0);
    write(0, reg);
    
    ceHigh(); // Correction from video, thanks schuhmann.
    
    sleep_us(130);
}

void sendMessage(char *msg){
    uint8_t reg = 0xA0;
    cnsLow();
    spi_write_blocking(r1.spi_port, &reg,1);
    spi_write_blocking(r1.spi_port, (uint8_t*)msg, 32);
    cnsHigh();
    ceHigh();
    sleep_us(10);
    ceLow();
}

void receiveMessage(char *msg){
    uint8_t cmd = 0x61;   
    cnsLow();
    spi_write_blocking(r1.spi_port, &cmd, 1);

    spi_read_blocking(r1.spi_port,0xff, (uint8_t*)msg,32);
    cnsHigh();
}

uint8_t newMessage(){
    uint8_t fifo_status = readReg(0x17);

    return !(0x00000001 & fifo_status);
}

void setChannel(uint8_t ch){
    write(5, ch);
}

void setRXName(char *name){
    if( strlen(name) != 5) return;
    write2(0x0a, (uint8_t*)name,5);
}

void setTXName(char *name){
    if( strlen(name) != 5) return;
    write2(0x10, (uint8_t*)name,5);
}
