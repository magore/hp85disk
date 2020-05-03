/**
 @file hardware/i2c.c

 @brief I2C Interrupt driven driver 
		Total rewrite
		Reference AVR315 for technical details

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @copyright Copyright &copy; 2014-2020 Mike Gore, Inc. All rights reserved.

 @copyright GNU Public License.

@author Mike Gore

*/

#include "user_config.h"

i2c_t i2c = { 0,0,0,0 };

// Number of I2C operations
#define I2C_OPS 16
// 20ms timeout on any OP - prevents hanging
#define I2C_TIMEOUT 20

i2c_op_t *i2c_op[I2C_OPS] = { NULL };


/// @brief Check if I2C structure pointer is valid
///
/// @param[in] index: index of i2c_op[] array
/// @return  1 if OK, 0 if NOT
uint8_t i2c_check_op(uint8_t index)
{
	if(index >= I2C_OPS)
	{
		printf("I2C op[INDEX %d >= %d]\n",(int) index, I2C_OPS);
		return(0);
	}
	if(i2c_op[index] == NULL)
	{
		return(0);
	}
	if(i2c_op[index]->buf == NULL)
	{
		printf("I2C op[%d]->buf == NULL\n",(int) index);
		return(0);
	}
	if(i2c_op[index]->len == 0)
	{
		printf("I2C op[%d]->len == 0\n",(int) index);
		return(0);
	}
	return(1);
}


/// @brief Free all allocated memory for i2c_[] pointers
/// NOTE: We ASSUME all i2c_op[].buf are statically allocated 
///
/// @return  void
void i2c_free_ops()
{
	i2c_op_t *o;
	int i;
	uint8_t sreg = SREG;

	cli();

	for(i=0;i<I2C_OPS;++i)
	{
		o = i2c_op[i];
		if(o)
		{
			// We ASSUME the buffer is static and not allocated
			o->buf = NULL;
			o->done = 0;
			o->enable = 0;
			o->flags = 0;
			o->timeout = 0;
			o->ind = 0;
			safefree(o);
			i2c_op[i] = NULL;
		}
	}
	i2c.enable = 0;
	i2c.ind = 0;
	i2c.done = 0;
	i2c.error = 0;
// disable interrupts
	TWCR = 0;
// disable clave mode
    TWAR = 0;

	// Reset status
	TWSR &= ~(_BV(TWPS0) | _BV(TWPS1));

    if(set_timers(i2c_task,1) == -1)
        printf("lcd_task init failed\n");

    SREG = sreg;
}



///@brief I2C setup new OP but do not run it yet
///
/// @param[in] address: I2C address
/// @param[in] mode: TW_READ or TW_WRITE
/// @param[in] *buf: pointer to buffer for send or receive
/// @param[in] len: size of buffer to read or write
/// @return  i2c_op_t * pointer 
i2c_op_t *i2c_op_add(uint8_t address, uint8_t mode, uint8_t *buf, uint8_t len)
{
	int i;
	uint8_t sreg = SREG;
	
	i2c_op_t *o = NULL;

    cli();
	for(i=0;i<I2C_OPS;++i)
	{
		if(i2c_op[i] == NULL)
		{
			o = safecalloc(1,sizeof(i2c_op_t));
			i2c_op[i] = o;
			break;
		}
	}
	
	if(o == NULL)
		return(o);

	o->enable = 0; // NOT enabled
	o->done = 0;
    o->address = (address << 1) | (mode & 1);
	o->timeout = I2C_TIMEOUT;
	o->flags = 0;
    o->len = len;
    o->ind = 0;
    o->buf = buf;
    // o->buf = bufcalloc(buf,len+1);

	SREG = sreg;
	return(o);
}


/// @brief I2C timer task - check for operation timeouts
/// The ISR will correctly handle any cleanup
/// @return  void
void i2c_task()
{
	i2c_op_t *o;
	uint8_t sreg = SREG;

	if(!i2c_check_op(i2c.ind))
		return;

    cli();
	o = i2c_op[i2c.ind];

	if(i2c.enable && o->enable && ! o->done )
	{
		if(o->timeout == 0)
		{
			o->flags |= (I2C_OP_TIMEOUT);
			o->done = 1;
		}
		else
		{
			o->timeout--;
		}
	}
	SREG = sreg;
}



