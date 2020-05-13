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

///@brief I2C interrupt state registers
i2c_op_t i2c;

///@brief I2C task list
///@ brief I2C callback function
/// Used when automattically sending several transactions
typedef int8_t (*i2c_callback_t)(void);
i2c_callback_t i2c_callback = (i2c_callback_t) NULL;

///@brief I2C task state
i2c_task_t i2c_task = { 0,0,0,0 };

// Number of I2C operations
#define I2C_OPS 16
// 20ms timeout on any OP - prevents hanging
#define I2C_TIMEOUT 20

i2c_op_t *i2c_task_op[I2C_OPS] = { NULL };


/// @brief Check if I2C structure pointer is valid
///
/// @param[in] index: index of i2c_task_op[] array
/// @return  1 if OK, 0 if NOT
uint8_t i2c_check_op(int8_t index)
{
	if(index < 0 || index >= I2C_OPS)
	{
		printf("I2C op[INDEX %d >= %d]\n",(int) index, I2C_OPS);
		return(0);
	}
	if(i2c_task_op[index] == NULL)
	{
		return(0);
	}
	if(i2c_task_op[index]->buf == NULL)
	{
		printf("I2C op[%d]->buf == NULL\n",(int) index);
		return(0);
	}
	if(i2c_task_op[index]->len == 0)
	{
		printf("I2C op[%d]->len == 0\n",(int) index);
		return(0);
	}
	
	return(1);
}

/// @brief Initialize I2C task op list
/// NOTE: We ASSUME all i2c_task_op[].buf are statically allocated 
///
/// @return  void
void i2c_task_init()
{

	int8_t i;
	uint8_t sreg = SREG;

	cli();

	for(i=0;i<I2C_OPS;++i)
		i2c_task_op[i] = NULL;
	i2c_task.enable = 0;
	i2c_task.done = 1;
	i2c_task.ind = 0;
	i2c_task.error = 0;

    SREG = sreg;
}


/// @brief Free all allocated memory for i2c_[] pointers
/// NOTE: We ASSUME all i2c_task_op[].buf are statically allocated 
///
/// @return  void
void i2c_task_free_ops()
{
	i2c_op_t *o;
	int8_t i;
	uint8_t sreg = SREG;

	cli();

	for(i=0;i2c_check_op(i);++i)
	{
		o = i2c_task_op[i];
		// We ASSUME the buffer is static and not allocated
		o->buf = NULL;
		o->done = 0;
		o->enable = 0;
		o->flags = 0;
		o->timeout = 0;
		o->ind = 0;
		safefree(o);
		i2c_task_op[i] = NULL;
	}

// disable interrupts
	TWCR = 0;

// disable slave mode
    TWAR = 0;

	// Reset status
	TWSR &= ~(_BV(TWPS0) | _BV(TWPS1));

    SREG = sreg;
}



///@brief I2C setup new OP but do not run it yet
///
/// @param[in] address: I2C address
/// @param[in] mode: TW_READ or TW_WRITE
/// @param[in] *buf: pointer to buffer for send or receive
/// @param[in] len: size of buffer to read or write
/// @return  i2c_op_t * pointer 
i2c_op_t *i2c_task_op_add(uint8_t address, uint8_t mode, uint8_t *buf, uint8_t len)
{
	uint8_t sreg = SREG;
	i2c_op_t *o;
	

	o = safecalloc(1,sizeof(i2c_op_t));
	if(o == NULL)
		return(o);

    cli();

	o->enable = 0; // NOT enabled
	o->done = 0;
    o->address = (address << 1) | (mode & 1);
	o->timeout = I2C_TIMEOUT;
	o->flags = 0;
    o->len = len;
    o->ind = 0;
    o->buf = buf;

	SREG = sreg;
	return(o);
}

///@brief I2C task next operation ISR callback functions
///
/// @return  void
int8_t i2c_task_next_op()
{
	uint8_t sreg = SREG;
	i2c_op_t *o;

	cli();

	if(i2c_task.enable)
	{
		// Save state of LAST operation
		if(i2c_check_op(i2c_task.ind) )
		{
			o = i2c_task_op[i2c_task.ind];
			// Save state in last opperation
			*o = i2c;
			if(o->flags)
				i2c_task.error = 1;
		}

		if(i2c_check_op(i2c_task.ind+1) )
		{
			i2c_task.ind++;

			o = i2c_task_op[i2c_task.ind];

			if( o->enable == 1 && o->done == 1)
			{
				o->timeout = I2C_TIMEOUT;
				o->flags = 0;
				o->ind = 0;
				o->enable = 1;
				o->done = 0;

				i2c = *o;

				i2c_send_start();

				SREG = sreg;
				return(1);
			}
		}
	}
	// program error
	i2c_task.enable = 0;
	i2c_task.done = 1;
	i2c_task.error = 1;
	i2c_send_stop();

	SREG = sreg;
	return(0);
}




