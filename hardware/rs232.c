/**
 @file hardware/rs232.c

 @brief RS232 Driver AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#include "user_config.h"
#include "rs232.h"

///@brief Uart ring buffers
struct _uart uarts[UARTS];

/// @brief  Flush receive ring buffer for specified uart.
///
/// @return  void
void uart_rx_flush(uint8_t uart)
{
    if(uart >= UARTS)
        return;

    cli();

    uarts[uart].rx_count = 0;
    uarts[uart].rx_head = 0;
    uarts[uart].rx_tail = 0;
    uarts[uart].rx_flow = 0;
    uarts[uart].rx_error = 0;

    sei();
}


/// @brief  UART receive character function using avr-libc.
///
/// @param[in] f: unused stream pointer.
///
/// @return  uart_getchar(0);.
/// @see fdevopen() from avr-libc.
int uart0_getchar(FILE *f)
{
    return( uart_getchar(0) );
}


/// @brief  UART transmit character function using avr-libc.
///
/// @param[in] c: character to send.
/// @param[in] f: unused stream pointer.
///
/// @return  uart_putchar(c, 0);.
/// @see fdevopen() from avr-libc.
int uart0_putchar(int c, FILE *f)
{
    uart_putchar(c, 0);
	return(c);
}


#if UARTS > 1
/// @brief  UART receive character function using avr-libc.
///
/// @param[in] f: unused stream pointer.
///
/// @return  uart_getchar(1);.
/// @see fdevopen() from avr-libc.
int uart1_getchar(FILE *f)
{
    return( uart_getchar(1) );
}


/// @brief  UART transmit character function using avr-libc.
///
/// @param[in] c: character to send.
/// @param[in] f: unused stream pointer.
///
/// @return  uart_putchar(c, 1);.
/// @see fdevopen() from avr-libc.
int uart1_putchar(int c, FILE *f)
{
    uart_putchar(c, 1);
	return(c);
}
#endif

/// @brief  UART initialization function that works with avr-libc functions.
///
/// @param[in] uart: UART number.
/// @param[in] baud: Baud Rate in HZ.
///
/// @return  0 on success.
/// @return  EOF on fail.
/// @see fdevopen() avr-libc function.
int uart_init(uint8_t uart, uint32_t baud)
{
    uint16_t uart_select_baud = (uint16_t) ( F_CPU/16L/baud );

    if(uart >= UARTS)
        return(EOF);

    if(uart == 0)                                 /* uart == 0( first serial ) */
    {
        uart_rx_flush(0);

        cli();

        GPIO_PIN_LATCH_HI(RX0);
        GPIO_PIN_LATCH_HI(TX0);
        GPIO_PIN_DIR_IN(RX0);
        GPIO_PIN_DIR_OUT(TX0);

        UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);

        UBRR0H = (uint8_t) 0xff & (uart_select_baud >> 8);
        UBRR0L = (uint8_t) 0xff & (uart_select_baud);

        sei();

        fdevopen((void *)uart0_putchar, (void *)uart0_getchar);
    }
#if UARTS > 1
    if(uart == 1)                                 /* uart == 0( first serial ) */
    {
        uart_rx_flush(1);

        cli();

        GPIO_PIN_LATCH_HI(RX1);
        GPIO_PIN_LATCH_HI(TX1);
        GPIO_PIN_DIR_IN(RX1);
        GPIO_PIN_DIR_OUT(TX1);

        UCSR1B = (1<<RXCIE1)|(1<<RXEN1)|(1<<TXEN1);

        UBRR1H = (uint8_t) 0xff & (uart_select_baud >> 8);
        UBRR1L = (uint8_t) 0xff & (uart_select_baud);

        sei();

        fdevopen((void *)uart1_putchar, (void *)uart1_getchar);
    }
#endif
    return(0);
}


/// @brief  UART 0 Receive Interrupt runction
/// @see uart_rx_interrupt().
ISR(USART0_RX_vect)
{
    uart_rx_interrupt(0, UDR0);
}


#if UARTS > 1
/// @brief  UART 1 Receive Interrupt runction.
/// @see uart_rx_interrupt().
ISR(USART1_RX_vect)
{
    uart_rx_interrupt(1, UDR1);
}
#endif

/// @brief  UART Receive Interrupt task.
///  - Add characters to ring receive buffer.
///
/// @param[in] uart: uart number.
/// @param[in] data: register data.
///
/// @return void.
void uart_rx_interrupt(uint8_t uart, uint8_t data)
{
    if (uarts[uart].rx_count < RX_BUF_SIZE )
    {
        uarts[uart].rx_buf[uarts[uart].rx_head++] = data;
        uarts[uart].rx_count++;
    }
    else                                          // Overflow
    {
        uarts[uart].rx_error |= RX_OVERFLOW;
    }

    if (uarts[uart].rx_head >= RX_BUF_SIZE )
    {
        uarts[uart].rx_head = 0;
    }
}