/// @brief I2C initialize
/// Clear all i2c_op[] pointers and disables I2C tasks
///
/// @return  void
void i2c_init(uint32_t speed)
{
	int i;
    uint8_t sreg = SREG;
    uint16_t rate = ((F_CPU / speed) - 16) / 2;

    if(rate & 0xff00)
	{
        printf("i2c_init prescale overflow\n");
		return;
	}

    cli();

    TWBR = rate;

	for(i=0;i<I2C_OPS;++i)
		i2c_op[i] = NULL;

	i2c.enable = 0;
	i2c.ind = 0;
	i2c.done = 0;
	i2c.error = 0;

    GPIO_PIN_LATCH_HI(SCL);                       // Pull Up on
    GPIO_PIN_LATCH_HI(SDA);                       // Pull Up on

	// TWI Enable
	// TWI Interrupt Disable
	// TWI Interrupt Clear
	TWCR = _BV(TWEN) | _BV(TWINT);

	// Disable SLAVE
    TWAR = 0;

	// Reset status
	TWSR &= ~(_BV(TWPS0) | _BV(TWPS1));

	// Enable timer task to monitor timeouts
	if(set_timers(i2c_task,1) == -1)
        printf("i2c_task init failed\n");

/* Restore the status register. */
    SREG = sreg;
}


/// @brief Run all valid i2c_op[] tasks
///
/// @return  void
void i2c_post()
{
	uint8_t sreg = SREG;
	uint8_t run = 0;
	i2c_op_t *o;
	int i;

	cli();

	for(i=0;i<I2C_OPS;++i)
	{
		if(!i2c_check_op(i))
			continue;
		o = i2c_op[i];
		o->enable = 1;
		o->done = 0;
		o->flags = 0;
		o->timeout = I2C_TIMEOUT;
		o->ind = 0;
		run = 1;
	}
	i2c.ind = 0;
	i2c.done = 0;
	i2c.error = 0;
	i2c.enable = 1;

	// Do We have something to do ?
	if(run)
	{
		// Reset Status
		TWSR &= ~(_BV(TWPS0) | _BV(TWPS1));
		// Start a transactions
		// TWI Enable
		// TWI Interrupt Enable
		// TWI Interrupt Clear
		// TWI SEND RESTART
		TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWSTA);
		// Disable Slave Mode
		TWAR = 0;
	}
	else
	{
		// FIXME we should notify the user ?
		// Nothing to do
		i2c.done = 1;
		i2c.enable = 0;
		// TWI Enable
		// TWI Disable Enable
		// TWI Interrupt Clear
		TWCR = _BV(TWEN) | _BV(TWINT);
		// Reset status
		TWSR &= ~(_BV(TWPS0) | _BV(TWPS1));
		// Disable Slave Mode
		TWAR = 0;
	}

    SREG = sreg;
}



///@brief Are all i2c_op[] pointers done sending/receiving ?
///
/// @return  1 DONE, 0 NOT DONE
int i2c_done()
{
	if( i2c.done )
		return(1);
	return(0);
}


///@brief check if I2C trasnaction detected an error
///@brief Did we get an error durring any i2c_op[] seand/receive operations
///
/// @return  1 OK, 0 NOT OK
int i2c_ok()
{
	if (i2c.error) 
		return(0);
	return(1);
}


///@brief check if I2C trasnaction detected an error
///@brief Did we get an error durring any i2c_op[] seand/receive operations
///
/// @return  void
static void i2c_next_op()
{
/*
 * Advance to next transaction, if possible.
 */
	while( (i2c.ind+1) < I2C_OPS)
	{
		if(i2c_check_op(++i2c.ind) )
		{
			if( i2c_op[i2c.ind]->enable == 0)
				continue;
			if( i2c_op[i2c.ind]->done == 1)
				continue;
			// We are NOT done
			// TWI Enable
			// TWI Interrupt Enable
			// TWI Interrupt Clear
			// TWI SEND RESTART
			TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWSTA);
			return;
		}
	}
	// We are DONE 
	// TWI Enable
	// TWI Interrupt Disable
	// TWI Interrupt Clear
	// TWI SEND STOP
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO);
	// All transactions are done
	i2c.done = 1;
	i2c.enable = 0;
}


