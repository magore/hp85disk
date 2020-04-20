/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Pieter Noordhuis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "user_config.h"

/* Pointers to current txn and op. */
static volatile i2c_txn_t *txn;
static volatile i2c_op_t *op;

/*
 * By default, the control register is set to:
 *  - TWEA: Automatically send acknowledge bit in receive mode.
 *  - TWEN: Enable the I2C system.
 *  - TWIE: Enable interrupt requests when TWINT is set.
 */
#define TWCR_DEFAULT (_BV(TWEA) | _BV(TWEN) | _BV(TWIE))

#define TWCR_NOT_ACK (_BV(TWINT) | _BV(TWEN) | _BV(TWIE))
#define TWCR_ACK (TWCR_NOT_ACK | _BV(TWEA))

void i2c_init(uint32_t speed)
{
    uint8_t sreg;
    uint16_t temp;

/* Store the status register and disable interrupts. */
    sreg = SREG;
    cli();

/*
 * From ATmega328P datasheet:
 *   SCL freq = (CPU Clock freq) / (16 + 2(TWBR) * (PrescalerValue))
 *
 * Which means:
 *   TWBR = ((CPU Clock freq) / (SCL freq) - 16) / (2 * (PrescalerValue))
 *
 * Disable the prescaler and set TWBR according to CPU freq and SCL freq.
 */
    TWSR &= ~(_BV(TWPS1) | _BV(TWPS0));
    temp = ((F_CPU / speed) - 16) / (2 * 1);
    if(temp & 0xff00)
        printf("i2c_init prescale overflow\n");

    TWBR = temp;

/*
 * Active internal pull-up resistors for SCL and SDA.
 * Their ports are SCL SDA
 */

    GPIO_PIN_LATCH_HI(SCL);                       // Pull Up on
    GPIO_PIN_LATCH_HI(SDA);                       // Pull Up on

// PORTC |= _BV(PC5) | _BV(PC4);

/* Enable interrupts via the control register. */
    TWCR = TWCR_DEFAULT;

/* Disable slave mode. */
    TWAR = 0;

/* Restore the status register. */
    SREG = sreg;
}


void i2c_post(i2c_txn_t *t)
{
    uint8_t sreg;

/* Reset transaction attributes. */
    t->flags = 0;
    t->opspos = 0;
    t->next = NULL;

    sreg = SREG;
    cli();

/* Append transaction to linked list. */
    if (txn == NULL)
    {
        txn = t;
        op = &txn->ops[0];

/* Transmit START to kickstart things. */
        TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
    }
    else
    {
        volatile i2c_txn_t *txn_ = txn;
        while (txn_ != NULL)
        {
            if (txn_->next != NULL)
            {
                txn_ = txn_->next;
            }
            else
            {
                txn_->next = t;
                break;
            }
        }
    }

    SREG = sreg;
}


ISR(TWI_vect, ISR_BLOCK)
{

    uint8_t status = TW_STATUS;

/* This interrupt should only fire if there is something to do. */
    assert(op != NULL);

    if ((op->address & _BV(0)) == TW_READ)
    {
/* Master Receiver mode. */
        switch (status)
        {

/* A START condition has been transmitted. */
            case TW_START:
/* A repeated START condition has been transmitted. */
            case TW_REP_START:
                assert(op->buflen > 0);
                op->bufpos = 0;
                TWDR = op->address;
                TWCR = TWCR_DEFAULT | _BV(TWINT);
                break;

/* Arbitration lost in SLA+R or NOT ACK bit. */
            case TW_MR_ARB_LOST:
/* A START condition will be transmitted when the bus becomes free. */
                TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
                break;

/* SLA+R has been transmitted; ACK has been received. */
            case TW_MR_SLA_ACK:
                if (op->buflen == 1)
                {
                    TWCR = TWCR_NOT_ACK;
                }
                else
                {
                    TWCR = TWCR_ACK;
                }
                break;

/* SLA+R has been transmitted; NOT ACK has been received. */
            case TW_MR_SLA_NACK:
                txn->flags = I2C_TXN_DONE | I2C_TXN_ERR;
                goto next_txn;

/* Data byte has been received; ACK has been returned. */
            case TW_MR_DATA_ACK:
                op->buf[op->bufpos++] = TWDR;
                if (op->bufpos+1 == op->buflen)
                {
                    TWCR = TWCR_NOT_ACK;
                }
                else
                {
                    TWCR = TWCR_ACK;
                }
                break;

/* Data byte has been received; NOT ACK has been returned. */
            case TW_MR_DATA_NACK:
                op->buf[op->bufpos++] = TWDR;
                goto next_op;

            default:
                assert(0 && "unknown status in master receiver mode");
        }
    }
    else
    {
/* Master Transmitter mode. */
        switch (status)
        {

/* A START condition has been transmitted. */
            case TW_START:
/* A repeated START condition has been transmitted. */
            case TW_REP_START:
                assert(op->buflen > 0);
                op->bufpos = 0;
                TWDR = op->address;
                TWCR = TWCR_DEFAULT | _BV(TWINT);
                break;

/* Arbitration lost in SLA+W or data bytes. */
            case TW_MT_ARB_LOST:
/* A START condition will be transmitted when the bus becomes free. */
                TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
                break;

/* SLA+W has been transmitted; ACK has been received. */
            case TW_MT_SLA_ACK:
                TWDR = op->buf[op->bufpos++];
                TWCR = TWCR_DEFAULT | _BV(TWINT);
                break;

/* SLA+W has been transmitted; NOT ACK has been received. */
            case TW_MT_SLA_NACK:
                txn->flags = I2C_TXN_DONE | I2C_TXN_ERR;
                goto next_txn;

/* Data byte has been transmitted; ACK has been received. */
            case TW_MT_DATA_ACK:
                if (op->bufpos < op->buflen)
                {
                    TWDR = op->buf[op->bufpos++];
                    TWCR = TWCR_DEFAULT | _BV(TWINT);
                    break;
                }

/* No more bytes left to transmit... */
                goto next_op;

/* Data byte has been transmitted; NOT ACK has been received. */
            case TW_MT_DATA_NACK:
                if (op->bufpos < op->buflen)
                {
/* There were more bytes left to transmit! */
                    txn->flags = I2C_TXN_DONE | I2C_TXN_ERR;
                    goto next_txn;
                }

                goto next_op;

            default:
                assert(0 && "unknown status in master transmitter mode");
        }
    }

    return;

    next_op:
/*
 * Advance to next operation in transaction, if possible.
 */
    if (++(txn->opspos) < txn->opslen)
    {
        op = &txn->ops[txn->opspos];

/* Repeated start. */
        TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
        return;
    }

/* No more operations, mark transaction as done. */
    txn->flags = I2C_TXN_DONE;

    next_txn:
/*
 * Advance to next transaction, if possible.
 */
    if (txn->next != NULL)
    {
        txn = txn->next;
        op = &txn->ops[0];

/* Repeated start. */
        TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
        return;
    }

    txn = NULL;
    op = NULL;

/* No more transaction, transmit STOP. */
    TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTO);
}