/// @brief Return character waiting in ring buffer without removing it.
/// @return character.
/// @return 0 on error.
int uart_peek_tail(uint8_t uart)
{
    uint8_t c;

    if(uart >= UARTS)
        return(EOF);

    cli();
    c = uarts[uart].rx_buf[uarts[uart].rx_tail];
    sei();
    return (c & 0xff);
}


/// @brief Return next character from ring buffer.
///  - Wait for at least one character.
///  - Update index pointers and character count.
///
/// @param[in] uart: uart number to read.
/// @return character.
/// @return 0 on error.
/// @see uart_rx_count().
int uart_get_tail(uint8_t uart)
{
    uint8_t c;
    if(uart >= UARTS)
        return(EOF);

    while ( uart_rx_count(uart) == 0 )
        ;
    cli();

    c = uarts[uart].rx_buf[uarts[uart].rx_tail++];
    if (uarts[uart].rx_tail > RX_BUF_SIZE)
        uarts[uart].rx_tail = 0;
    uarts[uart].rx_count--;

    sei();
    return (c & 0xff);
}


/// @brief  return count of character count waiting in UART ring buffer.
///
/// @param[in] uart: uart number to check count for.
///
/// @return  Character count in ring buffer.
int uart_rx_count(uint8_t uart)
{
    int count;

    if(uart >= UARTS)
        return(EOF);

    cli();

    count = uarts[uart].rx_count;

    sei();

    return (count );
}


/// @brief  Recive character character from uart.
///
/// @param[in] uart: uart number.
///
/// @return  Character.
int uart_rx_byte(uint8_t uart)
{
    return( uart_get_tail(uart) & 0xff);
}


/// @brief  Receive character from uart with option CR/LF conversion.
///
/// - CR/LF processing is disabled at the moment.
///
/// @param[in] uart: uart number.
///
/// @return  Character.
int uart_getchar(uint8_t uart)
{
    uint8_t c;

    if(uart >= UARTS)
        return(EOF);
#if 0
    while(1)
    {
        c = uart_rx_byte(uart);
        if(c == '\n')
            continue;
        break;
    }
    if(c == '\r')
        c = '\n';
#endif
    c = uart_rx_byte(uart);
    return (c);
}


/// @brief Transmit 1 byte on uart.
///
/// @param[in] c: transmit character.
/// @param[in] uart: uart number.
///
/// @return void.
int uart_tx_byte(int c, uint8_t uart)
{
    if(uart == 0)
    {
        while (!BIT_TST(UCSR0A, UDRE0))
            ;
        UDR0 = c & 0x7f;
		return(c);
    }
#ifdef UARTS > 1
    if(uart == 1)
    {
        while (!BIT_TST(UCSR1A, UDRE1))
            ;
        UDR1 = c & 0x7f;
		return(c);
    }
#endif
	return(EOF);
}


/// @brief Transmit 1 byte on uart.
///
///  - CR/LF Output translation.
/// @param[in] c: transmit character.
/// @param[in] uart: uart number.
///
/// @return void.
int uart_putchar(int c, int uart)
{
    uart_tx_byte(c, uart);
    if( c == '\n')
        uart_tx_byte('\r', uart);
	return(c);
}


/// @brief Do we have receive characters waiting ?.
///
/// - Returns non zero if there are any caracters.
///
/// @param[in] uart: uart number.
///
/// @return  Character count in receive buffer.
int uart_keyhit(uint8_t uart)
{
    return ( uart_rx_count( uart ) );
}


/// @brief  Transmit a character on UART 0
/// @param[in] c: character to write
/// @return  void
int uart_put(int c)
{
    return( uart0_putchar(c,0) );
}

/// @brief  Receive a character from UART 0
/// @return  character
int uart_get(void)
{
    return(uart0_getchar(0));
}


/// @brief  Get a line from UART 0 up to a maximum of len bytes
/// @param[in] buff: line input buffer
/// @param[in] len: line length maximum
/// @return void
int get_line (char *buff, int len)
{
    int c;
    int i = 0;

	memset(buff,0,len);
    while(1)
    {
        c = uart_get() & 0x7f;
		uart_put(c);
        if (c == '\n' || c == '\r')
		{
            break;
		}

        if (c == '\b')
        {
			if(i > 0) {
				i--;
			}
            continue;
        }

        if (i < (len - 1) )                       /* Visible chars */
        {
            buff[i++] = c;
        }
		else
		{
			break;
		}
    }

	// Discard remaining characters
    while(uart_keyhit(0))
    {
        c = uart_get() & 0x7f;
    }
    buff[i] = 0;
    uart_put('\n');
	return(i);
}

