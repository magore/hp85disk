/**
 @file fatfs/fatfs_tests.h

 @brief FatFs utilities and tests for HP85 disk emulator project.
 @par Credit: part of FatFs avr example project (C)ChaN, 2013.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.
 @par Copyright &copy; 2013 ChaN.

*/

#ifndef _FATFS_TESTS_H
#define _FATFS_TESTS_H

#include "hardware/hardware.h"
#include "ff.h"
#include "diskio.h"

/* fatfs_tests.c */
void fatfs_ls ( char *ptr );
void fatfs_rename ( const char *oldpath , const char *newpath );
void fatfs_cat ( char *name );
void fatfs_copy ( char *from , char *to );
void fatfs_create ( char *name , char *str );
void fatfs_rm ( char *name );
void fatfs_mkdir ( char *name );
void fatfs_rmdir ( char *name );
void fatfs_stat ( char *name );
void fatfs_cd ( char *name );
void fatfs_pwd ( void );
#endif                                            //_FATFS_TESTS_H
