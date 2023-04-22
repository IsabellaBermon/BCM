#ifndef RX_TX_H
#define RX_TX_H
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>


struct rx_tx
{
    spi_inst_t *spi_port;
    uint16_t CNS;
    uint16_t CE;

};

void rx_tx_init(spi_inst_t *spi_port, uint16_t CNS, uint16_t CE);

void cnsLow();
void cnsHigh();
void ceLow();
void ceHigh();

uint8_t readReg(uint8_t reg);
void write(uint8_t reg, uint8_t data);
void write2(uint8_t reg, uint8_t *data, uint8_t size);

void config();
void modeTX();
void modeRX();
void sendMessage(char *msg);
void receiveMessage(char *msg);

uint8_t newMessage();
void setChannel(uint8_t ch);

void setRXName(char *name);
void setTXName(char *name);

#endif