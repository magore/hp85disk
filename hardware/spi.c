/**
 @file hardware/spi.c

 @brief SPI Driver AVR8

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @copyright Copyright &copy; 2014-2020 Mike Gore, Inc. All rights reserved.

 @copyright GNU Public License.

 @author Mike Gore

*/

#include "user_config.h"
#include <stdlib.h>
#include "spi.h"


///@brief Saved SPI bus speed
static uint32_t SPI0_Speed_value = 0;

/// @brief Set AVR SPI bus rate in HZ.
///
/// - Compute precsale rate from desired frequency in HZ.
///  - Slowest rate: F_CPU/128.
///  - Fastest rate: F_CPU/2.
/// - Precsale value is truncated not rounded.
/// - Valid precsle values are 2,4,8,16,32,64,128.
/// @verbatim
/// SPI2X   SPR1    SPR0     SCK Frequency
/// 1       0       0        fosc/2
/// 0       0       0        fosc/4
/// 1       0       1        fosc/8
/// 0       0       1        fosc/16
/// 1       1       0        fosc/32
/// 0       1       0        fosc/64
/// 1       1       1        fosc/64
/// 0       1       1        fosc/128
/// @endverbatim
///
/// @param[in] speed: SPI clock rate in HZ.
///
/// @return  void
void SPI0_Speed(uint32_t speed)
{
    uint32_t rate;

// Computer Prescale rate - truncate.

#if 1
    // We only make changes if the speed actually changes
    if(speed == SPI0_Speed_value)
        return;
#endif

    rate = ((uint32_t) F_CPU) / speed;

    if(rate >= 128)
    {
		// 128
		BIT_CLR(SPSR, SPI2X);  
		BIT_SET(SPCR, SPR1);
		BIT_SET(SPCR, SPR0);
	}
    else if(rate >= 64)
    {
        // 64
		BIT_CLR(SPSR, SPI2X);  
		BIT_SET(SPCR, SPR1);
		BIT_SET(SPCR, SPR0);
    }
    else if(rate >= 32)
    {
        // 32
		BIT_SET(SPSR, SPI2X);  
		BIT_SET(SPCR, SPR1);
		BIT_CLR(SPCR, SPR0);
    }
    else if(rate >= 16)
    {
        // 16
		BIT_CLR(SPSR, SPI2X);
		BIT_CLR(SPCR, SPR1);
		BIT_SET(SPCR, SPR0);
    }
    else if(rate >= 8)
    {
        // 8;
		BIT_SET(SPSR, SPI2X);
		BIT_CLR(SPCR, SPR1);
		BIT_SET(SPCR, SPR0);
    }
    else if(rate >= 4)
    {
        // 4;
		BIT_CLR(SPSR, SPI2X);
		BIT_CLR(SPCR, SPR1);
		BIT_CLR(SPCR, SPR0);
    }
    else if(rate >= 2)
    {
        // 2;
		BIT_SET(SPSR, SPI2X);
		BIT_CLR(SPCR, SPR1);
		BIT_CLR(SPCR, SPR0);
    }
	else {
		// 2 fastest rate;
		BIT_SET(SPSR, SPI2X);
		BIT_CLR(SPCR, SPR1);
		BIT_CLR(SPCR, SPR0);
	}
	/// Save speed value
    SPI0_Speed_value = speed;
}


/// @brief  Return previously saved SPI BUS rate in HZ.
///
/// @return  SPI clock rate in HZ.
uint32_t SPI0_Get_Speed( void )
{
    return(SPI0_Speed_value);
}


///@brief Saved SPI Mode
///@see SPI0_Mode().
static int SPI0_Mode_value = 0;

/// @brief Set SPI clock mode.
///
/// @param[in] mode: valid mades:
///@verbatim
///  SPI Mode     CPOL    CPHA            Sample
///  0    0       0       Leading (Rising)   Edge
///  1    0       1       Trailing (Falling) Edge
///  2    1       0       Leading (Falling)  Edge
///  3    1       1       Trailing (Rising)  Edge
///@endverbatim
///
/// @return  void
void SPI0_Mode(int mode)
{
    switch(mode)
    {
        case 0:
            BIT_CLR(SPCR,CPOL);
            BIT_CLR(SPCR,CPHA);
            break;
        case 1:
            BIT_CLR(SPCR,CPOL);
            BIT_SET(SPCR,CPHA);
            break;
        case 2:
            BIT_SET(SPCR,CPOL);
            BIT_CLR(SPCR,CPHA);
            break;
        case 3:
            BIT_SET(SPCR,CPOL);
            BIT_SET(SPCR,CPHA);
            break;
        default:
            printf("SPI0_Mode: Invalid mode(%d)\n",mode);
            break;
    }
    SPI0_Mode_value = mode;
}


