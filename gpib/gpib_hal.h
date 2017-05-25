/**
 @file gpib/gpib_hal.h

 @brief GPIB emulator hardwware layer for HP85 disk emulator project for AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/


#ifndef _GPIB_HAL_H_
#define _GPIB_HAL_H_

#include "user_config.h"

/* 2 Microseconds */
#define GPIB_BUS_SETTLE() _delay_us(GPIB_BUS_SETTLE_DELAY)

#define GPIB_TASK_TIC_US SYSTEM_TASK_TIC_US       /* Interrupt time in US */

#define SYSTEM_ELAPSED_TIMER                      /* We have a system elapsed time function */

typedef struct _gpib_t
{
    volatile uint32_t elapsed;
    volatile uint32_t down_counter;
    volatile uint8_t  down_counter_done;
} gpib_t;

extern gpib_t gpib_timer;
void gpib_clock_task( void );


///  Notes:
///   "EOI" gets convereted into the required DDR and BIT definitions.
///   We use the comma operator to return the PIN test.
///   We do not use {} around the statements so they behave like functions.
///   Consider what would happen if you used breaces (should be obviious)
///      if ( GPIB_IO_RD(EOI) )
///      {
///       printf"EOI");
///      }
///      else
///      {
///  do stuff
///      }



///  Notes about AVR and PIC differences
///     Your CPU mave have other differences
///  AVR                             PIC
///  DDR 1=out, 0 =in                TRIS 0=out,1=in
///  PORT=val same as LATCH=val      PORT=val same as LATCH=val
///  val=PORT, reads LATCH           val=PORT reads PIN state
///  val=PIN,  reads PIN state       val=LATCH reads latch


///FIXME be AVR specific!!!
#define EOI		GPIO_B0
#define DAV 	GPIO_B1
#define NRFD 	GPIO_D2
#define NDAC 	GPIO_D3
#define IFC  	GPIO_D4
#define SRQ 	GPIO_D5
#define ATN 	GPIO_D6
#define REN 	GPIO_D7

#ifndef SOFTWARE_PP
#warning Hardware PP
#define PPE 	GPIO_B2
#endif

#define GPIB_IO_RD(a) 		GPIO_PIN_RD(a)
#define GPIB_PIN_FLOAT(a) 	GPIO_PIN_FLOAT(a)
#define GPIB_IO_LOW(a) 		GPIO_PIN_LOW(a)
#define GPIB_IO_HI(a)  		GPIO_PIN_HI(a)

#define GPIB_BUS_OUT()      GPIO_PORT_DIR_OUT(GPIO_A)
#define GPIB_BUS_IN()       GPIO_PORT_DIR_IN(GPIO_A)
#define GPIB_BUS_RD()		GPIO_PORT_RD(GPIO_A)
#define GPIB_BUS_WR(val)	GPIO_PORT_WR(GPIO_A,val)

#define GPIB_PPR_RD()     	GPIO_PORT_PINS_RD(GPIO_A)
#define GPIB_PPR_DDR_RD()	GPIO_PORT_DDR_RD(GPIO_A)

// OLD system
#if 0
#define EOI_PORT    PORTB
#define EOI_DDR     DDRB
#define EOI_PIN     PINB
#define EOI_BIT     0

#define DAV_PORT    PORTB
#define DAV_DDR     DDRB
#define DAV_PIN     PINB
#define DAV_BIT     1

#define NRFD_PORT   PORTD
#define NRFD_DDR    DDRD
#define NRFD_PIN    PIND
#define NRFD_BIT    2

#define NDAC_PORT   PORTD
#define NDAC_DDR    DDRD
#define NDAC_PIN    PIND
#define NDAC_BIT    3

#define IFC_PORT    PORTD
#define IFC_DDR     DDRD
#define IFC_PIN     PIND
#define IFC_BIT     4

#define SRQ_PORT    PORTD
#define SRQ_DDR     DDRD
#define SRQ_PIN     PIND
#define SRQ_BIT     5

#define ATN_PORT    PORTD
#define ATN_DDR     DDRD
#define ATN_PIN     PIND
#define ATN_BIT     6

#define REN_PORT    PORTD
#define REN_DDR     DDRD
#define REN_PIN     PIND
#define REN_BIT     7

#ifndef SOFTWARE_PP
#warning Hardware PP
#define PPE_DDR   DDRB
#define PPE_PORT  PORTB
#define PPE_PIN   PINB
#define PPE_BIT   2
#endif

#endif  // #if 0


#ifndef GPIB_BUS_RD
#error GPIB_BUS_RD read macro is not defined
#endif

#ifndef GPIB_BUS_WR
#error GPIB_BUS_WR write macro is not defined
#endif

#ifndef GPIB_BUS_IN
#error GPIB_BUS_IN read macro is not defined
#endif

#ifndef GPIB_BUS_OUT
#error GPIB_BUS_OUT write macro is not defined
#endif

#ifndef GPIB_IO_LOW
#error GPIB_IO_LOW is not defined
#endif
#ifndef GPIB_IO_HI
#error GPIB_IO_HI is not defined
#endif
#ifndef GPIB_PIN_FLOAT
#error GPIB_PIN_FLOAT is not defined
#endif
#ifndef GPIB_IO_RD
#error GPIB_IO_RD is not defined
#endif

/* gpib_hal.c */
void gpib_timer_init ( void );
uint8_t reverse_8bits ( uint8_t mask );
void ppr_set ( uint8_t mask );
void soft_ppr_assert ( void );
void soft_ppr_restore ( void );
uint8_t ppr_reg ( void );
void ppr_init ( void );
void ppr_bit_set ( uint8_t bit );
void ppr_bit_clr ( uint8_t bit );
FRESULT dbf_open ( FIL *fp , const TCHAR *path , BYTE mode );
FRESULT dbf_read ( FIL *fp , void *buff , UINT btr , UINT *br );
FRESULT dbf_write ( FIL *fp , const void *buff , UINT btw , UINT *bw );
FRESULT dbf_lseek ( FIL *fp , DWORD ofs );
FRESULT dbf_close ( FIL *fp );
int dbf_open_read ( char *name , uint32_t pos , void *buff , int size , int *errors );
int dbf_open_write ( char *name , uint32_t pos , void *buff , int size , int *errors );


#endif                                            // _GPIB_HAL_H_
