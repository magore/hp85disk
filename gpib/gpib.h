/**
 @file gpib/gpib.h
 
 @brief GPIB emulator for HP85 disk emulator project for AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

 @par Based on work by Anders Gustafsson.

 @par Copyright &copy; 2014 Anders Gustafsson All rights reserved..

*/

#ifndef _GPIB_H_
#define _GPIB_H_

#include "user_config.h"
#include "hal.h"
#include "gpib_hal.h"

#define GPIB_BUS_SETTLE_DELAY 2


#define EOI_FLAG 0x100
#define SRQ_FLAG 0x200
#define ATN_FLAG 0x400
#define REN_FLAG 0x800
#define PP_FLAG  0x1000

#define IFC_FLAG 0x2000
#define TIMEOUT_FLAG 0x4000
#define BUS_ERROR_FLAG 0x8000

#define ERROR_MASK (IFC_FLAG | TIMEOUT_FLAG | BUS_ERROR_FLAG)
#define CONTROL_MASK (EOI_FLAG | SRQ_FLAG | ATN_FLAG | REN_FLAG)

#define DATA_MASK 0x00ff
#define CMD_MASK 0x007f
#define STATUS_MASK 0xff00

enum
{
    GPIB_RX_START = 0,
    GPIB_RX_WAIT_FOR_DAV_LOW,
    GPIB_RX_DAV_IS_LOW,
    GPIB_RX_WAIT_FOR_DAV_HI,
    GPIB_RX_DAV_IS_HI,
    GPIB_RX_EOI,
    GPIB_RX_IFC_IS_LOW,
    GPIB_RX_ERROR,
    GPIB_RX_FINISH,
    GPIB_RX_DONE
};

enum
{
    GPIB_TX_START = 0,
    GPIB_TX_WAIT_READY,
    GPIB_TX_PUT_DATA,
    GPIB_TX_WAIT_FOR_NRFD_HI,
    GPIB_TX_SET_DAV_LOW,
    GPIB_TX_WAIT_FOR_NDAC_HI,
    GPIB_TX_SET_DAV_HI,
    GPIB_TX_WAIT_FOR_NDAC_LOW,
    GPIB_TX_FINISH,
    GPIB_TX_ERROR,
    GPIB_TX_DONE
};

enum
{
    GPIB_PP_IDLE = 0,
    GPIB_PP_DETECTED
};

#define GPIB_IOBUFF_LEN     256                   /* Max length of RX/TX GPIB string */

extern uint8_t debuglevel;
extern uint8_t gpib_iobuff[GPIB_IOBUFF_LEN];
extern unsigned char idbyte1;
extern unsigned char idbyte2;

extern uint8_t talk31;
extern uint8_t talking;
extern uint8_t talking_last;
extern uint8_t listening;
extern uint8_t listening_last;
extern uint8_t spoll;
extern uint16_t current,lastcmd;
extern uint8_t secondary;

/* gpib.c */
void gpib_timer_elapsed_begin ( void );
void gpib_timer_reset ( void );
void gpib_timer_elapsed_end ( char *msg );
void gpib_timer_task ( void );
void gpib_timeout_set ( uint32_t time );
uint8_t gpib_timeout_test ( void );
void gpib_bus_init ( int cold );
void gpib_state_init ( void );
void EnablePPR ( uint8_t val );
void DisablePPR ( uint8_t val );
uint8_t Parallel_Poll ( void );
void gpib_assert_ifc ( void );
void gpib_assert_ren ( unsigned char state );
uint16_t gpib_write_byte ( uint16_t ch );
uint16_t gpib_unread ( uint16_t ch );
uint16_t gpib_read_byte ( void );
void gpib_decode_header ( void );
void gpib_decode_str ( uint16_t ch , char *str );
void gpib_decode ( uint16_t ch );
int gpib_read_str ( uint8_t *buf , int size , uint16_t *status );
int gpib_write_str ( uint8_t *buf , int size , uint16_t *status );
#endif                                            // GPIB_H_
