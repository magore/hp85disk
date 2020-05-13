/**
 @file hardware/i2c.c

 @brief I2C Interrupt driven driver

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @copyright Copyright &copy; 2014-2020 Mike Gore, Inc. All rights reserved.

 @copyright GNU Public License.

@author Mike Gore

*/

#ifndef _I2C_H
#define _I2C_H

#include "user_config.h"
#include <util/twi.h>


// NOTE: TW_MR_ARB_LOST == TW_MT_ARB_LOST
#define TW_ARB_LOST TW_MT_ARB_LOST

// 16 I2C send/receive buffered operations
#define I2C_OPS 			16
// 20ms timeout per transaction
#define I2C_TIMEOUT 		20

#define I2C_OP_TIMEOUT      _BV(0)
#define I2C_OP_LEN          _BV(1)
#define I2C_OP_ERROR        _BV(2)
#define I2C_TW_MT_SLA_NACK  _BV(3)
#define I2C_TW_MR_SLA_NACK  _BV(4)
#define I2C_TW_MT_DATA_NACK _BV(5)
#define I2C_BUS_ERROR       _BV(6)

typedef struct
{
    uint8_t  address;
    uint8_t  enable;
    uint8_t  done;
    uint8_t flags;
    uint16_t timeout;
    uint8_t  len;
    uint8_t  ind;
    uint8_t  *buf;
} i2c_op_t;

typedef struct {
    uint8_t enable;
    uint8_t ind;
    uint8_t done;
    uint8_t error;
} i2c_t;


/* i2c.c */
uint8_t i2c_check_op ( uint8_t index );
void i2c_free_ops ( void );
i2c_op_t *i2c_op_add ( uint8_t address , uint8_t mode , uint8_t *buf , uint8_t len );
void i2c_task ( void );
void i2c_init ( uint32_t speed );
void i2c_post ( void );
int i2c_done ( void );
int i2c_ok ( void );
// int ISR ( int TWI_vect );
void i2c_print_error ( uint8_t index );

#endif