/// @brief  Return saved SPI mode as set by SPI0_Mode().
/// @return  saved mode.
int SPI0_Get_Mode( void )
{
    return(SPI0_Mode_value);
}


///@brief SPI0 initialization flag.
static int SPI0_Init_state = 0;

///@brief Initialize SPI0 device.
/// See Atmel App Note AVR151
/// Set default speed, IO pins and mode.
void SPI0_Init(uint32_t speed)
{

	volatile uint8_t D  __attribute__((unused));

#ifdef SPI_DEBUG
	printf("SPI0_Init speed:%ld\n",speed);
#endif


#ifdef SPI_DEBUG
	printf("SS HI\n");
	printf("Port B DDR:   0x%02x\n", (int) GPIO_PORT_DDR_RD(GPIO_B));
	printf("Port B LATCH: 0x%02x\n", (int) GPIO_PORT_LATCH_RD(GPIO_B));
	printf("Port B PINS:  0x%02x\n", (int) GPIO_PORT_LATCH_RD(GPIO_B));
#endif

	SPCR = 0;				// Clear SPCR in case we are not called after RESET

    GPIO_PIN_HI(SCK);       // SCK Output
    GPIO_PIN_HI(MOSI);                                  // MOSI Output
    GPIO_PIN_FLOAT(MISO);   // MISO Input, no pull-up

	// Warning *** MSTR MUST be set BEFORE SPE!!!! *** otherwise SS will NOT behave as an output
	GPIO_PIN_LOW(SS); 		// SS Output must be HI prevent slave mode from getting set while initializing
    BIT_SET(SPCR, MSTR);    // Master Mode
    BIT_SET(SPCR, SPE);     // Enable SPI

	// SPI Clear any pending interrupt flags
	D = SPSR;
	D = SPDR;

	/// Set SPI clock mode 0 
    ///  SPI Mode     CPOL    CPHA            Sample
    ///  0    0       0       Leading (Rising)   Edge
    SPI0_Mode(0);

	// Set SPI clock speed
    SPI0_Speed(speed);

    SPI0_TXRX_Byte(0xff);	// Send dummy 0xFF
	SPI0_Init_state = 1;

}


/// @brief SPI read/Write byte.
///
/// - We Write and read at the same time.
/// - If we want to read only set Data to 0xff.
///
/// @param[in] Data: Data to send.
///
/// @return  Data received.
uint8_t SPI0_TXRX_Byte(uint8_t Data)
{
    SPDR = Data;                                  // Transmit

    while( !BIT_TST(SPSR,SPIF) )                  // Wait for send
        ;

    return (SPDR);                                // Received Data
}


// =============================================
/// @brief
/// SPI buffered write functions

/// @brief HSPI write using FIFO
/// @param[in] *data: transmit buffer
/// @param[in] count: number of bytes to write
/// @return  void
void SPI0_TX(uint8_t *data, int count)
{

    while(count > 0)
    {
		SPI0_TXRX_Byte(*data);
		++data;
		--count;
	}
}

/// @brief HSPI write and read using FIFO
/// @param[in] *data: transmit / receive buffer
/// @param[in] count: number of bytes to write / read
/// @return  void

void SPI0_TXRX(uint8_t *data, int count)
{

    while(count > 0)
    {
		*data = SPI0_TXRX_Byte(*data);
		++data;
		--count;
	}
}

/// @brief HSPI read using FIFO
/// @param[in] *data: receive buffer
/// @param[in] count: number of bytes to read
/// @return  void
void SPI0_RX(uint8_t *data, int count)
{

    while(count > 0)
    {
		*data = SPI0_TXRX_Byte(0xff);
		++data;
		--count;
	}
}


