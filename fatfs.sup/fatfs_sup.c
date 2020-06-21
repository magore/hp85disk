/**
 @file fatfs/disk.c

@brief Allocate, Free and display FILINFO structurs, getfattime(), display error messages

 @par Copyright &copy; 2014-2020 Mike Gore, All rights reserved. GPL  License
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for specific Copyright details

 @par Credit: part of FatFs avr example project (C)ChaN, 2013.
 @par Copyright &copy; 2013 ChaN.

@par You are free to use this code under the terms of GPL
please retain a copy of this notice in any code you use it in.

This is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option)
any later version.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "user_config.h"
#include "fatfs.h"

#ifdef AVR
#include <stdlib.h>
#endif

#include "mathio.h"

#include "time.h"

///@brief FatFs Drive Volumes
FATFS Fatfs[FF_VOLUMES];                          /* File system object for each logical drive */

#if FF_MULTI_PARTITION != 0
/// @brief FatFs multiple partition drives
const PARTITION Drives[] =
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

#if FATFS_DEBUG > 0
///@brief FatFs Error Messages
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013
static const char *err_msg[] =
{
    "OK",
    "DISK_ERR",
    "INT_ERR",
    "NOT_READY",
    "NO_FILE",
    "NO_PATH",
    "INVALID_NAME",
    "DENIED",
    "EXIST",
    "INVALID_OBJECT",
    "WRITE_PROTECTED",
    "INVALID_DRIVE",
    "NOT_ENABLED",
    "NO_FILE_SYSTEM",
    "MKFS_ABORTED",
    "TIMEOUT",
    "LOCKED",
    "NOT_ENOUGH_CORE",
    "TOO_MANY_OPEN_FILES",
    "INVALID_PARAMETER",
    NULL
};
#endif

/// @brief FAT time structer reference.
/// @see rtc.h
/// @see http://lxr.free-electrons.com/source/fs/fat/misc.c
/// @verbatim
/// typedef struct
/// {
///     WORD   year;                                  /* 2000..2099 */
///     BYTE   month;                                 /* 1..12 */
///     BYTE   mday;                                  /* 1.. 31 */
///     BYTE   wday;                                  /* 1..7 */
///     BYTE   hour;                                  /* 0..23 */
///     BYTE   min;                                   /* 0..59 */
///     BYTE   sec;                                   /* 0..59 */
/// } RTC;
/// @endverbatim

/// @brief Convert Linux POSIX tm_t * to FAT32 time.
///
/// @param[in] t: POSIX struct tm * to convert.
///
/// @return  FAT32 time.
MEMSPACE
uint32_t tm_to_fat(tm_t *t)
{
    uint32_t fat;
/* Pack date and time into a uint32_t variable */
    fat = ((uint32_t)(t->tm_year - 80) << 25)
        | (((uint32_t)t->tm_mon+1) << 21)
        | (((uint32_t)t->tm_mday) << 16)
        | ((uint32_t)t->tm_hour << 11)
        | ((uint32_t)t->tm_min << 5)
        | ((uint32_t)t->tm_sec >> 1);
    return(fat);
}


/// @brief Read time and convert to FAT32 time.
///
/// @return FAT32 time.
/// @see tm_to_fat().
MEMSPACE
DWORD get_fattime (void)
{
    time_t t;
/* Get GMT time */
    time(&t);
    return( tm_to_fat(localtime(&t)));
}


/// @brief  display FatFs return code as ascii string
///
/// Credit: Part of FatFs avr example project (C)ChaN, 2013
/// @param[in] rc: FatFs status return code
/// @return  void

MEMSPACE
void put_rc (int rc)
{
#if FATFS_DEBUG > 0
    char *ptr;
    if(rc > 19)
        ptr = "INVALID ERROR MESSAGE";
    else
        ptr = (char *) err_msg[(int)rc];
    printf("rc=%u FR_%s\n", rc, ptr);
#else
    printf("rc=%u\n", rc);
#endif
}


///@brief Total file space used
DWORD   AccSize;

///@brief Total number or Files and Directories
WORD    AccFiles, AccDirs;

/// @brief  Compute space used, number of directories and files contained under a specified directory
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013
///
/// @param[in] path:
/// @see f_opendir()
/// @see f_readdir()
/// @see AccDirs:  Total number of directories
/// @see AccFiles: Total number of Files
/// @see AccSize:  Total size of all files
/// WARNING path MUST be big enought for the lonest path length
/// @return 0 if no error
static
MEMSPACE
int fatfs_scan_files (char *path) /* Pointer to the working buffer with start path */
{
    DIR dirs;
    FRESULT fr;
    int i;
    FILINFO info;
    fr = f_opendir(&dirs, path);
    if (fr == FR_OK)
    {
        while (((fr = f_readdir(&dirs, &info)) == FR_OK) && info.fname[0])
        {
            if (info.fattrib & AM_DIR)
            {
                AccDirs++;
                i = strlen(path);
                path[i] = '/'; strcpy(path+i+1, info.fname);
                fr = fatfs_scan_files(path);
                path[i] = 0;
                if (fr != FR_OK) break;
            }
            else
            {
//              xprintf(PSTR("%s/%s\n"), path, info.fname);
                AccFiles++;
                AccSize += info.fsize;
            }
#ifdef ESP8266
            optimistic_yield(1000);
            wdt_reset();
#endif
        }
    }

    return fr;
}