/// @brief Run all valid i2c_task_op[] tasks
///
/// @return  void
void i2c_task_run()
{
	uint8_t sreg = SREG;
	i2c_op_t *o;
    uint8_t run = 0;
	int8_t i;


	cli();

	i2c_task.enable = 0;
	i2c_task.done = 1;
	i2c_task.ind = 0;
	i2c_task.error = 0;

	// re-enable tasks
	for(i=0;i2c_check_op(i);++i)
	{
		o = i2c_task_op[i];
		o->enable = 1;
		o->done = 0;
		o->flags = 0;
		o->ind = 0;
		o->timeout = I2C_TIMEOUT;
		i2c = *o;
		run = 1;
	}

	// Clear Status
	TWSR &= ~(_BV(TWPS0) | _BV(TWPS1));
	if(run)
	{
		i2c_callback = i2c_task_next_op;
		i2c_task.enable = 1;
		i2c_task.done = 0;
		i2c_send_start();
	}
	else
	{
		// Nothing to DO
		// User Error
		i2c_callback = NULL;
		i2c.done = 1;
		i2c.enable = 0;
        // TWI Enable
        // TWI Disable Enable
        // TWI Interrupt Clear
        TWCR = _BV(TWEN) | _BV(TWINT);
	}
	// Disable Slave Mode
	TWAR = 0;

	SREG = sreg;
}


static i2c_init_status = 0;

/// =========================================================================
/// @brief I2C initialize
/// Clear all i2c_task_op[] pointers and disables I2C tasks
///
/// @return  void
void i2c_init(uint32_t speed)
{
    uint8_t sreg = SREG;
    uint16_t rate = ((F_CPU / speed) - 16) / 2;

    if(rate & 0xff00)
	{
        printf("i2c_init prescale overflow\n");
		return;
	}

    cli();

    TWBR = rate;

	if(i2c_init_status)
	{
		SREG = sreg;
		return;
	}

	i2c.enable = 0; // NOT enabled
	i2c.done = 1;
	i2c_task.enable = 0;
	i2c_task.done = 1;

    GPIO_PIN_LATCH_HI(SCL);                       // Pull Up on
    GPIO_PIN_LATCH_HI(SDA);                       // Pull Up on

	// TWI Enable
	// TWI Interrupt Disable
	// TWI Interrupt Clear
	TWCR = _BV(TWEN) | _BV(TWINT);

	// Disable SLAVE
    TWAR = 0;

	// Reset Status
	TWSR &= ~(_BV(TWPS0) | _BV(TWPS1));

	// Enable timer task to monitor timeouts
	if(set_timers(i2c_timer,1) == -1)
        printf("i2c_timer init failed\n");

/* Restore the status register. */
	i2c_init_status = 1;

    SREG = sreg;
}


///@brief Are all i2c_task_op[] pointers done sending/receiving ?
///
/// @return  1 DONE, 0 NOT DONE
int8_t i2c_done()
{
	if( i2c.done )
		return(1);
	return(0);
}


///@brief check if I2C trasnaction detected an error
///@brief Did we get an error durring any i2c_task_op[] seand/receive operations
///
/// @return  1 OK, 0 NOT OK
int8_t i2c_ok()
{
	if (i2c.flags) 
		return(0);
	return(1);
}


///@brief I2C setup new OP but do not run it yet
///
/// @param[in] address: I2C address
/// @param[in] mode: TW_READ or TW_WRITE
/// @param[in] *buf: pointer to buffer for send or receive
/// @param[in] len: size of buffer to read or write
/// @return  i2c.flags 0 = OK, 1 = ERROR
uint8_t i2c_fn(uint8_t address, uint8_t mode, uint8_t *buf, uint8_t len)
{
	uint8_t sreg = SREG;
	
    cli();

	// sign task only
	i2c_callback = NULL;

	i2c.enable = 1; // Enabled
	i2c.done = 0;
    i2c.address = (address << 1) | (mode & 1);
	i2c.timeout = I2C_TIMEOUT;
	i2c.flags = 0;
    i2c.len = len;
    i2c.ind = 0;
    i2c.buf = buf;

	// Reset Status
	TWSR &= ~(_BV(TWPS0) | _BV(TWPS1));
	i2c_send_start();
	// Disable Slave Mode
	TWAR = 0;

	SREG = sreg;

#if 0
	while( i2c.enable && !i2c.done )
		;
#endif
	return( i2c.flags ? 0 : 1);
}

