/**
 @file hardware/rs232.h

 @brief RS232 Driver AVR8

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved.

*/

#ifndef _RS232_H_
#define _RS232_H_

#include "user_config.h"

#define RX_BUF_SIZE 80
#define RX_FLOW_WINDOW 20

#define RX_OVERFLOW 1

#define XON_CHAR    0x11
#define XOFF_CHAR   0x13

#define XOFF_ENABLE 4
#define XOFF_PUT    2
#define XOFF_GET    1

#ifndef EOF
#define EOF (-1)
#endif

struct _uart
{
    uint8_t rx_head;
    uint8_t rx_tail;
    uint8_t rx_flow;
    int     rx_count;
    uint8_t rx_error;
    uint8_t rx_buf[RX_BUF_SIZE+1];
};

#define UARTS 1
#define kbhit( uart) uart_rx_count( uart )

#define uart_putc(a,c) uart0_putchar(c,NULL)

/* rs232.c */
void uart_rx_flush ( uint8_t uart );
int uart0_getchar ( void *f );
int uart0_putchar ( int c , void *f );
uint16_t uart_ubr ( uint32_t baud , int *u2x , uint32_t *actual );
uint32_t uart_init ( uint8_t uart , uint32_t baud );
void uart_rx_interrupt ( uint8_t uart , uint8_t data );
int uart_peek_tail ( uint8_t uart );
int uart_get_tail ( uint8_t uart );
int uart_rx_count ( uint8_t uart );
int uart_rx_byte ( uint8_t uart );
int uart_getchar ( uint8_t uart );
int uart_tx_byte ( int c , uint8_t uart );
int uart_putchar ( int c , int uart );
int uart_keyhit ( uint8_t uart );
int uart_put ( int c );
int uart_get ( void );
int get_line ( char *buff , int len );

#endif                                            // _RS232_H_
