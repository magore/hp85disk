/**
 @file fatfs/disk.h

 @brief FatFs utilities utilities and tests for HP85 disk emulator project.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 CHaN.
 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.
 @par Credit: used parts of FatFs avr example project (C)ChaN, 2013.

*/


#ifndef _DISK_H_
#define _DISK_H_

#include <hardware/hardware.h>

#include "ff.h"
#include "diskio.h"

#if _MULTI_PARTITION != 0
extern const PARTITION Drives[] =
{
    {
        0,0
    }
    ,
    {
        0,1
    }
};
#endif

extern DWORD   AccSize;                           // Total file space used
extern WORD    AccFiles, AccDirs;                 // Total files and directories
extern FATFS   Fatfs[_VOLUMES];                   // File system object for each logical drive

/* disk.c */
void put_rc ( int rc );
FILINFO *fatfs_alloc_finfo ( int allocate );
void fatfs_free_filinfo ( FILINFO *finfo );
int fatfs_scan_files ( char *path );
void fatfs_status ( char *ptr );
void fatfs_filinfo_list ( FILINFO *info );
#endif