///@brief Send I2C START and enable interrupts
///
/// @return  void
void i2c_send_start()
{
	uint8_t sreg = SREG;

	cli();

	i2c.done = 0;
	i2c.enable = 1;

	// Start a transactions
	// TWI Enable
	// TWI Interrupt Enable
	// TWI Interrupt Clear
	// TWI SEND RESTART
	TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWSTA);

    SREG = sreg;
}

///@brief Send I2C STOP and disable interrupts
///
/// @return  void
void i2c_send_stop()
{
	uint8_t sreg = SREG;

	cli();

    // All transactions are done
    i2c.done = 1;
    i2c.enable = 0;

    // We are DONE
    // TWI Enable
    // TWI Interrupt Disable
    // TWI Interrupt Clear
    // TWI SEND STOP
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO);

    SREG = sreg;
}


///@brief Is there anything else to send ?
/// Called after I2C transaction finishes
///
/// @return  void
static void i2c_next()
{

	// IF we have an i2c_callback() function then
	// it must save status and reset i2c structure for next operation

	if(i2c_callback)
	{
	    i2c_callback();
	}
	else
	{
		i2c.done = 1;
		i2c.enable = 0;
		i2c_send_stop();
	}
}

/// @brief I2C timer task - check for I2C operation timeouts
/// @return  void
void i2c_timer()
{
	uint8_t sreg = SREG;

    cli();
	if(i2c.enable && !i2c.done )
	{
		if(i2c.timeout)
		{
			i2c.timeout--;
		}
		else
		{
			i2c.enable = 0;
			i2c.done = 1;
			i2c.flags |= (I2C_OP_TIMEOUT);
			i2c_task.enable = 0;
			i2c_task.done = 1;
			i2c_task.error = 1;
			i2c_send_stop();
		}
	}
	SREG = sreg;
}


///@brief I2C ISR for send/receive
///
/// @return  void
ISR(TWI_vect)
{
	// FYI: reading TWSR clears the status
	// twi.h defines TW_STATUS
    // #define TW_STATUS (TWSR & TW_STATUS_MASK)
    uint8_t status = TW_STATUS;	

	// Are we Enabled to Receive/Send ?
	// Are we Done ?
	// Program error - this should not happen
	if(!i2c.enable || i2c.done || !i2c.buf)
	{
		i2c_send_stop();
		return;
	}
	
	// Master Receiver mode
	switch (status)
	{
		case TW_START:		// START has been transmitted
		case TW_REP_START:  // RE-START has been transmitted
			i2c.ind = 0;
			TWDR = i2c.address;
			// TWI Enable
			// TWI Interrupt Enable
			// TWI Interrupt Clear
			TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
			break;


		case TW_MT_SLA_ACK:	// SLA+W trasnmitted and ACK received
		case TW_MT_DATA_ACK:// Data trasnmitted and ACK received
			if (i2c.ind < i2c.len)
			{
				TWDR = i2c.buf[i2c.ind++];
				// TWI Enable
				// TWI Interrupt Enable
				// TWI Interrupt Clear
				// SEND ACK after receive
				TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
			}
			else
			{
				// Done
				i2c.done = 1;
				i2c.enable = 0;
				i2c_next();
			}
			break;

		case TW_MR_DATA_ACK:	// Data received ACK transmitted
			i2c.buf[i2c.ind++] = TWDR;
			// Fall through
		case TW_MR_SLA_ACK:		// SLA+R transmitted ACK received
			if ((i2c.ind+1) < i2c.len)
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
			i2c.buf[i2c.ind] = TWDR;
			i2c.done = 1;
			i2c.enable = 0;
			i2c_next();
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
			i2c.done = 1;
			i2c.enable = 0;
			i2c.flags |= I2C_TW_MT_SLA_NACK;
			// ERROR
			i2c_next();
			break;

		case TW_MR_SLA_NACK:	// SLA+R transmitted NACK received
			i2c.done = 1;
			i2c.enable = 0;
			i2c.flags |= I2C_TW_MR_SLA_NACK;
			// ERROR
			i2c_next();
			break;

		case TW_MT_DATA_NACK:	// Data Transmitted NACK received
			i2c.done = 1;
			i2c.enable = 0;
			i2c.flags |= I2C_TW_MT_DATA_NACK;
			// ERROR
			i2c_next();
			break;

		default:				// Error
			i2c.done = 1;
			i2c.enable = 0;
			// ERROR
			i2c.flags |= I2C_BUS_ERROR;
			i2c_next();
			break;
	}
}

/// @brief Display Errors for i2c_task_op[index]
///
/// @param[in] index: index of i2c_task_op[] array
/// @return  void
void i2c_print_error(i2c_op_t *o)
{
    int flags = o->flags;

	if(flags)
	{
		printf("  %s\n", (i2c.done ? "DONE" : "ACTIVE") );
		if(flags & I2C_OP_TIMEOUT)
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
