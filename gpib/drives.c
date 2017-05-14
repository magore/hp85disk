/**
 @file gpib/drives.c

 @brief SS80 disk emulator for HP85 disk emulator project for AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved.

 @par Based on work by Anders Gustafsson.

 @par Copyright &copy; 2014 Anders Gustafsson All rights reserved..

*/

#include "user_config.h"

#include "defines.h"
#include "gpib_hal.h"
#include "gpib.h"
#include "gpib_task.h"
#include "amigo.h"
#include "ss80.h"

/// @todo  Should we move this into the actual Disk image or config file ??
/// Size = 5 Bytes
///
/// - Notes: HP/LIF data is BIG endian
/// 
/// @brief HP9122D Controller Description

#if defined(HP9122D)
SS80DiskType SS80Disk = 
{
	SS80_DEFAULT_ADDRESS,
	SS80_DEFAULT_PPR,
	///@see defines.h
    /* Identify, 2 bytes */
    {
        0x02,
        0x22
    },
	///@see defines.h
    /* Controller Description, 5 bytes */
    {
        0b10000000,  //<  Installed unit byte
        0b00000001,  //< One unit
        0x02,        //< MSB Transfer rate in kb/s 744 = 0x2e8
        0xe8,
        0x05         //< Controller type 5 = SS/80 multi unit 
    },
	///@see defines.h
    /* Unit Description ,19 bytes*/
    {
        0x01,        //< Unit type = floppy
        0x09,        //< Type 9122
        0x12,
        0x20,
        0x01,        //< Bytes per block = 256
        0x00,
        0x01,        //< Number of buffered blocks
        0x00,        //< Burst size = 0
        0x17,        //< Block time in us = 5888 = 0x1700
        0x00,
        0x00,        //< Continous transfer rate = 45
        0x2d,
        0x11,        //< Optimal retry = 4500 = 0x1194
        0x94,
        0x20,        //< Access time = 8400 = 0x20d0
        0xd0,
        0x0f,        //< Maximum interleave
        0x00,        //< Fixed volume byte (no fixed volumes)
        0x01                                          //< Removable volume byte = 1
    },
	///@see defines.h
    /* Volume Description, 13 bytes */
    {
        0x00,        //< Max cylinder
        0x00,
        0x00,
        0x00,        //< Max head
        0x00,        //< Max sector
        0x00,
        0x00,        //< Max single vector address = 2463 = 99f
        0x00,
        0x00,
        0x00,
        0x09,
        0x9f,
        0x02         //< Current interleave = 2
    }
};
#endif

#if defined(HP9134L)
SS80DiskType SS80Disk = 
{
	SS80_DEFAULT_ADDRESS,
	SS80_DEFAULT_PPR,
	///@see defines.h
    /* Identify, 2 bytes */
    {
        0x02,
        0x21
    },
	///@see defines.h
    /* Controller Description, 5 bytes */
    {
        0b10000000,  //<  Installed unit byte
        0b00000001,  //< One unit
        0x02,        //< MSB Transfer rate in kb/s 744 = 0x2e8
        0xe8,
        0x05         //< Controller type 5 = SS/80 multi unit 
    },
	///@see defines.h
    /* Unit Description ,19 bytes*/
    {
        0x00,        //< Unit type = winchester
        0x09,        //< Type 9134
        0x13,
        0x40,
        0x01,        //< Bytes per block = 256
        0x00,
    	0x01,        //< Number of buffered blocks
        0x00,        //< Burst size = 0
        0x01,        //< Block time in us = 502 = 0x1F6
        0xF6,
        0x00,        //< Continous transfer rate = 140 kb/s
        0x8c,
        0x11,        //< Optimal retry = 4500 = 0x1194 = 45000 = 4,5s worstcase
        0x94,
        0x11,        //< Access time = 4500 = 0x1194
        0x94,
        0x1f,        //< Maximum interleave
        0x01,        //< Fixed volume byte (1 fixed volumes)
        0x00         //< Removable volume byte = 0 (fixed)
    },
	///@see defines.h
    /* Volume Description, 13 bytes */
    {
        0x00,        //< Max cylinder
        0x00,
        0x00,
        0x00,        //< Max head
        0x00,        //< Max sector
        0x00,
        0x00,        //< Max single vector address = 58176 = e340
        0x00,
        0x00,
        0x00,
        0xe3,
        0x40,
        0x07         //< Current interleave = 2
    }
};
#endif


/// @brief  AMIGO D9121D ident Bytes per sector, sectors per track, heads, cylinders
#if defined(HP9121D)
AMIGODiskType AMIGODisk =
{
	AMIGO_DEFAULT_ADDRESS,
	AMIGO_DEFAULT_PPR,
    {
        0x01, 0x04
    },
    256, 16, 2, 35
};
#endif

/// @brief  AMIGO D9885A ident Bytes per sector, sectors per track, heads, cylinders
#if defined(HP9995A)
AMIGODiskType AMIGODisk =
{
	AMIGO_DEFAULT_ADDRESS,
	AMIGO_DEFAULT_PPR,
    {
        0x00, 0x81
    },
    256, 30, 2, 77
};
#endif

/// @brief  AMIGO D9134A ident Bytes per sector, sectors per track, heads, cylinders
#if defined(HP9134A)
AMIGODiskType AMIGODisk =
{
	AMIGO_DEFAULT_ADDRESS,
	AMIGO_DEFAULT_PPR,
    {
        0x01, 0x06
    },
    256, 31, 4, 153
};
#endif
