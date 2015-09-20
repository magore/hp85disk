/*

             LUFA Library
     Copyright (C) Dean Camera, 2011.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org

*/

/*
  Copyright 2011  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

The author disclaim all warranties with regard to this
software, including all implied warranties of merchantability
and fitness.  In no event shall the author be liable for any
special, indirect or consequential damages or any damages
whatsoever resulting from loss of use, data or profits, whether
in an action of contract, negligence or other tortious action,
arising out of or in connection with the use or performance of
this software.
*/


/** \file
 *  \brief TWI Peripheral Driver (AVR8)
 *
 *  On-chip TWI driver for the 8-bit AVR microcontrollers.
 *
 *  \note This file should not be included directly. It is automatically include
d as needed by the TWI driver
 *        dispatch header located in LUFA/Drivers/Peripheral/TWI.h.
 */


#include "user_config.h"

///@brief Initialize I2C device.
///
///@param[in] Prescale: prescale value.
///@param[in] BitLength: Number of bits.
///
///@return void.
void TWI_Init(const uint8_t Prescale, const uint8_t BitLength)
{

    BIT_SET(SCL_PORT, SCL_BIT);                   // Pull Up on
    BIT_SET(SDA_PORT, SDA_BIT);                   // Pull Up on

    BIT_SET(TWCR,TWEN);

    TWSR  = Prescale;
    TWBR  = BitLength;

}


///@brief Disable I2C.
static inline void TWI_Disable(void)
{
    BIT_CLR(TWCR,TWEN);
}


/** @brief Sends a TWI STOP onto the TWI bus, terminating communication with the currently addressed device. */
static inline void TWI_StopTransmission(void)
{
    TWCR = ((1 << TWINT) | (1 << TWSTO) | (1 << TWEN));
}


///@brief Start I2C transmission.
///
///@param[in] SlaveAddress: I2C Address.
///@param[in] TimeoutMS: Timeout in Milliseconds.
///
///@return TWI_ERROR_NoError on success.
///@return TWI error value on fail.
uint8_t TWI_StartTransmission(const uint8_t SlaveAddress, const uint8_t TimeoutMS)
{
    for (;;)
    {
        bool     BusCaptured = false;
        uint16_t TimeoutRemaining;

        TWCR = ((1 << TWINT) | (1 << TWSTA) | (1 << TWEN));

        TimeoutRemaining = (TimeoutMS * 100);
        while (TimeoutRemaining-- && !(BusCaptured))
        {
            if (TWCR & (1 << TWINT))
            {
                switch (TWSR & TW_STATUS_MASK)
                {
                    case TW_START:
                    case TW_REP_START:
                        BusCaptured = true;
                        break;
                    case TW_MT_ARB_LOST:
                        TWCR = ((1 << TWINT) | (1 << TWSTA) | (1 << TWEN));
                        continue;
                    default:
                        TWCR = (1 << TWEN);
                        return TWI_ERROR_BusFault;
                }
            }

            _delay_us(10);
        }

        if (!(TimeoutRemaining))
        {
            TWCR = (1 << TWEN);
            return TWI_ERROR_BusCaptureTimeout;
        }

        TWDR = SlaveAddress;
        TWCR = ((1 << TWINT) | (1 << TWEN));

        TimeoutRemaining = (TimeoutMS * 100);
        while (TimeoutRemaining--)
        {
            if (TWCR & (1 << TWINT))
                break;

            _delay_us(10);
        }

        if (!(TimeoutRemaining))
            return TWI_ERROR_SlaveResponseTimeout;

        switch (TWSR & TW_STATUS_MASK)
        {
            case TW_MT_SLA_ACK:
            case TW_MR_SLA_ACK:
                return TWI_ERROR_NoError;
            default:
                TWCR = ((1 << TWINT) | (1 << TWSTO) | (1 << TWEN));
                return TWI_ERROR_SlaveNotReady;
        }
    }
}


