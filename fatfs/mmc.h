/**
 @file fatfs/mmc.h

 @brief part of FatFs avr example project (C)ChaN, 2013.
   - Specifically: avr_complex/main.c from ffsample.zip.
   - Minor Modifications by Mike Gore.
   - I added hardware abstraction layer or mmc.c to make porting easier.

 @par Edit History
 - [1.0]   [user name]  Initial revision of file.

 @par Copyright &copy; CHaN 2013.
 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.
 @see mmc.h
*/

#ifndef _MMC_H_
#define _MMC_H_

// Project includes
#include <hardware/hardware.h>

// Fatfs and MMC includes
#include "diskio.h"
#include "ff.h"
#include "mmc_hal.h"
#include "mmc.h"

/* mmc.c */
DSTATUS mmc_disk_initialize ( BYTE pdrv );
DSTATUS mmc_disk_status ( BYTE pdrv );
DRESULT mmc_disk_read ( BYTE pdrv , BYTE *buff , DWORD sector , UINT count );
DRESULT mmc_disk_write ( BYTE pdrv , const BYTE *buff , DWORD sector , UINT count );
DRESULT mmc_disk_ioctl ( BYTE pdrv , BYTE cmd , void *buff );
#endif                                            // _MMC_H_
