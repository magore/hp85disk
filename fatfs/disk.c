/**
 @file fatfs/disk.c

 @brief FatFs utilities utilities and tests for HP85 disk emulator project.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 CHaN.
 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.
 @par Credit: used parts of FatFs avr example project (C)ChaN, 2013.

*/

#include "hardware/hardware.h"

#include "ff.h"
#include "diskio.h"
#include "posix.h"


///@brief FatFs Drive Volumes
FATFS Fatfs[_VOLUMES];                            /* File system object for each logical drive */

#if _MULTI_PARTITION != 0
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

///@brief FatFs Error Messages
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013
static char *err_msg[] =
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

/// @brief  display FatFs return code as ascii string
///
/// Credit: Part of FatFs avr example project (C)ChaN, 2013
/// @param[in] rc: FatFs status return code
/// @return  void

void put_rc (int rc)
{
    char *ptr;
    if(rc > 19)
        ptr = "INVALID ERROR MESSAGE";
    else
        ptr = err_msg[(int)rc];

    myprintf("rc=%u FR_%s\n", rc, ptr);
}


///@brief Total file space used
DWORD   AccSize;

///@brief Total number or Files and Directories
WORD    AccFiles, AccDirs;

/// @brief  Use were FILINFO structure can be share in many functions
///  See: fatfs_alloc_finfo(), fatfs_scan_files() and fatfs_ls()
static FILINFO __finfo;

#if _USE_LFN
	static char __lfname[_MAX_LFN + 1];  /*< Common buffer to store LFN */
#endif

/// @brief  Allocate FILINFO structure and optional long file name buffer
///
/// @param[in] allocate: If allocate is true use safecalloc otherwise return static __finfo
/// @see fatfs_free_finfo() 
/// @see fatfs_scan_files()
/// @see fatfs_ls()
/// @return  FILINFO * on success
/// @return  NULL on error

FILINFO *fatfs_alloc_finfo( int allocate )
{
    FILINFO *finfo;

    if( allocate )
    {
        finfo = safecalloc(sizeof(FILINFO),1);
        if(finfo == NULL)
        {
            return(NULL);
        }
#if _USE_LFN
        finfo->lfname = safecalloc(_MAX_LFN + 1,1);
        finfo->lfsize = _MAX_LFN + 1;

        if(finfo->lfname == NULL)
        {
            free(finfo);
            return(NULL);
        }
#else
        finfo->lfname = NULL;
        finfo->lfsize = 0;
#endif
    }
    else
    {
        finfo = (FILINFO *) &__finfo;
#if _USE_LFN
        finfo->lfname = __lfname;
        finfo->lfsize = _MAX_LFN + 1;
#else
        finfo->lfname = NULL;
        finfo->lfsize = 0;
#endif
    }
    return( finfo );
}


/// @brief  Free a FILINFO structure and optional long file name buffer
/// allocated by fatfs_alloc_finfo()
///
/// @param[in] finfo: FatFs FILINFO pointer to free
/// @see fatfs_alloc_finfo()
/// @see fatfs_scan_files() 
/// @see fatfs_ls()
/// @return  void

void fatfs_free_filinfo( FILINFO *finfo )
{
#if _USE_LFN
    if(finfo->lfname && finfo->lfname != __lfname )
    {
        safefree(finfo->lfname);
    }
#endif
    if(finfo && finfo != (FILINFO *) &__finfo)
    {
        safefree(finfo);
    }
}


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
/// @return 0 if no error
/// @return FafFs error code

int fatfs_scan_files (
char* path                                        /* Pointer to the working buffer with start path */
)
{
    FRESULT res;
    FILINFO *fno;
    DIR dirs;
    int i;
    char *fn;

    fno = fatfs_alloc_finfo(0);
    if(fno == NULL)
    {
        errno = ENOMEM;
        return(FR_NOT_ENOUGH_CORE);
    }

    res = f_opendir(&dirs, path);
    if (res == FR_OK)
    {
        i = strlen(path);
        while (((res = f_readdir(&dirs, fno)) == FR_OK) && fno->fname[0])
        {
            if (_FS_RPATH && fno->fname[0] == '.') continue;
#if _USE_LFN
            fn = *fno->lfname ? fno->lfname : fno->fname;
#else
            fn = fno->fname;
#endif
            if (fno->fattrib & AM_DIR)
            {
                AccDirs++;
                *(path+i) = '/'; strcpy(path+i+1, fn);
                res = fatfs_scan_files(path);
                *(path+i) = '\0';
                if (res != FR_OK) break;
            }
            else
            {
                AccFiles++;
                AccSize += fno->fsize;
            }
        }
    }

    return res;
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
/// @return  void

void fatfs_status(char *ptr)
{
    long p2;
    int res;
    FATFS *fs;

    const BYTE ft[] = {0,12,16,32};

    while(*ptr == ' ' || *ptr == '\t')
        ++ptr;
    myprintf("fatfs status:%s\n",ptr);
    res = f_getfree(ptr, (DWORD*)&p2, &fs);
    if (res)
    {
        put_rc(res);
        return;
    }
    myprintf("FAT type = FAT%u\nBytes/Cluster = %lu\nNumber of FATs = %u\n"
        "Root DIR entries = %u\nSectors/FAT = %lu\nNumber of clusters = %lu\n"
        "FAT start (lba) = %lu\nDIR start (lba,clustor) = %lu\nData start (lba) = %lu\n\n...",
        ft[fs->fs_type & 3], (DWORD)fs->csize * 512, fs->n_fats,
        fs->n_rootdir, fs->fsize, fs->n_fatent - 2,
        fs->fatbase, fs->dirbase, fs->database
        );
    AccSize = AccFiles = AccDirs = 0;
    res = fatfs_scan_files(ptr);
    if (res)
    {
        put_rc(res);
        return;
    }
    myprintf("\r%u files, %lu bytes.\n%u folders.\n"
        "%lu KB total disk space.\n%lu KB available.\n",
        AccFiles, AccSize, AccDirs,
        (fs->n_fatent - 2) * (fs->csize / 2), p2 * (fs->csize / 2)
        );
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

void fatfs_filinfo_list(FILINFO *info)
{
    if(info->fname[0] == 0)
    {
        myprintf("fatfs_filinfo_list: empty\n");
        return;
    }
    myprintf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu %12s",
        (info->fattrib & AM_DIR) ? 'D' : '-',
        (info->fattrib & AM_RDO) ? 'R' : '-',
        (info->fattrib & AM_HID) ? 'H' : '-',
        (info->fattrib & AM_SYS) ? 'S' : '-',
        (info->fattrib & AM_ARC) ? 'A' : '-',
        (info->fdate >> 9) + 1980, (info->fdate >> 5) & 15, info->fdate & 31,
        (info->ftime >> 11), (info->ftime >> 5) & 63,
        info->fsize, &(info->fname[0]));

#if _USE_LFN
    myprintf("  %s", info->lfname);
#endif

    myprintf("\n");
}