///@brief TWI send byte.
///
///@return 1 if Acknowledge.
///@return 0 on fail.
bool TWI_SendByte(const uint8_t Byte)
{
    TWDR = Byte;
    TWCR = ((1 << TWINT) | (1 << TWEN));
    while (!(TWCR & (1 << TWINT)));

    return ((TWSR & TW_STATUS_MASK) == TW_MT_DATA_ACK);
}


bool TWI_ReceiveByte(uint8_t* const Byte,
const bool LastByte)
{
    uint8_t TWCRMask;

    if (LastByte)
        TWCRMask = ((1 << TWINT) | (1 << TWEN));
    else
        TWCRMask = ((1 << TWINT) | (1 << TWEN) | (1 << TWEA));

    TWCR = TWCRMask;
    while (!BIT_TST(TWCR, TWINT))
        ;
    *Byte = TWDR;

    uint8_t Status = (TWSR & TW_STATUS_MASK);

    return ((LastByte) ? (Status == TW_MR_DATA_NACK) : (Status == TW_MR_DATA_ACK));
}


///@brief TWI read packet.
///
///@param[in] SlaveAddress: TWI sleave address.
///@param[in] TimeoutMS: Timeout in Milliseconds.
///@param[in] InternalAddress: Our Address.
///@param[in] InternalAddressLen: Our Address Length.
///@param[out] Buffer: Buffer to Read Data into.
///@param[in] Length: Length of data to read.
///
///@return 0 on success.
///@return Error code on fail.
uint8_t TWI_ReadPacket(const uint8_t SlaveAddress,
const uint8_t TimeoutMS,
const uint8_t* InternalAddress,
uint8_t InternalAddressLen,
uint8_t* Buffer,
uint8_t Length)
{
    uint8_t ErrorCode;

    if ((ErrorCode = TWI_StartTransmission((SlaveAddress & TWI_DEVICE_ADDRESS_MASK) | TWI_ADDRESS_WRITE,
        TimeoutMS)) == TWI_ERROR_NoError)
    {
        while (InternalAddressLen--)
        {
            if (!(TWI_SendByte(*(InternalAddress++))))
            {
                ErrorCode = TWI_ERROR_SlaveNAK;
                break;
            }
        }

        if ((ErrorCode = TWI_StartTransmission((SlaveAddress & TWI_DEVICE_ADDRESS_MASK) | TWI_ADDRESS_READ,
            TimeoutMS)) == TWI_ERROR_NoError)
        {
            while (Length--)
            {
                if (!(TWI_ReceiveByte(Buffer++, (Length == 0))))
                {
                    ErrorCode = TWI_ERROR_SlaveNAK;
                    break;
                }
            }

            TWI_StopTransmission();
        }
    }

    return ErrorCode;
}


///@brief TWI write packet.
///
///@param[in] SlaveAddress: TWI sleave address.
///@param[in] TimeoutMS: Timeout in Milliseconds.
///@param[in] InternalAddress: Our Address.
///@param[in] InternalAddressLen: Our Address Length.
///@param[in] Buffer: Buffer to Write.
///@param[in] Length: Length of data to write.
///
///@return 0 on success.
///@return Error code on fail.
uint8_t TWI_WritePacket(const uint8_t SlaveAddress,
const uint8_t TimeoutMS,
const uint8_t* InternalAddress,
uint8_t InternalAddressLen,
const uint8_t* Buffer,
uint8_t Length)
{
    uint8_t ErrorCode;

    if ((ErrorCode = TWI_StartTransmission((SlaveAddress & TWI_DEVICE_ADDRESS_MASK) | TWI_ADDRESS_WRITE,
        TimeoutMS)) == TWI_ERROR_NoError)
    {
        while (InternalAddressLen--)
        {
            if (!(TWI_SendByte(*(InternalAddress++))))
            {
                ErrorCode = TWI_ERROR_SlaveNAK;
                break;
            }
        }

        while (Length--)
        {
            if (!(TWI_SendByte(*(Buffer++))))
            {
                ErrorCode = TWI_ERROR_SlaveNAK;
                break;
            }
        }

        TWI_StopTransmission();
    }

    return ErrorCode;
}
