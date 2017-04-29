/**
 @file hardware/rs232.h

 @brief RS232 Driver AVR8

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

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
uint8_t uart0_getchar ( FILE *f );
void uart0_putchar ( char c , FILE *f );
uint8_t uart1_getchar ( FILE *f );
void uart1_putchar ( char c , FILE *f );
int uart_init ( uint8_t uart , uint32_t baud );
void uart_rx_interrupt ( uint8_t uart , uint8_t data );
uint8_t uart_peek_tail ( uint8_t uart );
uint8_t uart_get_tail ( uint8_t uart );
uint8_t uart_rx_count ( uint8_t uart );
int uart_rx_byte ( uint8_t uart );
char uart_getchar ( uint8_t uart );
void uart_tx_byte ( char c , uint8_t uart );
void uart_putchar ( char c , int uart );
uint8_t uart_keyhit ( uint8_t uart );
void uart_put ( uint8_t c );
char uart_get ( void );
int get_line ( char *buff , int len );

#endif                                            // _RS232_H_
