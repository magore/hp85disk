/**
 @file hardware/spi.h

 @brief SPI Driver AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#ifndef _SPI_H
#define _SPI_H

#include "user_config.h"

#define SPI0_MODE0  0
#define SPI0_MODE1  1
#define SPI0_MODE2  2
#define SPI0_MODE3  3


/* spi.c */
void SPI0_cs_enable ( uint8_t cs );
void SPI0_cs_disable ( uint8_t cs );
void SPI0_Speed ( uint32_t speed );
uint32_t SPI0_Get_Speed ( void );
void SPI0_Mode ( int mode );
int SPI0_Get_Mode ( void );
void SPI0_Init ( uint32_t speed );
uint8_t SPI0_TXRX_Byte ( uint8_t Data );
void SPI0_TX ( uint8_t *data , int count );
void SPI0_TXRX ( uint8_t *data , int count );
void SPI0_RX ( uint8_t *data , int count );

#endif