///@brief I2C ISR for send/receive
///
/// @return  void
ISR(TWI_vect)
{
	i2c_op_t *o;
	// FYI: reading TWSR clears the status
	// twi.h defines TW_STATUS
    // #define TW_STATUS (TWSR & TW_STATUS_MASK)
    uint8_t status = TW_STATUS;	

	// Are we Enabled to Receive/Send ?
	// Are we Done ?
	if(!i2c.enable || i2c.done)
	{
		// TWI Enable
		// TWI Interrupt Disable
		// TWI Interrupt Clear
		// TWI SEND STOP
		TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO);
		return;
	}
	
	// Program error - should NEVER happen
	if(!i2c_check_op(i2c.ind) )
	{
		i2c.done = 1;
		i2c.enable = 0;
		i2c.error = 1;
		i2c_next_op();
		return;
	}

	// Get current Operation
	o = i2c_op[i2c.ind];

	// Program error - these conditions should NEVER happen
	if(!o->enable || o->done)
	{
		o->done = 1;
		o->enable = 0;
		o->flags |= I2C_OP_ERROR;
		i2c.error = 1;
		i2c_next_op();
		return;
	}

	// Master Receiver mode
	switch (status)
	{
		case TW_START:		// START has been transmitted
		case TW_REP_START:  // RE-START has been transmitted
			o->ind = 0;
			TWDR = o->address;
			// TWI Enable
			// TWI Interrupt Enable
			// TWI Interrupt Clear
			TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
			break;


		case TW_MT_SLA_ACK:	// SLA+W trasnmitted and ACK received
		case TW_MT_DATA_ACK:// Data trasnmitted and ACK received
			if (o->ind < o->len)
			{
				TWDR = o->buf[o->ind++];
				// TWI Enable
				// TWI Interrupt Enable
				// TWI Interrupt Clear
				// SEND ACK after receive
				TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
			}
			else
			{
				// Done
				o->done = 1;
				o->enable = 0;
				i2c_next_op();
			}
			break;

		case TW_MR_DATA_ACK:	// Data received ACK transmitted
			o->buf[o->ind++] = TWDR;
			// Fall through
		case TW_MR_SLA_ACK:		// SLA+R transmitted ACK received
			if ((o->ind+1) < o->len)
			{
				// TWI Enable
				// TWI Interrupt Enable
				// TWI Interrupt Clear
				// SEND ACK after receive
				TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA);
			}
			else
			{
				// LAST BYTE
				// TWI Enable
				// TWI Interrupt Enable
				// TWI Interrupt Clear
				// SEND NACK after receive
				TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
			}
			break;

		case TW_MR_DATA_NACK:	// Data received NACK transmitted
			o->buf[o->ind] = TWDR;
			o->done = 1;
			o->enable = 0;
			i2c_next_op();
			break;

		// Arbitration lost 
		// NOTE: TW_ARB_LOST == TW_MR_ARB_LOST == TW_MT_ARB_LOST
		case TW_ARB_LOST:
			// TWI Enable
			// TWI Interrupt Enable
			// TWI Interrupt Clear
			// TWI SEND RESTART
			TWCR = _BV(TWEN) |  _BV(TWIE) | _BV(TWINT) | _BV(TWSTA);
			break;

// Error cases
		case TW_MT_SLA_NACK:	// SLA+W transmitted NACK received
			o->done = 1;
			o->enable = 0;
			o->flags |= I2C_TW_MT_SLA_NACK;
			// ERROR
			i2c.error = 1;
			i2c_next_op();
			break;

		case TW_MR_SLA_NACK:	// SLA+R transmitted NACK received
			o->done = 1;
			o->enable = 0;
			o->flags |= I2C_TW_MR_SLA_NACK;
			// ERROR
			i2c.error = 1;
			i2c_next_op();
			break;

		case TW_MT_DATA_NACK:	// Data Transmitted NACK received
			o->done = 1;
			o->enable = 0;
			o->flags |= I2C_TW_MT_DATA_NACK;
			// ERROR
			i2c.error = 1;
			i2c_next_op();
			break;

		default:				// Error
			o->done = 1;
			o->enable = 0;
			// ERROR
			o->flags |= I2C_BUS_ERROR;
			i2c.error = 1;
			i2c_next_op();
			break;
	}
}

/// @brief Display Errors for i2c_op[index]
///
/// @param[in] index: index of i2c_op[] array
/// @return  void
void i2c_print_error(uint8_t index)
{
	i2c_op_t *o;
    int flags;

	if(!i2c_check_op(index))
	{
		printf("I2C op[%d] INVALID\n",(int) index);
		return;
	}

    o = i2c_op[index];

    flags = o->flags;

	if(flags)
	{
		printf("I2C op[%d] ERROR\n",(int) index);
		printf("  %s\n", (o->done ? "DONE" : "ACTIVE") );
		if(o->flags & I2C_OP_TIMEOUT)
			printf("  OP_TIMEOUT\n");
		if(flags & I2C_OP_LEN)
			printf("  OP_LEN\n");
		if(flags & I2C_OP_ERROR)
			printf("  OP_ERROR\n");
		if(flags & I2C_TW_MR_SLA_NACK)
			printf("  TW_MR_SLA_NACK\n");
		if(flags & I2C_TW_MT_SLA_NACK)
			printf("  TW_MT_SLA_NACK\n");
		if(flags & I2C_TW_MT_DATA_NACK)
			printf("  TW_MT_DATA_NACK\n");
		printf("\n");
	}
}