/// @brief  return a string with the file system type
/// @param[in] type: file system type
/// @return string with file system type
MEMSPACE
char *fatfs_fstype(int type)
{
    char *ptr;
    switch(type)
    {
        case FS_FAT12:
            ptr = "FAT12";
            break;
        case FS_FAT16:
            ptr = "FAT16";
            break;
        case FS_FAT32:
            ptr = "FAT32";
            break;
        case FS_EXFAT:
            ptr = "EXFAT";
            break;
        default:
            ptr = "UNKNOWN";
            break;
    }
    return(ptr);
}


/// @brief  Compute space used, number of directories and files contained used by a drive
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013
///
/// @param[in] ptr: Drive path like "/"
/// @see f_getfree()  drive free space
/// @see fatfs_scan_files()
/// @see AccDirs:  Total number of directories
/// @see AccFiles: Total number of Files
/// @see AccSize:  Total size of all files
/// @return  1 on success 0 on error
MEMSPACE
int fatfs_status(char *name)
{
    long p2;
    int res;
    FATFS *fs;
    char label[24+2];
    DWORD vsn;                                    // volume serial number
    char buff[MAX_NAME_LEN+1];

	memset(buff,0,sizeof(buff)-1);
    while(*name == ' ' || *name == '\t')
        ++name;

	printf("MMC SLOW clock=%8ld\n", (long)MMC_SLOW);
	printf("MMC FAST clock=%8ld\n", (long)MMC_FAST);

    if(!name || !*name)
    {
        strcpy(buff,".");
    }
    else
    {
        strcpy(buff,name);
	}
    printf("fatfs status:%s\n",buff);
    res = f_getfree(buff, (DWORD*)&p2, &fs);
    if (res)
    {
        printf("fatfs_status f_getfree failed\n");
        return(0);
    }
    printf("FAT type                = %s\n",  fatfs_fstype(fs->fs_type));
    printf("Bytes/Cluster           = %lu\n", (DWORD)fs->csize * 512);
    printf("Number of FATs          = %u\n",  fs->n_fats);
    printf("Root DIR entries        = %u\n",  fs->n_rootdir);
    printf("Sectors/FAT             = %lu\n", fs->fsize);
    printf("Number of clusters      = %lu\n", fs->n_fatent - 2);
    printf("FAT start (lba)         = %lu\n", fs->fatbase);
    printf("DIR start (lba,clustor) = %lu\n", fs->dirbase);
    printf("Data start (lba)        = %lu\n", fs->database);

#if FF_USE_LABEL
    res = f_getlabel(buff, label, (DWORD*)&vsn);
    if (res)
    {
        printf("fatfs_status f_getlabel failed\n");
        return(0);
    }
    printf("Volume name             = %s\n", label[0] ? label : "<blank>");
    printf("Volume S/N              = %04X-%04X\n", (WORD)((DWORD)vsn >> 16), (WORD)(vsn & 0xFFFF));
#endif

    AccSize = AccFiles = AccDirs = 0;
    res = fatfs_scan_files(buff);
    if (res)
    {
        printf("fatfs_status fatfs_scan_files failed\n");
        return(0);
    }
    printf("%u files, %lu bytes.\n%u folders.\n"
        "%lu KB total disk space.\n%lu KB available.\n",
        AccFiles, AccSize, AccDirs,
        (fs->n_fatent - 2) * fs->csize / 2, p2 * fs->csize / 2
        );
	return(1);
}


/// @brief  Display FILINFO structure in a readable format
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
/// - Example:
/// @verbatim
/// ----A 2014/10/16 00:39        14    test2.txt
/// D---- 2014/10/12 21:29         0          tmp
/// @endverbatim
///
/// @param[in] : FILINFO pointer
/// @return  void

MEMSPACE
void fatfs_filinfo_list(FILINFO *info)
{
    char attrs[6];
    if(info->fname[0] == 0)
    {
        printf("fatfs_filinfo_list: empty\n");
        return;
    }
    attrs[0] = (info->fattrib & AM_DIR) ? 'D' : '-';
    attrs[1] = (info->fattrib & AM_RDO) ? 'R' : '-';
    attrs[2] = (info->fattrib & AM_HID) ? 'H' : '-';
    attrs[3] = (info->fattrib & AM_SYS) ? 'S' : '-';
    attrs[4] = (info->fattrib & AM_ARC) ? 'A' : '-';
    attrs[5] = 0;
    printf("%s %u/%02u/%02u %02u:%02u %9lu %s",
        attrs,
        (info->fdate >> 9) + 1980, (info->fdate >> 5) & 15, info->fdate & 31,
        (info->ftime >> 11), (info->ftime >> 5) & 63,
        info->fsize, info->fname);
    printf("\n");
}
