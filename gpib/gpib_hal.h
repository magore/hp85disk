/**
 @file gpib/gpib_hal.h

 @brief GPIB emulator hardwware layer for HP85 disk emulator project for AVR.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, All rights reserved. GPL
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for Copyright details

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


// control pins are the same on V1 and V2 hardware
#define EOI     GPIO_B0
#define DAV     GPIO_B1
#define NRFD    GPIO_D2
#define NDAC    GPIO_D3
#define IFC     GPIO_D4
#define SRQ     GPIO_D5
#define ATN     GPIO_D6
#define REN     GPIO_D7

// V2 Hardware
#define TE      GPIO_C2
#define PE      GPIO_C3
#define DC      GPIO_C4
#define SC      GPIO_C5

// No loger being used
#ifndef SOFTWARE_PP
#warning Hardware PP
#define PPE     GPIO_B2
#endif


///@brief changes pin mode to read, then read
#define GPIB_IO_RD(a)       GPIO_PIN_RD(a)

///@brief changes pin mode to read
#define GPIB_PIN_FLOAT(a)   GPIO_PIN_FLOAT(a)

///@brief changes pin mode to write then set low
#define GPIB_IO_LOW(a)      GPIO_PIN_LOW(a)

///@brief changes pin mode to write then set hi
#define GPIB_IO_HI(a)       GPIO_PIN_HI(a)

///@brief checks the pin state without changing read/write mode
#define GPIB_PIN_TST(a)     GPIO_PIN_TST(a)

///@brief checks the port latch state without changing read/write mode
#define GPIB_LATCH_RD(a)     GPIO_PIN_LATCH_RD(a)

///@brief changes to state of full 8bit port to out
#define GPIB_BUS_OUT()      GPIO_PORT_DIR_OUT(GPIO_A)

///@brief changes to state of full 8bit port to in
#define GPIB_BUS_IN()       GPIO_PORT_DIR_IN(GPIO_A)

///@brief changes to state of full 8bit port to in then read
#define GPIB_BUS_RD()       GPIO_PORT_RD(GPIO_A)

///@brief writes GPIB port latch without changing to write direction
#define GPIB_BUS_LATCH_WR(val) GPIO_PORT_LATCH_WR(GPIO_A,val)

///@brief changes to state of full 8bit port to out then write
#define GPIB_BUS_WR(val)    GPIO_PORT_WR(GPIO_A,val)

///@brief We attempt to detect PPR states for logging only
/// PPR is handled in hardware - but useful if we can detect for logging
/// Optional - see gpib_detect_PPR

///@brief read full port pins
/// Optional - see gpib_detect_PPR
#define GPIB_PPR_RD()       GPIO_PORT_PINS_RD(GPIO_A)

///@brief read full port direction register state
/// Optional - see gpib_detect_PPR
#define GPIB_PPR_DDR_RD()   GPIO_PORT_DDR_RD(GPIO_A)

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


#endif  // #ifndef _GPIB_HAL_H_
