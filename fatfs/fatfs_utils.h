/**
 @file fatfs/fatfs_utils.h

 @brief FatFs utilities and tests for HP85 disk emulator project.
   - Based on FatFs AVR example library (C) ChaN 2013.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.
 @par Copyright &copy; 2013 ChaN.

*/


#ifndef _FATFS_UTILS_H
#define _FATFS_UTILS_H

#include "hardware/hardware.h"
#include "ff.h"
#include "diskio.h"

/* fatfs_utils.c */
void mmc_test ( void );
void fatfs_help ( void );
int fatfs_tests ( char *str );
#endif                                            //_FATFS_UTILS_H
